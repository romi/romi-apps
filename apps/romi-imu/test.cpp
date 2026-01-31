#include <stdexcept>
#include <util/ClockAccessor.h>
#include <util/Logger.h>
#include "IMURT.h"
                
int main()
{
        try {
                std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
                romi::ClockAccessor::SetInstance(clock);
                romi::IMURT imu;
                double interval = imu.get_preferred_update_interval();
                
                printf("Interval %f\n", interval);

                while (1) {
                        romi::ClockAccessor::GetInstance()->sleep(interval);
                        if (imu.update()) {
                                quaternion_t q = imu.get_orientation();
                                vector_t angles = convert_quaternion_to_euler(q);
                                printf("%0.3f: %.3f° / %.3f° / %.3f°\n",
                                       romi::ClockAccessor::GetInstance()->time(),
                                       180.0 * angles.x / M_PI,
                                       180.0 * angles.y / M_PI,
                                       180.0 * angles.z / M_PI);
                        }
                }
                
        } catch (std::exception& e) {
                r_err("romi-imu: main.cpp: caught exception: %s", e.what());
                r_err("romi-imu: main.cpp: quitting");
        }
        return 0;
}

