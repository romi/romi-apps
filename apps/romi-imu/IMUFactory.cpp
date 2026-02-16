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
#include <util/Logger.h>
#include "IMURT.h"
#include "FakeIMU.h"
#include "IMUFactory.h"

namespace romi {
	
        IMUFactory::IMUFactory()
        {
        }
        
        std::unique_ptr<IIMU> IMUFactory::create(nlohmann::json& config)
        {
                std::string type = config["type"];
                
                std::unique_ptr<IIMU> ptr;
                if (type == "fake-imu") {
                        ptr = std::make_unique<FakeIMU>();
                } else if (type == "rtimu") {
                        ptr = std::make_unique<IMURT>();
                } else {
                        r_err("IMUFactory::create: Uknown type: %s", type.c_str());
                        throw std::runtime_error("IMUFactory::create: Uknown type");
                }
                return ptr;
        }
}
