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
#include <rpc/MethodsDataLog.h>
#include "DataLogAdaptor.h"

namespace romi {

        DataLogAdaptor::DataLogAdaptor(IDataLog& datalog)
                : datalog_(datalog)
        {
        }

        void DataLogAdaptor::execute(const std::string&,
                                     const std::string&,
                                     nlohmann::json& result,
                                     rcom::MemBuffer&,
                                     rcom::RPCError& error)
        {
                result.clear();
                error.code = rcom::RPCError::kMethodNotFound;
                error.message = "Unknown method";
        }

        void DataLogAdaptor::execute(const std::string&,
                                     const std::string& method,
                                     nlohmann::json& params,
                                     nlohmann::json& result,
                                     rcom::RPCError& error)
        {
                error.code = 0;
                                
                try {
                        if (method == MethodsDataLog::kStore) {
                                execute_store(params, error);
                                
                        } else {
                                error.code = rcom::RPCError::kMethodNotFound;
                                error.message = "Unknown method";
                        }
                        
                } catch (std::exception& e) {
                        error.code = rcom::RPCError::kInternalError;
                        error.message = e.what();
                }
        }

        void DataLogAdaptor::execute_store(nlohmann::json& params,
                                           rcom::RPCError& error)
        {
                double time = params[MethodsDataLog::kTime];
                const std::string& topic = params[MethodsDataLog::kTopic];
                const std::string& name = params[MethodsDataLog::kName];
                double value = params[MethodsDataLog::kValue];
                datalog_.store(time, topic, name, value);
        }
}
