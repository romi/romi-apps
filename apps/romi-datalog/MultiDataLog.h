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

#ifndef __ROMI_MULTIDATALOG_H
#define __ROMI_MULTIDATALOG_H

#include <memory>
#include <vector>
#include <api/IDataLog.h>

namespace romi {

        class MultiDataLog : public IDataLog
        {
        protected:
                std::vector<std::unique_ptr<IDataLog>> logs_;
                
                MultiDataLog(const MultiDataLog& other) = delete;
                MultiDataLog& operator=(const MultiDataLog& other) = delete;
        
        public:
                MultiDataLog();
                ~MultiDataLog() override = default;

                void add(std::unique_ptr<IDataLog>& log);
                void store(double time, const std::string& topic,
                           const std::string& name, double value) override;
                void store(double time, const std::string& name, double value) override;
        };
}

#endif // __ROMI_MULTIDATALOG_H
