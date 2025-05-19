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

#include <RSerial.h>
#include <RomiSerialClient.h>
#include <util/RomiSerialLog.h>

#include <rcom/Linux.h>
#include <rcom/RcomServer.h>
#include <rcom/RegistryServer.h>
#include <rcom/RcomClient.h>
#include <rcom/RcomMessageHandler.h>

#include <api/DeviceData.h>
#include <api/Gps.h>
#include <api/GpsLocationProvider.h>
#include <api/Session.h>
#include <configuration/LocalConfig.h>
#include <rpc/RcomLog.h>
#include <rpc/RemoteConfig.h>
#include <util/ClockAccessor.h>
#include <util/DataLog.h>
#include <util/DataLogAccessor.h>

#include "BrushMotorDriver.h"
#include "MotorsAdaptor.h"
#include "MotorsOptions.h"
#include "NavigationSettings.h"

#include <romi_config.h>

std::atomic<bool> quit(false);

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "main.cpp segmentation fault");
                exit(signal);
        } else if (signal == SIGINT){
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        } else{
                r_err("Unknown signal received %d", signal);
        }
}

void assert_field(nlohmann::json& data, const std::string& name)
{
        if (!data.contains(name)) {
                r_err("main.cpp: missing field: %s", name.c_str());
                throw std::runtime_error("main.cpp: missing field");
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
                romi::MotorsOptions options;
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
                std::string topic = "motors";
                std::string type = "motors";
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
                
                
                // Session
                nlohmann::json device_config = config->get_section("device");
                assert_field(device_config, "type");
                assert_field(device_config, "hardware-id");
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
                
                // Log redirection for romi serial
                std::shared_ptr<romiserial::ILog> serial_log
                        = std::make_shared<romi::RomiSerialLog>();

                // DataLog
                std::string kDatalogFile = "datalog.txt";
                auto datalog = std::make_shared<romi::DataLog>(kDatalogFile, log, system);
                romi::DataLogAccessor::set(datalog);
                
                // Motor driver
                r_info("main: Creating motor driver");
                nlohmann::json motors_config = config->get_section(topic);
                
                assert_field(device_config, "rover");
                nlohmann::json rover_settings = motors_config["rover"];
                romi::NavigationSettings rover_config(rover_settings);
                
                assert_field(device_config, "dc-motors-driver");
                nlohmann::json driver_settings = motors_config["dc-motors-driver"];

                nlohmann::json ports_config = config->get_section("ports");
                assert_field(ports_config, "dc-motors-driver");
                std::string driver_device = ports_config["dc-motors-driver"]["port"];

                std::string client_name = "dc_motors_driver";
                auto driver_serial = romiserial::RomiSerialClient::create(driver_device,
                                                                          client_name,
                                                                          serial_log);
                romi::BrushMotorDriver motors(driver_serial,
                                              driver_settings,
                                              rover_config.compute_max_angular_speed(),
                                              rover_config.compute_max_angular_acceleration());

                
                // RPC access
                romi::MotorsAdaptor adaptor(motors);
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

