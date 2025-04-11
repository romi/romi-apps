
#ifndef ROMI_GPS_H
#define ROMI_GPS_H

#include "api/IGPS.h"

namespace romi {

        class Gps : public IGps{
        public:
                Gps() = default;
                virtual ~Gps() = default;
                void CurrentLocation(double &latitude, double &longitude) override;

        };

}
#endif // ROMI_GPS_H
