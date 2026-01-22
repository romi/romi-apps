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

#ifndef ROMI_GPIO_H
#define ROMI_GPIO_H

#include <cstdint>

/*
  Controls GPIO pins on Raspberry Pi (Linux) without external libraries.
  - Direct MMIO via /dev/gpiomem
  - Uses BCM numbering (not physical header numbers).
  - /dev/gpiomem lets you run without root if your user is in the 'gpio' group.

  Pins are BCM numbers (not physical). On the Pi Zero header, handy
  choices include:

  BCM 18 → physical 12 
  BCM 5  → physical 29 
  BCM 6  → physical 31 
  BCM 12 → physical 32 
  BCM 13 → physical 33 
  BCM 19 → physical 35 
  BCM 16 → physical 36 
  BCM 26 → physical 37
  BCM 20 → physical 38
  BCM 21 → physical 40

  Avoid pins in use by interfaces you’ve enabled (e.g., BCM2/3 I²C,
  BCM14/15 UART) unless you’ve disabled those interfaces.
*/

namespace romi {

                
        class Gpio
        {
        protected:

                static constexpr size_t kGpioLength = 0x1000; // map 4KB for GPIO block

                static constexpr uint32_t kSel0 = 0;    // 0x00
                static constexpr uint32_t kSet0 = 7;    // 0x1C
                static constexpr uint32_t kClr0 = 10;   // 0x28
                
                volatile uint32_t* gpio_;
                int fd_;
                std::map<uint32_t, bool> pins_;
                
                void assure_pin(uint32_t pin);
                void fsel(uint32_t pin, int mode);
                void set(uint32_t pin);
                void clr(uint32_t pin);
                bool map();
                void unmap();
                bool has_pin(uint32_t pin);
                void add_pin(uint32_t pin);
                void remove_pin(uint32_t pin);

        public:
                enum { kInput = 0, kOutput = 1 };
                
                Gpio();
                virtual ~Gpio();

                Gpio(const romi::Gpio&) = delete;
                Gpio& operator=(const Gpio&) = delete;

                void init(uint32_t pin, int mode);
                void forget(uint32_t pin);
                void write(uint32_t pin, bool high);
                
                void get_pins(std::vector<uint32_t>& pins);
                bool get_value(uint32_t pin);
        };
}

#endif
