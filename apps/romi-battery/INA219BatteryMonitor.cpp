#include <string>
#include <iostream>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "INA219BatteryMonitor.h"

namespace romi {
        
        INA219BatteryMonitor::INA219BatteryMonitor(int bus,
                                                   uint16_t i2c_addr,
                                                   double rshunt_ohms,
                                                   double imax_A)
                : bus_(bus),
                  addr_(i2c_addr),
                  rshunt_(rshunt_ohms),
                  imax_(imax_A)
        {
        }

        INA219BatteryMonitor::~INA219BatteryMonitor()
        {
                if (fd_ >= 0)
                        close(fd_);
        }
        
        bool INA219BatteryMonitor::is_charging()
        {
                double current = get_current();
                double voltage = get_voltage();
                // Allow a tidy negative discharge current. This may
                // happen when the battery is full.
                return (current >= -0.001); 
        }

        double INA219BatteryMonitor::get_voltage()
        {
                if (!ensure_init_())
                        return NaN();
                uint16_t be;
                if (!readReg16_(REG_BUS_V, be))
                        return NaN();
                // BUS_V: bits [15:3] are data, LSB = 4 mV.
                const uint16_t data = static_cast<uint16_t>(be >> 3);
                return static_cast<double>(data) * 0.004; // volts
        }

        double INA219BatteryMonitor::get_current()
        {
                if (!ensure_init_()) return NaN();
                uint16_t be;
                if (!readReg16_(REG_CURRENT, be)) return NaN();
                // CURRENT is signed; scale by Current_LSB set during calibration.
                const int16_t raw = static_cast<int16_t>(be);
                return static_cast<double>(raw) * current_lsb_A_;
        }

        bool INA219BatteryMonitor::ensure_init_()
        {
                if (initialized_) return true;
                return init_();
        }

        bool INA219BatteryMonitor::init_()
        {
                // Open bus (lazy)
                if (fd_ < 0) {
                        const std::string dev = "/dev/i2c-" + std::to_string(bus_);
                        fd_ = open(dev.c_str(), O_RDWR);
                        if (fd_ < 0) {
                                std::cerr << "INA219: open failed: "
                                          << std::strerror(errno) << "\n";
                                return false;
                        }
                }

                // Build CONFIG register: BRNG=1 (32V), PGA=3
                // (±320mV), BADC=SADC=0xD (12-bit, 32 samples),
                // MODE=0x7 (continuous shunt+bus)
                const uint16_t BRNG_32V = (1u << 13);
                const uint16_t PGA_GAIN_8 = (3u << 11);      // ±320 mV
                const uint16_t ADC_12BIT_32S = 0xD;             // per datasheet table
                const uint16_t BADC = static_cast<uint16_t>(ADC_12BIT_32S) << 7;
                const uint16_t SADC = static_cast<uint16_t>(ADC_12BIT_32S) << 3;
                const uint16_t MODE_CONT_SVB = 0x7;

                const uint16_t config = static_cast<uint16_t>(BRNG_32V | PGA_GAIN_8
                                                              | BADC | SADC
                                                              | MODE_CONT_SVB);

                if (!writeReg16_(REG_CONFIG, config)) {
                        std::cerr << "INA219: write CONFIG failed\n";
                        return false;
                }

                // Calibrate:
                // Current_LSB = Imax / 32768   (A/bit)
                // Cal = floor(0.04096 / (Current_LSB * Rshunt))
                current_lsb_A_ = imax_ / 32768.0;
                if (current_lsb_A_ <= 0.0)
                        current_lsb_A_ = 1e-6; // guard
                double cal_f = 0.04096 / (current_lsb_A_ * rshunt_);
                if (cal_f < 1.0)
                        cal_f = 1.0;
                if (cal_f > 65535.)
                        cal_f = 65535.;
                cal_ = static_cast<uint16_t>(std::floor(cal_f));
                power_lsb_W_ = 20.0 * current_lsb_A_;

                if (!writeReg16_(REG_CAL, cal_)) {
                        std::cerr << "INA219: write CAL failed\n";
                        return false;
                }

                initialized_ = true;
                return true;
        }

        bool INA219BatteryMonitor::writeReg16_(uint8_t reg, uint16_t value_be)
        {
                // value_be is already big-endian (msb first) for the bus write
                uint8_t buf[3] = { reg,
                        static_cast<uint8_t>((value_be >> 8) & 0xFF),
                        static_cast<uint8_t>( value_be       & 0xFF) };

                struct i2c_msg msg{};
                msg.addr = addr_;
                msg.flags = 0; // write
                msg.len = static_cast<__u16>(sizeof(buf));
                msg.buf = reinterpret_cast<__u8*>(buf);

                struct i2c_rdwr_ioctl_data pkt{};
                pkt.msgs = &msg;
                pkt.nmsgs = 1;

                if (ioctl(fd_, I2C_RDWR, &pkt) < 0) {
                        std::cerr << "INA219: I2C write fail: "
                                  << std::strerror(errno) << "\n";
                        return false;
                }
                return true;
        }

        bool INA219BatteryMonitor::readReg16_(uint8_t reg, uint16_t& out_be)
        {
                uint8_t regbuf = reg;
                uint8_t rbuf[2] = {0,0};
                struct i2c_msg msgs[2]{};

                msgs[0].addr = addr_;
                msgs[0].flags = 0; // write register pointer
                msgs[0].len = 1;
                msgs[0].buf = reinterpret_cast<__u8*>(&regbuf);

                msgs[1].addr = addr_;
                msgs[1].flags = I2C_M_RD; // read 2 bytes
                msgs[1].len = 2;
                msgs[1].buf = reinterpret_cast<__u8*>(rbuf);

                struct i2c_rdwr_ioctl_data pkt{};
                pkt.msgs = msgs;
                pkt.nmsgs = 2;

                if (ioctl(fd_, I2C_RDWR, &pkt) < 0) {
                        std::cerr << "INA219: I2C read fail: "
                                  << std::strerror(errno) << "\n";
                        return false;
                }
                out_be = (uint16_t) (rbuf[0] << 8 | rbuf[1]);
                return true;
        }

}
