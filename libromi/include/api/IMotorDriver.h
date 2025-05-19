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
#ifndef ROMI_MOTORDRIVER_H
#define ROMI_MOTORDRIVER_H

#include <stdint.h>
#include "api/IPowerDevice.h"

namespace romi {
        
        class IMotorDriver : public IPowerDevice
        {
        public:
                
                virtual ~IMotorDriver() = default;

                /** The left and right speed are relative speeds. They
                 * must have a value between -1 and 1, and indicate
                 * the fraction of the maximum allowed speed. */
                virtual bool moveat(double left, double right) = 0;

                /** Stop as quick as possible. */
                virtual bool stop() = 0;

                /** Returns the number of steps per revolution. */
                virtual int32_t get_encoder_steps() = 0;

                /** Returns the values of the encoders. The timestamp
                 * is in seconds. */
                virtual bool get_encoder_values(double& left, double& right,
                                                double& timestamp) = 0;

                enum Axis { kLeftWheel = 0, kRightWheel = 1 };

                virtual bool get_speeds(double& left_target, double& right_target,
                                        double& left_current, double& right_current,
                                        double& left_measured, double& right_measured) = 0;
        };
}

#endif // ROMI_MOTORDRIVER_H
