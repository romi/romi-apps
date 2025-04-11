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
#include <util/Logger.h>
#include "SimpleRadialDistortion.h"

namespace romi {
        
        SimpleRadialDistortion::SimpleRadialDistortion()
                : SimpleRadialDistortion(0.0, 0.0) 
        {
        }
        
        SimpleRadialDistortion::SimpleRadialDistortion(double k1, double k2)
                : type_(ICameraDistortion::kSimpleRadialDistortion),
                  k1_(k1),
                  k2_(k2)
        {
        }
        
        SimpleRadialDistortion::SimpleRadialDistortion(std::vector<double>& values)
                : SimpleRadialDistortion(0.0, 0.0)
        {
                set(values);
        }
        
        std::string& SimpleRadialDistortion::get_type()
        {
                return type_;
        }
        
        void SimpleRadialDistortion::get(std::vector<double>& values)
        {
                values.clear();
                values.resize(2);
                values[0] = k1_;
                values[1] = k2_;
        }
        
        void SimpleRadialDistortion::set(std::vector<double>& values)
        {
                if (values.size() != 2) {
                        r_err("SimpleRadialDistortion: wrong number of values: %d!=2",
                              values.size());
                        throw std::runtime_error("SimpleRadialDistortion: wrong number of values");
                }
                k1_ = values[0];
                k2_ = values[1];
        }
        
        double SimpleRadialDistortion::get(const std::string& name)
        {
                double result = 0.0;
                if (name == SimpleRadialDistortion::kK1) {
                        result = k1();
                } else if (name == SimpleRadialDistortion::kK2) {
                        result = k2();
                } else {
                        r_err("SimpleRadialDistortion::get: unknown parameter: %s",
                              name.c_str());
                        throw std::runtime_error("SimpleRadialDistortion::get: "
                                                 "unknown parameter");
                }
                return result;
        }
        
        void SimpleRadialDistortion::set(const std::string& name, double value)
        {
                if (name == SimpleRadialDistortion::kK1) {
                        k1_ = value;
                } else if (name == SimpleRadialDistortion::kK2) {
                        k2_ = value;
                } else {
                        r_err("SimpleRadialDistortion::set: unknown parameter: %s",
                              name.c_str());
                        throw std::runtime_error("SimpleRadialDistortion::set: "
                                                 "unknown parameter");
                }
        }

        double SimpleRadialDistortion::k1()
        {
                return k1_;
        }
        
        double SimpleRadialDistortion::k2()
        {
                return k2_;
        }
}
