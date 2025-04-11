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

#include "oquam/print.h"

namespace romi {

        void indent_text(rcom::MemBuffer& text, int size)
        {
                for (int i = 0; i < size; i++)
                        text.put(' ');
        }

        // REFACTOR. Shouldn't be making ad-hoc JSON. Use json Object.
        void print(Section& section, rcom::MemBuffer& text, int indent)
        {
                indent_text(text, indent);
                text.printf("{\n");
                
                indent_text(text, indent+2);
                text.printf("\"start-time\": %0.6f,\n",
                              section.start_time);
                
                indent_text(text, indent+2);
                text.printf("\"duration\": %0.6f,\n",
                              section.duration);
                
                indent_text(text, indent+2);
                text.printf("\"p0\": [%0.4f, %0.4f, %0.4f],\n",
                              section.p0[0], section.p0[1], section.p0[2]);
                
                indent_text(text, indent+2);
                text.printf("\"p1\": [%0.4f, %0.4f, %0.4f],\n",
                              section.p1[0], section.p1[1], section.p1[2]);
                
                indent_text(text, indent+2);
                text.printf("\"v0\": [%0.3f, %0.3f, %0.3f],\n",
                              section.v0[0], section.v0[1], section.v0[2]);
                
                indent_text(text, indent+2);
                text.printf("\"v1\": [%0.3f, %0.3f, %0.3f],\n",
                              section.v1[0], section.v1[1], section.v1[2]);
                
                indent_text(text, indent+2);
                text.printf("\"a\": [%0.3f, %0.3f, %0.3f]\n",
                              section.a[0], section.a[1], section.a[2]);
                
                indent_text(text, indent);
                text.printf( "}");
        }
        
        void print(Section& section)
        {
                rcom::MemBuffer text;
                print(section, text);
                printf("%s\n", text.tostring().c_str());
        }

        void print(ATDC& atdc, rcom::MemBuffer& text, int indent)
        {
            indent_text(text, indent);
            text.printf("{\n");

            indent_text(text, indent+2);
            text.printf("\"accelerate\":\n");
            print(atdc.accelerate, text, indent+2);
            text.printf(",\n");

            indent_text(text, indent+2);
            text.printf("\"travel\":\n");
            print(atdc.travel, text, indent+2);
            text.printf(",\n");

            indent_text(text, indent+2);
            text.printf("\"decelerate\":\n");
            print(atdc.decelerate, text, indent+2);
            text.printf(",\n");

            indent_text(text, indent+2);
            text.printf("\"curve\":\n");
            print(atdc.curve, text, indent+2);
            text.printf("\n");

            indent_text(text, indent);
            text.printf("}");
        }

        void print(ATDC& atdc)
        {
                rcom::MemBuffer buf;
                print(atdc, buf);
                printf("%s\n", buf.tostring().c_str());
        }

        void print(Move& move, rcom::MemBuffer& text)
        {
            text.printf(R"({"x": %.5f, "y": %.5f, "z": %.5f, "v": %.5f})",
                              move.p.x(), move.p.y(), move.p.z(), move.v);
        }
        
        void print_moves(SmoothPath& script, rcom::MemBuffer& text)
        {
                text.printf("  \"moves\": [\n");
                for (size_t i = 0; i < script.count_moves(); i++) {
                        indent_text(text, 4);
                        print(script.get_move(i), text);
                        if (i == script.count_moves() - 1) 
                                text.printf("\n");
                        else 
                                text.printf(",\n");
                }
                text.printf("  ],\n");
        }

        void print(Segment& segment, rcom::MemBuffer& text, int indent)
        {
                indent_text(text, indent);
                text.printf("{\n");
                
                indent_text(text, indent+2);
                text.printf("\"p0\": [%0.4f, %0.4f, %0.4f],\n",
                              segment.p0[0], segment.p0[1], segment.p0[2]);
                
                indent_text(text, indent+2);
                text.printf("\"p1\": [%0.4f, %0.4f, %0.4f],\n",
                              segment.p1[0], segment.p1[1], segment.p1[2]);
                
                indent_text(text, indent+2);
                text.printf("\"v\": [%0.3f, %0.3f, %0.3f]\n",
                              segment.v[0], segment.v[1], segment.v[2]);
                
                indent_text(text, indent);
                text.printf("}");
        }

        void print_segments(SmoothPath& script, rcom::MemBuffer& text)
        {
                text.printf("  \"segments\": [\n");
                for (size_t i = 0; i < script.count_segments(); i++) {
                        Segment& segment = script.get_segment(i);
                        print(segment, text, 4);
                        if (i < script.count_segments() - 1) 
                                text.printf(",\n");
                        else 
                                text.printf("\n");
                }
                text.printf("  ],\n");
        }
        
        void print_atdc(SmoothPath& script, rcom::MemBuffer& text)
        {
                text.printf("  \"atdc\": [\n");
                for (size_t i = 0; i < script.count_atdc(); i++) {
                        print(script.get_atdc(i), text, 4);
                        if (i < script.count_atdc() - 1) 
                                text.printf(",\n");
                        else 
                                text.printf("\n");
                }
                text.printf("  ]");
        }
        
        void print_slices(SmoothPath& script, rcom::MemBuffer& text)
        {
                text.printf("  \"slices\": [\n");
                for (size_t k = 0; k < script.count_slices(); k++) {
                        print(script.get_slice(k), text, 4);
                        if (k == script.count_slices() - 1) 
                                text.printf("\n");
                        else 
                                text.printf(",\n");
                }
                text.printf("  ]\n");
        }

        void print(SmoothPath& script, rcom::MemBuffer& text, bool include_slices)
        {
                text.printf("{\n");
                
                print_moves(script, text);
                
                print_segments(script, text);
                
                print_atdc(script, text);
                
                if (include_slices) {
                        text.printf(",\n");
                        print_slices(script, text);
                } else {
                        text.printf("\n");
                }
                text.printf("}\n");
        }

        void print(SmoothPath& script, bool include_slices)
        {
                rcom::MemBuffer buf;
                print(script, buf, include_slices);
                printf("%s\n", buf.tostring().c_str());
        }
}



