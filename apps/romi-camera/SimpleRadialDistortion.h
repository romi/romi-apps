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

#ifndef ROMI_SIMPLERADIALDISTORTION_H
#define ROMI_SIMPLERADIALDISTORTION_H

#include "ICameraDistortion.h"

namespace romi {
        
        class SimpleRadialDistortion : public ICameraDistortion
        {
        public:
                
                static constexpr const char *kK1 = "radial-k1";
                static constexpr const char *kK2 = "radial-k2";
                
        protected:
                std::string type_;
                double k1_;
                double k2_;
                
        public:
                SimpleRadialDistortion();
                SimpleRadialDistortion(double k1, double k2);
                SimpleRadialDistortion(std::vector<double>& values);
                virtual ~SimpleRadialDistortion() override = default;

                std::string& get_type() override;
                void get(std::vector<double>& values) override;
                void set(std::vector<double>& values) override;
                double get(const std::string& name) override;
                void set(const std::string& name, double value) override;

                double k1();
                double k2();
        };
}

#endif // ROMI_SIMPLERADIALDISTORTION_H
