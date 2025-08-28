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
                double timestamp_;
                double prev_current_;
                double prev_voltage_;
                double prev_timestamp_;
                uint32_t error_count_;
                double charge_;
                double energy_;
                int info_count_;

                void update();
                void updated_locked();
                void measure();
                void update_charge();
                void update_energy();
                void print();
                void set_capacity_if_charged();
                bool is_charging_locked();
                bool is_charged_locked();

        public:
                // capatity in mAh
                LithiumBattery(IBatteryMonitor& monitor, double voltage, double capacity);
                virtual ~LithiumBattery() override;

                bool is_charging() override;
                bool is_charged() override;
                double get_voltage() override;
                double get_current() override;
                double get_level() override;
        };
}
