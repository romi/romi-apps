/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include <chrono>
#include <sys/mman.h>
#include <string.h>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <time.h>                                
#include <stdlib.h>
#include <jpeglib.h>
#include <util/FileUtils.h>
#include <util/Logger.h>
#include <util/ClockAccessor.h>
#include <util/StringUtils.h>
#include "LibCamera.h"

namespace romi {

        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;

        static uint32_t count_ = 0;
        static double start_time_ = 0.0;

        void print_fps()
        {
                count_++;
                if (start_time_ == 0.0) {
                        struct timespec ts;
                        timespec_get(&ts, TIME_UTC);
                        start_time_ = (double) ts.tv_sec + (double) ts.tv_nsec * 1.0e-9;
                } else if ((count_ % 50) == 0) {
                        struct timespec ts;
                        timespec_get(&ts, TIME_UTC);
                        double now = (double) ts.tv_sec + (double) ts.tv_nsec * 1.0e-9;
                        r_debug("FPS: %f", (double) count_ / (now - start_time_));
                }
        }

        ////
        
        FrameAllocator::FrameAllocator()
                : memory_(nullptr),
                  total_length_(0),
                  frame_length_(0),
                  free_(nullptr),
                  mutex_()
        {
        }

        FrameAllocator::~FrameAllocator()
        {
                clear();
        }

        void FrameAllocator::init(size_t count, size_t image_size)
        {
                clear();
                compute_sizes(count, image_size);
                allocate_memory();
                chainlink_frames();
        }

        void FrameAllocator::clear()
        {
                if (memory_) {
                        ::munlock(memory_, total_length_);
                        ::free(memory_);
                        memory_ = nullptr;
                        total_length_ = 0;
                        frame_length_ = 0;
                        free_ = nullptr;
                }
        }
        
        void FrameAllocator::compute_sizes(size_t count, size_t image_size)
        {
                size_t page_size = (size_t) sysconf(_SC_PAGESIZE);
                frame_length_ = image_size;
                total_length_ = count * frame_length_;
                size_t num_pages = (total_length_ + page_size) / page_size;
                total_length_ = num_pages * page_size;
        }

        void FrameAllocator::allocate_memory()
        {
                int r = ::posix_memalign(&memory_, (size_t) sysconf(_SC_PAGESIZE),
                                       total_length_);
                if (r != 0) {
                        r_err("FrameAllocator: Out of memory: size=%d kB",
                              (int) total_length_/1024);
                        throw std::runtime_error("FrameAllocator: Out of memory");
                }
                
                if (::mlock(memory_, total_length_) != 0) {
                        r_err("FrameAllocator: mlock failed: %s", strerror(errno));
                        throw std::runtime_error("FrameAllocator: mlock failed");
                }
        }

        void FrameAllocator::chainlink_frames()
        {
                uint8_t *ptr = (uint8_t *) memory_;
                uint8_t *end = (uint8_t *) memory_ + total_length_;
                int count = 0;
                while (ptr + frame_length_ < end) {
                        FrameList *frame = (FrameList *) ptr;
                        frame->next = free_;
                        free_ = frame;
                        ptr += frame_length_;
                        count++;
                }
        }
        
        uint8_t *FrameAllocator::alloc()
        {
                SynchronizedCodeBlock sync(mutex_);
                uint8_t *r = nullptr;
                if (free_) {
                        r = (uint8_t *) free_;
                        free_ = free_->next;
                }
                return r;
        }
        
        void FrameAllocator::free(uint8_t * mem)
        {
                SynchronizedCodeBlock sync(mutex_);
                FrameList *p = (FrameList *) mem;
                p->next = free_;
                free_ = p;
        }

