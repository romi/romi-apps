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

#ifndef ROMI_IRANGE_H
#define ROMI_IRANGE_H

#include "json.hpp"
#include "v3.h"

namespace romi {
        
        class IRange
        {
        public:
                virtual ~IRange() = default;
                virtual void init(nlohmann::json &range)=0;
                virtual void init(v3 min, v3 max) = 0;
                virtual nlohmann::json to_json() = 0;
                
                virtual v3 min() const = 0;
                virtual v3 max() const = 0;
                virtual v3 dimensions() const = 0;
                
                virtual bool is_inside(double x, double y, double z) = 0;
                virtual bool is_inside(v3 p) = 0;
                
                // Computes the distance of a point that lies outside
                // the range to the border of the range.
                virtual double error(double x, double y, double z) = 0;
                virtual double error(v3 p) = 0;

                virtual v3 clamp(v3 p) const = 0;
                
                virtual double xmin() const = 0;
                virtual double xmax() const = 0;
                virtual double ymin() const = 0;
                virtual double ymax() const = 0;
                virtual double zmin() const = 0;
                virtual double zmax() const = 0;
                
        };
}

#endif // ROMI_IRANGE_H
