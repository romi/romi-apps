/*
  rcom

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  rcom is light-weight libary for inter-node communication.

  rcom is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include <iomanip>
#include <chrono>
#include <thread>
#include "util/Clock.h"

namespace romi {

        const double MILLISECONDS_IN_SECOND = 1000.0;
        
        double Clock::time()
        {
                using namespace std::chrono;
                milliseconds millisecs = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
                return static_cast<double>(millisecs.count()) / MILLISECONDS_IN_SECOND;
        }
        
        Date Clock::date()
        {
                Date d;
                using namespace std::chrono;
                system_clock::time_point now = system_clock::now();
                time_t tt = system_clock::to_time_t(now);
                struct tm local_tm;
                localtime_r(&tt, &local_tm);
                d.year_ = (int16_t) (1900 + local_tm.tm_year); 
                d.month_ = (uint8_t) local_tm.tm_mon; 
                d.day_ = (uint8_t) local_tm.tm_mday; 
                d.hour_ = (uint8_t) local_tm.tm_hour; 
                d.minute_ = (uint8_t) local_tm.tm_min; 
                d.second_ = (uint8_t) local_tm.tm_sec;
                return d;
        }

        std::string Clock::datetime_compact_string()
        {
            using namespace std::chrono;
            auto now = system_clock::now();
            auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
            auto now_time_t = system_clock::to_time_t(now);

            std::stringstream ss;
            ss << std::put_time(std::localtime(&now_time_t), "%Y%m%d-%H%M%S");
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return ss.str();
        }

        uint64_t Clock::timestamp()
        {
                auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
                auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
                return static_cast<uint64_t>(ns.count());
        }
        
        // TBD: Move this out of clock
        void Clock::sleep(double seconds)
        {
                long sleep_duration = long (seconds * MILLISECONDS_IN_SECOND);
                std::this_thread::sleep_for(std::chrono::milliseconds (sleep_duration));
        }
}

