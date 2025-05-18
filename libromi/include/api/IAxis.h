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

#ifndef ROMI_IAXIS_H
#define ROMI_IAXIS_H

#include <string>
#include "json.hpp"

namespace romi {
        
        struct AxisRange
        {
                AxisRange() :
                        min_(0),
                        max_(0) {
                }
                
                AxisRange(double min, double max) :
                        min_(min),
                        max_(max) {
                }
                
                double min_;
                double max_;
        };
        
        typedef enum {
                kLinearAxis,
                kAngularAxis
        } AxisType;
        
        typedef enum {
                kHomingContactAndBackup = 0,
                kHomingWithContact = 1
        } HomingMode;

        class IAxis
        {
        public:
                
                virtual ~IAxis() = default;
                
                virtual const std::string& name() const = 0;
                virtual void set_name(const std::string& s) = 0;
                virtual AxisType type() const = 0;
                virtual void set_type(AxisType type) = 0;
                virtual bool has_range() const = 0;
                virtual AxisRange get_range() const = 0;
                virtual void set_range(double min, double max) = 0;
                virtual bool homing() const = 0;
                virtual void set_homing(int order, HomingMode mode, double speed) = 0;
                virtual int homing_order() const = 0;
                virtual HomingMode homing_mode() const = 0;
                virtual double homing_speed() const = 0;
                virtual void to_json(nlohmann::json& obj) = 0;
        };
}

#endif // ROMI_IAXIS_H

