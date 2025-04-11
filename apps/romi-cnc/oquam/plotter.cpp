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
#include "v3.h"
#include "oquam/plotter.h"

namespace romi {

        plotter::plotter() :
        buffer_(), w_(), h_(),scale_(), d_(), L_(), xy_(), xz_(), yz_(){
        }

        int plotter::plot_open()
        {
                long w = lround(scale_ * w_ + 0.5);
                long h = lround(scale_ * h_ + 0.5);
                buffer_.clear();
                buffer_.printf(("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                               "<svg xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
                               "  xmlns=\"http://www.w3.org/2000/svg\"\n"
                               "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
                               "  xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"
                               "  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
                               "  version=\"1.0\" width=\"%dpx\" height=\"%dpx\">\n"),
                              w, h);

            buffer_.printf(("  <sodipodi:namedview id=\"namedview\" showgrid=\"false\">\n"
                               "    <inkscape:grid type=\"xygrid\" id=\"zgrid\" \n"
                               "        spacingx=\"0.1\" spacingy=\"0.1\" empspacing=\"10\" />\n"
                               "  </sodipodi:namedview>\n"));
            buffer_.printf("  <g transform=\"matrix(%f,0,0,-%f,0,%d)\"\n"
                              "    inkscape:groupmode=\"layer\"\n"
                              "    inkscape:label=\"graphs\"\n"
                              "    id=\"graphs\">\n",
                              scale_, scale_, h);
                return 0;
        }

        int plotter::plot_close()
        {
                buffer_.printf("  </g>\n");
                buffer_.printf( "</svg>\n");
//                membuf_append_zero(buffer);
                return 0;
        }

        void plotter::print_moveto(double x, double y)
        {
                buffer_.printf( "M %f,%f ", x, y);
        }

        void plotter::print_lineto(double x, double y)
        {
                buffer_.printf( "L %f,%f ", x, y);
        }

        void plotter::print_line(double x0, double y0, double x1, double y1, const char *color)
        {
                buffer_.printf( "        <path d=\"");
                print_moveto( x0, y0);
                print_lineto( x1, y1);
                buffer_.printf( "\" id=\"path\" style=\"fill:none;stroke:%s;"
                              "stroke-width:0.001;stroke-linecap:butt;"
                              "stroke-linejoin:miter;stroke-miterlimit:4;"
                              "stroke-opacity:1;stroke-dasharray:none\" />\n",
                              color);
        }

        void plotter::print_rect(double x, double y, double w, double h)
        {
                buffer_.printf(
                              "      <rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" "
                              "style=\"fill:none;stroke:#cecece;"
                              "stroke-width:0.002;stroke-linecap:butt;"
                              "stroke-linejoin:miter;stroke-miterlimit:4;"
                              "stroke-opacity:1;stroke-dasharray:none\" />\n",
                              x, y, w, h);
        }

        void plotter::print_text(const char *s, double x, double y)
        {
                buffer_.printf(
                              "      <g transform=\"matrix(1,0,0,-1,0,%f)\" >"
                              "<text x=\"%f\" y=\"%f\" "
                              "style=\"font-weight:300;"
                              "font-size:0.03px;"
                              "font-family:'serif';"
                              "font-style:italic;"
                              "fill:#000000;"
                              "text-anchor:end;"
                              "text-align:end;"
                              "fill-opacity:1;\" >"
                              "%s</text></g>\n",
                              2 * y, x, y, s);
        }

