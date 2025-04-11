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
#include <float.h>
#include <stdexcept>
#include <algorithm>
#include <math.h>

#include "v3.h"
#include "oquam/Helix.h"
#include "oquam/print.h"

namespace romi {

        HelixSection::HelixSection() : start_time_(0.0), duration_(0.0),
                         u0_(0.0), u1_(0.0),
                         vu0_(0.0), vu1_(0.0),
                         au_(0.0) {
        }
        
        HelixSection::HelixSection(double start, double duration,
                                   double u0, double u1, double v0, double v1,
                                   double a)
                : start_time_(start), duration_(duration),
                  u0_(u0), u1_(u1),
                  vu0_(v0), vu1_(v1),
                  au_(a) {
                assert_values();
        }
        
        void HelixSection::assert_values()
        {
                if (start_time_ < 0.0)
                        throw std::runtime_error("HelixSection: negative start time");
                if (duration_ < 0.0)
                        throw std::runtime_error("HelixSection: negative duration");
        }

        void HelixSection::accelerate(double v, double a)
        {
                start_time_ = 0.0;
                duration_ = v / a;
                if (duration_ < 0.0001)
                        duration_ = 0.0;                        
                u0_ = 0.0;
                u1_ = 0.5 * a * duration_ * duration_;
                vu0_ = 0.0;
                vu1_ = v;
                au_ = a;
        }

        void HelixSection::decelerate(double v, double a)
        {
                start_time_ = 0.0;
                duration_ = v / a;
                if (duration_ < 0.0001)
                        duration_ = 0.0;                        
                u0_ = 0.0;
                u1_ = v * duration_ - 0.5 * a * duration_ * duration_;
                vu0_ = v;
                vu1_ = 0.0;
                au_ = -a;
        }

        void HelixSection::travel(double len, double v)
        {
                start_time_ = 0.0;
                duration_ = len / v;
                if (duration_ < 0.0001)
                        duration_ = 0.0;                        
                u0_ = 0.0;
                u1_ = len;
                vu0_ = v;
                vu1_ = v;
                au_ = 0.0;
        }
        
        double HelixSection::get_position_at(double t)
        {
                return u0_ + vu0_ * t + 0.5 * au_ * t * t;
        }
        
        double HelixSection::get_speed_at(double t)
        {
                return vu0_ + au_ * t;
        }

        double HelixSection::get_acceleration()
        {
                return au_;
        }
        
        void HelixSection::slice(std::vector<HelixSection>& slices, double interval)
        {
                double offset = 0.0;
                double used_interval = interval;
        
                while (offset < duration_) {
                        double slice_duration = duration_ - offset;
                        if (slice_duration > used_interval) {
                                slice_duration = used_interval;
                                compute_slice(slices, offset, slice_duration);
                                offset += slice_duration;
                        } else {
                                last_slice(slices, offset);
                                offset = duration_;
                        }
                }
        }
        
        void HelixSection::compute_slice(std::vector<HelixSection>& slices,
                                         double offset, double slice_duration)
        {
                double t0 = start_time_ + offset;
                double t1 = t0 + slice_duration;
                double duration = t1 - t0;
                double u0 = get_position_at(offset);
                double u1 = get_position_at(offset + duration);
                double v0 = get_speed_at(offset);
                double v1 = get_speed_at(offset + duration);
                
                slices.push_back(HelixSection(t0, duration, u0, u1, v0, v1, au_));
        }
        
        void HelixSection::last_slice(std::vector<HelixSection>& slices, double offset)
        {
                double t0 = start_time_ + offset;
                double t1 = start_time_ + duration_;
                double duration = t1 - t0;
                double u0 = get_position_at(t0);
                double u1 = u1_;
                double v0 = get_speed_at(t0);
                double v1 = vu1_;
                
                slices.push_back(HelixSection(t0, duration, u0, u1, v0, v1, au_));
        }

        bool HelixSection::u_in_range(double u)
        {
                return (u0_ <= u && u < u1_);
        }

        bool HelixSection::time_in_range(double t)
        {
                return (start_time_ <= t && t < start_time_ + duration_);
        }

        void HelixSection::print()
        {
                r_debug("Section: start time: %f, duration: %f, "
                        "u0: %f, u1: %f, vu0: %f, vu1: %f, au: %f",
                        start_time_, duration_, u0_, u1_, vu0_, vu1_, au_);
        }
        
