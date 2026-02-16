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

#include "IIMU.h"
#include <util/ClockAccessor.h>

namespace romi {
	
        class FakeIMU : public IIMU 
        {
	protected:
		vector_t acceleration_;
		vector_t angular_velocity_;
		vector_t magnetic_field_;
		quaternion_t orientation_;

        public:
		FakeIMU() 
                        : acceleration_(),
                          angular_velocity_(),
                          magnetic_field_(),
                          orientation_() {
                }
                
                ~FakeIMU() override = default;

                double get_preferred_update_interval() override {
                        return 0.020;
                }
                
                bool update() override {
                        return true;
                }
                
                double get_timestamp() override {
                        return ClockAccessor::GetInstance()->time();
                }
                
		vector_t get_acceleration() override {
                        return acceleration_;
                }
                        
		vector_t get_angular_velocity() override {
                        return angular_velocity_;
                }
                
		vector_t get_magnetic_field() override {
                        return magnetic_field_;
                }
                
		quaternion_t get_orientation() override {
                        return orientation_;
                }
        };
}
