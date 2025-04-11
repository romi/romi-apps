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
#ifndef __ROMI_IMETAFOLDER_H
#define __ROMI_IMETAFOLDER_H

#include <filesystem>
#include <rcom/MemBuffer.h>
#include <rcom/json.hpp>
#include "api/Image.h"
#include "api/Path.h"

namespace romi {

        class IMetaFolder {
        public:
                IMetaFolder() = default;
                virtual ~IMetaFolder() = default;
                virtual void try_store_jpg(const std::string &filename,
                                           romi::Image &image,
                                           const std::string &observationId) = 0;
                virtual void try_store_jpg(const std::string &filename,
                                           rcom::MemBuffer& jpeg,
                                           const std::string &observationId) = 0;
                virtual void try_store_png(const std::string &filename,
                                           romi::Image &image,
                                           const std::string &observationId) = 0;
                virtual void try_store_svg(const std::string &filename,
                                           const std::string& body,
                                           const std::string &observationId) = 0;
                virtual void try_store_txt(const std::string &filename,
                                           const std::string& text,
                                           const std::string &observationId) = 0;
                virtual void try_store_path(const std::string &filename,
                                            romi::Path &weeder_path,
                                            const std::string &observationId) = 0;
                virtual void try_store_metadata(const std::string& name,
                                                nlohmann::json& data) = 0;
        };
}

#endif //__ROMI_IMETAFOLDER_H
