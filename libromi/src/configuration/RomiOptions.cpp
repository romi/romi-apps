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
#include <stdexcept>

#include "configuration/RomiOptions.h"

using namespace std;

namespace romi {

    static std::vector<Option> options = {
                { RomiOptions::kHelp, false, nullptr,
                  "Print help message" },
                
                { RomiOptions::kConfig, true, "config.json",
                  "Path of the config file" },
                
                { RomiOptions::kRegistry, true, nullptr,
                  "The IP address of the registry"},
                
                { RomiOptions::kDirectory, true, ".",
                  "The session directory where the output "
                  "files are stored (logs, images...)"},
                
                { RomiOptions::kTopic, true, nullptr,
                  "The topic/name of the node"},
        };

        RomiOptions::RomiOptions()
            : GetOpt(options)
        {}
        
        void RomiOptions::exit_if_help_requested()
        {
                if (is_help_requested()) {
                        print_usage();
                        exit(0);
                }
        }

        std::string RomiOptions::get_config_file()
        {
                std::string file = get_value(romi::RomiOptions::kConfig);
                if (file.empty()) {
                        throw std::runtime_error("No configuration file was given "
                                                 "(can't run without one...).");
                }
                return file;
        }

}

