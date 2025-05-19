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
#include <functional>
#include <util/Logger.h>
#include <util/StringUtils.h>
#include <util/ClockAccessor.h>
#include <util/DataLogAccessor.h>

#include "BrushMotorDriver.h"

namespace romi {

static const std::string kDriverLeftTargetSpeedName = "driver-left-target-speed";
static const std::string kDriverLeftCurrentSpeedName = "driver-left-current-speed";
static const std::string kDriverLeftMeasuredSpeedName = "driver-left-measured-speed";
static const std::string kDriverRightTargetSpeedName = "driver-right-target-speed";
static const std::string kDriverRightCurrentSpeedName = "driver-right-current-speed";
static const std::string kDriverRightMeasuredSpeedName = "driver-right-measured-speed";
        
        
        BrushMotorDriver::BrushMotorDriver(
                std::unique_ptr<romiserial::IRomiSerialClient>& serial,
                nlohmann::json &config,
                double max_angular_speed,
                double max_angular_acceleration)
                : serial_(),
                  settings_(),
                  max_angular_speed_(max_angular_speed),
                  recording_speeds_(false),
                  speeds_thread_()
        {
                serial_ = std::move(serial);
                if (!power_down()
                    || !configure_controller(config,
                                             max_angular_speed,
                                             max_angular_acceleration)) {
                        throw std::runtime_error("BrushMotorDriver: "
                                                 "Initialization failed");
                }
        }
        
        BrushMotorDriver::~BrushMotorDriver()
        {
                stop_recording_speeds();
        }

        const BrushMotorDriverSettings& BrushMotorDriver::get_settings()
        {
                return settings_;
        }

        int32_t BrushMotorDriver::get_encoder_steps()
        {
                return settings_.encoder_steps;
        }
        
        bool BrushMotorDriver::configure_controller(nlohmann::json &config,
                                                    double max_angular_speed,
                                                    double max_angular_acceleration)
        {
                settings_.parse(config);

                char command[100];
                StringUtils::rprintf(command, 100, "C[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",
                        settings_.encoder_steps,
                        settings_.dir_left,
                        settings_.dir_right,
                        (int) (1000.0 * max_angular_speed),
                        (int) (1000.0 * max_angular_acceleration),
                        settings_.kp_numerator,
                        settings_.kp_denominator,
                        settings_.ki_numerator,
                        settings_.ki_denominator,
                        settings_.max_signal);
                        
                nlohmann::json response;
                serial_->send(command, response);
                return check_response(command, response);
        }
        
        bool BrushMotorDriver::check_response(const char *command,
                                              nlohmann::json& response)
        {
                bool success = (response[romiserial::kStatusCode] == 0);
                if (!success) {
                        std::string message = "No message";
                        if (response.size() > 1)
                                message = response[romiserial::kErrorMessage];
                        r_warn("BrushMotorDriver: command %s returned error: %s",
                               command, message.c_str());
                }
                return success;
        }

        bool BrushMotorDriver::power_up()
        {
                nlohmann::json response;
                const char *command = "E[1]";
                serial_->send(command, response);
                return check_response(command, response);
        }

        bool BrushMotorDriver::power_down()
        {
                nlohmann::json response;
                const char *command = "E[0]";
                serial_->send(command, response);
                return check_response(command, response);
        }

        bool BrushMotorDriver::is_powered_up()
        {
                throw std::runtime_error("BrushMotorDriver::is_powered_up: not yet implemented"); // FIXME
        }
        
        bool BrushMotorDriver::stop()
        {
                nlohmann::json response;
                const char *command = "X";
                serial_->send(command, response);
                return check_response(command, response);
        }

        bool BrushMotorDriver::moveat(double left, double right)
        {
                bool success = false;

                r_warn("BrushMotorDriver::moveat: (%.3f, %.3f)", left, right);
                               
                if (-max_angular_speed_ <= left  && left <= max_angular_speed_
                    && -max_angular_speed_  <= right && right <= max_angular_speed_) {
                                
                        int32_t ileft = (int32_t) (1000.0 * left);
                        int32_t iright = (int32_t) (1000.0 * right);
                                
                        char command[64];
                        StringUtils::rprintf(command, 64, "V[%d,%d]", ileft, iright);
                                
                        nlohmann::json response;
                        serial_->send(command, response);
                        success = check_response(command, response);
                        
                } else {
                        r_warn("BrushMotorDriver::moveat: invalid speeds: (%f, %f)",
                               left, right);
                }

                return success;
        }

        bool BrushMotorDriver::get_encoder_values(double& left,
                                                  double& right,
                                                  double& timestamp)
        {
                nlohmann::json response;
                const char *command = "e";
                serial_->send(command, response);
                bool success = check_response(command, response);
                if (success) {
                        left = response[1];
                        right = response[2];
                        timestamp = response[3].get<double>() / 1000.0;
                }
                return success;
        }

        void BrushMotorDriver::start_recording_speeds()
        {
                if (!recording_speeds_
                    && speeds_thread_ == nullptr) {
                        
                        recording_speeds_ = true;
                        speeds_thread_ = std::make_unique<std::thread>(
                                [this]() {
                                        record_speeds_main();
                                });
                }
        }

        void BrushMotorDriver::stop_recording_speeds()
        {
                recording_speeds_ = false;
                if (speeds_thread_) {
                        speeds_thread_->join();
                        speeds_thread_ = nullptr;
                }
        }

        void BrushMotorDriver::record_speeds_main()
        {
                auto clock = romi::ClockAccessor::GetInstance();
                while (recording_speeds_) {
                        log_speeds();
                        clock->sleep(0.020);
                }
        }
        
        void BrushMotorDriver::log_speeds()
        {
                auto clock = romi::ClockAccessor::GetInstance();
                double now;
                double left_target;
                double right_target;
                double left_current;
                double right_current;
                double left_measured;
                double right_measured;

                now = clock->time();
                        
                bool success = get_speeds(left_target, right_target,
                                          left_current, right_current,
                                          left_measured, right_measured);

                if (success) {
                        romi::log_data(now, kDriverLeftTargetSpeedName, left_target);
                        romi::log_data(now, kDriverLeftCurrentSpeedName, left_current);
                        romi::log_data(now, kDriverLeftMeasuredSpeedName, left_measured);
                        romi::log_data(now, kDriverRightTargetSpeedName, right_target);
                        romi::log_data(now, kDriverRightCurrentSpeedName, right_current);
                        romi::log_data(now, kDriverRightMeasuredSpeedName, right_measured);
                }
        }
        
        bool BrushMotorDriver::get_speeds(double& left_target, double& right_target,
                                          double& left_current, double& right_current,
                                          double& left_measured, double& right_measured)
        {
                nlohmann::json response;
                char command[16];
                snprintf(command, sizeof(command), "v");

                serial_->send(command, response);
                bool success = check_response(command, response);
                if (success) {
                        left_target = response[1].get<double>() / 1000.0;
                        right_target = response[2].get<double>() / 1000.0;
                        left_current = response[3].get<double>() / 1000.0;
                        right_current = response[4].get<double>() / 1000.0;
                        left_measured = response[5].get<double>() / 1000.0;
                        right_measured = response[6].get<double>() / 1000.0;
                }
                return success;
        }
}