        // REFACTOR. INLINE FUNCTIONS WITH MEANINGFUL NAMES!
#define _X(_r, _v) (((_r).x1 - (_r).x0 != 0.0)? (_r).w * ((_v) - (_r).x0) / ((_r).x1 - (_r).x0) : (_r).x0)
#define _Y(_r, _v) (((_r).y1 - (_r).y0 != 0.0)? (_r).h * ((_v) - (_r).y0) / ((_r).y1 - (_r).y0) : (_r).y0)

        
        void plotter::print_atdc_ij(SmoothPath& script, int i, int j, rect_t& r)
        {
                buffer_.printf( "    <g transform=\"translate(%f %f)\">\n",
                              r.x, r.y);
        
                for (size_t k = 0; k < script.count_atdc(); k++) {
                        ATDC& t = script.get_atdc(k);
                        print_line(_X(r, t.accelerate.p0[i]), _Y(r, t.accelerate.p0[j]),
                                   _X(r, t.accelerate.p1[i]), _Y(r, t.accelerate.p1[j]),
                                   "#00ff00");
                        
                        print_line(_X(r, t.travel.p0[i]), _Y(r, t.travel.p0[j]),
                                   _X(r, t.travel.p1[i]), _Y(r, t.travel.p1[j]),
                                   "#ffff00");
                        
                        print_line(_X(r, t.decelerate.p0[i]), _Y(r, t.decelerate.p0[j]),
                                   _X(r, t.decelerate.p1[i]), _Y(r, t.decelerate.p1[j]),
                                   "#ff0000");
                
                        buffer_.printf( "        <path d=\"");
                        print_moveto(_X(r, t.curve.p0[i]), _Y(r, t.curve.p0[j]));

                        int ms = (int) (1000.0 * t.curve.duration);
                        int n = ms / 10;

                        // CHECK!
                        for (int k = 1; k < n; k++) {
                                double x[3];
                                double dt = k * 0.010;
                                t.curve.get_position_at(dt, x);
                                print_lineto(_X(r, x[i]), _Y(r, x[j]));
                        }
                
                        print_lineto(_X(r, t.curve.p1[i]), _Y(r, t.curve.p1[j]));
                        buffer_.printf( "\" id=\"path\" style=\"fill:none;stroke:#0000ce;"
                                      "stroke-width:0.001;stroke-linecap:butt;"
                                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                                      "stroke-opacity:1;stroke-dasharray:none\" />\n");
                }
        
                buffer_.printf( "    </g>\n");
        }

        void plotter::print_atdc(SmoothPath& script)
        {
                buffer_.printf(
                              "    <g inkscape:groupmode=\"layer\" "
                              "inkscape:label=\"atdc\" "
                              "id=\"atdc\">\n");
        
                print_atdc_ij(script, 0, 1, xy_);
                print_atdc_ij(script, 0, 2, xz_);
                print_atdc_ij(script, 1, 2, yz_);
        
                buffer_.printf( "    </g>\n");
        }

        void plotter::print_segment_ij(SmoothPath& script, int i, int j, rect_t& r)
        {
                buffer_.printf( "    <g transform=\"translate(%f %f)\">\n",
                              r.x, r.y);
                buffer_.printf( "        <path d=\"");
        
                for (size_t k = 0; k < script.count_segments(); k++) {
                        Segment& segment = script.get_segment(k);
                        if (k == 0)
                                print_moveto(_X(r, segment.p0[i]),
                                             _Y(r, segment.p0[j]));
                        else 
                                print_lineto(_X(r, segment.p0[i]),
                                             _Y(r, segment.p0[j]));
                        print_lineto(_X(r, segment.p1[i]),
                                     _Y(r, segment.p1[j]));
                }
        
                buffer_.printf(
                              "\" id=\"path\" style=\"fill:none;stroke:#000000;"
                              "stroke-width:0.001;stroke-linecap:butt;"
                              "stroke-linejoin:miter;stroke-miterlimit:4;"
                              "stroke-opacity:1;stroke-dasharray:none\" />\n");
                buffer_.printf( "    </g>\n");
        }

        void plotter::print_segments(SmoothPath& script)
        {
                buffer_.printf(
                              "    <g inkscape:groupmode=\"layer\" "
                              "inkscape:label=\"paths\" "
                              "id=\"paths\">\n");
        
                print_segment_ij(script, 0, 1, xy_);
                print_segment_ij(script, 0, 2, xz_);
                print_segment_ij(script, 1, 2, yz_);

                buffer_.printf( "    </g>\n");
        }

