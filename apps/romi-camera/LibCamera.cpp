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

#include <sys/mman.h>
#include <string.h>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>                                
#include <stdlib.h>
#include <jpeglib.h>
#include <util/FileUtils.h>
#include <util/Logger.h>
#include "LibCamera.h"

namespace romi {

        static uint32_t count_ = 0;
        static double start_time_ = 0.0;
        
        using SynchonizedCodeBlock = std::lock_guard<std::mutex>;

        LibCamera::LibCamera(size_t width, size_t height)
                : manager_(),
                  camera_(),
                  allocator_(nullptr),
                  stream_(nullptr),
                  requests_(),
                  //pixel_format_(libcamera::formats::RGB888),
                  pixel_format_(libcamera::formats::BGR888),
                  mutex_(),
                  cv_(),
                  image_requested_(false),
                  request_completed_(false),
                  image_(),
                  jpeg_(),
                  map_(),
                  buffer_(nullptr),
                  buffer_size_(0),
                  image_size_(0)
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
        
                streamConfig.size.width = (unsigned int) width;
                streamConfig.size.height = (unsigned int) height;
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

                if (streamConfig.size.width != width
                    || streamConfig.size.height != height) {
                        throw std::runtime_error("LibCamera: Invalid width or height.");
                }
                
                width_ = streamConfig.size.width;
                height_ = streamConfig.size.height;

                buffer_size_ = width_ * height_ * 3;
                buffer_ = (uint8_t *) malloc(buffer_size_);
                if (buffer_ == nullptr) {
                        r_err("LibCamera: malloc failed. size: %d", (int) buffer_size_);
                        std::runtime_error("LibCamera: malloc failed");
                }
                
                camera_->configure(config.get());
                
                stream_ = streamConfig.stream();
                allocator_ = new libcamera::FrameBufferAllocator(camera_);

                int ret = allocator_->allocate(stream_);
                if (ret < 0) {
                        manager_->stop();
                        std::runtime_error("LibCamera: Can't allocate buffers");
                }
                
                const std::vector<std::unique_ptr<libcamera::FrameBuffer>>& buffers
                        = allocator_->buffers(stream_);

                r_info("buffers.size = %d", (int) buffers.size());

