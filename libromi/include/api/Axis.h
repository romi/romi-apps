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

#ifndef ROMI_AXIS_H
#define ROMI_AXIS_H

#include "api/IAxis.h"
#include "json.hpp"

namespace romi {

        class Axis : public IAxis
        {
        protected:
                std::string name_;
                AxisType type_;
                bool has_range_;
                AxisRange range_;
                bool homing_;
                int homing_order_;
                HomingMode homing_mode_;
                double homing_speed_;
                
        public:
                
                Axis();
                Axis(size_t index, nlohmann::json data);
                ~Axis() override = default;

                void init(size_t index, nlohmann::json data);
                void to_json(nlohmann::json& obj) override;
                
                const std::string& name() const override;
                void set_name(const std::string& s) override;
                AxisType type() const override;
                void set_type(AxisType type) override;
                bool has_range() const override;
                AxisRange get_range() const override;
                void set_range(double min, double max) override;
                bool homing() const override;
                void set_homing(int order, HomingMode mode, double speed) override;
                int homing_order() const override;
                HomingMode homing_mode() const override;
                double homing_speed() const override;

                std::string& type_as_string(std::string& s);
                std::string& homing_mode_as_string(std::string& s);
                
        protected:
                void assert_field(nlohmann::json data, const std::string& name);
        };
}

#endif // ROMI_AXIS_H

