/*
  JCls.h - Standard classes for JOS
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

#ifndef __JINTF_H__
#define __JINTF_H__

#include <wiring.h>
#include <JOS.h>

namespace JOS {

struct OStream {
  virtual boolean write(const byte*, int len) = 0;
  template<typename T> bool write(const T v) {
    return write((byte*)&v, sizeof(T));
  }
};

struct IStream {
  virtual int in_avail() = 0;
  virtual int read(byte*, int len) = 0;
  template<typename T> boolean read(T* v) {
    if (in_avail() >= sizeof(T)) {
      return read((byte*)&v, sizeof(T));
    }
  }
};

struct Stream: IStream, OStream {
};

struct Block {
  virtual byte& operator[] (int) = 0;
};

struct Array {
  virtual Block& operator[] (int) = 0;
};

} // namespace JOS


#endif

