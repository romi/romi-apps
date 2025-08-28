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
#include "MethodsBattery.h"
#include "BatteryAdaptor.h"

namespace romi {

        BatteryAdaptor::BatteryAdaptor(IBattery& battery)
                : battery_(battery)
        {
        }

        void BatteryAdaptor::execute(const std::string&,
                                            const std::string&,
                                            nlohmann::json& result,
                                            rcom::MemBuffer&,
                                            rcom::RPCError& error)
        {
                r_debug("BatteryAdaptor::execute (binary)");
                result.clear();
                error.code = rcom::RPCError::kMethodNotFound;
                error.message = "Unknown method";
        }

        void BatteryAdaptor::execute(const std::string&,
                                    const std::string& method,
                                    nlohmann::json& params,
                                    nlohmann::json& result,
                                    rcom::RPCError& error)
        {
                error.code = 0;
                r_debug("BatteryAdaptor::execute (text)");
                                
                try {
                        if (method == MethodsBattery::kIsCharging) {
                                execute_is_charging(result, error);
                                
                        } else if (method == MethodsBattery::kGetVoltage) {
                                execute_get_voltage(result, error);
                                
                        } else if (method == MethodsBattery::kGetCurrent) {
                                execute_get_current(result, error);
                                
                        } else if (method == MethodsBattery::kGetLevel) {
                                execute_get_current(result, error);
                                
                        } else {
                                error.code = rcom::RPCError::kMethodNotFound;
                                error.message = "Unknown method";
                        }
                        
                } catch (std::exception& e) {
                        error.code = rcom::RPCError::kInternalError;
                        error.message = e.what();
                }
        }

        void BatteryAdaptor::execute_is_charging(nlohmann::json& result,
                                                        rcom::RPCError& error)
        {
                result = {{MethodsBattery::kCharging, battery_.is_charging()}};
        }

        void BatteryAdaptor::execute_get_voltage(nlohmann::json& result,
                                                        rcom::RPCError& error)
        {
                result = {{MethodsBattery::kVoltage, battery_.get_voltage()}};
        }

        void BatteryAdaptor::execute_get_current(nlohmann::json& result,
                                                        rcom::RPCError& error)
        {
                result = {{MethodsBattery::kCurrent, battery_.get_current()}};
        }

        void BatteryAdaptor::execute_get_level(nlohmann::json& result,
                                               rcom::RPCError& error)
        {
                result = {{MethodsBattery::kLevel, battery_.get_level()}};
        }
}
