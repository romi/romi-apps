#pragma once

namespace romi {
        
        class IBattery 
        {
        public:
                virtual ~IBattery() = default;
                
                virtual bool is_charging() = 0;
                virtual bool is_charged() = 0;
                virtual double get_voltage() = 0;
                virtual double get_current() = 0;
                virtual double get_level() = 0;
        };
}
