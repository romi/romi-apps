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

#include <math.h>
#include "Ambit.h"

namespace romi {
        
        Ambit::Ambit(const std::string &serial_port)
        {
        }
        
        Ambit::~Ambit()
        {
        }
                
        void Ambit::measure(double intensity, size_t length,
                            double frequency,
                            std::vector<double>& measurements)
        {
                for (size_t i = 0; i < length; i++) {
                        // Just to send something back
                        double x = (double) i / (double) length;
                        double phase = 2.0 * M_PI * frequency * x;
                        double signal = intensity * sin(phase);
                        measurements.push_back(signal);
                }
        }
}
