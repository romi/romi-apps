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

#ifndef ROMI_LIBCAMERA_H
#define ROMI_LIBCAMERA_H

#include <string>
#include <stdexcept>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <libcamera/libcamera.h>
#include <api/ICamera.h>
#include <util/ImageIO.h>

namespace romi {
        
        struct MmapKey
        {
                const int fd_;
                size_t length_;

                MmapKey(const int fd, size_t length)
                        : fd_(fd), 
                          length_(length) {
                }
                
                ~MmapKey() {
                }
        };
        
        struct MmapKeyHasher
        {
                size_t operator()(const MmapKey& a) const {
                        return (std::hash<int>{}(a.fd_)
                                + std::hash<size_t>{}(a.length_));
                }
        };

        struct MmapKeyEquals
        {
                bool operator()(const MmapKey& a, const MmapKey& b) const {
                        return ((a.fd_ == b.fd_) && (a.length_ == b.length_));
                }
        };
        
        class LibCamera : public ICamera
        {
        public:
                static constexpr const char *ClassName = "libcamera";
                
        protected:
                std::unique_ptr<libcamera::CameraManager> manager_;
                std::shared_ptr<libcamera::Camera> camera_;
                libcamera::FrameBufferAllocator *allocator_;
                libcamera::Stream *stream_;
                std::vector<std::unique_ptr<libcamera::Request>> requests_;
                libcamera::PixelFormat pixel_format_;
                size_t width_;
                size_t height_;
                std::mutex mutex_;
                std::condition_variable cv_;
                bool image_requested_;
                bool request_completed_;
                Image image_;
                rcom::MemBuffer jpeg_;
                std::unordered_map<MmapKey, const uint8_t *,
                                   MmapKeyHasher, MmapKeyEquals> map_;
                
        public:
                uint8_t *buffer_;
                size_t buffer_size_;
                size_t image_size_;
                
                explicit LibCamera(size_t width, size_t height);
                ~LibCamera() override;
        
                bool grab(Image &image) override;
                rcom::MemBuffer& grab_jpeg() override;
                nlohmann::json get_camera_info() override;
                
                bool set_value(const std::string& name, double value) override;
                bool select_option(const std::string& name,
                                   const std::string& value) override;
                const ICameraSettings& get_settings() override;

                // Power device interface
                bool power_up() override;
                bool power_down() override;
                bool is_powered_up() override;

        protected:
                void assert_format();
                void send_request();
                void wait_request_completed();
                void signal_request_completed();
                void request_complete(libcamera::Request *request);
                void process_request_buffer(libcamera::Request *request);
                void convert_to_jpeg(const uint8_t *data);
        };
}

#endif // ROMI_LIBCAMERA_H
