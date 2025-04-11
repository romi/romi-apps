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

#ifndef ROMI_CAMERAINTRINSICS_H
#define ROMI_CAMERAINTRINSICS_H

#include "ICameraIntrinsics.h"

namespace romi {
        
        class CameraIntrinsics : public ICameraIntrinsics
        {
        protected:
                double focal_length_x_;
                double focal_length_y_;
                double central_point_x_;
                double central_point_y_;
                
        public:
                CameraIntrinsics();
                CameraIntrinsics(double fx, double fy, double cx, double cy);
                ~CameraIntrinsics() override = default;

                void get_focal_length(double& fx, double& fy) override;
                void set_focal_length(double fx, double fy) override;                
                void get_central_point(double& cx, double& cy) override;
                void set_central_point(double cx, double cy) override;
                
        protected:
                void assert_values();
        };
}

#endif // ROMI_ICAMERAINTRINSICS_H
