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
#include "api/JsonFieldNames.h"
#include "json.hpp"
#include "api/DeviceData.h"

namespace romi {
        
        DeviceData::DeviceData(const std::string& type, 
                               const std::string& hardware_id,
                               const std::string& software_version)
                : type_(type),
                  hardware_id_(hardware_id),
                  software_version_(software_version)
        {
        }

        std::string DeviceData::type()
        {
                return type_;
        }

        std::string DeviceData::hardware_id()
        {
                return hardware_id_;
        }

        std::string DeviceData::software_version()
        {
                return software_version_;
        }

        nlohmann::json DeviceData::get_identity()
        {
                nlohmann::json identity{
                        {JsonFieldNames::romi_device_type.data(),  type_},
                        {JsonFieldNames::romi_hardware_id.data(), hardware_id_},
                        {JsonFieldNames::software_version_current.data(), software_version_}
                };

                return identity;
        }
}
