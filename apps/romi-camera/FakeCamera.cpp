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

#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include "util/FileUtils.h"
#include "util/Logger.h"
#include <util/ClockAccessor.h>
#include "font8x8_basic.h"
#include "FakeCamera.h"

namespace romi {

        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;

        void Character::init(char c, char* bit_pattern)
        {
                c_ = c;
                for (int y = 0; y < 8; y++) {
                        char pattern = bit_pattern[y];
                        for (int x = 0; x < 8; x++) {
                                char value = (char) ((pattern >> x) & 0x01);
                                int idx = (y * kColumns + x) * 3;
                                if (value) {
                                        rgb_[idx] = 1.0f;
                                        rgb_[idx+1] = 1.0f;
                                        rgb_[idx+2] = 1.0f;
                                } else {
                                        rgb_[idx] = 0.0f;
                                        rgb_[idx+1] = 0.0f;
                                        rgb_[idx+2] = 0.0f;
                                }
                        }
                }
        }

        void Character::insert(Image& image, uint32_t x0, uint32_t y0)
        {
                float *p = image.data().data();
                if (x0 + 8 > (uint32_t) image.width()
                    || y0 + 8 > (uint32_t) image.width()) {
                        r_warn("Character::insert: (x,y) out of range");
                        return;
                }
                for (uint32_t y = 0, yi = y0; y < kLines; y++, yi++) {
                        uint32_t off1 = 3 * (x0 + yi * (uint32_t) image.width());
                        uint32_t off2 = (y * kColumns) * 3;
                        memcpy(p + off1, rgb_ + off2, 24 * sizeof(float));
                }
        }

        FakeCamera::FakeCamera(size_t width, size_t height, uint32_t framerate)
                : width_(width),
                  height_(height),
                  framerate_(framerate),
                  image_(Image::RGB, width, height),
                  jpeg_(),
                  powered_up_(false),
                  count_(0),
                  recording_(false),
                  recording_id_(),
                  file_(nullptr),
                  mutex_(),
                  thread_(nullptr)
        {
                r_debug("FakeCamera::FakeCamera");
                for (int i = 0; i < 128; i++) {
                        characters_[i].init((char) i, &font8x8_basic[i][0]);
                }
        }

        FakeCamera::~FakeCamera()
        {
                if (recording_)
                        stop_recording(recording_id_);
        }

        void FakeCamera::make_image()
        {
                image_.clear();
                
                char text[100];
                snprintf(text, sizeof(text), "%06d", (int) count_++);
                print(text, 10, 10);
        }

        void FakeCamera::make_jpeg()
        {
                std::vector<uint8_t> buffer;
                ImageIO::store_jpg_to_buffer(image_, buffer);
                jpeg_.clear();
                jpeg_.append(buffer.data(), buffer.size());
        }

        void FakeCamera::print(char* text, uint32_t x0, uint32_t y0)
        {
                uint32_t x = x0;
                size_t len = strlen(text);
                for (size_t i = 0; i < len; i++) {
                        int c = (int) text[i];
                        characters_[c].insert(image_, x, y0);
                        x += 8;
                }
        }
        
        bool FakeCamera::set_value(const std::string& name, double value)
        {
                (void) name;
                (void) value;
                return true;
        }
        
        bool FakeCamera::select_option(const std::string& name,
                                       const std::string& value)
        {
                (void) name;
                (void) value;
                return true;
        }
        
        bool FakeCamera::grab(Image &image)
        {
                SynchronizedCodeBlock sync(mutex_);
                if (!powered_up_) {
                        r_warn("FakeCamera::grab: not powered up");
                        std::runtime_error("FakeCamera::grab: not powered up");
                }
                make_image();
                image = image_;
                return true;
        }

        rcom::MemBuffer& FakeCamera::grab_jpeg()
        {
                SynchronizedCodeBlock sync(mutex_);
                if (!powered_up_) {
                        r_warn("FakeCamera::grab: not powered up");
                        std::runtime_error("FakeCamera::grab: not powered up");
                }
                make_image();
                make_jpeg();
                return jpeg_;
        }
        
