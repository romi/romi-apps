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

#include <iostream>
#include "api/JsonFieldNames.h"
#include "api/MetaFolder.h"
#include "util/ClockAccessor.h"

namespace romi {
        MetaFolder::MetaFolder(std::shared_ptr<IDeviceData> device,
                               std::shared_ptr<ILocationProvider> location,
                               std::filesystem::path folder_path)
                : device_(std::move(device)),
                  location_(std::move(location)),
                  folderPath_(folder_path),
                  metadata_(),
                  metadata_file_mutex_() {
                
                if (nullptr == device_){
                        throw std::invalid_argument("device");
                }
                if (nullptr == location_){
                        throw std::invalid_argument("location");
                }
                try_create();
        }

        std::filesystem::path
        MetaFolder::build_filename_with_extension(const std::string& filename,
                                                  const std::string& extension)
        {
                std::filesystem::path filename_extension(filename);
                filename_extension.replace_extension(extension);
                return filename_extension;
        }
        
        void MetaFolder::try_create()
        {
                std::filesystem::create_directories(folderPath_);

                metadata_ = std::make_unique<nlohmann::json>();
                (*metadata_)[JsonFieldNames::romi_identity] = device_->get_identity();
                save_metadata();
        }

        void MetaFolder::add_file_metadata(const std::string &filename,
                                           const std::string &observationId)
        {
                nlohmann::json newFile;
                newFile[JsonFieldNames::observation_id] = observationId;
                newFile[JsonFieldNames::location] = location_->location();
                newFile[JsonFieldNames::date_time] = romi::ClockAccessor::GetInstance()->datetime_compact_string();
                (*metadata_)[filename] = newFile;
                save_metadata();
        }

        void MetaFolder::check_input(Image& image) const
        {
                if (metadata_ == nullptr)
                        throw std::runtime_error("Session not created");
                if (image.data().empty())
                        throw std::runtime_error("Image data empty");
        }

        void MetaFolder::check_input(const std::string& string_data,
                                    bool empty_ok) const
        {
                (void) string_data;
                if (metadata_ == nullptr)
                        throw std::runtime_error("Session not created");
                if (string_data.empty() && !empty_ok)
                        throw std::runtime_error("String data empty");
        }

        void MetaFolder::check_input(rcom::MemBuffer& jpeg) const
        {
                if (metadata_ == nullptr)
                        throw std::runtime_error("Session not created");
                if (jpeg.data().empty())
                        throw std::runtime_error("Image data empty");
        }

        void MetaFolder::try_store_jpg(const std::string &filename,
                                       romi::Image &image,
                                       const std::string &observationId)
        {
                std::scoped_lock<std::recursive_mutex> scopedLock(metadata_file_mutex_);
                check_input(image);
                auto filename_extension = build_filename_with_extension(filename, "jpg");
                if (!ImageIO::store_jpg(image, (folderPath_ / filename_extension).c_str()))
                        throw std::runtime_error(std::string("try_store_jpg failed to write ")
                                                 + filename_extension.string());
                add_file_metadata(filename_extension, observationId);;
        }

        void MetaFolder::try_store_jpg(const std::string &filename,
                                       rcom::MemBuffer& jpeg,
                                       const std::string &observationId)
        {
                std::scoped_lock<std::recursive_mutex> scopedLock(metadata_file_mutex_);
                check_input(jpeg);
                auto filename_extension = build_filename_with_extension(filename, "jpg");
                FileUtils::TryWriteVectorAsFile((folderPath_ / filename_extension), jpeg.data());
                add_file_metadata(filename_extension, observationId);;
        }

        void MetaFolder::try_store_png(const std::string &filename,
                                       romi::Image &image,
                                       const std::string &observationId)
        {
                std::scoped_lock<std::recursive_mutex> scopedLock(metadata_file_mutex_);
                check_input(image);
                auto filename_extension = build_filename_with_extension(filename, "png");
                if (!ImageIO::store_png(image, (folderPath_ / filename_extension).c_str()))
                        throw std::runtime_error(std::string("try_store_png failed to write ")
                                                 + filename_extension.string());
                add_file_metadata(filename_extension, observationId);;
        }

        void MetaFolder::try_store_svg(const std::string &filename,
                                       const std::string& body,
                                       const std::string &observationId)
        {
                std::scoped_lock<std::recursive_mutex> scopedLock(metadata_file_mutex_);
                check_input(body, false);
                auto filename_extension = build_filename_with_extension(filename, "svg");
                FileUtils::TryWriteStringAsFile((folderPath_ / filename_extension), body);
                add_file_metadata(filename_extension, observationId);
        }

        void MetaFolder::try_store_txt(const std::string &filename,
                                       const std::string& text,
                                       const std::string &observationId)
        {
                std::scoped_lock<std::recursive_mutex> scopedLock(metadata_file_mutex_);
                check_input(text, true);
                auto filename_extension = build_filename_with_extension(filename, "txt");
                FileUtils::TryWriteStringAsFile((folderPath_ / filename_extension), text);
                add_file_metadata(filename_extension, observationId);
        }

        void MetaFolder::try_store_path(const std::string &filename,
                                        romi::Path &weeder_path,
                                        const std::string &observationId)
        {
                std::scoped_lock<std::recursive_mutex> scopedLock(metadata_file_mutex_);
                std::stringstream ss;
                Path::iterator ptr;
                for (ptr = weeder_path.begin(); ptr < weeder_path.end(); ptr++)  {
                        v3 p = *ptr;
                        ss << p.x() << "\t" << p.y() << "\r\n";
                }
                std::string path_data = ss.str();
                check_input(path_data, true);
                auto filename_extension = build_filename_with_extension(filename, "path");
                FileUtils::TryWriteStringAsFile((folderPath_ / filename_extension), path_data);
                add_file_metadata(filename_extension, observationId);
        }

        void MetaFolder::save_metadata() const
        {
                FileUtils::TryWriteStringAsFile((folderPath_ / metadata_filename_), (*metadata_).dump(4));
        }

        void MetaFolder::try_store_metadata(const std::string& name, nlohmann::json& data)
        {
                (*metadata_)[name] = data;
                save_metadata();
        }
}
