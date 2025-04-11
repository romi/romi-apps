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

#ifndef ROMI_ICAMERA_H
#define ROMI_ICAMERA_H

#include <rcom/json.hpp>
#include <rcom/MemBuffer.h>
#include "api/Image.h"
#include "api/IPowerDevice.h"
#include "api/ICameraSettings.h"

namespace romi {

        class ICamera : public IPowerDevice
        {
        public:
                virtual ~ICamera() = default;
                virtual bool grab(Image &image) = 0;
                virtual rcom::MemBuffer& grab_jpeg() = 0;
                
                virtual bool set_value(const std::string& name, double value) = 0;
                virtual bool select_option(const std::string& name,
                                           const std::string& value) = 0;

                virtual const ICameraSettings& get_settings() = 0;
                
                //virtual bool load_settings(const std::string& name, double value) = 0;
        };
}

#endif // ROMI_ICAMERA_H
