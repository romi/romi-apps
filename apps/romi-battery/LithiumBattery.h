#pragma once

#include <thread>
#include <mutex>
#include <api/IDataLog.h>
#include "IBatteryStatusIndicator.h"
#include "IBatteryMonitor.h"
#include "IBattery.h"

namespace romi {
        
        class LithiumBattery : public IBattery 
        {
        protected:
                IBatteryMonitor& monitor_;
                IBatteryStatusIndicator& status_;
                IDataLog& datalog_;
                std::string topic_;
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
                int reset_count_;

                void update();
                void updated_locked();
                void measure();
                void update_charge();
                void update_energy();
                void update_status();
                bool do_print();
                void log(double time);
                void print(double time);
                void reset_perhaps();
                void set_capacity_if_charged();
                bool is_charging_locked();
                bool is_charged_locked();
                bool is_low_locked();

        public:
                // capatity in mAh
                LithiumBattery(IBatteryMonitor& monitor,
                               IBatteryStatusIndicator& status,
                               IDataLog& datalog,
                               const std::string& topic,
                               double voltage,
                               double capacity);
                virtual ~LithiumBattery() override;

                bool is_charging() override;
                bool is_charged() override;
                double get_voltage() override;
                double get_current() override;
                double get_level() override;
        };
}
