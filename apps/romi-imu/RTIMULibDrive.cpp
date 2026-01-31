////////////////////////////////////////////////////////////////////////////
//
//  This file is part of RTIMULib
//
//  Copyright (c) 2014-2015, richards-tech, LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
//  Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include <stdexcept>
#include <util/ClockAccessor.h>
#include <util/Logger.h>
#include "IIMU.h"
#include "RTIMULib.h"

namespace romi {
	
        class IMURT : public IIMU 
        {
	protected:
                RTIMUSettings *settings_;
                RTIMU *imu_;
                RTIMU_DATA measurements_;

        public:
		IMURT();
                virtual ~IMURT() override;

                double get_preferred_update_interval() override;
                void update() override;
                double get_timestamp() override;
		vector_t get_acceleration() override;
		vector_t get_angular_velocity() override;
		vector_t get_magnetic_field() override;
		quaternion_t get_orientation() override;

	protected:

        };


        IMURT::IMURT()
                : settings_(nullptr),
                  imu_(nullptr)
        {
                settings_ = new RTIMUSettings();
                settings_->m_imuType = 6; // STM LSM9DS1
                settings_->m_fusionType = 2; // RTQF
                settings_->m_busIsI2C = 1;
                settings_->m_I2CBus = 1;
                settings_->m_SPIBus = 0;
                settings_->m_SPISelect = 0;
                settings_->m_SPISpeed = 500000;
                settings_->m_I2CSlaveAddress = 107;
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

                imu_->IMUInit();

                //  this is a convenient place to change fusion parameters
                
                imu_->setSlerpPower(0.02f);
                imu_->setGyroEnable(true);
                imu_->setAccelEnable(true);
                imu_->setCompassEnable(true);
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

        void IMURT::update()
        {
                if (!imu_->IMURead()) {
                        r_err("IMURT::update: Failed");
                        throw std::runtime_error("IMURT::update: Failed");
                }                        
                measurements_ = imu_->getIMUData();
        }

        double IMURT::get_timestamp() 
        {
                return (double) measurements_.timestamp / 1000000.0;
        }
        
        vector_t IMURT::get_acceleration()
        {
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
                
                
int main()
{
        try {
                std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
                romi::ClockAccessor::SetInstance(clock);
                romi::IMURT imu;
                double interval = imu.get_preferred_update_interval();
        
                while (1) {
                        romi::ClockAccessor::GetInstance()->sleep(interval);
                        imu.update();
                        quaternion_t q = imu.get_orientation();
                        vector_t angles = convert_quaternion_to_euler(q);
                        printf("%f - %f - %f\r", angles.x, angles.y, angles.z);
                }
                
        } catch (std::exception& e) {
                r_err("romi-imu: main.cpp: caught exception: %s", e.what());
                r_err("romi-imu: main.cpp: quitting");
        }
        return 0;
}

