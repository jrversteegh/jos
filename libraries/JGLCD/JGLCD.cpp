/*
  JLCD.cpp - Graphic LCD library (KS0108) for JOS
  Copyright (c) 2012 Jaap Versteegh.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "JGLCD.h"

namespace JOS {

void GLCDTask::write_data(const byte value)
{
}

virtual boolean GInit::run()
{
}

virtual boolean GDisplay::run()
{
}

virtual GLCD::boolean write(const byte* data, int size)
{
}

void GLCD::clear();
{
}

// Graphics interface
void GLCD::set_fg_color(const uint8_t color)
{
}

void GLCD::set_bg_color(const uint8_t color)
{
}

void GLCD::dot(const uint8_t x, const uint8_t y, const boolean clear=false)
{
}

void GLCD::go_to(const uint8_t x, const uint8_t y)
{
}

void GLCD::line_to(const uint8_t x, const uint8_t y)
{
}

void GLCD::line(const uint8_t x1, const uint8_t y1,
          const uint8_t x2, const uint8_t y2)
{
}

void GLCD::vline(const uint8_t x, const uint8_t y, const uint8_t l)
{
}

void GLCD::hline(const uint8_t x, const uint8_t y, const uint8_t l)
{
}

void GLCD::circle(const uint8_t x, const uint8_t y, const uint8_t r, 
            const boolean fill=false)
{
}

void GLCD::rect(const uint8_t x, const uint8_t y,
          const uint8_t w, const uint8_t h,
          const boolean fill=false)
{
}

void GLCD::clear(const uint8_t x, const uint8_t y,
           const uint8_t w, const uint8_t h)
{
}
void GLCD::bitmap(const uint8_t x, const uint8_t y,
            const uint8_t* data)
{
}

} // namespace JOS
