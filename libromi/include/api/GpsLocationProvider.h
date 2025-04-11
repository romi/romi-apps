//
// Created by douglas on 17/03/2021.
//

#ifndef ROMI_GPSLOCATIONPROVIDER_H
#define ROMI_GPSLOCATIONPROVIDER_H

#include "ILocationProvider.h"
#include "JsonFieldNames.h"
#include "IGPS.h"
#include "json.hpp"

namespace romi {

        class GpsLocationProvider : public ILocationProvider {
        public:
                explicit GpsLocationProvider(IGps &Gps);

                ~GpsLocationProvider() override = default;

                nlohmann::json location() override;
                bool update_location_estimate() override;
                v3 coordinates() override;

        private:
                double startup_latitude_;
                double startup_longitude_;
                double latitude_;
                double longitude_;
                IGps &gps_;
        };
}

#endif // ROMI_GPSLOCATIONPROVIDER_H
