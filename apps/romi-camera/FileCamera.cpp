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

#include "util/FileUtils.h"
#include "util/Logger.h"
#include "FileCamera.h"

namespace romi {

        FileCamera::FileCamera(const std::string& filename)
                : filename_(filename),
                  image_(),
                  jpeg_()
        {
                if (filename_.length() == 0)
                        throw std::runtime_error("FileCamera: Invalid filename");
                
                if (!load_image()) {
                        r_err("Failed to load the file: %s", filename_.c_str());
                        throw std::runtime_error("FileCamera::open failed");
                }
                
                load_jpeg();
        }

        bool FileCamera::load_image()
        {
                return ImageIO::load(image_, filename_.c_str());
        }

        void FileCamera::load_jpeg()
        {
            std::vector<uint8_t> image_data;
            jpeg_.clear();
            FileUtils::TryReadFileAsVector(filename_, image_data);
            jpeg_.append(image_data.data(), image_data.size());
        }

        bool FileCamera::set_value(const std::string& name, double value)
        {
                (void) name;
                (void) value;
                return true;
        }
        
        bool FileCamera::select_option(const std::string& name,
                                       const std::string& value)
        {
                (void) name;
                (void) value;
                return true;
        }
        
        bool FileCamera::grab(Image &image)
        {
                image = image_;
                return true;
        }

        rcom::MemBuffer& FileCamera::grab_jpeg()
        {
            return jpeg_;
        }

        bool FileCamera::power_up()
        {
                return true;
        }
        
        bool FileCamera::power_down()
        {
                return true;
        }
        
        bool FileCamera::stand_by()
        {
                return true;
        }
        
        bool FileCamera::wake_up()
        {
                return true;
        }

        const ICameraSettings& FileCamera::get_settings()
        {
                throw std::runtime_error("FileCamera::get_settings: not implemented");
        }
}

