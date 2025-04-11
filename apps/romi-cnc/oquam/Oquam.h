/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef ROMI_OQUAM_H
#define ROMI_OQUAM_H


#include <mutex>

#include "api/ISession.h"
#include "api/ICNC.h"
#include "v3.h"
#include "oquam/ICNCController.h"
#include "oquam/SmoothPath.h" 
#include "oquam/OquamSettings.h" 

namespace romi {

        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;
        
        class Oquam : public ICNC
        {
        public:
                ICNCController& controller_;
                OquamSettings settings_;
                ISession& session_;
                std::mutex mutex_;
                bool store_script_;
                
        public:
                
                Oquam(ICNCController& controller,
                      OquamSettings& settings,
                      ISession& session);

                Oquam(const Oquam&) = delete;
                Oquam& operator=(const Oquam&) = delete;
                
                ~Oquam() override = default;
                
                // ICNC interface, See ICNC.h for more info
                bool moveto(double x, double y, double z, double relative_speed) override;
                // bool moveat(int16_t speed_x, int16_t speed_y, int16_t speed_z) override;
                bool travel(Path &path, double relative_speed) override;
                bool helix(double xc, double yc, double alpha, double z,
                           double relative_speed) override;
                bool spindle(double speed) override;
                uint8_t count_relays() override;
                bool set_relay(uint8_t index, bool value) override;
                bool homing() override;
                bool get_range(CNCRange &range) override;
                bool get_position(v3& position) override; 
                bool synchronize(double timeout_seconds) override;

                // IActivity interface
                bool pause_activity() override;
                bool continue_activity() override;
                bool reset_activity() override;

                // Power device interface
                bool power_up() override;
                bool power_down() override;
                bool stand_by() override;
                bool wake_up() override;

        protected:
                
                bool moveto_synchronized(double x, double y, double z, double rel_speed);
                bool do_moveto(double x, double y, double z, double rel_speed);
                v3 moveto_determine_xyz(double x, double y, double z);
                bool travel_synchronized(Path &path, double relative_speed);
                void do_travel(Path &path, double relative_speed);
                void convert_path_to_script(Path &path, double speed, SmoothPath& script); 
                void convert_script(SmoothPath& script, v3& vmax);
                void store_script(SmoothPath& script);
                void store_script_svg(SmoothPath &script);
                void store_script_json(SmoothPath &script);
                void check_script(SmoothPath& script, v3& vmax);
                void execute_script(SmoothPath& script);
                void execute_move(Section& section, int32_t *pos_steps);
                void wait_end_of_script(SmoothPath& script); 
                bool get_position(int32_t *position); 
                v3 assert_get_position(); 
                void assert_relative_speed(double relative_speed); 
                void assert_in_range(v3 p);

                bool is_zero(int16_t *params);
                void assert_move(int16_t *params);
                void assert_synchronize(double timeout);
                void convert_position_to_steps(const double *position, int32_t *steps);

                bool enable_driver();
                bool disable_driver();
                bool helix_synchronized(double xc, double yc, double alpha, double z,
                                        double relative_speed);
                void do_helix(double xc, double yc, double alpha, double z,
                              double relative_speed);

        private:
            bool position_changed_;
        };
}

#endif // ROMI_OQUAM_H
