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
#include "util/Logger.h"
#include "MethodsIMU.h"
#include "IMUAdaptor.h"

namespace romi {

        IMUAdaptor::IMUAdaptor(IIMU& imu)
                : imu_(imu)
        {
        }

        void IMUAdaptor::execute(const std::string&,
				 const std::string&,
				 nlohmann::json& result,
				 rcom::MemBuffer&,
				 rcom::RPCError& error)
        {
                result.clear();
                error.code = rcom::RPCError::kMethodNotFound;
                error.message = "Unknown method";
        }

        void IMUAdaptor::execute(const std::string&,
				 const std::string& method,
				 nlohmann::json& params,
				 nlohmann::json& result,
				 rcom::RPCError& error)
        {
                error.code = 0;
                                
                try {
                        if (method == MethodsIMU::kGetAcceleration) {
                                execute_get_acceleration(result, error);
                                
                        } else if (method == MethodsIMU::kGetAngularVelocity) {
                                execute_get_angular_velocity(result, error);
                                
                        } else if (method == MethodsIMU::kGetMagneticField) {
                                execute_get_magnetic_field(result, error);
                                
                        } else if (method == MethodsIMU::kGetOrientation) {
                                execute_get_orientation(result, error);
                                
                        } else {
                                error.code = rcom::RPCError::kMethodNotFound;
                                error.message = "Unknown method";
                        }
                        
                } catch (std::exception& e) {
                        error.code = rcom::RPCError::kInternalError;
                        error.message = e.what();
                }
        }

        void IMUAdaptor::execute_get_acceleration(nlohmann::json& // result
						  ,
						  rcom::RPCError& // error
		)
        {
                // vector_t r = imu_.get_acceleration();
                // result = {{MethodsIMU::kAcceleration, r}};
        }

        void IMUAdaptor::execute_get_angular_velocity(nlohmann::json& // result
						      ,
						      rcom::RPCError& // error
		)
        {
                // vector_t r = imu_.get_angular_velocity();
                //result = {{MethodsIMU::kAngularVelocity, r}};
        }

        void IMUAdaptor::execute_get_magnetic_field(nlohmann::json& // result
						    ,
						    rcom::RPCError& // error
		)
        {
                // vector_t r = imu_.get_magnetic_field();
                //result = {{MethodsIMU::kMagneticField, r}};
        }

        void IMUAdaptor::execute_get_orientation(nlohmann::json& // result
						 ,
						 rcom::RPCError& // error
		)
        {
                // quaternion_t r = imu_.get_orientation();
                //result = {{MethodsIMU::kOrientation, r}};
        }
}
