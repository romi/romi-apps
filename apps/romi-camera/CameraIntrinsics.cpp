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
#include "util/Logger.h"
#include "CameraIntrinsics.h"

namespace romi {
                
        CameraIntrinsics::CameraIntrinsics()
                : CameraIntrinsics(1.0, 1.0, 0.0, 0.0)
        {
        }
        
        CameraIntrinsics::CameraIntrinsics(double fx, double fy, double cx, double cy)
                : focal_length_x_(fx),
                  focal_length_y_(fy),
                  central_point_x_(cx),
                  central_point_y_(cy)
        {
                assert_values();
        }

        void CameraIntrinsics::get_focal_length(double& fx, double& fy)
        {
                fx = focal_length_x_;
                fy = focal_length_y_;
        }
        
        void CameraIntrinsics::set_focal_length(double fx, double fy)
        {
                focal_length_x_ = fx;
                focal_length_y_ = fy;
                assert_values();
        }
        
        void CameraIntrinsics::get_central_point(double& cx, double& cy)
        {
                cx = central_point_x_;
                cy = central_point_y_;
        }
        
        void CameraIntrinsics::set_central_point(double cx, double cy)
        {
                central_point_x_ = cx;
                central_point_y_ = cy;
                assert_values();
        }

        void CameraIntrinsics::assert_values()
        {
                if (focal_length_x_ <= 0.0 || focal_length_x_ > 1000000.0) {
                        r_err("CameraIntrinsics: Invalid fx");
                        throw std::runtime_error("CameraIntrinsics: Invalid fx");
                }
                if (focal_length_y_ <= 0.0 || focal_length_y_ > 1000000.0) {
                        r_err("CameraIntrinsics: Invalid fy");
                        throw std::runtime_error("CameraIntrinsics: Invalid fy");
                }
                if (central_point_x_ < 0.0 || central_point_x_ > 100000.0) {
                        r_err("CameraIntrinsics: Invalid cx");
                        throw std::runtime_error("CameraIntrinsics: Invalid cx");
                }
                if (central_point_y_ < 0.0 || central_point_y_ > 100000.0) {
                        r_err("CameraIntrinsics: Invalid cy");
                        throw std::runtime_error("CameraIntrinsics: Invalid cy");
                }
        }
}

