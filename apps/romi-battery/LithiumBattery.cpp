#include <util/Logger.h>
#include <util/ClockAccessor.h>
#include "LithiumBattery.h"

namespace romi {

        using SynchonizedCodeBlock = std::lock_guard<std::mutex>;
        
        LithiumBattery::LithiumBattery(IBatteryMonitor& monitor,
                                       double voltage,
                                       double capacity)
                : monitor_(monitor),
                  nominal_voltage_(voltage),
                  capacity_charge_(capacity),
                  capacity_energy_(0),
                  done_(false),
                  thread_(),
                  mutex_(),
                  current_(0),
                  voltage_(0),
                  timestamp_(0),
                  prev_current_(0),
                  prev_voltage_(0),
                  prev_timestamp_(0),
                  error_count_(0),
                  charge_(capacity),
                  energy_(0),
                  info_count_(0)
        {
                capacity_energy_ = nominal_voltage_ * capacity_charge_ / 1000.0; // Wh
                // Ussume that the battery is fully charged at start-up
                energy_ = capacity_energy_;
                r_info("Battery: nominal voltage: %.2f V, "
                       "capacity (charge): %.2f mAh, "
                       "capacity (energy): %.2f Wh",
                       nominal_voltage_, capacity_charge_, capacity_energy_);
                thread_ = std::thread(&LithiumBattery::update, this);
        }

        LithiumBattery::~LithiumBattery()
        {
                done_ = true;
                if (thread_.joinable())
                        thread_.join();
        }
        
        bool LithiumBattery::is_charging()
        {
                SynchonizedCodeBlock synchonized(mutex_);
                return is_charging_locked(); 
        }
        
        bool LithiumBattery::is_charging_locked()
        {
                // Allow a tidy negative discharge current. This may
                // happen when the battery is full.
                return (current_ >= -0.001); 
        }

        bool LithiumBattery::is_charged()
        {
                return ((voltage_ > 4.15) &&
                        ((current_ > -0.001)
                         && (current_ < 0.001))) ;
        }

        double LithiumBattery::get_voltage()
        {
                SynchonizedCodeBlock synchonized(mutex_);
                return voltage_;
        }

        double LithiumBattery::get_current()
        {
                SynchonizedCodeBlock synchonized(mutex_);
                return current_;
        }

        void LithiumBattery::update()
        {
                timestamp_ = ClockAccessor::GetInstance()->time();
                while (!done_) {
                        updated_locked();
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
        }

        void LithiumBattery::updated_locked()
        {
                SynchonizedCodeBlock synchonized(mutex_);
                measure();
                update_charge();
                update_energy();
                set_capacity_if_charged();
                print();
        }
        
        void LithiumBattery::measure()
        {
                prev_current_ = current_;
                prev_voltage_ = voltage_;
                prev_timestamp_ = timestamp_;
                voltage_ = monitor_.get_voltage();
                current_ = monitor_.get_current();
                timestamp_ = ClockAccessor::GetInstance()->time();
        }
        
        void LithiumBattery::update_charge()
        {
                // seconds to hour
                double h = (timestamp_ - prev_timestamp_) / 3600.0;
                double I = (prev_current_ + current_) / 2.0;
                double delta_charge_ = I * h * 1000.0; // x1000 → mAh
                
                charge_ += delta_charge_;
                if (charge_ > capacity_charge_)
                        charge_ = capacity_charge_;
                if (charge_ < 0)
                        charge_ = 0;
        }
        
        void LithiumBattery::update_energy()
        {
                // seconds to hour
                double h = (timestamp_ - prev_timestamp_) / 3600.0;
                double I = (prev_current_ + current_) / 2.0;
                double V = (prev_voltage_ + voltage_) / 2.0;
                double P = V * I;
                double delta_energy = P * h; // in Wh
                
                energy_ += delta_energy; 
                if (energy_ > capacity_energy_)
                        energy_ = capacity_energy_;
                if (energy_ < 0)
                        energy_ = 0;
        }
        
        void LithiumBattery::print()
        {
                if (info_count_++ == 60) {
                        r_info("Battery: %.3f A, %.2f V, "
                               "%.1f mAh, %.2f Wh, "
                               "%s, %s",
                               current_, voltage_,
                               charge_, energy_,
                               is_charging_locked()? "charging" : "discharging",
                               is_charged()? "charged" : "...");
                        info_count_ = 0;
                }
        }
        
        void LithiumBattery::set_capacity_if_charged()
        {
                if (is_charged()) {
                        charge_ = capacity_charge_;
                        energy_ = capacity_energy_;
                }
        }
}
