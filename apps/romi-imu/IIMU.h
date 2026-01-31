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
#pragma once

#include "quaternion.h"

namespace romi {
	
        class IIMU 
        {
        public:
                virtual ~IIMU() = default;

                // The update interval recommended by the IMU, in
                // seconds.
                virtual double get_preferred_update_interval() = 0;

                // Renew the measurements.
                virtual void update() = 0;

		// Timestamp in seconds since epoch
                virtual double get_timestamp() = 0;

		// Three axes of acceleration (gravity + linear
		// motion) in m/s²
                virtual vector_t get_acceleration() = 0;

		// Three axes of rotation speed in rad/s
                virtual vector_t get_angular_velocity() = 0;

		// Three axes of magnetic field sensing in micro Tesla
		// (uT)
                virtual vector_t get_magnetic_field() = 0;

		// Absolute orientation as a quaternion
                virtual quaternion_t get_orientation() = 0;
        };
}
