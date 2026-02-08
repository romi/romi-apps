/*
  romi-battery

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
#include <csignal>

#include <rcom/Linux.h>
#include <rcom/RegistryServer.h>
#include <rcom/RcomServer.h>
#include <rcom/RcomClient.h>
#include <rcom/RcomMessageHandler.h>
#include <rcom/MessageHub.h>
#include <rcom/IMessageListener.h>
#include <rcom/DummyListener.h>
#include <rcom/util.h>

#include <configuration/RomiOptions.h>
#include <rpc/RcomLog.h>
#include <rpc/RemoteDataLog.h>
#include <util/ClockAccessor.h>
#include <util/Logger.h>

#include "IMUAdaptor.h"
#include "IMURT.h"

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

rcom::MemBuffer message;

void broadcast_orientation(rcom::IMessageHub& hub, romi::IIMU& imu)
{
        double t = imu.get_timestamp();
        quaternion_t q = imu.get_orientation();
        message.clear();
        message.printf("[%.6f,%.6f,%.6f,%.6f,%.6f]", t, q.s, q.v.x, q.v.y, q.v.z);
        hub.broadcast(message, rcom::kTextMessage, nullptr);
}

int main(int argc, char **argv)
{
        try {
                std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
                romi::ClockAccessor::SetInstance(clock);
                romi::RcomLog log;
                rcom::Linux system(log);
                
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
                std::string topic = "imu";
                std::string type = "imu";
                if (options.is_set(romi::RomiOptions::kTopic)) {
                        topic = options.get_value(romi::RomiOptions::kTopic);
                }
                
                log_set_application(topic);

                // TODO: Get options from config file
                romi::IMURT imu; 
                romi::IMUAdaptor adaptor(imu);
                rcom::RcomMessageHandler listener(adaptor);
                auto server = rcom::RcomServer::create(topic, type, listener, log, system);

                rcom::DummyListener dummy;
                std::string broadcast_topic = topic + "-broadcast";
                auto hub = rcom::MessageHub::create(broadcast_topic,
                                                    type, dummy, log, system);
                
                quit_on_control_c();
        
                double interval = imu.get_preferred_update_interval() - 0.001;
                while (!quit) {
                        imu.update();
                        broadcast_orientation(*hub, imu);
                        hub->handle_events();
                        server->handle_events();
                        clock->sleep(interval);
                }
                
        } catch (std::exception& e) {
                r_err("RomiBattery: caught exception: %s", e.what());
        }
        return 0;
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
