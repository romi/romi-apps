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
#ifndef ROMI_DEVICEDATA_H
#define ROMI_DEVICEDATA_H

#include "IDeviceData.h"

namespace romi {

        class DeviceData : public IDeviceData
        {
        protected:
                std::string type_;
                std::string hardware_id_;
                std::string software_version_;

        public:
                DeviceData(const std::string& type, 
                           const std::string& hardware_id, 
                           const std::string& software_version);
                
                std::string type() override;
                std::string hardware_id() override;
                std::string software_version() override;
                nlohmann::json get_identity() override;
        };

}

#endif // ROMI_DEVICEDATA_H
