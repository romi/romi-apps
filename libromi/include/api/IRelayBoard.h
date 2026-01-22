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
#ifndef ROMI_IRELAYBOARD_H
#define ROMI_IRELAYBOARD_H

namespace romi {

        class IRelayBoard
        {
        public:
                virtual ~IRelayBoard() = default;
                
                virtual size_t count_relays() = 0;
                virtual void set(uint16_t index, bool value) = 0;
                virtual bool get(uint16_t index) = 0;
        };
}

#endif // ROMI_IRELAYBOARD_H
