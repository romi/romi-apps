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
#ifndef __ROMI_METHODSIMU_H
#define __ROMI_METHODSIMU_H

namespace romi {
        
        class MethodsIMU
        {
        public:
                static constexpr const char *kGetAcceleration = "imu:get-acceleration";
                static constexpr const char *kGetAngularVelocity = "imu:get-angular-velocity";
                static constexpr const char *kGetMagneticField = "imu:get-magnetic-field";
                static constexpr const char *kGetOrientation = "imu:get-orientation";

                static constexpr const char *kTimestamp = "timestamp";
                static constexpr const char *kAcceleration = "acceleration";
                static constexpr const char *kAngularVelocity = "angular-velocity";
                static constexpr const char *kMagneticField = "magnetic-field";
                static constexpr const char *kOrientation = "orientation";
        };
}

#endif // __ROMI_METHODSIMU_H
