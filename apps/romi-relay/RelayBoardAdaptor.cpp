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
#include "MethodsRelayBoard.h"
#include "RelayBoardAdaptor.h"

namespace romi {

        RelayBoardAdaptor::RelayBoardAdaptor(IRelayBoard& board)
                : board_(board)
        {
        }

        void RelayBoardAdaptor::execute(const std::string&,
                                        const std::string&,
                                        nlohmann::json& result,
                                        rcom::MemBuffer&,
                                        rcom::RPCError& error)
        {
                result.clear();
                error.code = rcom::RPCError::kMethodNotFound;
                error.message = "Unknown method";
        }

        void RelayBoardAdaptor::execute(const std::string&,
                                        const std::string& method,
                                        nlohmann::json& params,
                                        nlohmann::json& result,
                                        rcom::RPCError& error)
        {
                error.code = 0;
                                
                try {
                        if (method == MethodsRelayBoard::kCountRelays) {
                                execute_count_relays(result);
                                
                        } else if (method == MethodsRelayBoard::kGet) {
                                execute_get(params, result);
                                
                        } else if (method == MethodsRelayBoard::kSet) {
                                execute_set(params);
                                
                        } else {
                                error.code = rcom::RPCError::kMethodNotFound;
                                error.message = "Unknown method";
                        }
                        
                } catch (std::exception& e) {
                        error.code = rcom::RPCError::kInternalError;
                        error.message = e.what();
                }
        }

        void RelayBoardAdaptor::execute_count_relays(nlohmann::json& result)
        {
                result = {{MethodsRelayBoard::kCount, board_.count_relays()}};
        }

        void RelayBoardAdaptor::execute_get(nlohmann::json& params,
                                            nlohmann::json& result)
        {
                uint16_t index = params[MethodsRelayBoard::kIndex];
                result = {{MethodsRelayBoard::kValue, board_.get(index)}};
        }

        void RelayBoardAdaptor::execute_set(nlohmann::json& params)
        {
                uint16_t index = params[MethodsRelayBoard::kIndex];
                bool value = params[MethodsRelayBoard::kValue];
                board_.set(index, value);
        }
}