                for (unsigned int i = 0; i < buffers.size(); i++) {
                        std::unique_ptr<libcamera::Request> request = camera_->createRequest();
                        if (!request) {
                                manager_->stop();
                                std::runtime_error("LibCamera: Can't create request");
                        }

                        const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[i];
                        int ret = request->addBuffer(stream_, buffer.get());
                        if (ret < 0) {
                                if (ret < 0) {
                                        manager_->stop();
                                        std::runtime_error("LibCamera: Can't set buffer for request");
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

                r_info("camera_->start()");
                camera_->start();
                
                for (std::unique_ptr<libcamera::Request> &request : requests_) {
                        r_info("camera_->queueRequest");
                        camera_->queueRequest(request.get());
                }
        }

        LibCamera::~LibCamera()
        {
                camera_->stop();
                allocator_->free(stream_);
                delete allocator_;
                camera_->release();
                camera_.reset();
                manager_->stop();
                if (buffer_) {
                        free(buffer_);
                }
                for (auto& it: map_) {
                        MmapKey key = it.first;
                        const uint8_t *data = it.second;
                        munmap((void *) data, key.length_);
                }                
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

        bool LibCamera::set_value(const std::string& name, double value)
        {
                r_debug("LibCamera: set_value('%s', %f): NOT IMPLEMENTED",
                        name.c_str(), value);
                return true;
        }
        
        bool LibCamera::select_option(const std::string& name,
                                       const std::string& value)
        {
                r_debug("CameraConfigManager: set_option('%s', '%s'): NOT IMPLEMENTED",
                        name.c_str(), value.c_str());
                return true;
        }

        void LibCamera::send_request()
        {
                //r_debug("LibCamera::send_request");
                request_completed_ = false;
                image_requested_ = true;
                // camera_->queueRequest(request_.get());
        }

        void LibCamera::wait_request_completed()
        {
                r_debug("LibCamera::wait_request_completed");
                // semaphore_.acquire();
                r_debug("LibCamera::wait_request_completed: OK");
        }

        void LibCamera::signal_request_completed()
        {
                //r_debug("LibCamera::signal_request_completed");
                request_completed_ = true;
                image_requested_ = false;
                cv_.notify_one();
        }

        void LibCamera::request_complete(libcamera::Request *request)
        {
                if (request->status() == libcamera::Request::RequestCancelled)
                        return;

                if (image_requested_) {
                        process_request_buffer(request);
                        signal_request_completed();
                }
                
                request->reuse(libcamera::Request::ReuseBuffers);
                camera_->queueRequest(request);


                if (0) {
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
                // jpeg_my_dest_mgr_t* my_mgr = (jpeg_my_dest_mgr_t*) cinfo->dest;
                // LibCamera *camera = my_mgr->camera;
        
                // size_t oldsize = buffer->size();
                // buffer->resize(oldsize + BLOCKSIZE);
                // cinfo->dest->next_output_byte = buffer->data() + oldsize;
                // cinfo->dest->free_in_buffer = buffer->size() - oldsize;
                return 0;
        }

        static void jpeg_bufferterminate(j_compress_ptr cinfo)
        {
                jpeg_my_dest_mgr_t* my_mgr = (jpeg_my_dest_mgr_t*) cinfo->dest;
                LibCamera *camera = my_mgr->camera;
                camera->image_size_ = camera->buffer_size_ - cinfo->dest->free_in_buffer;
        }

        void LibCamera::convert_to_jpeg(const uint8_t *data)
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
                        row_pointer[0] = (JSAMPROW) &data[cinfo.next_scanline * cinfo.image_width
                                                          * cinfo.input_components];
                        jpeg_write_scanlines(&cinfo, row_pointer, 1);
                }

                jpeg_finish_compress(&cinfo);
                jpeg_destroy_compress(&cinfo);
        }

        void LibCamera::process_request_buffer(libcamera::Request *request)
        {
                r_debug("LibCamera::process_request_buffer");
                
                // const libcamera::ControlList &requestMetadata = request->metadata();
                // for (const auto &ctrl : requestMetadata) {
                //         const libcamera::ControlId *id = libcamera::controls::controls.at(ctrl.first);
                //         const libcamera::ControlValue &value = ctrl.second;
                
                //         std::cout << "\t" << id->name() << " = " << value.toString()
                //                   << std::endl;
                // }
                
                const std::map<const libcamera::Stream *, libcamera::FrameBuffer *> &buffers = request->buffers();
                
                for (auto bufferPair : buffers) {
                        libcamera::FrameBuffer *buffer = bufferPair.second;
                
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

                                convert_to_jpeg(data);

                                jpeg_.clear();
                                jpeg_.append(buffer_, image_size_);
                                
                                //munmap(map_address, mapLength);
                        }
                }
        }

        bool LibCamera::grab(Image &image)
        {
                r_debug("LibCamera::grab");
                std::unique_lock<std::mutex> lk(mutex_);
                send_request();
                // wait_request_completed();
                cv_.wait(lk, [this]{ return request_completed_; });
                image = image_;
                return true;
        }

        rcom::MemBuffer& LibCamera::grab_jpeg()
        {
                r_debug("LibCamera::grab_jpeg");
                std::unique_lock<std::mutex> lk(mutex_);
                send_request();
                //wait_request_completed();
                cv_.wait(lk, [this]{ return request_completed_; });
                //r_debug("LibCamera::grab_jpeg: request completed: jpeg size: %d", (int) jpeg_.size());
                return jpeg_;
        }
        
        bool LibCamera::power_up()
        {
                // FIXME
                return true; 
        }
        
        bool LibCamera::power_down()
        {
                // FIXME
                return true;
        }
        
        bool LibCamera::is_powered_up()
        {
                return true;
        }

        const ICameraSettings& LibCamera::get_settings()
        {
                throw std::runtime_error("LibCamera::get_settings: not implemented");
        }

        nlohmann::json LibCamera::get_camera_info()
        {
                r_err("LibCamera::get_camera_info: not implemented");
                throw std::runtime_error("LibCamera::get_camera_info: not implemented");
        }
}

