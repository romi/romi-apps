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

#ifndef _OQUAM_HELIX_H_
#define _OQUAM_HELIX_H_

#include <vector>

#include "v3.h"
#include "oquam/Section.h"

namespace romi {

        struct HelixSection
        {
                double start_time_;
                double duration_;
                double u0_;
                double u1_;
                double vu0_;
                double vu1_;
                double au_;
                
                HelixSection();
                HelixSection(double start, double duration,
                             double u0, double u1, double v0, double v1,
                             double a);
                
                void accelerate(double v, double a);
                void decelerate(double v, double a);
                void travel(double len, double v);
                double get_position_at(double t);
                double get_speed_at(double t);
                double get_acceleration();
                void slice(std::vector<HelixSection>& slices, double interval);
                void compute_slice(std::vector<HelixSection>& slices,
                                   double offset, double slice_duration);
                void last_slice(std::vector<HelixSection>& slices, double offset);
                bool u_in_range(double u);
                bool time_in_range(double t);
                void assert_values();
                void print();
        };
                
        class Helix
        {
        protected:
                
                double xc_;
                double yc_;
                double z0_;
                double delta_z_;
                double r_;
                double alpha0_;
                double delta_alpha_;

                HelixSection acceleration_;
                HelixSection travel_;
                HelixSection deceleration_;
                
                double distance(double x0, double y0, double x1, double y1);
                double angle(double x0, double y0, double x1, double y1);
                double convert_speed(double vxy, double vz);
                double convert_acceleration(double axy, double az);
                double compute_travel_speed(double len, double v, double a);
                double compute_maximum_speed(double len, double a);
                void update_offsets();

                void slice_parametric_curve(std::vector<HelixSection>& slices,
                                            double interval);
                void map_positions(std::vector<HelixSection>& h_slices,
                                   std::vector<Section>& slices);
                Section map_position(HelixSection slice);

                double get_u_speed_at(double t);
                double get_u_acceleration_at(double t);
                v3 get_speed_at_u(double u, double vu);
                v3 get_acceleration_at_u(double u, double vu, double au);

        public:
                                
                Helix(double x0, double y0,
                      double xc, double yc, double alpha,
                      double z0, double z1,
                      double vxy, double axy,
                      double vz, double az);
                virtual ~Helix() = default;
                
                void slice(std::vector<Section>& slices, double interval);

                double get_u_position_at(double t);
                double get_duration();
                v3 get_position_at_u(double u);
                v3 get_position_at(double t);
                v3 get_speed_at(double t);
                void print();
        };
}

#endif // _OQUAM_HELIX_H_
