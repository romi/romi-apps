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
#ifndef ROMI_DATALOGACCESSOR_H
#define ROMI_DATALOGACCESSOR_H

#include <memory>
#include <string>
#include "DataLog.h"

namespace romi {

        class DataLogAccessor
        {
        public:
                static const std::shared_ptr<IDataLog>& get();
            
                static void set(const std::shared_ptr<IDataLog>& log) {
                        log_ = log;
                }
            
        private:
                static std::shared_ptr<IDataLog> log_;
        };

    
        inline void log_data(double time, const std::string& name, double value) {
                DataLogAccessor::get()->store(time, name, value);
        }

        inline void log_data(const std::string& name, double value) {
                DataLogAccessor::get()->store(name, value);
        }
}

#endif // ROMI_DATALOGACCESSOR_H
