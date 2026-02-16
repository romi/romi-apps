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

#ifndef ROMI_FAKECAMERA_H
#define ROMI_FAKECAMERA_H

#include <string>
#include <stdexcept>
#include <atomic>
#include <thread>
#include <mutex>
#include <util/ImageIO.h>
#include <api/ICamera.h>

namespace romi {

        class Character {
        public:
                static const uint32_t kColumns = 8;
                static const uint32_t kLines = 8;
                static const uint32_t kLineSize = 8 * 3;
                static const uint32_t kLength = kLineSize * 8;
                static const uint32_t kByteLength = kLineSize * sizeof(float);
                
                char c_;
                float rgb_[kLength];

                Character() {}
                ~Character() = default;

                void init(char c, char* bit_pattern);
                void insert(Image& image, uint32_t x, uint32_t y);
        };
        
        class FakeCamera : public ICamera
        {
        public:
                static constexpr const char *ClassName = "fake-camera";
                
        protected:
                size_t width_;
                size_t height_;
                uint32_t framerate_;
                Image image_;
                rcom::MemBuffer jpeg_;
                bool powered_up_;
                uint32_t count_;
                bool recording_;
                std::string recording_id_;
                Character characters_[128];
                std::ofstream *file_;
                std::mutex mutex_;
                std::unique_ptr<std::thread> thread_;

                void make_image();
                void make_jpeg();
                void print(char* text, uint32_t x, uint32_t y);
                std::string new_recording_id();
                void open_mjpeg_file(RecordingID id);
                std::string get_mjpeg_filename(RecordingID id);
                void close_mjpeg_file();
                void record();
                
        public:
                
                explicit FakeCamera(size_t width, size_t height, uint32_t framerate);
                ~FakeCamera() override;
        
                bool grab(Image &image) override;
                rcom::MemBuffer& grab_jpeg() override;
                nlohmann::json get_camera_info() override;

                RecordingID start_recording() override;
                void stop_recording(RecordingID id) override;
                std::filesystem::path get_recording(RecordingID id) override;
                
                bool set_value(const std::string& name, double value) override;
                bool select_option(const std::string& name,
                                   const std::string& value) override;
                const ICameraSettings& get_settings() override;

                // Power device interface
                bool power_up() override;
                bool power_down() override;
                bool is_powered_up() override;
        };
}

#endif // ROMI_FAKECAMERA_H
