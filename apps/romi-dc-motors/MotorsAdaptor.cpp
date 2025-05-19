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
#include <iostream>
#include <util/Logger.h>
#include <rpc/MethodsActivity.h>
#include <rpc/MethodsNavigation.h>
#include <rpc/MethodsPowerDevice.h>
#include "MotorsAdaptor.h"

namespace romi {

        MotorsAdaptor::MotorsAdaptor(IMotorDriver& motors)
                : motors_(motors)
        {
        }

        void MotorsAdaptor::execute(const std::string& id,
                                 const std::string& method,
                                 nlohmann::json &params,
                                 rcom::MemBuffer& result,
                                 rcom::RPCError &error)
        {
                (void) id;
                (void) method;
                (void) params;
                (void) result;
                error.code = rcom::RPCError::kMethodNotFound;
                error.message = "Unknown method";
        }

        void MotorsAdaptor::execute(const std::string& id,
                                 const std::string& method,
                                 nlohmann::json& params,
                                 nlohmann::json& result,
                                 rcom::RPCError &error)
        {
                r_debug("MotorsAdaptor::execute");

                (void) id;
                error.code = 0;
                
                try {

                        if (method.empty()) {
                                error.code = rcom::RPCError::kMethodNotFound;
                                error.message = "No method specified";
                                
                        } else if (method == MethodsNavigation::kMoveAt) {
                                execute_moveat(params, error);
                                 
                        } else if (method == MethodsNavigation::kMove) {
                                execute_move(params, error);
                                
                        } else if (method == MethodsNavigation::kStop) {
                                execute_stop(error);
                                
                        } else if (method == MethodsPowerDevice::kPowerUp) {
                                execute_power_up(error);
                                
                        } else if (method == MethodsPowerDevice::kPowerDown) {
                                execute_power_down(error);
                                
                        } else if (method == MethodsPowerDevice::kIsPoweredUp) {
                                execute_is_powered_up(result, error);
                                
                        } else {
                                r_err("MotorsAdaptor::execute: method not found: %s",
                                      method.c_str());
                                error.code = rcom::RPCError::kMethodNotFound;
                                error.message = "Unknown method";
                        }

                } catch (std::exception &e) {
                        error.code = rcom::RPCError::kInternalError;
                        error.message = e.what();
                }
        }

        void MotorsAdaptor::execute_moveat(nlohmann::json& params, rcom::RPCError &error)
        {
                r_debug("MotorsAdaptor::execute_moveat");

                // if (!params.contains(MethodsCNC::kMoveXParam)
                //     && !params.contains(MethodsCNC::kMoveYParam)
                //     && !params.contains(MethodsCNC::kMoveZParam)) {
                //         r_err("MotorsAdaptor::execute_moveto failed: missing parameters");
                //         error.code = rcom::RPCError::kInvalidParams;
                //         error.message = "missing x, y, or z parameters";
                        
                // } else {
                //         double x = params.value(MethodsCNC::kMoveXParam, ICNC::UNCHANGED);
                //         double y = params.value(MethodsCNC::kMoveYParam, ICNC::UNCHANGED);
                //         double z = params.value(MethodsCNC::kMoveZParam, ICNC::UNCHANGED);
                //         double v = params.value(MethodsCNC::kSpeedParam, 0.2);
                //         bool sync = params.value(MethodsCNC::kSyncParam, true);
                        
                //         r_debug("MotorsAdaptor::execute_moveto: %f, %f, %f", x, y, z);
                                
                //         if (!cnc_.moveto(x, y, z, v, sync)) {
                //                 error.code = 1;
                //                 error.message = "moveto failed";
                //         }
                // }
        }

        void MotorsAdaptor::execute_move(nlohmann::json& params, rcom::RPCError &error)
        {
                r_debug("MotorsAdaptor::execute_move");
                
                // try {
                //         double speed = params[MethodsCNC::kSpeedParam];

                //         if (!cnc_.spindle(speed)) {
                //                 error.code = 1;
                //                 error.message = "spindle failed";
                //         }

                // } catch (nlohmann::json::exception & je) {
                //         r_err("MotorsAdaptor::execute_move failed: %s", je.what());
                //         error.code = rcom::RPCError::kInvalidParams;
                //         error.message = je.what();
                // }
        }
        
        void MotorsAdaptor::execute_stop(rcom::RPCError &error)
        {
                r_debug("MotorsAdaptor::execute_stop");
                if (!motors_.stop()) {
                        error.code = 1;
                        error.message = "stop failed";
                }
        }

        void MotorsAdaptor::execute_power_up(rcom::RPCError &error)
        {
                r_debug("MotorsAdaptor::power_up");
                if (!motors_.power_up()) {
                        error.code = 1;
                        error.message = "power up failed";
                }
        }
        
        void MotorsAdaptor::execute_power_down(rcom::RPCError &error)
        {
                r_debug("MotorsAdaptor::power_down");
                if (!motors_.power_down()) {
                        error.code = 1;
                        error.message = "power down failed";
                }
        }
        
        void MotorsAdaptor::execute_is_powered_up(nlohmann::json& result,
                                               rcom::RPCError &error)
        {
                r_debug("MotorsAdaptor::is_powered_up");
                result = {{MethodsPowerDevice::kPoweredUp, motors_.is_powered_up()}};
        }
}

