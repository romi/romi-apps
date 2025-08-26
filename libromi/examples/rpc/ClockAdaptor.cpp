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
#include <ctime>
#include <util/Logger.h>
#include <util/IClock.h>
#include "ClockAdaptor.h"

namespace romi {

        ClockAdaptor::ClockAdaptor(IClock& clock)
                : clock_(clock)
        {
        }
        
        void ClockAdaptor::execute(const std::string&,
                                  const std::string&,
                                  nlohmann::json& result,
                                  rcom::MemBuffer&,
                                  rcom::RPCError& error)
        {
                r_debug("ClockAdaptor::execute (binary)");
                result.clear();
                error.code = rcom::RPCError::kMethodNotFound;
                error.message = "Unknown method";
        }

        void ClockAdaptor::execute(const std::string&,
                                    const std::string& method,
                                    nlohmann::json&,
                                    nlohmann::json& result,
                                    rcom::RPCError& error)
        {
                error.code = 0;
                r_debug("ClockAdaptor::execute (text)");
                                
                try {
                        if (method == "get-time") {
                                execute_get_time(result, error);
                                
                        } else {
                                error.code = rcom::RPCError::kMethodNotFound;
                                error.message = "Unknown method";
                        }
                        
                } catch (std::exception& e) {
                        error.code = rcom::RPCError::kInternalError;
                        error.message = e.what();
                }
        }

        void ClockAdaptor::execute_get_time(nlohmann::json& result, rcom::RPCError&)
        {
                r_debug("ClockAdaptor: get_time");
                result = clock_.time();
        }
}
