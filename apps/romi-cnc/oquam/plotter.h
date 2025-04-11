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

#ifndef _OQUAM_PLOTTER_H_
#define _OQUAM_PLOTTER_H_

#include <rcom/MemBuffer.h>
#include "api/CNCRange.h"
#include "oquam/SmoothPath.h"

namespace romi {

        typedef struct _rect_t {
                // the coordinates to the rectangle on the page
                double x, y, w, h;

                // the min and max coordinates of the data to be plotted
                double x0, x1;
                double y0, y1;
        } rect_t;


        class plotter {
        public:
                plotter();
                int plot_open();
                int plot_close();
                void print_axes();
                void plot_init(const double *xmin, const double *xmax);
                void plot_draw_graphs(SmoothPath& script, const double *vmax_, const double *amax);
                rcom::MemBuffer buffer();
        private:
                void print_moveto(double x, double y);
                void print_lineto(double x, double y);
                void print_line(double x0, double y0, double x1, double y1, const char *color);
                void print_rect(double x, double y, double w, double h);
                void print_text(const char *s, double x, double y);
                void print_atdc_ij(SmoothPath& script, int i, int j, rect_t& r);
                void print_atdc(SmoothPath& script);
                void print_segment_ij(SmoothPath& script, int i, int j, rect_t& r);
                void print_segments(SmoothPath& script);
                void print_speed_i(SmoothPath& script, int i, double duration, const double *vm);
                void print_speeds(SmoothPath& script, double duration, const double *vmax);
                void print_acceleration_i(SmoothPath& script, int i, double duration, const double *amax);
                void print_accelerations(SmoothPath& script, double duration, const double *amax);
                void print_slices_speed_i(SmoothPath& script, int i, double duration, const double *vm);
                void print_slices_ij(SmoothPath& script, int i, int j, rect_t& r);
                void print_slices(SmoothPath& script, double duration, const double *vm);
                void plot_init_dimensions(const double *xmin, const double *xmax);
                void plot_init_xy(const double *xmin, const double *xmax);
                void plot_init_xz(const double *xmin, const double *xmax);
                void plot_init_yz(const double *xmin, const double *xmax);
                void plot_init_v();
                void plot_init_a();

                rcom::MemBuffer buffer_;

                double w_;
                double h_;
                double scale_;

                double d_; // margin
                double L_; // max(xmax)

                rect_t xy_;
                rect_t xz_;
                rect_t yz_;

                rect_t v_[3];
                rect_t a_[3];

        };


        rcom::MemBuffer plot_to_mem(SmoothPath& script,
                                    CNCRange& range,
                                    const double *vmax,
                                    const double *amax);
}

#endif // _OQUAM_PLOTTER_H_
