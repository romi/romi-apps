/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  rcom is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef _ROMI_I_CLOCK_H_
#define _ROMI_I_CLOCK_H_

#include <string>
#include <stdint.h>

namespace romi {

        // The ranges are similar to the standard localtime()
        // function, with the exception of "year", which returns the
        // actual year (ex. the year 1984 returns 1984 and not 84).
        struct Date {
                int16_t year_;   // ex. 2022, [-32768, 32767]
                uint8_t month_;  // [0, 11],  
                uint8_t day_;    // [1, 31]
                uint8_t hour_;   // [0, 23]
                uint8_t minute_; // [0, 59]
                uint8_t second_; // [0, 59]

                Date() : year_(0), month_(0), day_(0),
                         hour_(0), minute_(0), second_(0) {
                }

                Date(int16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second)
                        : year_(year), month_(month), day_(day),
                          hour_(hour), minute_(minute), second_(second) {
                }
        };
        
        class IClock {
        public:
                IClock() = default;
                
                virtual ~IClock() = default;
                
                virtual double time() = 0;
                virtual Date date() = 0; // In local calendar
                virtual std::string datetime_compact_string() = 0;
                virtual uint64_t timestamp() = 0;
                // TBD: Move this out of clock
                virtual void sleep(double seconds)= 0;
        };
}

#endif // _ROMI_I_CLOCK_H_

