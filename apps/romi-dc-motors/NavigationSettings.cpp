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
#include <math.h>
#include "NavigationSettings.h"

namespace romi {
        
        NavigationSettings::NavigationSettings(nlohmann::json &config)
                : wheel_diameter(0.0),
                  wheelbase(0.0),
                  wheeltrack(0.0),
                  maximum_speed(0.0),
                  maximum_acceleration(0.0)
        {
                wheel_diameter = config[kWheelDiameterKey];
                wheelbase = config[kWheelbaseKey];
                wheeltrack = config[kWheeltrackKey];
                maximum_speed = config[kMaximumSpeedKey];
                maximum_acceleration = config[kMaximumAccelerationKey];
        }
        
        double NavigationSettings::wheel_circumference()
        {
                return M_PI * wheel_diameter;
        }
        
        double NavigationSettings::to_angular_speed(double linear_speed)
        {
                return linear_speed / wheel_circumference();
        }

        double NavigationSettings::compute_max_angular_speed()
        {
                return maximum_speed / wheel_circumference();
        }
        
        double NavigationSettings::compute_max_angular_acceleration()
        {
                return maximum_acceleration / wheel_circumference();
        }
}
