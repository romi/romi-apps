
#ifndef ROMI_IDEVICEDATA_H
#define ROMI_IDEVICEDATA_H

#include "json.hpp"
#include <string>

namespace romi {

        class IDeviceData {
        public:
                virtual ~IDeviceData() = default;
                virtual std::string type() = 0;
                virtual std::string hardware_id() = 0;
                virtual std::string software_version() = 0;
                virtual nlohmann::json get_identity() = 0;
        };

}

#endif // ROMI_IDEVICEDATA_H
