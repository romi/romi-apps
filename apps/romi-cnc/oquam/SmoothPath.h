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

#ifndef _OQUAM_SMOOTH_PATH_H_
#define _OQUAM_SMOOTH_PATH_H_

#include <vector>

#include "v3.h"

#include "oquam/Section.h"
#include "oquam/ATDC.h"

namespace romi {
        
        struct Move {
                v3 p;
                double v;

                Move(double x, double y, double z, double speed) : p(x,y,z), v(speed){
                        v = speed;
                }
                
                Move(v3& p_, double speed) : p(p_), v(speed){
                        p = p_;
                        v = speed;
                }
        };

        /** A segment is a data structure introduced for
         *  convenience. It converts the data of the move requests
         *  into vectors. It is used to build up the ATDC list.
         */
        struct Segment {
                double p0[3];
                double p1[3];
                double v[3];
                
                //Section section;
                
                Segment *prev;
                Segment *next;

                Segment() : prev(nullptr), next(nullptr) {}

                double *displacement(double *d) {
                        vsub(d, p1, p0);
                        return d;
                }

                double length() {
                        double d[3];
                        displacement(d);
                        return vnorm(d);
                }
                
                double *direction(double *d) {
                        displacement(d);
                        normalize(d, d);
                        return d;
                }

        };
        
        class SmoothPath
        {
        protected:
                
                /* The list of moves given by the user. */
                std::vector<Move> _moves;
                
                /* The list of segments is an intermediate
                 * representation of the move actions to facilitate to
                 * computation of the ADTC below. */
                std::vector<Segment> _segments;
                
                /* The list of move actions rewitten as a list of
                 * Accelerate-Travel-Decelerate-Curve sections
                 * (ATDC). */
                std::vector<ATDC> _atdc;

                /* The smooth curve described by the list of ATDCs is
                 * sliced into a long list of short sections with
                 * constant speeds. */
                std::vector<Section> _slices;
                
                v3 _start_position;
                v3 _current_position;
                
        public:
                                
                SmoothPath(v3 start_position);
                virtual ~SmoothPath() = default;
                
                /* Move to absolute position (x,y,z) in meters at an
                 * absolute speed of v, in m/s.
                 */
                void moveto(double x, double y, double z, double v);
                void moveto(v3 p, double v);

                void convert(const double *vmax,
                             const double *amax,
                             double deviation,
                             double slice_duration,
                             double max_slice_duration);

                size_t count_moves() {
                        return _moves.size();
                }
                
                Move& get_move(size_t index) {
                        return _moves[index];
                }

                size_t count_segments() {
                        return _segments.size();
                }
                
                Segment& get_segment(size_t index) {
                        return _segments[index];
                }

                size_t count_atdc() {
                        return _atdc.size();
                }
                
                ATDC& get_atdc(size_t index) {
                        return _atdc[index];
                }

                size_t count_slices() {
                        return _slices.size();
                }
                
                Section& get_slice(size_t index) {
                        return _slices[index];
                }

                double get_duration();
                v3 get_start_position();
                
        protected:
                
                void set_current_position(v3 p);
                void convert_moves_to_segments();
                void convert_segment(Move& move);
                void init_segment_positions(Segment& segment, Move& move);
                void init_segment_speed(Segment& segment, Move& move);
                void convert_segments_to_atdc(double d, const double *vmax,
                                              const double *amax);
                void copy_start_position_to_atdc();
                void check_max_speeds(const double *vmax);
                void check_max_speed(size_t index, const double *vmax);
                void create_atdc();
                void slice(double period, double max_slice_duration);
                void slice(ATDC& atdc, double period, double max_slice_duration);
                void compute_curves_and_speeds(double d, const double *amax);
                void compute_curve_and_speeds(size_t index, double d, const double *amax);
                void compute_curve(size_t index, double deviation, const double *amax);
                void no_curve(size_t index);                
                void default_curve(size_t index, double deviation, const double *amax);
                void initialize_next_acceleration(size_t index);
                bool update_speeds_if_needed(size_t index, const double *amax);

                // Returns true if the changes should be
                // back-propagated to the previous sections
                bool adjust_speeds(ATDC& t, const double *amax);
                
                bool update_curve_speed_if_needed(size_t index);
                void reduce_entry_speed(ATDC& t, const double *amax);
                void reduce_exit_speed(ATDC& t, const double *amax);
                void back_propagate_speed_change(size_t index, const double *amax);
                void assert_not_first(size_t index);
                void compute_accelerations(const double *amax);
                void compute_accelerations(size_t index, const double *amax);
                void update_start_times();
                double get_curve_speed_magnitude(Segment& s0, Segment& s1);
                void get_curve_speed_vector(Segment& s, double magnitude, double *w);
                void compute_curve_entry_point(Segment& s, double distance, double *p);
                void compute_curve_exit_point(Segment& s, double distance, double *p);
                double scale_speed(double am, double deviation, double speed);
                double scale_distance(double distance, Segment& s0, Segment& s1);
                double shortest_length(Segment& s0, Segment& s1);
                double required_acceleration_path_length(ATDC& t, const double *amax);
                double required_acceleration_path_length(double *v0, double *v1,
                                                         const double *amax);
                double required_acceleration_path_length(double *v0, double *v1,
                                                         double *d, const double *amax);
                double required_acceleration_path_length(double v0, double v1, double a);
                double maximum_entry_speed(double *displacement, double v1,
                                           const double *amax);
                double maximum_exit_speed(double *displacement, double v0,
                                          const double *amax);
                void assert_vmax(const double *vmax);
                void assert_amax(const double *amax);
                void assert_deviation(double deviation);
                void assert_slice_duration(double duration, double max_duration);
                bool has_next_segment(size_t index);
                bool has_next_atdc(size_t index);
        };
}

#endif // _OQUAM_SMOOTH_PATH_H_
