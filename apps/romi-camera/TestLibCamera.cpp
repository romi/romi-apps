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
#include <time.h>                                
#include <stdlib.h>
#include <jpeglib.h>
#include <util/Logger.h>
#include "TestLibCamera.h"

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

        TestLibCamera::TestLibCamera(size_t width, size_t height)
                : manager_(),
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
                  jpeg_(),
                  map_(),
                  buffer_(nullptr),
                  buffer_size_(0),
                  image_size_(0),
		  recording_(false),
		  frame_count_(0),
		  queue_(),
		  quitting_(false),
		  thread1_(),
		  thread2_()
        {
                width_ = width;
                height_ = height;
		stride_ = width_ * 3; // By default

		thread1_ = std::make_unique<std::thread>([this]() {
			this->store_frames_to_disk();
		});
		// thread2_ = std::make_unique<std::thread>([this]() {
		// 	this->store_frames_to_disk();
		// });
	}

        TestLibCamera::~TestLibCamera()
        {
		quitting_ = true;
		r_debug("*** join ***");
		if (thread1_) {
			thread1_->join();
		}
		if (thread2_) {
			thread2_->join();
		}
		r_debug("*** join: done ***");
                power_down();
                if (buffer_) {
                        free(buffer_);
                        buffer_ = nullptr;
                }
        }

        // API

        rcom::MemBuffer& TestLibCamera::grab_jpeg()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_debug("TestLibCamera::grab_jpeg");
                if (running_) {
                        std::unique_lock<std::mutex> lock(cv_mutex_);
                        send_request();
                        //wait_request_completed();
                        cv_.wait(lock, [this]{ return request_completed_; });
                        //r_debug("TestLibCamera::grab_jpeg: request completed: jpeg size: %d", (int) jpeg_.size());
                        //r_debug("TestLibCamera::grab_jpeg DONE");
                        return jpeg_;
                } else {
                        r_info("TestLibCamera::grab: Not powered up");
                        //r_debug("TestLibCamera::grab_jpeg DONE");
                        throw std::runtime_error("Not powered up");
                }
        }

        bool TestLibCamera::power_up()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_debug("TestLibCamera::power_up");
                if (!running_) {
                        running_ = true;
                        init_camera();
                }
                //r_debug("TestLibCamera::power_up DONE");
                return true; 
        }
        
        bool TestLibCamera::power_down()
        {
                SynchronizedCodeBlock sync(api_mutex_);
                r_debug("TestLibCamera::power_down");
                if (running_) {
                        //r_debug("TestLibCamera::power_down running");
                        running_ = false;
                        release_camera();
                }
                return true;
        }

        //
        
        void TestLibCamera::init_camera()
        {
                manager_ = std::make_unique<libcamera::CameraManager>();
                manager_->start();
                
                auto cameras = manager_->cameras();
                if (cameras.empty()) {
                        manager_->stop();
                        throw std::runtime_error("TestLibCamera: No cameras were "
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

                std::cout << "streamConfig.stride: " << streamConfig.stride << std::endl;

		  
                libcamera::CameraConfiguration::Status status = config->validate();
                if (status == libcamera::CameraConfiguration::Status::Invalid) {
                        manager_->stop();
                        throw std::runtime_error("TestLibCamera: Failed to validate "
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
                        throw std::runtime_error("TestLibCamera: Invalid width or height.");
                }
                
                width_ = streamConfig.size.width;
                height_ = streamConfig.size.height;
		stride_ = streamConfig.stride;
		
                // Jpeg buffer
                buffer_size_ = width_ * height_ * 3;
                buffer_ = (uint8_t *) malloc(buffer_size_);
                if (buffer_ == nullptr) {
                        r_err("TestLibCamera: malloc failed. size: %d", (int) buffer_size_);
                        throw std::runtime_error("TestLibCamera: malloc failed");
                }
                
                camera_->configure(config.get());
                
                stream_ = streamConfig.stream();
                allocator_ = new libcamera::FrameBufferAllocator(camera_);

                int ret = allocator_->allocate(stream_);
                if (ret < 0) {
                        manager_->stop();
                        throw std::runtime_error("TestLibCamera: Can't allocate buffers");
                }
                
                const std::vector<std::unique_ptr<libcamera::FrameBuffer>>& buffers
                        = allocator_->buffers(stream_);

                r_info("buffers.size = %d", (int) buffers.size());

                for (unsigned int i = 0; i < buffers.size(); i++) {
                        std::unique_ptr<libcamera::Request> request = camera_->createRequest();
                        if (!request) {
                                manager_->stop();
                                throw std::runtime_error("TestLibCamera: Can't create request");
                        }

                        const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[i];
                        int ret = request->addBuffer(stream_, buffer.get());
                        if (ret < 0) {
                                if (ret < 0) {
                                        manager_->stop();
                                        throw std::runtime_error("TestLibCamera: Can't set buffer for request");
                                }
                        }

                        /*
                         * Controls can be added to a request on a per frame basis.
                         */
                        // ControlList &controls = request->controls();
                        // controls.set(controls::Brightness, 0.5);
                        
                        requests_.push_back(std::move(request));
                }

                camera_->requestCompleted.connect(this, &TestLibCamera::request_complete);

		////
		auto camcontrols = std::unique_ptr<libcamera::ControlList>(new libcamera::ControlList());
		camcontrols->set(libcamera::controls::FrameDurationLimits,
				 libcamera::Span<const std::int64_t, 2>({16600, 16600}));
		////
		
                r_info("camera_->start()");
                if (camera_->start(camcontrols.get()) != 0) {
			//if (camera_->start() != 0) {
                        release_camera();
                        throw std::runtime_error("TestLibCamera: camera->start failed");
                }
                        
                for (std::unique_ptr<libcamera::Request> &request : requests_) {
                        camera_->queueRequest(request.get());
                }
        }

        void TestLibCamera::release_camera()
        {
                camera_->stop();
                allocator_->free(stream_);
                delete allocator_;
                camera_->release();
                camera_.reset();
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

        void TestLibCamera::assert_format()
        {
                if (pixel_format_ == libcamera::formats::RGB888) {
                        r_info("TestLibCamera: RGB888 format");
                } else if (pixel_format_ == libcamera::formats::BGR888) {
                        r_info("TestLibCamera: BGR888 format");
                } else if (pixel_format_ == libcamera::formats::MJPEG) {
                        r_info("TestLibCamera: MJPEG format");                        
                } else {
                        throw std::runtime_error("TestLibCamera: Unsupported format");
                }
        }

        void TestLibCamera::send_request()
        {
                request_completed_ = false;
                image_requested_ = true;
        }

        void TestLibCamera::request_complete(libcamera::Request *request)
        {
                if (request->status() == libcamera::Request::RequestCancelled) {
                        r_debug("TestLibCamera::request_complete DONE (cancelled)");
                        return;
                }
                
                {
                        std::unique_lock<std::mutex> lock(cv_mutex_);
                        if (image_requested_ || recording_) {
                                process_request_buffer(request);
                                request_completed_ = true;
                                image_requested_ = false;
                                cv_.notify_one();
                        }
                }

                if (running_) {
                        request->reuse(libcamera::Request::ReuseBuffers);
                        camera_->queueRequest(request);
                }
                
                print_fps();
        }

        void TestLibCamera::process_request_buffer(libcamera::Request *request)
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

                for (auto bufferPair : buffers) {
                        libcamera::FrameBuffer *buffer = bufferPair.second;

			const libcamera::FrameMetadata &metadata = buffer->metadata();
			uint64_t timestamp = metadata.timestamp;
			
                        for (const libcamera::FrameBuffer::Plane &plane : buffer->planes()) {
				
                                int mmapFlags = PROT_READ;
                                size_t dmabufLength = 0;
                                
                                const int fd = plane.fd.get();
                                
                                dmabufLength = lseek(fd, 0, SEEK_END);
                                if (plane.offset > dmabufLength ||
                                    plane.offset + plane.length > dmabufLength) {
                                        r_err("TestLibCamera: plane is out of buffer: "
                                              "buffer length=%d, plane offset=%d, "
                                              "plane length=%d", 
                                              (int) dmabufLength, (int) plane.offset,
                                              (int) plane.length);
                                        return;
                                }

				// r_debug("Plane offset: %d, Plane length: %d",
				// 	(int) plane.offset, (int) plane.length);
				
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
                                                r_err("TestLibCamera: Failed to mmap plane: %s",
                                                      strerror(errno));
                                                return;
                                        }

                                        data = (const uint8_t *) map_address;
                                        map_[key] = data;
                                }

				if (recording_) {
					auto frame = std::make_shared<Frame>(frame_count_++, timestamp,
									     data, plane.length);
					queue_.push(frame);
				}
				
                                // convert_to_jpeg(data);
				// if (false && recording_) {
				// 	char filename[128];
				// 	snprintf(filename, 128, "frame-%06ld.jpg", frame_count_++);
				// 	r_debug("%s", filename);
				// 	std::ofstream ofs(filename, std::ios::binary);
				// 	ofs.write((const char*) buffer_, image_size_);
				// }
				
                                jpeg_.clear();
                                jpeg_.append(buffer_, image_size_);
                        }
                }
        }

	void TestLibCamera::start_recording()
	{
		recording_ = true;
	}
	
	void TestLibCamera::stop_recording()
	{
		recording_ = false;
	}
	
	void TestLibCamera::store_frames_to_disk()
	{
		while (true) {
			
			r_debug("Queue size: %d", (int) queue_.size());

			if (quitting_ && queue_.size() == 0)
				break;
			
			auto frame = queue_.try_pop();
			if (frame) {
				save_bgr_to_jpg(frame);
				//save_bgr_to_ppm(frame);
				
			} else {
				usleep(10000);
			}
		}
		r_debug("Quitting store_frames_to_disk");
	}
	
	void TestLibCamera::save_bgr_to_jpg(std::shared_ptr<Frame>& frame)
	{
		save_bgr_to_jpg(frame->index_, frame->data_.data(),
				width_, height_, stride_);
	}
	
	void TestLibCamera::save_bgr_to_jpg(size_t index, const uint8_t* buffer,
					    size_t width, size_t height,
					    size_t stride)
	{
		char filename[128];
		snprintf(filename, 128, "frame-%06ld.jpg", index);
		r_debug("%s", filename);
		save_bgr_to_jpg(filename, buffer, width, height, stride);
	}
	
	void TestLibCamera::save_bgr_to_jpg(const char* filename,
					    const uint8_t* buffer,
					    size_t width,
					    size_t height,
					    size_t stride)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		convert_to_jpeg(buffer);
		auto convertTime = std::chrono::high_resolution_clock::now();
		std::ofstream ofs(filename, std::ios::binary);
		ofs.write((const char*) buffer_, image_size_);
		auto saveTime = std::chrono::high_resolution_clock::now();

		double t_convert = (double) std::chrono::duration_cast<std::chrono::microseconds>(convertTime - startTime).count();
		double t_save = (double) std::chrono::duration_cast<std::chrono::microseconds>(saveTime - convertTime).count();
		r_debug("Convert: %f, save: %f", t_convert, t_save);
	}
	
	void TestLibCamera::save_bgr_to_ppm(std::shared_ptr<Frame>& frame)
	{
		save_bgr_to_ppm(frame->index_, frame->data_.data(),
				width_, height_, stride_);
	}
	
	void TestLibCamera::save_bgr_to_ppm(size_t index,
					    const uint8_t* buffer,
					    size_t width,
					    size_t height,
					    size_t stride)
	{
		char filename[128];
		snprintf(filename, 128, "frame-%06ld.ppm", index);
		r_debug("%s", filename);
		save_bgr_to_ppm(filename, buffer, width, height, stride);
	}
	
        void TestLibCamera::save_bgr_to_ppm(const char* filename,
					    const uint8_t* buffer,
					    size_t width,
					    size_t height,
					    size_t stride)
        {
                if (!buffer || !filename) {
                        throw std::runtime_error("save_bgr_to_ppm: Invalid pointer");
                }
                if (width > stride) {
                        throw std::runtime_error("save_bgr_to_ppm: Invalid stride or width");
                }

                // Open file in binary mode
                std::ofstream out(filename, std::ios::binary);
                if (!out.is_open()) {
                        throw std::runtime_error("save_bgr_to_ppm: Failed to open file");
                }

		auto startTime = std::chrono::high_resolution_clock::now();
		
                // Convert BGR → RGB
                const uint8_t* p;
		size_t j = 0;
                for (size_t y = 0; y < height; y++) {
                        p = &buffer[y * stride];
                        for (size_t x = 0, i = 0; x < width; x++, i += 3, j+= 3) {
                                buffer_[j + 2] = p[i + 0];
                                buffer_[j + 1] = p[i + 1];
                                buffer_[j + 0] = p[i + 2];
                        }
                }

		auto convertTime = std::chrono::high_resolution_clock::now();
                                
                // Write PPM header (P6 = binary RGB)
                out << "P6\n" << width << " " << height << "\n255\n";
		out.write((char*) buffer_, buffer_size_);
		out.close();

		auto saveTime = std::chrono::high_resolution_clock::now();

		double t_convert = (double) std::chrono::duration_cast<std::chrono::microseconds>(convertTime - startTime).count();
		double t_save = (double) std::chrono::duration_cast<std::chrono::microseconds>(saveTime - convertTime).count();
		r_debug("Convert: %f, save: %f", t_convert, t_save);
	}
	
        typedef struct _jpeg_my_dest_mgr_t {
                struct jpeg_destination_mgr mgr;
                TestLibCamera *camera;
        } jpeg_my_dest_mgr_t;

        static void jpeg_bufferinit(j_compress_ptr cinfo)
        {
                jpeg_my_dest_mgr_t* my_mgr = (jpeg_my_dest_mgr_t*) cinfo->dest;
                TestLibCamera *camera = my_mgr->camera;

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
                TestLibCamera *camera = my_mgr->camera;
                camera->image_size_ = camera->buffer_size_ - cinfo->dest->free_in_buffer;
        }

        void TestLibCamera::convert_to_jpeg(const uint8_t *data)
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
                        //row_pointer[0] = (JSAMPROW) &data[cinfo.next_scanline * cinfo.image_width
                        row_pointer[0] = (JSAMPROW) &data[cinfo.next_scanline * stride_];
                        jpeg_write_scanlines(&cinfo, row_pointer, 1);
                }

                jpeg_finish_compress(&cinfo);
                jpeg_destroy_compress(&cinfo);
        }
}


#include <iostream>
#include <fstream>
#include <unistd.h>

int main()
{
        romi::TestLibCamera camera(1456, 1088);
        camera.power_up();

	camera.start_recording();
	usleep(10*1000*1000);
	camera.stop_recording();
	
        // rcom::MemBuffer& image = camera.grab_jpeg();
        // auto data = image.data();

        // std::ofstream ofs("test.jpg", std::ios::binary);
        // ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
	
        camera.power_down();
}