        void plotter::print_speed_i(SmoothPath& script,
                                  int i,
                                  double duration,
                                  const double *vm)
        {
                double xscale = v_[i].w / duration;
                double yscale = v_[i].h / vmax(vm);
                double t = 0.0;

                buffer_.printf( "    <g transform=\"translate(%f %f)\">\n", v_[i].x, v_[i].y);

                for (size_t k = 0; k < script.count_segments(); k++) {
                        Segment& segment = script.get_segment(k);
                        ATDC& atdc = script.get_atdc(k);

                        double t0 = t;
                        double t1 = t + atdc.accelerate.duration;
                        print_line(xscale * t0,
                                   yscale * atdc.accelerate.v0[i],
                                   xscale * t1,
                                   yscale * atdc.accelerate.v1[i],
                                   "#00ff00");
                
                        t0 = t1;
                        t1 += atdc.travel.duration;
                        print_line(xscale * t0,
                                   yscale * atdc.travel.v0[i],
                                   xscale * t1,
                                   yscale * atdc.travel.v1[i],
                                   "#ffff00");
                        
                        t0 = t1;
                        t1 += atdc.decelerate.duration;
                        print_line(xscale * t0,
                                   yscale * atdc.decelerate.v0[i],
                                   xscale * t1,
                                   yscale * atdc.decelerate.v1[i],
                                   "#ff0000");

                        t0 = t1;
                        t1 += atdc.curve.duration;
                        print_line(xscale * t0,
                                   yscale * atdc.curve.v0[i],
                                   xscale * t1,
                                   yscale * atdc.curve.v1[i],
                                   "#0000ce");
                
                        print_line(xscale * t,
                                   yscale * segment.v[i],
                                   xscale * t1,
                                   yscale * segment.v[i],
                                   "#000000");
                        t = t1;
                }
                buffer_.printf( "    </g>\n");
        }

        void plotter::print_speeds(SmoothPath& script,
                                 double duration,
                                 const double *vmax)
        {
                for (int i = 0; i < 3; i++)
                        print_speed_i(script, i, duration, vmax);
        }

        void plotter::print_acceleration_i(SmoothPath& script,
                                         int i,
                                         double duration,
                                         const double *amax)
        {
                double xscale = a_[i].w / duration;
                double yscale = a_[i].h / vmax(amax);
                double t = 0.0;

                buffer_.printf( "    <g transform=\"translate(%f %f)\">\n", a_[i].x, a_[i].y);

                // curve
                for (size_t k = 0; k < script.count_segments(); k++) {
                        //Segment& segment = script.get_segment(k);
                        ATDC& atdc = script.get_atdc(k);
                        double t0 = t;
                        double t1 = t + atdc.accelerate.duration;
                        print_line(xscale * t0,
                                   yscale * atdc.accelerate.a[i],
                                   xscale * t1,
                                   yscale * atdc.accelerate.a[i],
                                   "#00ff00");
                
                        t0 = t1;
                        t1 += atdc.travel.duration;
                        /* print_line(*/
                        /*            xscale * t0, */
                        /*            yscale * atdc.travel.a_[i], */
                        /*            xscale * t1, */
                        /*            yscale * atdc.travel.a_[i], */
                        /*            "#ffff00"); */
                        
                        t0 = t1;
                        t1 += atdc.decelerate.duration;
                        print_line(xscale * t0,
                                   yscale * atdc.decelerate.a[i],
                                   xscale * t1,
                                   yscale * atdc.decelerate.a[i],
                                   "#ff0000");

                        t0 = t1;
                        t1 += atdc.curve.duration;
                        print_line(xscale * t0,
                                   yscale * atdc.curve.a[i],
                                   xscale * t1,
                                   yscale * atdc.curve.a[i],
                                   "#0000ce");
                        t = t1;
                }
                buffer_.printf( "    </g>\n");
        }

        void plotter::print_accelerations(SmoothPath& script,
                                        double duration,
                                        const double *amax)
        {
                for (int i = 0; i < 3; i++)
                        print_acceleration_i(script, i, duration, amax);
        }

        void plotter::print_slices_speed_i(SmoothPath& script, int i,
                                         double duration, const double *vm)
        {
                double xscale = v_[i].w / duration;
                double yscale = v_[i].h / vmax(vm);
        
                buffer_.printf("    <g transform=\"translate(%f %f)\">\n", v_[i].x, v_[i].y);

                for (size_t k = 0; k < script.count_slices(); k++) {
                        Section& section = script.get_slice(k);
                        print_line(xscale * section.start_time,
                                   yscale * section.v0[i],
                                   xscale * section.end_time(),
                                   yscale * section.v1[i], 
                                   "#ff00ff");
                }
        
                buffer_.printf( "    </g>\n");
        }

        void plotter::print_slices_ij(SmoothPath& script, int i, int j, rect_t& r)
        {
                buffer_.printf( "    <g transform=\"translate(%f %f)\">\n",
                              r.x, r.y);

                for (size_t k = 0; k < script.count_slices(); k++) {
                        Section& section = script.get_slice(k);
                        print_line(_X(r, section.p0[i]), _Y(r, section.p0[j]),
                                   _X(r, section.p1[i]), _Y(r, section.p1[j]),
                                   "#ff00ff");
                }
        
                buffer_.printf( "    </g>\n");
        }

