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
#include "MethodsFluoSensor.h"
#include "FluoSensorAdaptor.h"

namespace romi {

        FluoSensorAdaptor::FluoSensorAdaptor(IFluoSensor& sensor)
                : sensor_(sensor)
        {
        }

        void FluoSensorAdaptor::execute(const std::string&,
                                        const std::string&,
                                        nlohmann::json& result,
                                        rcom::MemBuffer&,
                                        rcom::RPCError& error)
        {
                result.clear();
                error.code = rcom::RPCError::kMethodNotFound;
                error.message = "Unknown method";
        }

        void FluoSensorAdaptor::execute(const std::string&,
                                    const std::string& method,
                                    nlohmann::json& params,
                                    nlohmann::json& result,
                                    rcom::RPCError& error)
        {
                error.code = 0;
                                
                try {
                        if (method == MethodsFluoSensor::kMeasure) {
                                execute_measure(params, result, error);
                                
                        } else {
                                error.code = rcom::RPCError::kMethodNotFound;
                                error.message = "Unknown method";
                        }
                        
                } catch (std::exception& e) {
                        error.code = rcom::RPCError::kInternalError;
                        error.message = e.what();
                }
        }

        void FluoSensorAdaptor::execute_measure(nlohmann::json& params,
                                                nlohmann::json& result,
                                                rcom::RPCError& error)
        {
                double intensity = params[MethodsFluoSensor::kIntensity];
                size_t length = params[MethodsFluoSensor::kLength];
                double frequency = params[MethodsFluoSensor::kFrequency];
                std::vector<double> measurements;

                sensor_.measure(intensity, length, frequency, measurements);
                result = {{MethodsFluoSensor::kMeasurements, measurements}};
        }
}
