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

#ifndef __ROMI_FILEDATALOG_H
#define __ROMI_FILEDATALOG_H

#include <stdio.h>
#include <string>
#include <thread>
#include <atomic>
#include <filesystem>
#include "DataStore.h"
#include <api/IDataLog.h>

namespace romi {

        class FileDataLog : public IDataLog
        {
        protected:

                static const size_t kCacheSize = 100;
                
                std::string topic_;
                DataStore datastore_;
                FILE *fp_;
                std::unique_ptr<std::thread> thread_;
                std::atomic<bool> quitting_;
                double last_handle_events_;
                
                uint32_t get_index(const std::string& name);
                void write_entries_to_storage_in_background();
                void try_writing_entries_to_storage();
                void write_entries_to_storage(std::vector<DataLogEntry>& entries);
                void write_entry_to_storage(DataLogEntry& entry);
                const std::string& get_name(uint32_t index);
                void copy_entries(std::vector<DataLogEntry>& entries);
                
                FileDataLog(const FileDataLog& other) = delete;
                FileDataLog& operator=(const FileDataLog& other) = delete;
        
        public:
                FileDataLog(const std::string& topic,
                            const std::string& filepath);
                FileDataLog(const std::string& topic,
                            const std::filesystem::path& filepath);
                ~FileDataLog() override;
                
                void store(double time, const std::string& topic,
                           const std::string& name, double value) override;
                void store(double time, const std::string& name, double value) override;
        };
}

#endif // __ROMI_FILEDATALOG_H
