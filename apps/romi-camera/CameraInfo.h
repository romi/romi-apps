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

#include "ICameraInfo.h"

namespace romi {
        
        class CameraInfo : public ICameraInfo
        {
        protected:
                std::string id_;
                std::string type_;
                std::string name_;
                std::string lens_;
                std::pair<double,double> sensor_resolution_;
                std::pair<double,double> sensor_dimensions_;
                std::string calibration_date_;
                std::string calibration_person_;
                std::string calibration_method_;
                std::unique_ptr<ICameraIntrinsics> intrinsics_;
                std::unique_ptr<ICameraSettings> settings_;
                std::unique_ptr<ICameraDistortion> distortion_;
                
        public:
                CameraInfo(const std::string& id,
                           const std::string& type,
                           std::unique_ptr<ICameraIntrinsics>& intrinsics,
                           std::unique_ptr<ICameraSettings>& settings,
                           std::unique_ptr<ICameraDistortion>& distortion);
                ~CameraInfo() override = default;

                std::string& get_id() override;
                std::string& get_type() override;
                
                std::string& get_name() override;
                void set_name(const std::string& value) override;
                
                std::string& get_lens() override;
                void set_lens(const std::string& value) override;
                
                std::pair<double,double>& get_sensor_resolution() override;
                void set_sensor_resolution(double rx, double ry) override;
                
                std::pair<double,double>& get_sensor_dimensions() override;
                void set_sensor_dimensions(double dx, double dy) override;
                
                std::string& get_calibration_date() override;
                void set_calibration_date(const std::string& value) override;
                
                std::string& get_calibration_person() override;
                void set_calibration_person(const std::string& value) override;
                
                std::string& get_calibration_method() override;
                void set_calibration_method(const std::string& value) override;

                ICameraIntrinsics& get_intrinsics() override;
                ICameraSettings& get_settings() override;
                ICameraDistortion& get_distortion() override;
                
        protected:
                void assert_sensor_dimensions(double x, double y);
                void assert_sensor_resolution(double x, double y);
        };
}

#endif // ROMI_CAMERAINFO_H
