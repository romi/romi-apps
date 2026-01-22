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
#include <algorithm>
#include "util/Logger.h"
#include "util/Gpio.h"

#include <unistd.h>

namespace romi {

        Gpio::Gpio()
                : gpio_(nullptr),
                  fd_(-1),
                  pins_()
        {
                if (!map()) {
                        r_err("Failed to map GPIO registers. Try with sudo, "
                              "and ensure /dev/gpiomem or /dev/mem is accessible.)");
                        throw std::runtime_error("Failed to map GPIO registers");
                }
        }
        
        Gpio::~Gpio()
        {
                for (const auto& pair: pins_) {
                        clr(pair.first);
                        fsel(pair.first, kInput);
                }
                unmap();
        }
                
        void Gpio::assure_pin(uint32_t pin)
        {
                if (pin > 53) {
                        r_err("Invalid BCM pin: %d (must be [0,53]).", (int) pin); 
                        throw std::runtime_error("Invalid BCM pin");
                }
        }
        
        bool Gpio::has_pin(uint32_t pin)
        {
                return pins_.contains(pin);
        }
        
        void Gpio::add_pin(uint32_t pin)
        {
                pins_.insert({pin, false});
        }
        
        void Gpio::remove_pin(uint32_t pin)
        {
                pins_.erase(pin);
        }

        void Gpio::fsel(uint32_t pin, int mode)
        {
                uint32_t reg = pin / 10;           // 10 pins per SEL register
                uint32_t shift = (pin % 10) * 3;     // 3 bits per pin
                volatile uint32_t* fsel = &gpio_[kSel0 + reg];
                uint32_t val = *fsel;
                val &= ~(0x7u << shift);
                val |= (static_cast<uint32_t>(mode) & 0x7u) << shift;
                *fsel = val;
        }

        void Gpio::set(uint32_t pin)
        {
                gpio_[kSet0 + (pin / 32)] = (1u << (pin % 32));
        }

        void Gpio::clr(uint32_t pin)
        {
                gpio_[kClr0 + (pin / 32)] = (1u << (pin % 32));
        }

        void Gpio::init(uint32_t pin, int mode)
        {
                assure_pin(pin);
                
                if (!has_pin(pin)) {
                        fsel(pin, mode);
                        clr(pin);
                        add_pin(pin);
                        
                } else {
                        r_err("Gpio::init: pin %d already initialized", (int) pin);
                        throw std::runtime_error("Gpio::init: pin already initialized");
                }
        }
        
        void Gpio::forget(uint32_t pin)
        {
                if (has_pin(pin)) {
                        clr(pin);
                        fsel(pin, kInput);
                        remove_pin(pin);
                        
                } else {
                        r_err("Gpio::forget: pin %d not initialized", (int) pin);
                        throw std::runtime_error("Gpio::forget: pin not initialized");
                }
        }

        void Gpio::write(uint32_t pin, bool value)
        {
                if (has_pin(pin)) {
                        if (value) {
                                set(pin);
                        } else {
                                clr(pin);
                        }
                        pins_[pin] = value;
                } else {
                        r_err("Gpio::write: pin %d not initialized", (int) pin);
                        throw std::runtime_error("Gpio::write: pin not initialized");
                }
        }

        bool Gpio::map()
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

        void Gpio::unmap()
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

        void Gpio::get_pins(std::vector<uint32_t>& pins)
        {
                pins.clear();
                for (const auto& pair: pins_) {
                        pins.push_back(pair.first);
                }
        }
        
        bool Gpio::get_value(uint32_t pin)
        {
                if (has_pin(pin)) {
                        return pins_[pin];
                } else {
                        r_err("Gpio::write: pin %d not initialized", (int) pin);
                        throw std::runtime_error("Gpio::write: pin not initialized");
                }
        }
}
