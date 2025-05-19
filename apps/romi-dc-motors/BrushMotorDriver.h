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
#ifndef ROMI_BRUSHMOTORDRIVER_H
#define ROMI_BRUSHMOTORDRIVER_H

#include <stdexcept>
#include <memory>
#include <atomic>
#include <thread>
#include <api/IMotorDriver.h>
#include <RomiSerialClient.h>

namespace romi {

        struct BrushMotorDriverSettings
        {
                int32_t encoder_steps;
                int8_t dir_left;
                int8_t dir_right;
                int32_t kp_numerator;
                int32_t kp_denominator;
                int32_t ki_numerator;
                int32_t ki_denominator;
                int32_t max_signal;

                static constexpr const char *kEncoderSteps = "encoder-steps"; 
                static constexpr const char *kMaxSignalAmpKey = "maximum-signal-amplitude"; 
                static constexpr const char *kPidKey = "pid"; 
                static constexpr const char *kKpKey = "kp"; 
                static constexpr const char *kKiKey = "ki"; 
                static constexpr const char *kEncoderDirectionsKey = "encoder-directions"; 
                static constexpr const char *kLeftKey = "left"; 
                static constexpr const char *kRightKey = "right"; 

                void parse(nlohmann::json &params) {
                        encoder_steps = (int32_t) params[kEncoderSteps];
                        dir_left = (int8_t) params[kEncoderDirectionsKey][kLeftKey];
                        dir_right = (int8_t) params[kEncoderDirectionsKey][kRightKey];
                        kp_numerator = (int32_t) params[kPidKey][kKpKey][0];
                        kp_denominator = (int32_t) params[kPidKey][kKpKey][1];
                        ki_numerator = (int32_t) params[kPidKey][kKiKey][0];
                        ki_denominator = (int32_t) params[kPidKey][kKiKey][1];
                        max_signal = (int32_t) params[kMaxSignalAmpKey];
                }
        };

        struct PidStatus
        {
                double time_;
                double target_;
                double measured_speed_;
                double output_;
                double error_p_;
                double error_i_;
                double error_d_;
                double controller_input_;
                
                PidStatus(double t, double target, double measured_speed,
                          double output, double error_p, double error_i,
                          double error_d, double controller_input)
                        : time_(t),
                          target_(target),
                          measured_speed_(measured_speed),
                          output_(output),
                          error_p_(error_p),
                          error_i_(error_i),
                          error_d_(error_d),
                          controller_input_(controller_input) {
                };
        };

        struct Speeds
        {
                double time_;
                double left_absolute_;
                double right_absolute_;
                double left_normalized_;
                double right_normalized_;
                
                Speeds(double t, double left_absolute, double right_absolute,
                       double left_normalized, double right_normalized)
                        : time_(t),
                          left_absolute_(left_absolute),
                          right_absolute_(right_absolute),
                          left_normalized_(left_normalized),
                          right_normalized_(right_normalized) {
                };
        };
        
        class BrushMotorDriver : public IMotorDriver
        {
        protected:
                std::unique_ptr<romiserial::IRomiSerialClient> serial_;
                BrushMotorDriverSettings settings_;
                double max_angular_speed_;
                
                bool configure_controller(nlohmann::json &config,
                                          double max_angular_speed, 
                                          double max_angular_acceleration);
                        
                bool check_response(const char *command,
                                    nlohmann::json& response);


                std::atomic<bool> recording_speeds_;
                std::unique_ptr<std::thread> speeds_thread_;
                void log_speeds();
                void record_speeds_main();
                
        public:

                BrushMotorDriver(std::unique_ptr<romiserial::IRomiSerialClient>& serial,
                                 nlohmann::json &config,
                                 double max_angular_speed,
                                 double max_angular_acceleration);
                
                ~BrushMotorDriver() override;

                const BrushMotorDriverSettings& get_settings();
                
                bool stop() override;
                bool moveat(double left, double right) override;
                int32_t get_encoder_steps() override;
                bool get_encoder_values(double& left, double& right,
                                        double& timestamp) override;

                bool get_speeds(double& left_target, double& right_target,
                                double& left_current, double& right_current,
                                double& left_measured, double& right_measured) override;
                
                void start_recording_speeds();
                void stop_recording_speeds();
                
                // Power device interface
                bool power_up() override;
                bool power_down() override;
                bool is_powered_up() override;
        };
}

#endif // ROMI_BRUSHMOTORDRIVER_H