        void plotter::print_slices(SmoothPath& script, double duration, const double *vm)
        {
                buffer_.printf(
                              "    <g inkscape:groupmode=\"layer\" "
                              "inkscape:label=\"slices\" "
                              "id=\"slices\">\n");

                print_slices_ij(script, 0, 1, xy_);
                print_slices_speed_i(script, 0, duration, vm);
                print_slices_speed_i(script, 1, duration, vm);
                print_slices_speed_i(script, 2, duration, vm);

                buffer_.printf( "    </g>\n");
        }

        void plotter::print_axes()
        {
                buffer_.printf(
                              "    <g inkscape:groupmode=\"layer\" "
                              "inkscape:label=\"axes\" "
                              "id=\"axes\">\n");
        
                // xy
                print_rect(xy_.x, xy_.y, xy_.w, xy_.h);
                print_text("x", xy_.x + xy_.w / 2, xy_.y - 0.3 * d_);
                print_text("y", xy_.x - 0.15 * d_, xy_.y + xy_.h / 2);

                // xz
                print_rect(xz_.x, xz_.y, xz_.w, xz_.h);
                print_text("x", xz_.x + xz_.w / 2, xz_.y - 0.3 * d_);
                print_text("z", xz_.x - 0.15 * d_, xz_.y + xz_.h / 2);

                // yz
                print_rect(yz_.x, yz_.y, yz_.w, yz_.h);
                print_text("y", yz_.x + yz_.w / 2, yz_.y - 0.3 * d_);
                print_text("z", yz_.x - 0.15 * d_, yz_.y + yz_.h / 2);

                // vx
                print_line(v_[0].x, v_[0].y,
                           v_[0].x + v_[0].w, v_[0].y,
                           "#cecece");
        
                print_line(v_[0].x, v_[0].y - v_[0].h,
                           v_[0].x, v_[0].y + v_[0].h,
                           "#cecece");

                print_text("vx", v_[0].x - 0.15 * d_, v_[0].y);
        
                // vy
                print_line(v_[1].x, v_[1].y,
                           v_[1].x + v_[1].w, v_[1].y,
                           "#cecece");
        
                print_line(v_[1].x, v_[1].y - v_[1].h,
                           v_[1].x, v_[1].y + v_[1].h,
                           "#cecece");
        
                print_text("vy", v_[1].x - 0.15 * d_, v_[1].y);
        
                // vz
                print_line(v_[2].x, v_[2].y,
                           v_[2].x + v_[2].w, v_[2].y,
                           "#cecece");
        
                print_line(v_[2].x, v_[2].y - v_[2].h,
                           v_[2].x, v_[2].y + v_[2].h,
                           "#cecece");

                print_text("vz", v_[2].x - 0.15 * d_, v_[2].y);
        
                // ax
                print_line(a_[0].x, a_[0].y,
                           a_[0].x + a_[0].w, a_[0].y,
                           "#cecece");
        
                print_line(a_[0].x, a_[0].y - a_[0].h,
                           a_[0].x, a_[0].y + a_[0].h,
                           "#cecece");
        
                print_text("ax", a_[0].x - 0.15 * d_, a_[0].y);
        
                // ay
                print_line(
                           a_[1].x, a_[1].y,
                           a_[1].x + a_[1].w, a_[1].y,
                           "#cecece");
        
                print_line(
                           a_[1].x, a_[1].y - a_[1].h,
                           a_[1].x, a_[1].y + a_[1].h,
                           "#cecece");
        
                print_text("ay", a_[1].x - 0.15 * d_, a_[1].y);
        
                // az
                print_line(
                           a_[2].x, a_[2].y,
                           a_[2].x + a_[2].w, a_[2].y,
                           "#cecece");
        
                print_line(
                           a_[2].x, a_[2].y - a_[2].h,
                           a_[2].x, a_[2].y + a_[2].h,
                           "#cecece");
        
                print_text("az", a_[2].x - 0.15 * d_, a_[2].y);
        
                buffer_.printf( "    </g>\n");
        }

