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

#pragma once

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <chrono>
#include <cstring>
#include <string>

namespace romi {

        class ISubprocess {
        public:
                struct Result {
                        int status = 0;  // status from waitpid
                        int exit_code;   // set if exited normally
                        int term_signal; // set if terminated by signal
                };

                virtual ~ISubprocess() = default;

                virtual void start(std::string commandLine,
                                   bool newProcessGroup = true) = 0;
                virtual bool is_running() = 0;
                virtual Result wait() = 0;
                virtual bool stop(std::chrono::milliseconds timeout,
                                  bool toProcessGroup = true,
                                  int gracefulSig = SIGTERM,
                                  int forceSig = SIGKILL) = 0;
                virtual Result last_result() const = 0;
}
