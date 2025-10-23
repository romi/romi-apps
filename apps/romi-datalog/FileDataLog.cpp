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
#include <thread>
#include <util/ClockAccessor.h>
#include <util/Logger.h>
#include "FileDataLog.h"

namespace romi {
        
        FileDataLog::FileDataLog(const std::string& topic,
                                 const std::string& path)
                : topic_(topic),
                  datastore_(),
                  fp_(nullptr),
                  thread_(nullptr),
                  quitting_(false)
        {
                fp_ = fopen(path.c_str(), "a");
                if (fp_ == nullptr) {
                        r_err("FileDataLog: can't open file %s", path.c_str());
                        throw std::runtime_error("FileDataLog: can't open file");
                }
                thread_ = std::make_unique<std::thread>([this]() {
                                this->write_entries_to_storage_in_background();
                        });
        }
        
        FileDataLog::FileDataLog(const std::string& topic,
                                 const std::filesystem::path& filepath)
                : FileDataLog(topic, filepath.string())
        {
        }
        
        FileDataLog::~FileDataLog()
        { 
                quitting_ = true;
                if (thread_ != nullptr) {
                        thread_->join();
                }
                fclose(fp_);
        }
        
        void FileDataLog::store(double time, const std::string& topic,
                                const std::string& name, double value)
        {
                datastore_.store(time, topic, name, value);
        }
        
        void FileDataLog::store(double time, const std::string& name, double value)
        {
                store(time, topic_, name, value);
        }
        
        void FileDataLog::write_entries_to_storage_in_background()
        {
                auto clock = romi::ClockAccessor::GetInstance();
                
                while (!quitting_) {
                        try_writing_entries_to_storage();
                        clock->sleep(1.0);
                }

                // Make sure the remaining entries get stored.
                try_writing_entries_to_storage();
        }
                
        void FileDataLog::try_writing_entries_to_storage()
        {
                try {
                        std::vector<DataLogEntry> entries;
                        datastore_.copy(entries);
                        if (entries.size() > 0) {
                                write_entries_to_storage(entries);
                        }

                } catch (const std::runtime_error& e) {
                        r_err("FileDataLog: failed to store the data: %s", e.what());
                }
        }
        
        void FileDataLog::write_entries_to_storage(std::vector<DataLogEntry>& entries)
        {
                for (auto entry: entries)
                        write_entry_to_storage(entry);
                fflush(fp_);
        }
        
        void FileDataLog::write_entry_to_storage(DataLogEntry& entry)
        {
                const std::string& topic = datastore_.get_name(entry.topic_index_);
                const std::string& name = datastore_.get_name(entry.name_index_);
                fprintf(fp_, "%f,%s,%s,%f\n",
                        entry.time_, topic.c_str(),
                        name.c_str(), entry.value_); 
        }
}

