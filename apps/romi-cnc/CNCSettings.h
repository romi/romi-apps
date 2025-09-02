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
#ifndef ROMI_CNCSETTINGS_H
#define ROMI_CNCSETTINGS_H

#include <v3.h>
#include <api/CNCRange.h>
#include <api/Axis.h>
#include "ICNCController.h"

namespace romi {
        
        class CNCSettings
        {
        public:
                Axis axis_[3];
                v3 vmax_; // in m/s
                v3 amax_; // in m/s²
                // The maximum deviation allowed when computed a
                // continuous path, in m.
                double path_max_deviation_;
                v3 scale_meters_to_steps_;
                double path_slice_duration_;
                double path_max_slice_duration_;
                
                CNCRange range_;
                AxisIndex homing_axes_[3];
                int16_t homing_speeds_[3];
                
                CNCSettings(Axis *axis,
                              const double *vmax,
                              const double *amax,
                              const double *scale_meters_to_steps, 
                              double path_max_deviation,
                              double path_slice_duration,
                              double path_max_slice_duration);


                void init_range();
                void init_homing_axes();
                void init_homing_speeds();
        };
}

#endif // ROMI_CNCSETTINGS_H
