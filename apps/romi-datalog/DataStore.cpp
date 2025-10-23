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
#include "DataStore.h"

namespace romi {

        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;
        
        DataStore::DataStore()
                : name_to_index_(),
                  index_to_name_(),
                  entries_(),
                  mutex_()
        {
        }

        uint32_t DataStore::get_index_locked(const std::string& name)
        {
                uint32_t index;
                std::map<std::string,uint32_t>::iterator it;
                
                it = name_to_index_.find(name);
                if (it != name_to_index_.end()) {
                        index = it->second;
                } else {
                        index = (uint32_t) name_to_index_.size();
                        name_to_index_.insert(std::pair<std::string,uint32_t>(name, index));
                        index_to_name_.insert(std::pair<uint32_t,std::string>(index, name));
                }
                return index;
        }
        
        const std::string& DataStore::get_name_locked(uint32_t index)
        {
                std::map<uint32_t,std::string>::iterator it;
                
                it = index_to_name_.find(index);
                if (it == index_to_name_.end()) {
                        r_err("DataStore: Couldn't find name for index %u", index);
                        throw std::runtime_error("DataStore: couldn't find name");
                } 
                return it->second;                
        }
        
        const std::string& DataStore::get_name(uint32_t index)
        {
                SynchronizedCodeBlock synchronize(mutex_);
                return get_name_locked(index);
        }
        
        void DataStore::store(double time, const std::string& topic,
                              const std::string& name, double value)
        {
                SynchronizedCodeBlock synchronize(mutex_);
                uint32_t t_index = get_index_locked(topic);
                uint32_t n_index = get_index_locked(name);
                entries_.emplace_back(time, t_index, n_index, value);
        }
        
        void DataStore::copy(std::vector<DataLogEntry>& entries)
        {
                // The maximum time that store() can get blocked is
                // the time required to make this copy.
                SynchronizedCodeBlock synchronize(mutex_);
                entries = entries_;
                entries_.clear();
        }
}

