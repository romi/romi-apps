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
#ifndef __ROMI_METHODSRELAYBOARD_H
#define __ROMI_METHODSRELAYBOARD_H

namespace romi {
        
        class MethodsRelayBoard
        {
        public:
                static constexpr const char *kCountRelays = "relay-board:count-relays";
                static constexpr const char *kSet = "relay-board:set";
                static constexpr const char *kGet = "relay-board:get";

                static constexpr const char *kCount = "count";
                static constexpr const char *kIndex = "index";
                static constexpr const char *kValue = "value";
        };
}

#endif // __ROMI_METHODSRELAYBOARD_H