        void plotter::plot_init_dimensions(const double *xmin, const double *xmax)
        {
                d_ = 0.1;

                double dx[3];
                vsub(dx, xmax, xmin);
                L_ = vmax(dx);
        
                w_ = (4 * d_
                           + 2 * (xmax[0] - xmin[0])
                           + (xmax[1] - xmin[1]));
        
                h_ = 3 * d_ + vmax(dx) + 0.7;
                scale_ = 1000.0;
        }

        void plotter::plot_init_xy(const double *xmin, const double *xmax)
        {
                xy_.x = d_;
                xy_.y = h_ - d_ - L_;
                xy_.w = xmax[0] - xmin[0];
                xy_.h = xmax[1] - xmin[1];
                xy_.x0 = xmin[0];
                xy_.x1 = xmax[0];
                xy_.y0 = xmin[1];
                xy_.y1 = xmax[1];
        }

        void plotter::plot_init_xz(const double *xmin, const double *xmax)
        {
                xz_.x = xy_.x + xy_.w + d_;
                xz_.y = xy_.y;
                xz_.w = xmax[0] - xmin[0];
                xz_.h = xmax[2] - xmin[2];
                xz_.x0 = xmin[0];
                xz_.x1 = xmax[0];
                xz_.y0 = xmin[2];
                xz_.y1 = xmax[2];
        }
        
        void plotter::plot_init_yz(const double *xmin, const double *xmax)
        {
                yz_.x = xz_.x + xz_.w + d_;
                yz_.y = xy_.y;
                yz_.w = xmax[1] - xmin[1];
                yz_.h = xmax[2] - xmin[2];
                yz_.x0 = xmin[1];
                yz_.x1 = xmax[1];
                yz_.y0 = xmin[2];
                yz_.y1 = xmax[2];
        }
        
        void plotter::plot_init_v()
        {
                v_[0].x = d_;
                v_[0].y = 7.5 * d_;
                v_[0].w = w_ - 2 * d_;
                v_[0].h = 0.9 * d_ / 2.0;
        
                v_[1].x = d_;
                v_[1].y = 6.5 * d_;
                v_[1].w = w_ - 2 * d_;
                v_[1].h = 0.9 * d_ / 2.0;
                
                v_[2].x = d_;
                v_[2].y = 5.5 * d_;
                v_[2].w = w_ - 2 * d_;
                v_[2].h = 0.9 * d_ / 2.0;
        }
        
        void plotter::plot_init_a()
        {
                a_[0].x = d_;
                a_[0].y = 3.5 * d_;
                a_[0].w = w_ - 2 * d_;
                a_[0].h = 0.9 * d_ / 2.0;
        
                a_[1].x = d_;
                a_[1].y = 2.5 * d_;
                a_[1].w = w_ - 2 * d_;
                a_[1].h = 0.9 * d_ / 2.0;
        
                a_[2].x = d_;
                a_[2].y = 1.5 * d_;
                a_[2].w = w_ - 2 * d_;
                a_[2].h = 0.9 * d_ / 2.0;
        }
        
        void plotter::plot_init(const double *xmin, const double *xmax)
        {
                plot_init_dimensions(xmin, xmax);
                plot_init_xy(xmin, xmax);
                plot_init_xz(xmin, xmax);
                plot_init_yz(xmin, xmax);
                plot_init_v();
                plot_init_a();
        }
        
        void plotter::plot_draw_graphs(SmoothPath& script, const double *vmax_, const double *amax)
        {
                double duration = script.get_duration();
                if (duration > 0) {
                        print_segments(script);
                        print_atdc(script);
                        print_slices(script, duration, vmax_);
                        print_speeds(script, duration, vmax_);
                        print_accelerations(script, duration, amax);
                }
        }

    rcom::MemBuffer plotter::buffer() {
        return buffer_;
    }


    rcom::MemBuffer plot_to_mem(SmoothPath& script,
                              const double *xmin, const double *xmax,
                              const double *vmax_, const double *amax)
        {
                plotter plot;
                plot.plot_init(xmin, xmax);
                plot.plot_open();
                plot.print_axes();
                plot.plot_draw_graphs(script, vmax_, amax);
                plot.plot_close();

                return plot.buffer();
        }

        rcom::MemBuffer plot_to_mem(SmoothPath& script, CNCRange& range,
                              const double *vmax, const double *amax)
        {
                v3 min = range.min();
                v3 max = range.max();
                return plot_to_mem(script, min.values(), max.values(), vmax, amax);
        }
}
