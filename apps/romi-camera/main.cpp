/*
  romi-camera

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-camera is a camera app for the Romi platform.

  romi-camera is free software: you can redistribute it and/or modify
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
#include <stdexcept>
#include <memory>

#include <rcom/Linux.h>
#include <rcom/RegistryServer.h>
#include <rcom/RcomServer.h>
#include <rcom/RcomClient.h>

#include <rpc/RcomLog.h>

#include <api/ICameraSettings.h>
#include <configuration/GetOpt.h>
#include <configuration/LocalConfig.h>
#include <configuration/RomiOptions.h>
#include <util/ClockAccessor.h>
#include <util/Logger.h>
#include <rpc/RemoteConfig.h>
#include <romi_config.h>

// Session
// #include <api/DeviceData.h>
// #include <api/DummyLocationProvider.h>
// #include <api/Session.h>

#include "CameraInfoIO.h"
#include "CameraFactory.h"
#include "CameraAdaptor.h"

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

int main(int argc, char **argv)
{
        std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
        romi::ClockAccessor::SetInstance(clock);
        
        try {
                // Linux
                rcom::Linux linux;
                std::shared_ptr<rcom::ILog> rcomlog = std::make_shared<romi::RcomLog>();
                
                // Options
                romi::RomiOptions options;
                options.parse(argc, argv);
                if (options.is_help_requested()) {
                        options.print_usage();
                        exit(0);
                }
                
                if (options.is_set(romi::RomiOptions::kRegistry)) {
                        std::string ip = options.get_value(romi::RomiOptions::kRegistry);
                        r_info("Registry IP set to %s", ip.c_str());
                        rcom::RegistryServer::set_address(ip.c_str());
                }

                // Topic
                std::string topic = "camera";
                if (options.is_set(romi::RomiOptions::kTopic)) {
                        topic = options.get_value(romi::RomiOptions::kTopic);
                }
                
                log_set_application(topic);
                
                // Config
                std::shared_ptr<romi::IConfigManager> config;
                
                if (options.is_set(romi::RomiOptions::kConfig)) {
                        std::string config_value =
                                options.get_value(romi::RomiOptions::kConfig);
                        
                        r_info("romi-camera: Using local configuration file: '%s'",
                               config_value.c_str());
                
                        std::filesystem::path config_path = config_value;
                        config = std::make_shared<romi::LocalConfig>(config_path);
                } else {
                        r_info("romi-camera: Using remote configuration");
                        auto client = rcom::RcomClient::create("config", 10.0, rcomlog);
                        config = std::make_shared<romi::RemoteConfig>(client);
                }
                
                // Camera settings
                std::shared_ptr<romi::ICameraInfoIO> info_io;
                try {
                        info_io = std::make_shared<romi::CameraInfoIO>(config, topic);
                        
                } catch (std::exception& e) {
                        r_debug("romi-camera: Failed to load the camera configuration");
                        throw;
                }

                // Camera
                r_debug("romi-camera: Initializing camera");
                std::unique_ptr<romi::ICamera> camera;
                try {
                        camera = romi::CameraFactory::create(linux, info_io);
                        
                } catch (std::exception& e) {
                        r_debug("romi-camera: Failed to create the camera");
                        throw;
                }
                
                // Session
                // nlohmann::json device_config = config->get_section("device");
                // std::string device_type = device_config["type"];
                // std::string device_id = device_config["hardware-id"];
                // std::string software_version = PROJECT_VERSION;
                // std::unique_ptr<romi::IDeviceData> device
                //         = std::make_unique<romi::DeviceData>(device_type, device_id,
                //                                              software_version);
                // std::unique_ptr<romi::ILocationProvider> location
                //         = std::make_unique<romi::DummyLocationProvider>();
                // std::string directory = options.get_value(romi::RomiOptions::kDirectory);
                // romi::Session session(linux, directory,
                //                       std::move(device),
                //                       std::move(location));


                //
                romi::CameraAdaptor adaptor(*camera);
                auto camera_server = rcom::RcomServer::create(topic, adaptor);
                
                quit_on_control_c();
                
                while (!quit) {
                        camera_server->handle_events();
                        clock->sleep(0.001);
                }

        } catch (std::exception& e) {
                r_err("RomiCamera: caught exception: %s", e.what());
        }
}

static void set_quit(int sig, siginfo_t *info, void *ucontext)
{
        (void) sig;
        (void) info;
        (void) ucontext;
        quit = true;
}

static void quit_on_control_c()
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));

        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = set_quit;
        if (sigaction(SIGINT, &act, nullptr) != 0) {
                perror("init_signal_handler");
                exit(1);
        }
}
