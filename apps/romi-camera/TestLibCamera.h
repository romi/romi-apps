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

#ifndef ROMI_TESTLIBCAMERA_H
#define ROMI_TESTLIBCAMERA_H

#include <string>
#include <stdexcept>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unordered_map>
#include <libcamera/libcamera.h>
#include <util/ImageIO.h>
#include <rcom/MemBuffer.h>

namespace romi {

	struct Frame
	{
		std::size_t index_;
		std::vector<std::uint8_t> data_;
		std::uint64_t timestamp_;
		

		Frame(std::size_t index,
		      std::uint64_t timestamp,
		      const std::uint8_t* buffer,
		      std::size_t length)
			: index_(index),
			  data_(buffer, buffer + length),
			  timestamp_(timestamp)
			{
			}
	};
	
	class FrameQueue
	{
	public:
		void push(std::shared_ptr<Frame> frame) {
			{
				std::lock_guard<std::mutex> lock(mutex_);
				queue_.push(std::move(frame));
			}
			cond_.notify_one();
		}

		// Pop (blocking). 
		std::shared_ptr<Frame> pop() {
			std::unique_lock<std::mutex> lock(mutex_);
			cond_.wait(lock, [&]{ return !queue_.empty(); });

			auto frame = queue_.front();
			queue_.pop();
			return frame;
		}

		// Non-blocking try_pop
		std::shared_ptr<Frame> try_pop() {
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.empty())
				return nullptr;

			auto frame = queue_.front();
			queue_.pop();
			return frame;
		}

		size_t size() {
			return queue_.size();
		}
		
	private:
		std::queue<std::shared_ptr<Frame>> queue_;
		std::mutex mutex_;
		std::condition_variable cond_;
	};
        
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
        
        class TestLibCamera
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
                size_t stride_;
                std::mutex api_mutex_;
                std::mutex cv_mutex_;
                std::condition_variable cv_;
                bool image_requested_;
                bool request_completed_;
                std::atomic<bool> running_;
                rcom::MemBuffer jpeg_;
                std::unordered_map<MmapKey, const uint8_t *,
                                   MmapKeyHasher, MmapKeyEquals> map_;
	public: // Fixme: needed for JPEG compression
                uint8_t *buffer_;
                size_t buffer_size_;
                size_t image_size_;
	protected:
                bool recording_;
		size_t frame_count_;
		FrameQueue queue_;
		bool quitting_;
                std::unique_ptr<std::thread> thread_;
		 
                void init_camera();
                void release_camera();
                
        public:
                
                explicit TestLibCamera(size_t width, size_t height);
                ~TestLibCamera();
        
                rcom::MemBuffer& grab_jpeg();
                
                // Power device interface
                bool power_up();
                bool power_down();

		void start_recording();
                void stop_recording();

        protected:
                void assert_format();
                void send_request();
                void request_complete(libcamera::Request *request);
                void process_request_buffer(libcamera::Request *request);
                void convert_to_jpeg(const uint8_t *data);
                void store_frames_to_disk();
        };
}

#endif // ROMI_TESTLIBCAMERA_H
