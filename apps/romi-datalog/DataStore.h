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

#ifndef __ROMI_DATASTORE_H
#define __ROMI_DATASTORE_H

#include <string>
#include <mutex>
#include <map>
#include <vector>

namespace romi {

        struct DataLogEntry
        {
                double time_;
                uint32_t topic_index_;
                uint32_t name_index_;
                double value_;

                DataLogEntry(double time, uint32_t topic_index,
                             uint32_t name_index, double value)
                        : time_(time),
                          topic_index_(topic_index),
                          name_index_(name_index),
                          value_(value) {
                }
        };

        class DataStore
        {
        protected:
                
                std::map<std::string, uint32_t> name_to_index_;
                std::map<uint32_t, std::string> index_to_name_;
                std::vector<DataLogEntry> entries_;
                std::mutex mutex_;
                
                uint32_t get_index_locked(const std::string& name);
                const std::string& get_name_locked(uint32_t index);
                
                DataStore(const DataStore& other) = delete;
                DataStore& operator=(const DataStore& other) = delete;
        
        public:
                DataStore();
                ~DataStore() = default;
                
                void store(double time, const std::string& topic,
                           const std::string& name, double value);
                void copy(std::vector<DataLogEntry>& entries);
                const std::string& get_name(uint32_t index);
        };
}

#endif // __ROMI_DATASTORE_H
