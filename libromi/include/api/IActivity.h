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
#ifndef ROMI_IACTIVITY_H
#define ROMI_IACTIVITY_H

#include <stdexcept>

namespace romi {

        class ActivityResetException : public std::exception
        {
        public:
                ActivityResetException() : std::exception() {}
                
                virtual const char* what() const noexcept override {
                        return "The activity was cancelled"; 
                }
        };

        class IActivity
        {
        public:
                virtual ~IActivity() = default;
                
                virtual bool pause_activity() = 0;
                virtual bool continue_activity() = 0;
                virtual bool reset_activity() = 0;
        };
}

#endif // ROMI_IACTIVITY_H
