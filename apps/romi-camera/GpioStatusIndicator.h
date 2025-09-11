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

#ifndef ROMI_GPIOSTATUSINDICATOR_H
#define ROMI_GPIOSTATUSINDICATOR_H

#include <atomic>
#include <thread>
#include <cstdint>
#include "ICameraStatusIndicator.h"


/*
  Controls 2 GPIO pins on Raspberry Pi Zero 2 (Linux) without external libraries.
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

  Avoid pins in use by interfaces you’ve enabled (e.g., BCM2/3 I²C,
  BCM14/15 UART) unless you’ve disabled those interfaces.
*/

namespace romi {

                
        class GpioStatusIndicator : public ICameraStatusIndicator
        {
        protected:
                enum { kInput = 0, kOutput = 1 };

                static constexpr size_t kGpioLength = 0x1000; // map 4KB for GPIO block

                static constexpr uint32_t kSel0 = 0;    // 0x00
                static constexpr uint32_t kSet0 = 7;    // 0x1C
                static constexpr uint32_t kClr0 = 10;   // 0x28
                
                std::atomic<CameraStatus> status_;
                int red_pin_;
                int green_pin_;
                int pwm_count_;
                volatile uint32_t* gpio_;
                int fd_;
                std::atomic<bool> running_;
                std::thread thread_;

                void assure_pin(int pin);
                void fsel(int pin, int mode);
                void set(int pin);
                void clr(int pin);
                void write(int pin, bool high);
                bool map();
                void unmap();

                void run();
                void update();

        public:
                GpioStatusIndicator(int red_pin, int green_pin);
                virtual ~GpioStatusIndicator() override;
                void set(CameraStatus status) override;
        };
}

#endif
