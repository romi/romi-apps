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

#include <mutex>
#include "IIMU.h"
#include "RTIMULib.h"

namespace romi {
	
        class IMURT : public IIMU 
        {
	protected:
                std::mutex mutex_;
                RTIMUSettings *settings_;
                RTIMU *imu_;
                RTIMU_DATA measurements_;

        public:
		IMURT();
                virtual ~IMURT() override;

                double get_preferred_update_interval() override;
                bool update() override;
                double get_timestamp() override;
		vector_t get_acceleration() override;
		vector_t get_angular_velocity() override;
		vector_t get_magnetic_field() override;
		quaternion_t get_orientation() override;
        };
}
