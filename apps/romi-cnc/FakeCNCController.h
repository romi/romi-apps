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
#ifndef _OQUAM_FAKE_CNC_CONTROLLER_HPP_
#define _OQUAM_FAKE_CNC_CONTROLLER_HPP_

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "ICNCController.h"

namespace romi {

        enum FakeCommandType {
                kMove,
                kMoveAt,
                kMoveTo
        };
        
	struct FakeCommand
	{
                FakeCommandType type_;
                int16_t millis_;
                int16_t x_, y_, z_;
                
		FakeCommand()
			: type_(kMove),
                          millis_(0),
                          x_(0),
                          y_(0),
                          z_(0) {
                }
                
		FakeCommand(FakeCommandType type,
                            int16_t millis,
                            int16_t x, 
                            int16_t y,
                            int16_t z)
			: type_(type),
                          millis_(millis),
                          x_(x),
                          y_(y),
                          z_(z) {
                }
	};
	
	class CommandQueue
	{
	private:
		std::queue<FakeCommand> queue_;
		std::mutex mutex_;
		std::condition_variable cond_;
                
	public:
                
                void move(int16_t millis, int16_t steps_x,
                          int16_t steps_y, int16_t steps_z) {
                        push(kMove, millis, steps_x, steps_y, steps_z);
                }
                
                void moveat(int16_t vx, int16_t vy, int16_t vz) {
                        push(kMoveAt, 0, vx, vy, vz);
                };
                
                void moveto(int16_t millis, int16_t x, int16_t y, int16_t z) {
                        push(kMoveTo, millis, x, y, z);
                }

		void push(FakeCommandType type, int16_t millis,
                          int16_t x, int16_t y, int16_t z) {
                        r_debug("FakeCNCController::push 1");
			{
				std::lock_guard<std::mutex> lock(mutex_);
				queue_.emplace(type, millis, x, y, z);
			}
                        r_debug("FakeCNCController::push 2");
			cond_.notify_one();
                        r_debug("FakeCNCController::push 3");
		}

		// Pop (blocking). 
		FakeCommand pop() {
			std::unique_lock<std::mutex> lock(mutex_);
			cond_.wait(lock, [&]{ return !queue_.empty(); });

			auto command = queue_.front();
			queue_.pop();
			return command;
		}
                
		bool try_pop(FakeCommand& command) {
			std::lock_guard<std::mutex> lock(mutex_);
                        bool result = false;
			if (queue_.size() > 0) {
                                r_debug("FakeCNCController::try_pop");
                                command = queue_.front();
                                queue_.pop();
                                result = true;
                        }
			return result;
		}

		size_t size() {
			std::unique_lock<std::mutex> lock(mutex_);
                        size_t r = queue_.size();
			return r;
		}
	};

        
        class FakeCNCController : public ICNCController
        {
        public:
                static constexpr const char *ClassName = "fake-cnc-controller";
                
        protected:
                CommandQueue commands_;
                double pos_[3];
                bool enabled_;
                std::unique_ptr<std::thread> thread_;
                std::atomic<bool> quitting_;
                std::atomic<bool> active_;

                void handle_commands();
                void handle_command();
                void handle_move(int16_t millis, int16_t steps_x,
                                 int16_t steps_y, int16_t steps_z);
                void handle_moveat(int16_t vx, int16_t vy, int16_t vz);
                void handle_moveto(int16_t dt, int16_t x, int16_t y, int16_t z);
                
        public:
                
                FakeCNCController();
                virtual ~FakeCNCController() override;
                
                bool set_homing_axes(AxisIndex, AxisIndex, AxisIndex) override {
                        return true;
                }
                
                bool set_homing_mode(HomingMode) override {
                        return true;
                }
                
                bool set_homing_speeds(int16_t, int16_t, int16_t) override {
                        return true;
                }
        
                int is_idle() override;
                bool get_position(int32_t *pos) override;
                bool synchronize(double timeout) override;
                
                bool homing() override {
                        for (int i = 0; i < 3; i++)
                                pos_[i] = 0.0;
                        return true;
                }
                
                bool spindle(double speed) override
                {
                        (void) speed;
                        return true;
                }
                
                bool move(int16_t millis, int16_t steps_x,
                          int16_t steps_y, int16_t steps_z) override {
                        r_debug("FakeCNCController::move");
                        commands_.move(millis, steps_x, steps_y, steps_z);
                        return true;
                }
                bool moveat(int16_t vx, int16_t vy, int16_t vz) override {
                        r_debug("FakeCNCController::moveat");
                        commands_.moveat(vx, vy, vz);
                        return true;
                };
                
                bool moveto(int16_t dt, int16_t x, int16_t y, int16_t z) override {
                        r_debug("FakeCNCController::moveto");
                        commands_.moveto(dt, x, y, z);
                        return true;
                }
                
                bool pause_activity() override {
                        return true;
                }
                
                bool continue_activity() override {
                        return true;
                }
                
                bool reset_activity() override {
                        return homing();
                }

                bool enable() override {
                        enabled_ = true;
                        return true;
                }
                
                bool disable() override {
                        enabled_ = false;
                        return true;
                }

                bool is_enabled() override {
                        return enabled_;
                }

                bool stop() override {
                    return true;
                }
        };
}

#endif // _OQUAM_FAKE_CNC_CONTROLLER_HPP_
