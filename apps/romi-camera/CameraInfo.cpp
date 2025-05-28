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
#include "CameraInfo.h"

namespace romi {
        
        CameraInfo::CameraInfo(const std::string& id,
                               const std::string& type,
                               std::unique_ptr<ICameraSettings>& settings,
                               std::unique_ptr<ICameraDistortion>& distortion)
                : id_(id),
                  type_(type),
                  name_("unspecified"),
                  lens_(),
                  sensor_resolution_(),
                  sensor_dimensions_(),
                  calibration_date_(),
                  calibration_person_(),
                  calibration_method_(),
                  intrinsics_(),
                  settings_(),
                  distortion_()
        {
                settings_ = std::move(settings);
                distortion_ = std::move(distortion);
        }
        
        std::string& CameraInfo::get_id()
        {
                return id_;
        }
        
        std::string& CameraInfo::get_type()
        {
                return type_;
        }
        
        std::string& CameraInfo::get_name()
        {
                return name_;
        }
        
        void CameraInfo::set_name(const std::string& value)
        {
                 name_ = value;
        }
        
        std::string& CameraInfo::get_lens()
        {
                return lens_;
        }
        
        void CameraInfo::set_lens(const std::string& value)
        {
                lens_ = value;
        }
        
        SensorResolution& CameraInfo::get_sensor_resolution()
        {
                return sensor_resolution_;
        }
        
        void CameraInfo::set_sensor_resolution(size_t rx, size_t ry)
        {
                assert_sensor_resolution(rx, ry);
                sensor_resolution_.set(rx, ry);
        }
        
        void CameraInfo::assert_sensor_resolution(size_t x, size_t y)
        {
                if (x > 100000 || y > 100000) {
                        r_err("CameraInfo: invalid sensor resolution");
                        throw std::runtime_error("CameraInfo: invalid sensor resolution");
                }
        }
        
        SensorDimensions& CameraInfo::get_sensor_dimensions()
        {
                return sensor_dimensions_;
        }
        
        void CameraInfo::set_sensor_dimensions(double dx, double dy)
        {
                assert_sensor_dimensions(dx, dy);
                sensor_dimensions_.set(dx, dy);
        }
        
        void CameraInfo::assert_sensor_dimensions(double x, double y)
        {
                if (x < 0.0 || x > 100000.0 || y < 0.0 || y > 100000.0) {
                        r_err("CameraInfo: invalid sensor dimensions");
                        throw std::runtime_error("CameraInfo: invalid sensor dimensions");
                }
        }
        
        std::string& CameraInfo::get_calibration_date()
        {
                return calibration_date_;
        }
        
        void CameraInfo::set_calibration_date(const std::string& value)
        {
                 calibration_date_ = value;
        }
        
        std::string& CameraInfo::get_calibration_person()
        {
                return calibration_person_;
        }
        
        void CameraInfo::set_calibration_person(const std::string& value)
        {
                calibration_person_ = value;
        }
        
        std::string& CameraInfo::get_calibration_method()
        {
                return calibration_method_;
        }
        
        void CameraInfo::set_calibration_method(const std::string& value)
        {
                 calibration_method_ = value;
        }        

        CameraIntrinsics& CameraInfo::get_intrinsics()
        {
                return intrinsics_;
        }
        
        ICameraSettings& CameraInfo::get_settings()
        {
                return *settings_;
        }
        
        ICameraDistortion& CameraInfo::get_distortion()
        {
                return *distortion_;
        }
}
