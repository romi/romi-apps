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

#ifndef ROMI_CAMERAINFO_H
#define ROMI_CAMERAINFO_H

#include "api/ICameraSettings.h"
#include "CameraIntrinsics.h"
#include "ICameraDistortion.h"

namespace romi {

        class SensorResolution
        {
        public:
                size_t columns_;
                size_t rows_;
                
                SensorResolution()
                        : columns_(0),
                          rows_(0) {
                }
                
                ~SensorResolution() {}

                size_t columns() {
                        return columns_;
                }

                size_t rows() {
                        return rows_;
                }

                void set(size_t cols, size_t rows) {
                        columns_ = cols;
                        rows_ = rows;
                }
        };

        class SensorDimensions
        {
        public:
                double width_;
                double height_;
                
                SensorDimensions()
                        : width_(0),
                          height_(0) {
                }
                
                ~SensorDimensions() {}

                double width() {
                        return width_;
                }

                double height() {
                        return height_;
                }

                void set(double width, double height) {
                        width_ = width;
                        height_ = height;
                }
        };
                        
        class CameraInfo
        {
        protected:
                std::string id_;
                std::string type_;
                std::string name_;
                std::string lens_;
                SensorResolution sensor_resolution_;
                SensorDimensions sensor_dimensions_;
                std::string calibration_date_;
                std::string calibration_person_;
                std::string calibration_method_;
                
                CameraIntrinsics intrinsics_;
                std::unique_ptr<ICameraSettings> settings_;
                std::unique_ptr<ICameraDistortion> distortion_;
                
        public:
                CameraInfo(const std::string& id,
                           const std::string& type,
                           std::unique_ptr<ICameraSettings>& settings,
                           std::unique_ptr<ICameraDistortion>& distortion);
                ~CameraInfo() = default;

                std::string& get_id();
                std::string& get_type();
                
                std::string& get_name();
                void set_name(const std::string& value);
                
                std::string& get_lens();
                void set_lens(const std::string& value);
                
                SensorResolution& get_sensor_resolution();
                void set_sensor_resolution(size_t rx, size_t ry);
                
                SensorDimensions& get_sensor_dimensions();
                void set_sensor_dimensions(double dx, double dy);
                
                std::string& get_calibration_date();
                void set_calibration_date(const std::string& value);
                
                std::string& get_calibration_person();
                void set_calibration_person(const std::string& value);
                
                std::string& get_calibration_method();
                void set_calibration_method(const std::string& value);

                CameraIntrinsics& get_intrinsics();
                ICameraSettings& get_settings();
                ICameraDistortion& get_distortion();
                
        protected:
                void assert_sensor_dimensions(double x, double y);
                void assert_sensor_resolution(size_t x, size_t y);
        };
}

#endif // ROMI_CAMERAINFO_H
