#pragma once

namespace romi {

        class IBatteryMonitor
        {
        public:
                virtual ~IBatteryMonitor() = default;
                virtual double get_voltage() = 0;
                virtual double get_current() = 0;
        };
}

