/*
  JDbg.h - Main task library for JOS
  Copyright (c) 2010 Jaap Versteegh.  All right reserved.

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

#ifndef __JDBG_H__
#define __JDBG_H__

#ifdef DEBUG
#include "Print.h"
#define D_JOS(debug_str) JOS::debug.println(debug_str) 
#define J_ASSERT(condition, debug_str) \
  if (!(condition)) { \
    D_JOS(debug_str); \
    panic(); \
  }
#else
#define D_JOS(debug_str) 
#define J_ASSERT(condition, debug_str) 
#endif

namespace JOS {

#ifdef DEBUG

class Debug: public Print {
  void init_when_required();
public:
  // Return type changed in Arduino 1.0
#if defined(ARDUINO) && ARDUINO >= 100
  virtual size_t write(uint8_t c);
#else
  virtual void write(uint8_t c);
#endif
};

extern Debug debug;

#endif

}  // namespace JOS

#endif
