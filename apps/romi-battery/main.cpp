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

#include <iostream>
#include "INA219BatteryMonitor.h"

// static bool quit = false;
// static void set_quit(int sig, siginfo_t *info, void *ucontext);
// static void quit_on_control_c();

int main(int argc, char **argv)
{
        romi::INA219BatteryMonitor monitor;
        monitor.begin();
        std::cout << "Bus Voltage (V): " << monitor.get_voltage() << "\n";
        std::cout << "Current (A)   : " << monitor.get_current() << "\n";
        return 0;
}

// static void set_quit(int sig, siginfo_t *info, void *ucontext)
// {
//         (void) sig;
//         (void) info;
//         (void) ucontext;
//         quit = true;
// }

// static void quit_on_control_c()
// {
//         struct sigaction act;
//         memset(&act, 0, sizeof(struct sigaction));

//         act.sa_flags = SA_SIGINFO;
//         act.sa_sigaction = set_quit;
//         if (sigaction(SIGINT, &act, nullptr) != 0) {
//                 perror("init_signal_handler");
//                 exit(1);
//         }
// }