        Helix::Helix(double x0, double y0,
                     double xc, double yc, double alpha,
                     double z0, double z1,
                     double vxy, double axy,
                     double vz, double az)
                : xc_(xc), yc_(yc), z0_(z0), delta_z_(z1-z0), 
                  r_(0.0), alpha0_(0.0), delta_alpha_(alpha),
                  acceleration_(), travel_(), deceleration_()
        {
                r_ = distance(x0, y0, xc_, yc_);
                alpha0_ = angle(xc_, yc_, x0, y0);

                double vu = convert_speed(vxy, vz);
                double au = convert_acceleration(axy, az);
                double vt = compute_travel_speed(1.0, vu, au);
                
                acceleration_.accelerate(vt, au);
                deceleration_.decelerate(vt, au);

                double length = (1.0
                                 - (acceleration_.u1_ - acceleration_.u0_)
                                 - (deceleration_.u1_ - deceleration_.u0_));
                if (length > 0)
                        travel_.travel(length, vt);
                else
                        travel_.travel(0.0, vt);
                
                update_offsets();
        }

        double Helix::distance(double x0, double y0, double x1, double y1)
        {
                v3 a(x0, y0, 0.0);
                v3 b(x1, y1, 0.0);
                return a.dist(b);
        }

        double Helix::angle(double x0, double y0, double x1, double y1)
        {
                double dx = x1 - x0;
                double dy = y1 - y0;
                return atan2(dy, dx);
        }
        
        double Helix::convert_speed(double vxy, double vz)
        {
                double vuz;
                double vuxy;
                
                if (delta_z_ == 0.0)
                        vuz = 10e10;
                else
                        vuz = fabs(vz / delta_z_);
                
                if (r_ * delta_alpha_ == 0.0)
                        vuxy = 10e10;
                else
                        vuxy = fabs(vxy /(r_ * delta_alpha_));
                
                return std::min(vuz, vuxy);
        }
        
        double Helix::convert_acceleration(double axy, double az)
        {
                double auz;
                double auxy;
                
                if (delta_z_ == 0.0)
                        auz = 10e10;
                else
                        auz = fabs(az / delta_z_);;
                
                if (r_ * delta_alpha_ == 0.0)
                        auxy = 10e10;
                else
                        auxy = fabs(axy / (r_ * delta_alpha_));
                
                return std::min(auz, auxy);
        }
        
        double Helix::compute_travel_speed(double len, double v, double a)
        {
                double vm = compute_maximum_speed(len, a);
                if (vm < v)
                        v = vm;
                return v;
        }
        
        double Helix::compute_maximum_speed(double len, double a)
        {
                /*
                  The maximum speed that can be reached by accelerating and
                  decelerating. This can be found as follows:
        
                  dt is the time needed to accelerate to
                  maximum speed (vm) and the time needed to
                  decelerate.

                  dt = vm / a                              (1)
                   
                  dx0 = 0.5a.dt²                length traveled during acceleration 
                  dx1 = vm.dt - 0.5a.dt²        length traveled during deceleration

                  len = dx0 + dx1 
                  => len = 0.5a.dt² + vm.dt - 0.5a.dt²  (3)
                  => len = vm²/a                         (substituting 1 in 3) 
                  => vm = sqrt(a.len)
                */
                return sqrt(a * len);
        }
        
        void Helix::update_offsets()
        {
                travel_.start_time_ = acceleration_.start_time_ + acceleration_.duration_;
                travel_.u0_ += acceleration_.u1_;
                travel_.u1_ += acceleration_.u1_;
                
                deceleration_.start_time_ = travel_.start_time_ + travel_.duration_;
                deceleration_.u0_ += travel_.u1_;
                deceleration_.u1_ += travel_.u1_;
        }

        void Helix::slice(std::vector<Section>& slices, double interval)
        {
                std::vector<HelixSection> h_slices;
                slice_parametric_curve(h_slices, interval);
                map_positions(h_slices, slices);
        }

        void Helix::slice_parametric_curve(std::vector<HelixSection>& slices,
                                           double interval)
        {
                acceleration_.slice(slices, interval);
                travel_.slice(slices, interval);
                deceleration_.slice(slices, interval);
        }

        void Helix::map_positions(std::vector<HelixSection>& h_slices,
                                  std::vector<Section>& slices)
        {
                for (auto h_slice: h_slices) {
                        slices.push_back(map_position(h_slice));
                }
        }

        Section Helix::map_position(HelixSection slice)
        {
                // The acceleration is not constant during the
                // slice. The orientation of the acceleration changes
                // because the movement is along a circular curve. We
                // will assume that the movement takes place along a
                // straight line segment with zero acceleration.
                //
                // This is okay if the Section is used to generate the
                // move commands of the CNC because the conversion
                // only uses p0 and p1.
                //
                // If, however, the slices are used for a more precise
                // operation then this code should be adapted using
                // get_acceleration_at_u()..
                
                v3 p0 = get_position_at_u(slice.u0_);
                v3 p1 = get_position_at_u(slice.u1_);
                v3 v0 = get_speed_at_u(slice.u0_, slice.vu0_);
                v3 v1 = get_speed_at_u(slice.u1_, slice.vu1_);
                v3 a(0.0);
                
                return Section(slice.duration_, slice.start_time_, p0, p1, v0, v1, a);
        }
        
