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
#ifndef ROMI_METAFOLDER_H
#define ROMI_METAFOLDER_H

#include <mutex>
#include "util/ImageIO.h"
#include "api/IDeviceData.h"
#include "api/ILocationProvider.h"
#include "api/IMetaFolder.h"

namespace romi {

        class MetaFolder : public IMetaFolder
        {

        public:
                MetaFolder(std::shared_ptr<IDeviceData> device,
                           std::shared_ptr<ILocationProvider> location,
                           std::filesystem::path folder_path);

                // TBD: Refactor this out to the constuctor.
                void try_store_jpg(const std::string &filename,
                                   romi::Image &image,
                                   const std::string &observationId) override;
                void try_store_jpg(const std::string &filename,
                                   rcom::MemBuffer& jpeg,
                                   const std::string &observationId) override;
                void try_store_png(const std::string &filename,
                                   romi::Image &image,
                                   const std::string &observationId) override;
                void try_store_svg(const std::string &filename,
                                   const std::string& body,
                                   const std::string &observationId) override;
                void try_store_txt(const std::string &filename,
                                   const std::string& text,
                                   const std::string &observationId) override;
                void try_store_path(const std::string &filename,
                                    romi::Path &weeder_path,
                                    const std::string &observationId) override;
                void try_store_metadata(const std::string& name,
                                        nlohmann::json& data) override;

                inline static const std::string metadata_filename_ = "metadata.json";

        private:
                void try_create();
                
                std::filesystem::path
                build_filename_with_extension(const std::string& filename,
                                              const std::string& extension);
                void add_file_metadata(const std::string &filename,
                                       const std::string &ovservationId);
                void check_input(Image& image) const;
                void check_input(const std::string& string_data, bool empty_ok) const;
                void check_input(rcom::MemBuffer& jpeg) const;
                void save_metadata() const;
                std::shared_ptr<IDeviceData> device_;
                std::shared_ptr<ILocationProvider> location_;
                std::filesystem::path folderPath_;
                std::unique_ptr<nlohmann::json> metadata_;
                std::recursive_mutex metadata_file_mutex_;
        };

}

#endif // ROMI_METAFOLDER_H