        RecordingID FakeCamera::start_recording()
        {
                SynchronizedCodeBlock sync(mutex_);
                if (!powered_up_) {
                        r_warn("FakeCamera::start_recording: not recording");
                        std::runtime_error("FakeCamera::start_recording: not recording");
                }
                if (recording_) {
                        r_err("FakeCamera::start_recording: already recording");
                        throw std::runtime_error("FakeCamera::start_recording: "
                                                 "already recording");
                }

                recording_id_ = new_recording_id();
                open_mjpeg_file(recording_id_);
                recording_ = true;

		thread_ = std::make_unique<std::thread>([this]() {
			this->record();
		});
                
                return recording_id_;
        }

        std::string FakeCamera::new_recording_id()
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
        
	void FakeCamera::open_mjpeg_file(RecordingID id)
	{
                if (file_ != nullptr) {
                        r_warn("LibCamera::open_mjpeg_file: already open");
                        std::runtime_error("LibCamera::open_mjpeg_file: already open");
                }
                auto filename = get_mjpeg_filename(id);
                file_ = new std::ofstream(filename, std::ios::binary);
                r_debug("LibCamera::open_mjpeg_file: %s → %s",
                        id.c_str(), filename.c_str());
        }
	
        std::string FakeCamera::get_mjpeg_filename(RecordingID id)
        {
                return romi::StringUtils::string_format("recording-%s.mjpeg", id.c_str());
        }
	
	void FakeCamera::close_mjpeg_file()
	{
                if (file_ != nullptr) {
                        r_debug("LibCamera::close_mjpeg_file, id=%s",
                                recording_id_.c_str());
                        file_->close();
                        delete file_;
                        file_ = nullptr;
                }
        }
        
        void FakeCamera::stop_recording(RecordingID id)
        {
                SynchronizedCodeBlock sync(mutex_);
                r_debug("LibCamera::stop_recording %s", id.c_str());
                if (!recording_) {
                        r_warn("FakeCamera::stop_recording: not recording");
                        std::runtime_error("FakeCamera::stop_recording: not recording");
                }
                if (id != recording_id_) {
                        r_err("FakeCamera::stop_recording: invalid recording ID");
                        throw std::runtime_error("FakeCamera::stop_recording: "
                                                 "invalid recording ID");
                }
                
                recording_ = false;
                r_debug("LibCamera::stop_recording: join");
		thread_->join();
                thread_ = nullptr;
                r_debug("LibCamera::stop_recording: join done");
        }

        void FakeCamera::record()
        {
                auto clock = ClockAccessor::GetInstance();
                double interval = 1.0 / (double) framerate_;
                uint32_t count = 0;
                double t0 = clock->time();
                
                while (recording_) {
                        make_image();
                        make_jpeg();
                        auto data = jpeg_.data();

                        file_->write((const char*) data.data(), data.size());

                        count++;
                        double t = clock->time();
                        double t1 = t0 + count * interval;
                        if (t < t1) {
                                clock->sleep(t1 - t);
                        }
                }

                close_mjpeg_file();
        }
        
        std::filesystem::path FakeCamera::get_recording(RecordingID id)
        {
                std::filesystem::path path = get_mjpeg_filename(id);
                return path;
        }

        nlohmann::json FakeCamera::get_camera_info()
        {
                r_err("FakeCamera::get_camera_info: not implemented");
                throw std::runtime_error("FakeCamera::get_camera_info: not implemented");
        }
        
        bool FakeCamera::power_up()
        {
                SynchronizedCodeBlock sync(mutex_);
                powered_up_ = true;
                return true;
        }
        
        bool FakeCamera::power_down()
        {
                SynchronizedCodeBlock sync(mutex_);
                powered_up_ = false;
                return true;
        }
        
        bool FakeCamera::is_powered_up()
        {
                SynchronizedCodeBlock sync(mutex_);
                return powered_up_;
        }

        const ICameraSettings& FakeCamera::get_settings()
        {
                throw std::runtime_error("FakeCamera::get_settings: not implemented");
        }
}

