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

#include <util/Logger.h>
#include <util/ClockAccessor.h>
#include "FakeCNCController.h"

namespace romi {

        FakeCNCController::FakeCNCController()
                : commands_(),
                  enabled_(false),
                  thread_(nullptr),
                  quitting_(false),
                  active_(false)
        {
                homing();
                thread_ = std::make_unique<std::thread>([this]() {
                        this->handle_commands();
                });
        }
        
        FakeCNCController::~FakeCNCController()
        {
                if (thread_) {
                        quitting_ = true;
                        thread_->join();
                }
        }

        void FakeCNCController::handle_commands()
        {
                r_debug("FakeCNCController::handle_commands");
                auto clock = ClockAccessor::GetInstance();
                
                while (!quitting_) {
                        handle_command();
                        clock->sleep(0.01);
                }                
        }

        void FakeCNCController::handle_command()
        {
                FakeCommand cmd;
                if (commands_.try_pop(cmd)) {
                        r_debug("FakeCNCController::handle_command");
                        active_ = true;
                        switch (cmd.type_) {
                        case kMove:
                                handle_move(cmd.millis_, cmd.x_, cmd.y_, cmd.z_);
                                break;
                        case kMoveAt:
                                handle_moveat(cmd.x_, cmd.y_, cmd.z_);
                                break;
                        case kMoveTo:
                                handle_moveto(cmd.millis_, cmd.x_, cmd.y_, cmd.z_);
                                break;
                        default:
                                r_warn("FakeCNCController::handle_commands: "
                                       "Unknown command type");
                        }
                        active_ = false;
                }
        }
        
        void FakeCNCController::handle_move(int16_t millis, int16_t steps_x,
                                            int16_t steps_y, int16_t steps_z)
        {
                r_debug("FakeCNCController::handle_move");
                double pos0[3];
                double dt = (double) millis / 1000.0;
                double dx = (double) steps_x;
                double dy = (double) steps_y;
                double dz = (double) steps_z;
                auto clock = ClockAccessor::GetInstance();
                double now = clock->time();
                double start = now;
                double end = now + dt;
                
                for (int i = 0; i < 3; i++)
                        pos0[i] = pos_[i];
                
                while (true) {
                        clock->sleep(0.01);
                        
                        now = clock->time();
                        if (now > end)
                                break;

                        pos_[0] = pos0[0] + dx * (now - start) / dt;
                        pos_[1] = pos0[1] + dy * (now - start) / dt;
                        pos_[2] = pos0[2] + dz * (now - start) / dt;
                }
                
                pos_[0] = pos0[0] + dx;
                pos_[1] = pos0[1] + dy;
                pos_[2] = pos0[2] + dz;
        }
                
        void FakeCNCController::handle_moveat(int16_t speed_x, int16_t speed_y,
                                              int16_t speed_z)
        {
                r_debug("FakeCNCController::handle_moveat");
                auto clock = ClockAccessor::GetInstance();
                double t = clock->time();
                double t0 = t;
                double vx = (double) speed_x;
                double vy = (double) speed_y;
                double vz = (double) speed_z;
                double pos0[3];

                for (int i = 0; i < 3; i++)
                        pos0[i] = pos_[i];
                
                while (!quitting_ && commands_.size() == 0) {
                        clock->sleep(0.01);
                        
                        t = clock->time();

                        pos_[0] = pos0[0] + (t - t0) * vx;
                        pos_[1] = pos0[1] + (t - t0) * vy;
                        pos_[2] = pos0[2] + (t - t0) * vz;
                }
        }
                
        void FakeCNCController::handle_moveto(int16_t millis, int16_t steps_x,
                                              int16_t steps_y, int16_t steps_z)
        {
                r_debug("FakeCNCController::handle_moveto");
                double pos0[3];
                double dt = (double) millis / 1000.0;
                double x = (double) steps_x;
                double y = (double) steps_y;
                double z = (double) steps_z;
                auto clock = ClockAccessor::GetInstance();
                double t = clock->time();
                double t0 = t;
                double end = t + dt;
                
                for (int i = 0; i < 3; i++)
                        pos0[i] = pos_[i];
                
                while (true) {
                        clock->sleep(0.01);
                        
                        t = clock->time();
                        if (t > end)
                                break;

                        double alpha = (t - t0) / dt;
                        pos_[0] = pos0[0] * (1 - alpha) + x * alpha;
                        pos_[1] = pos0[1] * (1 - alpha) + y * alpha;
                        pos_[2] = pos0[2] * (1 - alpha) + z * alpha;
                }
                
                pos_[0] = x;
                pos_[1] = y;
                pos_[2] = z;
        }

        int FakeCNCController::is_idle()
        {
                int idle = 0;
                r_debug("FakeCNCController::is_idle: active %d && size %d",
                        (int) active_, (int) commands_.size());
                if (!active_ && commands_.size() == 0) {
                        idle = 1;
                }
                return idle;
        }

        bool FakeCNCController::get_position(int32_t *pos)
        {
                for (int i = 0; i < 3; i++)
                        pos[i] = (int32_t) pos_[i];
                return true;
        }
        
        bool FakeCNCController::synchronize(double timeout)
        {
                bool result = false;
                auto clock = ClockAccessor::GetInstance();
                double now = clock->time();
                double end = now + timeout;
                
                while (true) {
                        if (timeout > 0.0
                            && clock->time() > end)
                                break;
                        if (is_idle() != 0) {
                                result = true;
                                break;
                        }
                        clock->sleep(0.01);
                }
                return result;
        }
}
