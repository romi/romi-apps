#include <stdexcept>
#include <rcom/json.hpp>
#include "api/JsonFieldNames.h"
#include "api/GpsLocationProvider.h"
#include "util/Logger.h"

namespace romi {

        GpsLocationProvider::GpsLocationProvider(IGps &gps)
                : startup_latitude_(0.0),
                  startup_longitude_(0.0),
                  latitude_(0.0),
                  longitude_(0.0),
                  gps_(gps) {
                gps_.CurrentLocation(startup_longitude_, startup_longitude_);
        }

        bool GpsLocationProvider::update_location_estimate()
        {
                gps_.CurrentLocation(latitude_, longitude_);
                return true;
        }
        
        nlohmann::json GpsLocationProvider::location()
        {
                if (!update_location_estimate()) {
                        r_warn("GpsLocationProvider: update failed. "
                               "Returning old estimates");
                }

            nlohmann::json coordinate_object = nlohmann::json::object({ {JsonFieldNames::latitude.data(),  latitude_},
                                                                        {JsonFieldNames::longitude.data(), longitude_}});

            return coordinate_object;
        }

        v3 GpsLocationProvider::coordinates()
        {
                throw std::runtime_error("GpsLocationProvider::coordinates: "
                                         "NOT IMPLEMENTED YET");
        }
}
