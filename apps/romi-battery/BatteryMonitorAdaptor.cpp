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
#include "MethodsBatteryMonitor.h"
#include "BatteryMonitorAdaptor.h"

namespace romi {

        BatteryMonitorAdaptor::BatteryMonitorAdaptor(IBatteryMonitor& monitor)
                : monitor_(monitor)
        {
        }

        void BatteryMonitorAdaptor::execute(const std::string&,
                                            const std::string&,
                                            nlohmann::json& result,
                                            rcom::MemBuffer&,
                                            rcom::RPCError& error)
        {
                r_debug("BatteryMonitorAdaptor::execute (binary)");
                result.clear();
                error.code = rcom::RPCError::kMethodNotFound;
                error.message = "Unknown method";
        }

        void BatteryMonitorAdaptor::execute(const std::string&,
                                    const std::string& method,
                                    nlohmann::json& params,
                                    nlohmann::json& result,
                                    rcom::RPCError& error)
        {
                error.code = 0;
                r_debug("BatteryMonitorAdaptor::execute (text)");
                                
                try {
                        if (method == MethodsBatteryMonitor::kIsCharging) {
                                execute_is_charging(result, error);
                                
                        } else if (method == MethodsBatteryMonitor::kGetVoltage) {
                                execute_get_voltage(result, error);
                                
                        } else if (method == MethodsBatteryMonitor::kGetCurrent) {
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

        void BatteryMonitorAdaptor::execute_is_charging(nlohmann::json& result,
                                                        rcom::RPCError& error)
        {
                result = {{MethodsBatteryMonitor::kCharging, monitor_.is_charging()}};
        }

        void BatteryMonitorAdaptor::execute_get_voltage(nlohmann::json& result,
                                                        rcom::RPCError& error)
        {
                result = {{MethodsBatteryMonitor::kVoltage, monitor_.get_voltage()}};
        }

        void BatteryMonitorAdaptor::execute_get_current(nlohmann::json& result,
                                                        rcom::RPCError& error)
        {
                result = {{MethodsBatteryMonitor::kCurrent, monitor_.get_current()}};
        }
}
