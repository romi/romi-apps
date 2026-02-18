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
#include "IMURT.h"

namespace romi {

        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;

        IMURT::IMURT()
                : mutex_(),
                  settings_(nullptr),
                  imu_(nullptr)
        {
                r_debug("IMURT::IMURT: starting initialisation");
                settings_ = new RTIMUSettings();
                settings_->setDefaults();
                
                settings_->m_imuType = 6; // STM LSM9DS1
                settings_->m_fusionType = 2; // RTQF
                settings_->m_busIsI2C = 1;
                settings_->m_I2CBus = 1;
                settings_->m_SPIBus = 0;
                settings_->m_SPISelect = 0;
                settings_->m_SPISpeed = 500000;
                settings_->m_I2CSlaveAddress = 107; // 0x6b
                settings_->m_axisRotation = 0;
                settings_->m_pressureType = 0;
                settings_->m_I2CPressureAddress = 0;
                settings_->m_humidityType = 0;
                settings_->m_I2CHumidityAddress = 0;

                // compass calibration and adjustment
                settings_->m_compassCalValid = true;
                settings_->m_compassCalMin.setX(-65.205940f);
                settings_->m_compassCalMin.setY(-43.796043f);
                settings_->m_compassCalMin.setZ(-31.085081f);
                settings_->m_compassCalMax.setX(39.973198f);
                settings_->m_compassCalMax.setY(63.016579f);
                settings_->m_compassCalMax.setZ(72.963326f);
	    
                settings_->m_compassAdjDeclination = 0.0f;

                // compass ellipsoid calibration
                settings_->m_compassCalEllipsoidValid = true;
                settings_->m_compassCalEllipsoidOffset.setX(2.568609f);
                settings_->m_compassCalEllipsoidOffset.setY(0.919799f);
                settings_->m_compassCalEllipsoidOffset.setZ(0.296187f);
                settings_->m_compassCalEllipsoidCorr[0][0] = 0.944992f;
                settings_->m_compassCalEllipsoidCorr[0][1] = -0.025820f;
                settings_->m_compassCalEllipsoidCorr[0][2] = 0.012700f;
                settings_->m_compassCalEllipsoidCorr[1][0] = -0.025820f;
                settings_->m_compassCalEllipsoidCorr[1][1] = 0.929308f;
                settings_->m_compassCalEllipsoidCorr[1][2] = 0.009464f;
                settings_->m_compassCalEllipsoidCorr[2][0] = 0.012700f;
                settings_->m_compassCalEllipsoidCorr[2][1] = 0.009464f;
                settings_->m_compassCalEllipsoidCorr[2][2] = 0.996858f;

                imu_ = RTIMU::createIMU(settings_);

                if ((imu_ == NULL) || (imu_->IMUType() == RTIMU_TYPE_NULL)) {
                        r_err("IMURT: No IMU found");
			throw std::runtime_error("IMURT: No IMU found");
                }

                //  This is an opportunity to manually override any
                //  settings before the call IMUInit
                
                //  set up IMU

                r_debug("IMURT::IMURT: calling IMUInit");
                
                imu_->IMUInit();

                r_debug("IMURT::IMURT: calling IMUInit: done");

                //  this is a convenient place to change fusion parameters

                
                imu_->setSlerpPower(0.02f);
                imu_->setGyroEnable(true);
                imu_->setAccelEnable(true);
                imu_->setCompassEnable(true);
                
                r_debug("IMURT::IMURT: initialisation done");
        }

        IMURT::~IMURT()
        {
                delete settings_;
                delete imu_;
        }

        double IMURT::get_preferred_update_interval()
        {
                return (double) imu_->IMUGetPollInterval() / 1000.0;
        }

        bool IMURT::update()
        {
                SynchronizedCodeBlock synchronize(mutex_);
                bool success = false;
                if (imu_->IMURead()) {
                        measurements_ = imu_->getIMUData();
                        success = true;
                }
                return success;
        }

        double IMURT::get_timestamp() 
        {
                SynchronizedCodeBlock synchronize(mutex_);
                return (double) measurements_.timestamp / 1000000.0;
        }
        
        vector_t IMURT::get_acceleration()
        {
                SynchronizedCodeBlock synchronize(mutex_);
                if (!measurements_.accelValid) {
                        r_err("IMURT::get_acceleration: Not configured");
                        throw std::runtime_error("IMURT::get_acceleration: Not configured");
                }
                vector_t r(measurements_.accel.x(),
                           measurements_.accel.y(),
                           measurements_.accel.z());
                return r;
        }
        
        vector_t IMURT::get_angular_velocity()
        {
                SynchronizedCodeBlock synchronize(mutex_);
                if (!measurements_.gyroValid) {
                        r_err("IMURT::get_angular_velocity: Not configured");
                        throw std::runtime_error("IMURT::get_angular_velocity: Not configured");
                }
                vector_t r(measurements_.gyro.x(),
                           measurements_.gyro.y(),
                           measurements_.gyro.z());
                return r;
        }
        
        vector_t IMURT::get_magnetic_field()
        {
                SynchronizedCodeBlock synchronize(mutex_);
                if (!measurements_.compassValid) {
                        r_err("IMURT::get_magnetic_field: Not configured");
                        throw std::runtime_error("IMURT::get_magnetic_field: Not configured");
                }
                vector_t r(measurements_.compass.x(),
                           measurements_.compass.y(),
                           measurements_.compass.z());
                return r;
        }
        
        quaternion_t IMURT::get_orientation()
        {
                SynchronizedCodeBlock synchronize(mutex_);
                if (!measurements_.fusionQPoseValid) {
                        r_err("IMURT::get_orientation: Not configured");
                        throw std::runtime_error("IMURT::get_orientation: Not configured");
                }
                quaternion_t r = quaternion(measurements_.fusionQPose.scalar(),
                                            measurements_.fusionQPose.x(),
                                            measurements_.fusionQPose.y(),
                                            measurements_.fusionQPose.z());
                return r;
        }

}
