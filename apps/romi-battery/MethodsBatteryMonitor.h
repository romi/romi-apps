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
#ifndef __ROMI_METHODSBATTERYMONITOR_H
#define __ROMI_METHODSBATTERYMONITOR_H

namespace romi {
        
        class MethodsBatteryMonitor
        {
        public:
                static constexpr const char *kIsCharging = "battery-monitor:is-charging";
                static constexpr const char *kGetVoltage = "battery-monitor:get-voltage";
                static constexpr const char *kGetCurrent = "battery-monitor:get-current";

                static constexpr const char *kCharging = "battery-monitor:charging";
                static constexpr const char *kVoltage = "battery-monitor:voltage";
                static constexpr const char *kCurrent = "battery-monitor:current";
        };
}

#endif // __ROMI_METHODSBATTERYMONITOR_H
