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

#ifndef ROMI_ICAMERAINTRINSICS_H
#define ROMI_ICAMERAINTRINSICS_H

namespace romi {
        
        class ICameraIntrinsics
        {
        public:
               
                virtual ~ICameraIntrinsics() = default;

                virtual void get_focal_length(double& fx, double& fy) = 0;
                virtual void set_focal_length(double fx, double fy) = 0;                
                virtual void get_central_point(double& cx, double& cy) = 0;
                virtual void set_central_point(double cx, double cy) = 0;
        };
}

#endif // ROMI_ICAMERAINTRINSICS_H
