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
#ifndef ROMI_ISESSION_H
#define ROMI_ISESSION_H

#include <filesystem>
#include <string>
#include <rcom/MemBuffer.h>
#include <rcom/json.hpp>
#include "api/Image.h"
#include "api/Path.h"

namespace romi {

        class ISession {
        public:
                ISession() = default;
                virtual ~ISession() = default;
                virtual void start(const std::string& observation_id) = 0;
                virtual void stop() = 0;
                virtual bool store_jpg(const std::string& name, Image& image) = 0;
                virtual bool store_jpg(const std::string& name, rcom::MemBuffer& jpeg) = 0;
                virtual bool store_png(const std::string& name, Image& image)  = 0;
                virtual bool store_svg(const std::string& name, const std::string& body) = 0;
                virtual bool store_txt(const std::string& name, const std::string& body) = 0;
                virtual bool store_path(const std::string& filename, int32_t path_number, Path& weeder_path) = 0;
                virtual bool store_metadata(const std::string& name, nlohmann::json& data) = 0;
                virtual std::filesystem::path current_path() = 0;
                virtual std::filesystem::path base_directory() = 0;
                virtual std::filesystem::path create_session_file(const std::string& name) =0;
        };
}

#endif // ROMI_ISESSION_H
