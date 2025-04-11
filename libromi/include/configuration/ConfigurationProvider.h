/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
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
#ifndef _LIBROMI_CONFIGURATIONPROVIDER_H
#define _LIBROMI_CONFIGURATIONPROVIDER_H

#include "json.hpp"
#include <rover/RoverOptions.h>

namespace romi
{
    std::string get_session_directory(romi::IOptions& options, nlohmann::json& config);
    
    std::string get_brush_motor_device_in_config(nlohmann::json& config);
    std::string get_brush_motor_device(romi::IOptions& options, nlohmann::json& config);
    std::string get_script_file(romi::IOptions& options, nlohmann::json& config);
    std::string get_camera_image(romi::IOptions& options, nlohmann::json& config);
    std::string get_camera_device_in_config(nlohmann::json& config);
    std::string get_camera_device(romi::IOptions& options, nlohmann::json& config);
    std::string get_camera_classname_in_config(nlohmann::json& config);
    std::string get_camera_classname(romi::IOptions& options, nlohmann::json& config);
}

#endif // _LIBROMI_CONFIGURATIONPROVIDER_H
