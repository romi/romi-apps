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

#include "BatteryStatusIndicator.h"

namespace romi {

        BatteryStatusIndicator::BatteryStatusIndicator(int red_pin, int green_pin)
                : GpioStatusIndicator(red_pin, green_pin),
                  status_(kInitializing)
        {
        }
        
        void BatteryStatusIndicator::set(BatteryStatus status)
        {
                status_ = status;
        }
        
        void BatteryStatusIndicator::update()
        {
                switch (status_) {
                case kInitializing:
                        blink(green_pin_, 2);
                        break;
                case kDischarging:
                        blink(red_pin_, 8);
                        break;
                case kCharging:
                        blink(green_pin_, 8);
                        break;
                case kCharged:
                        light(green_pin_);
                        break;
                case kLow:
                        blink(red_pin_, 2);
                        break;
                default:
                case kError:
                        light(red_pin_);
                        break;
                }
        }
}
