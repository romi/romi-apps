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
#include <util/Logger.h>
#include "CNCSettings.h"

namespace romi {
        
        CNCSettings::CNCSettings(Axis *axis,
                                     const double *vmax,
                                     const double *amax,
                                     const double *scale_meters_to_steps, 
                                     double path_max_deviation,
                                     double path_slice_duration,
                                     double path_max_slice_duration)
                : vmax_(vmax),
                  amax_(amax),
                  path_max_deviation_(path_max_deviation),
                  path_slice_duration_(path_slice_duration),
                  path_max_slice_duration_(path_max_slice_duration),
                  range_()
        {
                axis_[0] = axis[0];
                axis_[1] = axis[1];
                axis_[2] = axis[2];
                
                // 32 seconds = 32000 ms < 2^16/2, which is the
                // maximum value in the int16_t used to send block
                // moves.
                if (path_max_slice_duration_ > 32.0)
                        path_max_slice_duration_ = 32.0;
                
                scale_meters_to_steps_ = scale_meters_to_steps;
                
                init_range();
                init_homing_axes();
                init_homing_speeds();
        }

        void CNCSettings::init_range()
        {
                v3 min;
                v3 max;
                AxisRange r;

                r = axis_[0].get_range();
                min.x(r.min_);
                max.x(r.max_);
                
                r = axis_[1].get_range();
                min.y(r.min_);
                max.y(r.max_);
                
                r = axis_[2].get_range();
                min.z(r.min_);
                max.z(r.max_);

                range_.init(min, max);
        }
        
        void CNCSettings::init_homing_axes()
        {
                for (int index = 0; index < 3; index++) {
                        homing_axes_[index] = kNoAxis;
                }

                for (int index = 0; index < 3; index++) {
                        if (axis_[index].homing()) {
                                int order = axis_[index].homing_order();
                                if (homing_axes_[order] != kNoAxis) {
                                        r_err("CNCSettings: invalid homing order"
                                              ": axis %s", axis_[index].name().c_str());
                                        throw std::runtime_error("CNCSettings: "
                                                                 "invalid homing order");
                                }

                                switch (index) {
                                case 0:
                                        homing_axes_[order] = kAxisX;
                                        break;
                                case 1:
                                        homing_axes_[order] = kAxisY;
                                        break;
                                case 2:
                                        homing_axes_[order] = kAxisZ;
                                        break;
                                default: // to satisfy the compiler
                                        break; 
                                }
                        }
                }
        }

        void CNCSettings::init_homing_speeds()
        {
                double speeds[3];
                for (int i = 0; i < 3; i++) {
                        speeds[i] = (vmax_[i]
                                     * scale_meters_to_steps_[i]
                                     * axis_[i].homing_speed());
                        r_debug("CNC:: vmax[%d]=%f", i, vmax_[i]);
                        r_debug("CNC:: scale[%d]=%f", i, scale_meters_to_steps_[i]);
                        r_debug("CNC:: homing speed[%d]=%f", i, speeds[i]);
                }
                
                for (int i = 0; i < 3; i++) {
                        homing_speeds_[i] = 0;
                }
                
                for (int i = 0; i < 3; i++) {
                        AxisIndex axis = homing_axes_[i];
                        if (axis >= 0) {
                                homing_speeds_[i] = (int16_t) speeds[axis];
                        }
                }
        }
}