        v3 Helix::get_position_at_u(double u)
        {
                double alpha = alpha0_ + u * delta_alpha_;
                double x = xc_ + r_ * cos(alpha);
                double y = yc_ + r_ * sin(alpha);
                double z = z0_ + u * delta_z_;
                return v3(x, y, z);
        }
        
        v3 Helix::get_speed_at_u(double u, double vu)
        {
                double alpha = alpha0_ + u * delta_alpha_;
                double vx = -r_ * sin(alpha) * delta_alpha_ * vu;
                double vy = r_ * cos(alpha) * delta_alpha_ * vu;
                double vz = vu * delta_z_;
                return v3(vx, vy, vz);
        }
        
        v3 Helix::get_acceleration_at_u(double u, double vu, double au)
        {
                double alpha = alpha0_ + u * delta_alpha_;
                double ax = -r_ * delta_alpha_ * (sin(alpha) * au
                                            + cos(alpha) * delta_alpha_ * vu * vu);
                double ay = r_ * alpha * (cos(alpha) * au
                                          - sin(alpha) * delta_alpha_ * vu * vu);
                double az = au * delta_z_;
                return v3(ax, ay, az);
        }

        double Helix::get_u_position_at(double t)
        {
                double u;
                if (t < acceleration_.start_time_)
                        u = 0.0;
                
                else if (acceleration_.time_in_range(t))
                        u = acceleration_.get_position_at(t - acceleration_.start_time_);
                
                else if (travel_.time_in_range(t))
                        u = travel_.get_position_at(t - travel_.start_time_);
                
                else if (deceleration_.time_in_range(t))
                        u = deceleration_.get_position_at(t - deceleration_.start_time_);
                
                else
                        u = 1.0;
                
                return u;
        }
        
        double Helix::get_u_speed_at(double t)
        {
                double vu;
                if (t < acceleration_.start_time_)
                        vu = 0.0;
                
                else if (acceleration_.time_in_range(t))
                        vu = acceleration_.get_speed_at(t - acceleration_.start_time_);
                
                else if (travel_.time_in_range(t))
                        vu = travel_.get_speed_at(t - travel_.start_time_);
                
                else if (deceleration_.time_in_range(t))
                        vu = deceleration_.get_speed_at(t - deceleration_.start_time_);
                
                else
                        vu = 0.0;
                
                return vu;
        }
        
        double Helix::get_u_acceleration_at(double t)
        {
                double au;
                if (t < acceleration_.start_time_)
                        au = 0.0;
                
                else if (acceleration_.time_in_range(t))
                        au = acceleration_.get_acceleration();
                
                else if (travel_.time_in_range(t))
                        au = travel_.get_acceleration();
                
                else if (deceleration_.time_in_range(t))
                        au = deceleration_.get_acceleration();
                
                else
                        au = 0.0;
                
                return au;
        }

        v3 Helix::get_position_at(double t)
        {
                double u = get_u_position_at(t);
                return get_position_at_u(u);
        }
        
        v3 Helix::get_speed_at(double t)
        {
                double u = get_u_position_at(t);
                double vu = get_u_speed_at(t);
                return get_speed_at_u(u, vu);
        }

        double Helix::get_duration()
        {
                return deceleration_.start_time_ + deceleration_.duration_;
        }


        void Helix::print()
        {
                v3 from, to;
                from = get_position_at_u(acceleration_.u0_);
                to = get_position_at_u(acceleration_.u1_);
                r_debug("Acceleration: from (%.3f, %.3f, %.3f) to (%.3f, %.3f, %.3f)",
                        from.x(), from.y(), from.z(), to.x(), to.y(), to.z());
                acceleration_.print();
                
                from = get_position_at_u(travel_.u0_);
                to = get_position_at_u(travel_.u1_);
                r_debug("Travel: from (%.3f, %.3f, %.3f) to (%.3f, %.3f, %.3f)",
                        from.x(), from.y(), from.z(), to.x(), to.y(), to.z());
                travel_.print();

                from = get_position_at_u(deceleration_.u0_);
                to = get_position_at_u(deceleration_.u1_);
                r_debug("Deceleration: from (%.3f, %.3f, %.3f) to (%.3f, %.3f, %.3f)",
                        from.x(), from.y(), from.z(), to.x(), to.y(), to.z());
                deceleration_.print();
        }
}
