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

#ifndef ROMI_CNCRANGE_H
#define ROMI_CNCRANGE_H

#include "json.hpp"
#include "v3.h"
#include "Range.h"

namespace romi {

        class CNCRange : public Range
        {
            public:
                CNCRange();
                explicit CNCRange(nlohmann::json &range);
                CNCRange(const double *min, const double *max);
                CNCRange(v3 min, v3 max);
        };
}

#endif // ROMI_CNCRANGE_H