        LibCamera::LibCamera(ICameraStatusIndicator& indicator, size_t width, size_t height)
                : indicator_(indicator),
                  manager_(),
                  camera_(),
                  allocator_(nullptr),
                  stream_(nullptr),
                  requests_(),
                  //pixel_format_(libcamera::formats::RGB888),
                  pixel_format_(libcamera::formats::BGR888),
		  stride_(0),
                  api_mutex_(),
                  cv_mutex_(),
                  cv_(),
                  image_requested_(false),
                  request_completed_(false),
                  running_(false),
                  image_(),
                  jpeg_(),
                  map_(),
                  buffer_(nullptr),
                  buffer_size_(0),
                  image_size_(0),
		  recording_(false),
		  recording_id_(),
		  frame_count_(0),
                  frame_skipped_(0),
		  queue_(),
                  buffer_queue_(),
		  quitting_frame_thread_(false),
		  frame_thread_(),
		  quitting_buffer_thread_(false),
		  buffer_thread_(),
                  file_buffer_size_(0),
                  file_buffer_current_(0),
                  file_buffer_offset_(0),
                  file_(nullptr),
                  file_buffer_image_count_(0),
                  frame_allocator_()                  
        {
                width_ = width;
                height_ = height;
		stride_ = width_ * 3; // By default
                indicator_.set(ICameraStatusIndicator::kPoweredDown);
		frame_thread_ = std::make_unique<std::thread>([this]() {
			this->convert_frames_to_jpeg();
		});
		buffer_thread_ = std::make_unique<std::thread>([this]() {
			this->store_buffers_to_disk();
		});
                
                file_buffer_size_ = 32 * 1024 * 1024; // 32 MB
                file_buffer_[0] = (uint8_t *) malloc(file_buffer_size_);
                file_buffer_[1] = (uint8_t *) malloc(file_buffer_size_);
                if (file_buffer_[0] == nullptr || file_buffer_[1] == nullptr) {
                        r_err("LibCamera: Not enough memory for file buffer: size=%d kB",
                              (int) file_buffer_size_/1024);
                        throw std::runtime_error("LibCamera: Not enough memory");
                }
        }

        LibCamera::~LibCamera()
        {
		quitting_frame_thread_ = true;
		r_debug("~LibCamera: *** join thread1 ***");
		if (frame_thread_) {
			frame_thread_->join();
		}
		r_debug("~LibCamera: *** join thread1: done ***");
		quitting_buffer_thread_ = true;
		r_debug("~LibCamera: *** join thread2 ***");
		if (buffer_thread_) {
			buffer_thread_->join();
		}
		r_debug("~LibCamera: *** join thread2: done ***");
                power_down();
                if (buffer_) {
                        free(buffer_);
                        buffer_ = nullptr;
                }
        }

        // API

