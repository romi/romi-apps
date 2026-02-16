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

#include <stdexcept>
#include "util/Logger.h"
#include "api/Axis.h"

namespace romi {

        Axis::Axis()
                : name_(), 
                  type_(kLinearAxis),
                  has_range_(true),
                  range_(0, 0),
                  homing_(false),
                  homing_order_(-1),
                  homing_mode_(kHomingContactAndBackup),
                  homing_speed_(0.1)
        {
        }

        Axis::Axis(size_t index, nlohmann::json data)
                : Axis()
        {
                init(index, data);
        }

        void Axis::assert_field(nlohmann::json& data, const std::string& name)
        {
                if (!data.contains(name)) {
                        r_err("Axis: missing field: %s", name.c_str());
                        throw std::runtime_error("Axis: missing field");
                }
        }

        void Axis::init(size_t index, nlohmann::json data)
        {
                assert_field(data, "type");

                if (data.contains("name")) {
                        std::string s = data["name"];
                        // FIXME: validate s
                        name_ = s;
                } else {
                        switch (index) {
                        case 0:
                                name_ = "x";
                                break;
                        case 1:
                                name_ = "y";
                                break;
                        case 2:
                                name_ = "z";
                                break;
                        default:
                                r_err("Axis: invalid index: %d", index);
                                throw std::runtime_error("Axis: invalid index");
                        }
                }
                
                std::string type_s = data["type"];
                if (type_s == "linear") {
                        type_ = kLinearAxis;
                } else if (type_s == "angular") {
                        type_ = kAngularAxis;
                } else if (type_s == "unused") {
                        type_ = kUnusedAxis;
                } else {
                        r_err("Axis: invalid type: %s", type_s.c_str());
                        throw std::runtime_error("Axis: invalid type");
                }

                if (data.contains("range")) {
                        nlohmann::json range = data["range"];
                        if (!range.is_array()) {
                                r_err("Axis: range not an array");
                                throw std::runtime_error("Axis: range not an array");
                        }
                        if (range.size() != 2) {
                                r_err("Axis: range has invalid array size");
                                throw std::runtime_error("Axis: range has invalid "
                                                         "array size");
                        }
                        has_range_ = true;
                        range_.min_ = data["range"][0];
                        range_.max_ = data["range"][1];
                } else {
                        has_range_ = false;
                }
                
                if (data.contains("homing")) {
                        nlohmann::json homing = data["homing"];
                        assert_field(homing, "order");
                        assert_field(homing, "mode");
                        assert_field(homing, "speed");

                        homing_ = true;
                        homing_order_ = homing["order"];
                        homing_speed_ = homing["speed"];
                        std::string s = homing["mode"];
                        if (s == "contact-and-backup") {
                                homing_mode_ = kHomingContactAndBackup;
                        } else if (s == "contact") {
                                homing_mode_ = kHomingWithContact;
                        } else {
                                r_err("Axis: invalid homing mode");
                                throw std::runtime_error("Axis: invalid homing mode");
                        }
                        
                } else {
                        homing_ = false;
                }
        }

        void Axis::to_json(nlohmann::json& obj)
        {
                std::string s;
                
                obj["name"] = name();
                obj["type"] = type_as_string(s);
                
                if (homing()) {
                        obj["homing"] = nlohmann::json::object({
                                        { "order", homing_order()},
                                        { "mode", homing_mode_as_string(s)},
                                        { "speed", homing_speed()} });
                }

                if (has_range()) {
                        obj["range"] = nlohmann::json::array({range_.min_, range_.max_});
                }
        }
        
        const std::string& Axis::name() const
        {
                return name_;
        }
        
        void Axis::set_name(const std::string& s)
        {
                name_ = s;
        }
        
        AxisType Axis::type() const
        {
                return type_;
        }

        std::string& Axis::type_as_string(std::string& s)
        {
                switch (type()) {
                default:
                case kUnusedAxis:
                        s = "unused";
                        break;
                case kLinearAxis:
                        s = "linear";
                        break;
                case kAngularAxis:
                        s = "angular";
                        break;
                }
                return s;
        }
                                                                
        void Axis::set_type(AxisType type)
        {
                type_ = type;
        }
        
        bool Axis::has_range() const
        {
                return has_range_;
        }
        
        AxisRange Axis::get_range() const
        {
                return range_;
        }
        
        void Axis::set_range(double min, double max)
        {
                range_.min_ = min;
                range_.max_ = max;
        }
        
        bool Axis::homing() const
        {
                return homing_;
        }
        
        void Axis::set_homing(int order, HomingMode mode, double speed)
        {
                homing_ = true;
                homing_order_ = order;
                homing_mode_ = mode;
                homing_speed_ = speed;
        }
        
        int Axis::homing_order() const
        {
                return homing_order_;
        }
        
        HomingMode Axis::homing_mode() const
        {
                return homing_mode_;
        }

        std::string& Axis::homing_mode_as_string(std::string& s)
        {
                switch (homing_mode()) {
                default:
                case kHomingWithContact:
                        s = "contact";
                        break;
                case kHomingContactAndBackup:
                        s = "contact-and-backup";
                        break;
                }
                return s;
        }

        double Axis::homing_speed() const
        {
                return homing_speed_;
        }
}
