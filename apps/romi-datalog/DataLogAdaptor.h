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
#ifndef ROMI_DATALOGADAPTOR_H
#define ROMI_DATALOGADAPTOR_H

#include <rcom/IRPCHandler.h>
#include <api/IDataLog.h>

namespace romi {

        class DataLogAdaptor : public rcom::IRPCHandler
        {
        protected:
                IDataLog& datalog_;

                void execute_store(nlohmann::json& params,
                                   rcom::RPCError& error);
                
        public:
                DataLogAdaptor(IDataLog& datalog);
                ~DataLogAdaptor() override = default;
        
                void execute(const std::string& id,
                             const std::string& method,
                             nlohmann::json& params,
                             nlohmann::json& result,
                             rcom::RPCError& status) override;
                void execute(const std::string& id,
                             const std::string& method,
                             nlohmann::json& params,
                             rcom::MemBuffer& result,
                             rcom::RPCError &status) override;
        };
}

#endif // ROMI_DATALOGADAPTOR_H
