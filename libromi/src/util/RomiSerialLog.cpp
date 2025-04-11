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
#include <stdarg.h>
#include <string>
#include "util/RomiSerialLog.h"
#include "util/StringUtils.h"
#include "util/Logger.h"

namespace romi {

        RomiSerialLog::RomiSerialLog() {
        }
        
        void RomiSerialLog::error(const char *format, ...)
        {
                std::string message;
                va_list ap;
                va_start(ap, format);
                StringUtils::string_vprintf(message, format, ap);
                va_end(ap);

                r_err("%s", message.c_str());
        }
        
        void RomiSerialLog::warn(const char *format, ...)
        {
                std::string message;
                va_list ap;
                va_start(ap, format);
                StringUtils::string_vprintf(message, format, ap);
                va_end(ap);

                r_warn("%s", message.c_str());
        }
        
        void RomiSerialLog::debug(const char *format, ...)
        {
                std::string message;
                va_list ap;
                va_start(ap, format);
                StringUtils::string_vprintf(message, format, ap);
                va_end(ap);

                r_debug("%s", message.c_str());
        }
}