        bool LibCamera::grab(Image &image)
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_debug("LibCamera::grab");
                bool result = false;
                if (running_) {
                        std::unique_lock<std::mutex> lock(cv_mutex_);
                        indicator_.set(ICameraStatusIndicator::kGrabbing);
                        send_request();
                        // wait_request_completed();
                        cv_.wait(lock, [this]{ return request_completed_; });
                        image = image_;
                        result = true;
                        indicator_.set(ICameraStatusIndicator::kPoweredUp);
                } else {
                        r_info("LibCamera::grab: Not powered up");
                        throw std::runtime_error("Not powered up");
                }
                //r_debug("LibCamera::grab DONE");
                return result;
        }

        rcom::MemBuffer& LibCamera::grab_jpeg()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_debug("LibCamera::grab_jpeg");
                if (running_) {
                        std::unique_lock<std::mutex> lock(cv_mutex_);
                        indicator_.set(ICameraStatusIndicator::kGrabbing);
                        send_request();
                        //wait_request_completed();
                        cv_.wait(lock, [this]{ return request_completed_; });
                        //r_debug("LibCamera::grab_jpeg: request completed: jpeg size: %d", (int) jpeg_.size());
                        //r_debug("LibCamera::grab_jpeg DONE");
                        indicator_.set(ICameraStatusIndicator::kPoweredUp);
                        return jpeg_;
                } else {
                        r_info("LibCamera::grab: Not powered up");
                        //r_debug("LibCamera::grab_jpeg DONE");
                        throw std::runtime_error("Not powered up");
                }
        }

        bool LibCamera::power_up()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_debug("LibCamera::power_up");
                if (!running_) {
                        running_ = true;
                        init_camera();
                }
                //r_debug("LibCamera::power_up DONE");
                indicator_.set(ICameraStatusIndicator::kPoweredUp);
                return true; 
        }
        
        bool LibCamera::power_down()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_debug("LibCamera::power_down");
                if (running_) {
                        //r_debug("LibCamera::power_down running");
                        running_ = false;
                        release_camera();
                }
                indicator_.set(ICameraStatusIndicator::kPoweredDown);
                return true;
        }
        
        bool LibCamera::is_powered_up()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                return running_;
        }
        
        bool LibCamera::set_value(const std::string& name, double value)
        {
                r_debug("LibCamera: set_value('%s', %f): NOT IMPLEMENTED",
                        name.c_str(), value);
                SynchronizedCodeBlock sync(api_mutex_);
                return true;
        }
        
        bool LibCamera::select_option(const std::string& name,
                                       const std::string& value)
        {
                r_debug("CameraConfigManager: set_option('%s', '%s'): NOT IMPLEMENTED",
                        name.c_str(), value.c_str());
                SynchronizedCodeBlock sync(api_mutex_);
                return true;
        }

        const ICameraSettings& LibCamera::get_settings()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_err("LibCamera::get_settings: not implemented");
                throw std::runtime_error("LibCamera::get_settings: not implemented");
        }

        nlohmann::json LibCamera::get_camera_info()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_err("LibCamera::get_camera_info: not implemented");
                throw std::runtime_error("LibCamera::get_camera_info: not implemented");
        }

        //
        
        void LibCamera::init_camera()
        {
                manager_ = std::make_unique<libcamera::CameraManager>();
                manager_->start();
                
                auto cameras = manager_->cameras();
                if (cameras.empty()) {
                        manager_->stop();
                        throw std::runtime_error("LibCamera: No cameras were "
                                                 "identified on the system.");
                }

                std::string cameraId = cameras[0]->id();
                camera_ = manager_->get(cameraId);
                camera_->acquire();
                
                std::unique_ptr<libcamera::CameraConfiguration> config
                        = camera_->generateConfiguration( { libcamera::StreamRole::StillCapture } );

                libcamera::StreamConfiguration &streamConfig = config->at(0);
                std::cout << "Default stream configuration is: "
                          << streamConfig.toString() << std::endl;
        
                streamConfig.size.width = (unsigned int) width_;
                streamConfig.size.height = (unsigned int) height_;
                streamConfig.pixelFormat = pixel_format_;
        
                libcamera::CameraConfiguration::Status status = config->validate();
                if (status == libcamera::CameraConfiguration::Status::Invalid) {
                        manager_->stop();
                        throw std::runtime_error("LibCamera: Failed to validate "
                                                 "the stream configuration.");
                } else if (status == libcamera::CameraConfiguration::Status::Adjusted) {
                        std::cout << "The stream configuration was adjusted." << std::endl;
                        std::cout << "Validated stream configuration is: "
                                  << streamConfig.toString() << std::endl;

                        // FIXME: Check whether we can handle the selected pixel format
                        pixel_format_ = streamConfig.pixelFormat;
                        assert_format();
                        
                } else {
                        std::cout << "Validated stream configuration is: "
                                  << streamConfig.toString() << std::endl;
                }

                if (streamConfig.size.width != width_
                    || streamConfig.size.height != height_) {
                        throw std::runtime_error("LibCamera: Invalid width or height.");
                }
                
                width_ = streamConfig.size.width;
                height_ = streamConfig.size.height;
                stride_ = streamConfig.stride;

                // Memory to store the frames coming from the camera
                frame_allocator_.init(4, stride_ * height_);
                
                // Jpeg buffer
                buffer_size_ = width_ * height_ * 3;
                buffer_ = (uint8_t *) malloc(buffer_size_);
                if (buffer_ == nullptr) {
                        r_err("LibCamera: malloc failed. size: %d", (int) buffer_size_);
                        throw std::runtime_error("LibCamera: malloc failed");
                }
                
                camera_->configure(config.get());
                
                stream_ = streamConfig.stream();
                allocator_ = new libcamera::FrameBufferAllocator(camera_);

                int ret = allocator_->allocate(stream_);
                if (ret < 0) {
                        manager_->stop();
                        throw std::runtime_error("LibCamera: Can't allocate buffers");
                }
                
                const std::vector<std::unique_ptr<libcamera::FrameBuffer>>& buffers
                        = allocator_->buffers(stream_);

                r_info("buffers.size = %d", (int) buffers.size());

                for (unsigned int i = 0; i < buffers.size(); i++) {
                        std::unique_ptr<libcamera::Request> request = camera_->createRequest();
                        if (!request) {
                                manager_->stop();
                                throw std::runtime_error("LibCamera: Can't create request");
                        }

                        const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[i];
                        int ret = request->addBuffer(stream_, buffer.get());
                        if (ret < 0) {
                                if (ret < 0) {
                                        manager_->stop();
                                        throw std::runtime_error("LibCamera: Can't set buffer for request");
                                }
                        }

                        /*
                         * Controls can be added to a request on a per frame basis.
                         */
                        // ControlList &controls = request->controls();
                        // controls.set(controls::Brightness, 0.5);
                        
                        requests_.push_back(std::move(request));
                }

                camera_->requestCompleted.connect(this, &LibCamera::request_complete);

		////
		auto camcontrols = std::unique_ptr<libcamera::ControlList>(new libcamera::ControlList());
                uint32_t fps = 10;
                std::int64_t interval = 1000000 / (std::int64_t) fps;
		camcontrols->set(libcamera::controls::FrameDurationLimits,
				 libcamera::Span<const std::int64_t, 2>({ interval, interval }));
		////
		
                r_info("camera_->start()");
                if (camera_->start(camcontrols.get()) != 0) {
			//if (camera_->start() != 0) {
                        release_camera();
                        throw std::runtime_error("LibCamera: camera->start failed");
                }
                        
                for (std::unique_ptr<libcamera::Request> &request : requests_) {
                        camera_->queueRequest(request.get());
                }
        }

        void LibCamera::release_camera()
        {
                camera_->stop();
                allocator_->free(stream_);
                delete allocator_;
                camera_->release();
                camera_.reset();
                if (buffer_) {
                        free(buffer_);
                        buffer_ = nullptr;
                }
                for (auto& it: map_) {
                        MmapKey key = it.first;
                        const uint8_t *data = it.second;
                        munmap((void *) data, key.length_);
                }
                map_.clear();
                requests_.clear();
                manager_->stop();
                manager_.reset();
        }

        void LibCamera::assert_format()
        {
                if (pixel_format_ == libcamera::formats::RGB888) {
                        r_info("LibCamera: RGB888 format");
                } else if (pixel_format_ == libcamera::formats::BGR888) {
                        r_info("LibCamera: BGR888 format");
                } else if (pixel_format_ == libcamera::formats::MJPEG) {
                        r_info("LibCamera: MJPEG format");                        
                } else {
                        throw std::runtime_error("LibCamera: Unsupported format");
                }
        }

        void LibCamera::send_request()
        {
                request_completed_ = false;
                image_requested_ = true;
        }

        void LibCamera::request_complete(libcamera::Request *request)
        {
                if (request->status() == libcamera::Request::RequestCancelled) {
                        r_debug("LibCamera::request_complete DONE (cancelled)");
                        return;
                }
                
                process_request_buffer(request);

                if (running_) {
                        request->reuse(libcamera::Request::ReuseBuffers);
                        camera_->queueRequest(request);
                }
                
                if (0) print_fps();
        }

        void LibCamera::process_request_buffer(libcamera::Request *request)
        {
		/*
		  GS Camera:
		  
		  ExposureTime = 66651
		  ColourGains = [ 1.827997, 4.266054 ]
		  AnalogueGain = 9.332543
		  FrameDuration = 66725
		  Lux = 9.230788
		  AeState = 2
		  DigitalGain = 1.006130
		  ColourTemperature = 2445
		  SensorBlackLevels = [ 3840, 3840, 3840, 3840 ]
		  FocusFoM = 1575
		  ColourCorrectionMatrix = [ 1.950781, -0.575667, -0.375113, -0.469078, 1.867868, -0.398791, 0.078772, -1.138487, 2.059715 ]
		  ScalerCrop = (0, 0)/1456x1088
		  FrameWallClock = 1764191123542183424
		  SensorTimestamp = 10834765298000
		*/
	  
		// const libcamera::ControlList &requestMetadata = request->metadata();
		
                // for (const auto &ctrl : requestMetadata) {
                //         const libcamera::ControlId *id = libcamera::controls::controls.at(ctrl.first);
                //         const libcamera::ControlValue &value = ctrl.second;
                
                //         std::cout << "\t" << id->name() << " = " << value.toString()
                //                   << std::endl;
                // }
                
                auto buffers = request->buffers();
                double timestamp = ClockAccessor::GetInstance()->time();
                
                for (auto bufferPair : buffers) {
                        libcamera::FrameBuffer *buffer = bufferPair.second;

			//const libcamera::FrameMetadata &metadata = buffer->metadata();
			//uint64_t timestamp = metadata.timestamp;
			
                        for (const libcamera::FrameBuffer::Plane &plane : buffer->planes()) {

                                int mmapFlags = PROT_READ;
                                size_t dmabufLength = 0;
                                
                                const int fd = plane.fd.get();
                                
                                dmabufLength = lseek(fd, 0, SEEK_END);
                                if (plane.offset > dmabufLength ||
                                    plane.offset + plane.length > dmabufLength) {
                                        r_err("LibCamera: plane is out of buffer: "
                                              "buffer length=%d, plane offset=%d, "
                                              "plane length=%d", 
                                              (int) dmabufLength, (int) plane.offset,
                                              (int) plane.length);
                                        return;
                                }
                        
                                size_t mapLength = (size_t) (plane.offset + plane.length);
                                const uint8_t *data = nullptr;
                                MmapKey key(fd, mapLength);
                                                        
                                if (map_.contains(key)) {
                                        data = map_[key];
                                } else {
                                        void *map_address = mmap(nullptr, mapLength,
                                                                 mmapFlags, MAP_SHARED,
                                                                 fd, 0);
                                        if (map_address == MAP_FAILED) {
                                                r_err("LibCamera: Failed to mmap plane: %s",
                                                      strerror(errno));
                                                return;
                                        }

                                        data = (const uint8_t *) map_address;
                                        map_[key] = data;
                                }

				if (recording_) {
                                        process_video_frame(data, plane.length, timestamp);
                                        
				} else if (image_requested_) {
                                        process_image_data(data, plane.length, timestamp);
                                        
                                } else {
                                        // The image is not needed.
                                }
                        }
                }
        }

	void LibCamera::process_video_frame(const uint8_t *data, size_t length,
                                            double timestamp)
	{
                // Process the frame in a separate thread.
                frame_count_++;
                                        
                uint8_t *p = frame_allocator_.alloc();
                if (p == nullptr) {
                        frame_skipped_++;
                        r_debug("LibCamera: frame count %zu, "
                                "skipped %zu frames",
                                frame_count_, frame_skipped_);
                } else {
                        memcpy(p, data, length);
                        auto frame = std::make_shared<Frame>(frame_count_,
                                                             timestamp,
                                                             p,
                                                             length);
                        queue_.push(frame);
                }
	}

	void LibCamera::process_image_data(const uint8_t *data, size_t length,
                                           double timestamp)
	{
                std::unique_lock<std::mutex> lock(cv_mutex_);
                convert_to_jpeg(data, timestamp);
                jpeg_.clear();
                jpeg_.append(buffer_, image_size_);
                request_completed_ = true;
                image_requested_ = false;
                cv_.notify_one();
	}

	RecordingID LibCamera::start_recording()
	{
                SynchronizedCodeBlock sync(api_mutex_);
                if (!running_) {
                        r_info("LibCamera::start_recording: Not powered up");
                        throw std::runtime_error("LibCamera::start_recording: "
                                                 "Not powered up");
                }
                if (recording_) {
                        r_warn("LibCamera::start_recording: already recording");
                        std::runtime_error("LibCamera::start_recording: already recording");
                }
                recording_id_ = new_recording_id();
                open_mjpeg_file(recording_id_);
		recording_ = true;
                return recording_id_;
	}

        std::string LibCamera::new_recording_id()
        {
                using clock = std::chrono::system_clock;
                std::tm tm{};
                std::ostringstream oss;
                
                const auto now = clock::now();
                std::time_t tt = clock::to_time_t(now);
                localtime_r(&tt, &tm);
                oss << std::put_time(&tm, "%Y%m%d-%H%M%S");
                return oss.str();
        }
	
	void LibCamera::stop_recording(RecordingID id)
	{
                SynchronizedCodeBlock sync(api_mutex_);
                if (!recording_) {
                        r_warn("LibCamera::stop_recording: not recording");
                        std::runtime_error("LibCamera::stop_recording: not recording");
                }
                if (id != recording_id_) {
                        r_warn("LibCamera::stop_recording: Bad recording ID");
                        std::runtime_error("LibCamera::stop_recording: Bad recording ID");
                }
		recording_ = false;
	}
        
        std::filesystem::path LibCamera::get_recording(RecordingID id)
        {
                std::filesystem::path path = get_mjpeg_filename(id);
                return path;
        }
        
	void LibCamera::store_buffers_to_disk()
	{
		while (true) {

                        if (buffer_queue_.size() > 0)
                                r_debug("store_buffers_to_disk: Queue size: %d",
                                        (int) buffer_queue_.size());
                        
			if (quitting_buffer_thread_ && buffer_queue_.size() == 0)
				break;
			
			if (buffer_queue_.size() > 0) {
                                auto buffer = buffer_queue_.pop();
                                store_buffer_sync(buffer.index_, buffer.length_);
				
			} else {
				usleep(100000);
			}

                        {
                                SynchronizedCodeBlock sync(api_mutex_);
                                if (!recording_) {
                                        close_mjpeg_file();
                                }
                        }
		}
                
		r_debug("Quitting store_buffers_to_disk, offset=%d",
                        (int) file_buffer_offset_);
                
                if (file_buffer_offset_ > 0) {
                        store_buffer_sync(file_buffer_current_, file_buffer_offset_);
                }
                
                close_mjpeg_file();
	}
	
        std::string LibCamera::get_mjpeg_filename(RecordingID id)
        {
                return romi::StringUtils::string_format("recording-%s.mjpeg", id.c_str());
        }
        
	void LibCamera::open_mjpeg_file(RecordingID id)
	{
                if (file_ != nullptr) {
                        r_warn("LibCamera::open_mjpeg_file: already open");
                        std::runtime_error("LibCamera::open_mjpeg_file: already open");
                }
                auto filename = get_mjpeg_filename(id);
                file_ = new std::ofstream(filename, std::ios::binary);
                r_info("LibCamera::open_mjpeg_file: %s → %s", id.c_str(), filename.c_str());
        }
	
	void LibCamera::close_mjpeg_file()
	{
                if (file_ != nullptr) {
                        r_info("LibCamera::close_mjpeg_file");
                        file_->close();
                        delete file_;
                        file_ = nullptr;
                }
        }
        
	void LibCamera::convert_frames_to_jpeg()
	{
		while (true) {
			if (quitting_frame_thread_ && queue_.size() == 0)
				break;
			
			auto frame = queue_.try_pop();
			if (frame) {
				convert_frame_to_jpeg(frame);
                                frame_allocator_.free(frame->data_);
                                
			} else {
				usleep(10000);
			}
		}                
	}
        
	void LibCamera::convert_frame_to_jpeg(std::shared_ptr<Frame>& frame)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		convert_to_jpeg(frame->data_, frame->timestamp_);
		auto convertTime = std::chrono::high_resolution_clock::now();

                buffer_image();
		double t_convert = (double) std::chrono::duration_cast<std::chrono::microseconds>(convertTime - startTime).count();
		r_debug("convert_frame_to_jpeg: %.0f µs", t_convert);
	}

        void LibCamera::buffer_image()
	{
                size_t length = file_buffer_offset_ + image_size_;
                if (length < file_buffer_size_) {
                        buffer_append();
                } else {
                        r_debug("buffer_image: Requesting writing %d kB to file, %d images",
                                (int) file_buffer_offset_ / 1024,
                                (int) file_buffer_image_count_);
                        store_buffer_async(file_buffer_current_, file_buffer_offset_);
                        swap_buffers();
                        buffer_append();
                }
        }

        void LibCamera::buffer_append()
	{
                uint8_t *dst = (file_buffer_[file_buffer_current_] + file_buffer_offset_);
                memcpy(dst, buffer_, image_size_);
                file_buffer_offset_ += image_size_;
                file_buffer_image_count_++;
        }
        
        void LibCamera::swap_buffers()
	{
                file_buffer_current_ = 1 - file_buffer_current_;
                file_buffer_offset_ = 0;
                file_buffer_image_count_ = 0;
        }

        void LibCamera::store_buffer_async(size_t index, size_t length)
	{
                buffer_queue_.push(index, length);
        }
        
        void LibCamera::store_buffer_sync(size_t index, size_t length)
	{
                r_debug("store_buffer_sync: Writing %d kB to file", (int) length / 1024);
                if (file_ == nullptr) {
                        r_warn("store_buffer_sync: No file!");
                        return;
                }
		auto startTime = std::chrono::high_resolution_clock::now();
		file_->write((const char*) file_buffer_[index], length);                
		auto saveTime = std::chrono::high_resolution_clock::now();
		double t_save = (double) std::chrono::duration_cast<std::chrono::microseconds>(saveTime - startTime).count();
		r_debug("store_buffer_sync: Save: %f µs", t_save);
        }
	
	
        typedef struct _jpeg_my_dest_mgr_t {
                struct jpeg_destination_mgr mgr;
                LibCamera *camera;
        } jpeg_my_dest_mgr_t;

        static void jpeg_bufferinit(j_compress_ptr cinfo)
        {
                jpeg_my_dest_mgr_t* my_mgr = (jpeg_my_dest_mgr_t*) cinfo->dest;
                LibCamera *camera = my_mgr->camera;

                cinfo->dest->next_output_byte = camera->buffer_;
                cinfo->dest->free_in_buffer = camera->buffer_size_;
        }

        static boolean jpeg_bufferemptyoutput(j_compress_ptr /* cinfo */ )
        {
                return 0;
        }

        static void jpeg_bufferterminate(j_compress_ptr cinfo)
        {
                jpeg_my_dest_mgr_t* my_mgr = (jpeg_my_dest_mgr_t*) cinfo->dest;
                LibCamera *camera = my_mgr->camera;
                camera->image_size_ = camera->buffer_size_ - cinfo->dest->free_in_buffer;
        }

        void LibCamera::convert_to_jpeg(const uint8_t *data, double)
        {
                struct jpeg_compress_struct cinfo;
                struct jpeg_error_mgr jerr;
                jpeg_my_dest_mgr_t* my_mgr;

                JSAMPROW row_pointer[1];

                cinfo.err = jpeg_std_error(&jerr);
                jpeg_create_compress(&cinfo);

                cinfo.dest = (struct jpeg_destination_mgr *) 
                        (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
                                                   sizeof(jpeg_my_dest_mgr_t));       
                cinfo.dest->init_destination = &jpeg_bufferinit;
                cinfo.dest->empty_output_buffer = &jpeg_bufferemptyoutput;
                cinfo.dest->term_destination = &jpeg_bufferterminate;

                my_mgr = (jpeg_my_dest_mgr_t*) cinfo.dest;
                my_mgr->camera = this;

                cinfo.image_width = (JDIMENSION) width_;	
                cinfo.image_height = (JDIMENSION) height_;
                cinfo.input_components = 3;
                cinfo.in_color_space = JCS_RGB;

                jpeg_set_defaults(&cinfo);
                jpeg_set_quality(&cinfo, 95, TRUE);

                jpeg_start_compress(&cinfo, TRUE);

                // feed data
                while (cinfo.next_scanline < cinfo.image_height) {
                        row_pointer[0] = (JSAMPROW) &data[cinfo.next_scanline * stride_];
                        jpeg_write_scanlines(&cinfo, row_pointer, 1);
                }

                jpeg_finish_compress(&cinfo);
                jpeg_destroy_compress(&cinfo);
        }
}

