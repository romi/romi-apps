#pragma once

#include <thread>
#include <mutex>
#include "IBatteryMonitor.h"
#include "IBattery.h"

namespace romi {
        
        class LithiumBattery : public IBattery 
        {
        protected:
                IBatteryMonitor& monitor_;
                double nominal_voltage_;
                double capacity_charge_;
                double capacity_energy_;
                double nominal_power_;
                bool done_;
                std::thread thread_;
                std::mutex mutex_;
                double current_;
                double voltage_;
                uint32_t error_count_;
                double timestamp_;
                double charge_;
                double energy_;

                void update();
                void measure();
                bool is_charged();

        public:
                // capatity in mAh
                LithiumBattery(IBatteryMonitor& monitor, double voltage, double capacity);
                virtual ~LithiumBattery() override;

                bool is_charging() override;
                double get_voltage() override;
                double get_current() override;
                //double get_percentage() override;
        };
}
