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
#include "util/Logger.h"
#include "GpioRelayBoard.h"

namespace romi {

        GpioRelayBoard::GpioRelayBoard(std::vector<uint32_t> pins)
                : gpio_(),
                  pins_(pins)
        {
                for (const uint32_t& pin: pins) {
                        gpio_.init(pin, Gpio::kOutput);
                }
        }

        size_t GpioRelayBoard::count_relays()
        {
                return pins_.size();
        }
        
        void GpioRelayBoard::set(uint16_t index, bool value)
        {
                uint32_t pin = pins_[index];
                gpio_.write(pin, value);
        }
        
        bool GpioRelayBoard::get(uint16_t index)
        {
                uint32_t pin = pins_[index];
                return gpio_.get_value(pin);
        }
}
