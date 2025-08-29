/*
  romi-config

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-config provide a single config file to distributed Romi apps.

  romi-config is free software: you can redistribute it and/or modify
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
#include <exception>
#include <stdexcept>
#include <string>
#include <atomic>
#include <syslog.h>
#include <csignal>

#include <rcom/Linux.h>
#include <rcom/RcomServer.h>
#include <rcom/RegistryServer.h>
#include <rcom/RcomClient.h>
#include <rcom/RcomMessageHandler.h>

#include <api/Axis.h>
#include <api/DeviceData.h>
#include <api/Gps.h>
#include <api/GpsLocationProvider.h>
#include <api/Session.h>
#include <configuration/LocalConfig.h>
#include <rpc/RcomLog.h>
#include <rpc/RemoteConfig.h>
#include <util/ClockAccessor.h>

#include <romi_config.h>

#include "Oquam.h"
#include "StepperSettings.h"
#include "CNCOptions.h"
#include "CNCFactory.h"
#include "CNCAdaptor.h"

std::atomic<bool> quit(false);

static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
}

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "rcom-registry segmentation fault");
                exit(signal);
        }
        else if (signal == SIGINT){
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        }
        else{
                r_err("Unknown signam received %d", signal);
        }
}

int main(int argc, char** argv)
{
        std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
        romi::ClockAccessor::SetInstance(clock);

        int retval = 1;

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {
                romi::RcomLog log;
                rcom::Linux system(log);

                // Options
                romi::CNCOptions options;
                options.parse(argc, argv);
                if (options.is_help_requested()) {
                        options.print_usage();
                        return 0;
                }
                
                if (options.is_set(romi::RomiOptions::kRegistry)) {
                        std::string ip = options.get_value(romi::RomiOptions::kRegistry);
                        r_info("Registry IP set to %s", ip.c_str());
                        rcom::RegistryServer::set_address(ip.c_str());
                }

                // Topic
                std::string topic = "cnc";
                std::string type = "cnc";
                if (options.is_set(romi::RomiOptions::kTopic)) {
                        topic = options.get_value(romi::RomiOptions::kTopic);
                }

                log_set_application(topic);
		
                // Config
                std::shared_ptr<romi::IConfigManager> config;
                
                if (options.is_set(romi::RomiOptions::kConfig)) {
                        std::string config_value = options.get_value(romi::RomiOptions::kConfig);
                        
                        r_info("romi-camera: Using local configuration file: '%s'",
                               config_value.c_str());
                
                        std::filesystem::path config_path = config_value;
                        config = std::make_shared<romi::LocalConfig>(config_path);
                } else {
                        r_info("romi-camera: Using remote configuration");
                        auto client = rcom::RcomClient::create("config", 10.0, log, system);
                        config = std::make_shared<romi::RemoteConfig>(client);
                }

                if (!config->has_section(topic)) {
                        r_debug("main.cpp: The '%s' section (=topic name) is missing in "
                                "the configuration file.", topic.c_str());
                        throw std::runtime_error("Missing topic section in configuration");
                }
                
                nlohmann::json cnc_config = config->get_section(topic);
                
                // Session
                nlohmann::json device_config = config->get_section("device");
                std::string device_type = device_config["type"];
                std::string device_id = device_config["hardware-id"];
                std::string software_version = PROJECT_VERSION;
                std::unique_ptr<romi::IDeviceData> device
                        = std::make_unique<romi::DeviceData>(device_type, device_id,
                                                             software_version); 
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> location
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string directory
                        = options.get_value(romi::RomiOptions::kDirectory);
                r_info("Session directory: %s", directory.c_str());
                romi::Session session(system, directory, std::move(device),
                                      std::move(location));
                session.start("oquam_observation_id");
                
                // CNC
                romi::CNCFactory factory;

                //
                romi::Axis axes[3];
                
                if (cnc_config.contains("axes")) {

                        nlohmann::json axes_data = cnc_config["axes"];
                        if (!axes_data.is_array()) {
                                throw std::runtime_error("main: axis config must "
                                                         "be an array");
                        }
                        
                        for (size_t i = 0; i < axes_data.size(); i++) {
                                axes[i].init(i, axes_data[i]);
                        }
                        
                } else {
                        nlohmann::json range_data = cnc_config["cnc-range"];
                        romi::CNCRange range(range_data);

                        axes[0].set_type(romi::kLinearAxis);
                        axes[1].set_type(romi::kLinearAxis);
                        axes[2].set_type(romi::kLinearAxis);
                        axes[0].set_name("x");
                        axes[1].set_name("y");
                        axes[2].set_name("z");
                        axes[0].set_range(range.xmin(), range.xmax());
                        axes[1].set_range(range.ymin(), range.ymax());
                        axes[2].set_range(range.zmin(), range.zmax());
                        
                        romi::AxisIndex homing_axes[3] = {
                                romi::kAxisX,
                                romi::kAxisY,
                                romi::kAxisZ };
                        
                        nlohmann::json homing_settings = cnc_config["homing"];
                        auto homing_mode = (romi::HomingMode) homing_settings["mode"];
                        homing_axes[0] = homing_settings["axes"][0];
                        homing_axes[1] = homing_settings["axes"][1];
                        homing_axes[2] = homing_settings["axes"][2];

                        for (int i = 0; i < 3; i++) {
                                if (homing_axes[i] >= 0) {
                                        int axis = homing_axes[i];
                                        axes[axis].set_homing(i, homing_mode, 0.1);
                                }
                        }
                }

                
                nlohmann::json stepper_data = cnc_config["stepper-settings"];
                romi::StepperSettings stepper_settings(stepper_data);
        
                double slice_duration = (double) cnc_config["path-slice-duration"];
                double maximum_deviation = (double) cnc_config["path-maximum-deviation"];

                nlohmann::json ports_config = config->get_section("ports");
                romi::ICNCController& controller = factory.create_controller(options,
                                                                             cnc_config,
                                                                             ports_config);

                double max_steps_per_block = 32000.0; // Should be less than 2^15/2-1
                double max_slice_duration = stepper_settings.compute_minimum_duration(max_steps_per_block);
                                        
                        
                romi::OquamSettings oquam_settings(axes,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   max_slice_duration);
                
                romi::Oquam oquam(controller, oquam_settings, session);

                // RPC access
                romi::CNCAdaptor adaptor(oquam);
                rcom::RcomMessageHandler listener(adaptor);
                auto server = rcom::RcomServer::create(topic, type, listener, log, system);
                
                while (!quit) {
                        server->handle_events();
                        system.sleep(0.010);
                }
                
                retval = 0;
                
        } catch (nlohmann::json::exception& je) {
                r_err("main: Failed to read the config file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: std::exception: %s", e.what());
        }

        return retval;
}

