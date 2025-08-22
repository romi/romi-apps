#pragma once

namespace romi {

        class IBatteryMonitor
        {
        public:
                virtual ~IBatteryMonitor() = default;
                virtual bool is_charging() = 0;
                virtual double get_voltage() = 0;
                virtual double get_current() = 0;
        };
}

