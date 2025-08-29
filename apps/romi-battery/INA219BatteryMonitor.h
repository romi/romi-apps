#pragma once

#include <cstdint>
#include <limits>
#include "IBatteryMonitor.h"

namespace romi {
        
        class INA219BatteryMonitor final : public IBatteryMonitor
        {
        public:
                // Construct with your bus, address, shunt value, and
                // expected max current.  Defaults are typical for
                // many INA219 breakout boards.
                explicit INA219BatteryMonitor(int bus = 1,
                                              uint16_t i2c_addr = 0x40,
                                              double rshunt_ohms = 0.1,
                                              double imax_A = 3.2);

                ~INA219BatteryMonitor() override;

                double get_voltage() override;
                double get_current() override;

                // Optional: call explicitly if you prefer to fail
                // fast at startup.
                bool begin() {
                        return open_();
                }
                
                bool reset() override {
                        return close_() && open_();
                }

        private:
                static constexpr uint8_t REG_CONFIG   = 0x00;
                static constexpr uint8_t REG_SHUNT_V  = 0x01; // 10 µV LSB (not exposed here)
                static constexpr uint8_t REG_BUS_V    = 0x02; // 4 mV LSB after >>3
                static constexpr uint8_t REG_POWER    = 0x03; // 20 × Current_LSB (W/bit)
                static constexpr uint8_t REG_CURRENT  = 0x04; // Current_LSB (A/bit)
                static constexpr uint8_t REG_CAL      = 0x05;

                static double NaN() { return std::numeric_limits<double>::quiet_NaN(); }

                bool ensure_init_();
                bool open_();
                bool close_();
                bool writeReg16_(uint8_t reg, uint16_t value_be);
                bool readReg16_(uint8_t reg, uint16_t& out_be);

        private:
                int fd_ = -1;
                int bus_ = 1;
                uint16_t addr_ = 0x40;
                double rshunt_ = 0.1;
                double imax_ = 3.2;
                bool initialized_ = false;
                uint16_t cal_ = 0;
                double current_lsb_A_ = 0.0;
                double power_lsb_W_ = 0.0;
        };
}
