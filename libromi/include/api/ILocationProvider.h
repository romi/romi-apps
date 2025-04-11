
#ifndef ROMI_ILOCATIONPROVIDER_H
#define ROMI_ILOCATIONPROVIDER_H

#include <string>
#include "v3.h"
#include "json.hpp"

namespace romi {

        class ILocationProvider {
        public:
                ILocationProvider() = default;
                virtual ~ILocationProvider() = default;
        
                virtual nlohmann::json location() = 0;
        
                virtual bool update_location_estimate() = 0;
                virtual v3 coordinates() = 0;
        };

}
#endif // ROMI_ILOCATIONPROVIDER_H
