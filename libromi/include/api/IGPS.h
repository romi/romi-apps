
#ifndef ROMI_IGPS_H
#define ROMI_IGPS_H

namespace romi {

        class IGps {
        public:
                virtual ~IGps() = default;
                virtual void CurrentLocation(double &latitude, double &longitude) = 0;
        };

}

#endif // ROMI_IGPS_H
