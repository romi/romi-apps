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
#ifndef ROMI_NAVIGATIONSETTINGS_H
#define ROMI_NAVIGATIONSETTINGS_H

#include <json.hpp>

namespace romi {
        
        class NavigationSettings
        {
        public:

                static constexpr const char *kWheelDiameterKey = "wheel-diameter"; 
                static constexpr const char *kWheelbaseKey = "wheelbase"; 
                static constexpr const char *kWheeltrackKey = "wheeltrack"; 
                static constexpr const char *kMaximumSpeedKey = "maximum-speed"; 
                static constexpr const char *kMaximumAccelerationKey = "maximum-acceleration"; 
                
                double wheel_diameter;
                double wheelbase;
                double wheeltrack;
                double maximum_speed;
                double maximum_acceleration;

                NavigationSettings(nlohmann::json &config);
                virtual ~NavigationSettings() = default;

                double wheel_circumference();
                double to_angular_speed(double linear_speed);
                double compute_max_angular_speed();
                double compute_max_angular_acceleration();
        };
}

#endif // ROMI_NAVIGATIONSETTINGS_H
