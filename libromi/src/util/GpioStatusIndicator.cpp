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

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "util/Logger.h"
#include "util/ClockAccessor.h"
#include "util/GpioStatusIndicator.h"

#include <iostream>

namespace romi {

        GpioStatusIndicator::GpioStatusIndicator(uint32_t red_pin, uint32_t green_pin)
                : red_pin_(red_pin),
                  green_pin_(green_pin),
                  pwm_count_(false),
                  gpio_(nullptr),
                  fd_(-1),
                  running_(false),
                  thread_()
        {
                if (!map()) {
                        r_err("Failed to map GPIO registers. Try with sudo, "
                              "and ensure /dev/gpiomem or /dev/mem is accessible.)");
                        throw std::runtime_error("Failed to map GPIO registers");
                }

                assure_pin(red_pin_);
                assure_pin(green_pin_);
                
                fsel(red_pin_, kOutput);
                clr(red_pin_);
                fsel(green_pin_, kOutput);
                clr(green_pin_);

                thread_ = std::thread(&GpioStatusIndicator::run, this);
        }
        
        GpioStatusIndicator::~GpioStatusIndicator()
        {
                running_ = false;
                if (thread_.joinable())
                        thread_.join();
                
                fsel(red_pin_, kInput);
                clr(red_pin_);
                fsel(green_pin_, kInput);
                clr(green_pin_);
                unmap();
        }
                
        void GpioStatusIndicator::assure_pin(uint32_t pin)
        {
                if (pin > 53) {
                        r_err("Invalid BCM pin: %d (must be [0,53]).", (int) pin); 
                        throw std::runtime_error("Invalid BCM pin");
                }
        }

        void GpioStatusIndicator::fsel(uint32_t pin, int mode)
        {
                uint32_t reg = pin / 10;           // 10 pins per SEL register
                uint32_t shift = (pin % 10) * 3;     // 3 bits per pin
                volatile uint32_t* fsel = &gpio_[kSel0 + reg];
                uint32_t val = *fsel;
                val &= ~(0x7u << shift);
                val |= (static_cast<uint32_t>(mode) & 0x7u) << shift;
                *fsel = val;
        }

        void GpioStatusIndicator::set(uint32_t pin)
        {
                gpio_[kSet0 + (pin / 32)] = (1u << (pin % 32));
        }

        void GpioStatusIndicator::clr(uint32_t pin)
        {
                gpio_[kClr0 + (pin / 32)] = (1u << (pin % 32));
        }

        void GpioStatusIndicator::write(uint32_t pin, bool high)
        {
                if (high) {
                        set(pin);
                } else {
                        clr(pin);
                }
        }

        bool GpioStatusIndicator::map()
        {
                bool result = false;
                
                fd_ = ::open("/dev/gpiomem", O_RDWR | O_SYNC);
                if (fd_ >= 0) {
                        void* p = ::mmap(nullptr, kGpioLength,
                                         PROT_READ | PROT_WRITE, MAP_SHARED,
                                         fd_, 0);
                        if (p != MAP_FAILED) {
                                gpio_ = reinterpret_cast<volatile uint32_t*>(p);
                                result = true;
                        } else {
                                ::close(fd_); fd_ = -1;
                        }
                }
                
                return result;
        }

        void GpioStatusIndicator::unmap()
        {
                if (gpio_ && gpio_ != MAP_FAILED) {
                        ::munmap((void*) gpio_, kGpioLength);
                }
                if (fd_ >= 0) {
                        ::close(fd_);
                }
                gpio_ = nullptr;
                fd_ = -1;
        }

        void GpioStatusIndicator::run()
        {
                running_ = true;
                while (running_) {
                        update();
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                }
        }

        void GpioStatusIndicator::blink(uint32_t pin, uint32_t period)
        {
                if (pin == red_pin_) {
                        clr(green_pin_);
                } else {
                        clr(red_pin_);
                }
                write(pin, pwm_count_ < period/2);
                if (++pwm_count_ >= period) {
                        pwm_count_ = 0;
                }
        }

        void GpioStatusIndicator::light(uint32_t pin)
        {
                pwm_count_ = 0;
                if (pin == red_pin_) {
                        clr(green_pin_);
                        set(red_pin_);
                } else {
                        clr(red_pin_);
                        set(green_pin_);
                }
        }
}
