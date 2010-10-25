/*
  JCls.cpp - Standard classes for JOS
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

#include "JCls.h"
#include "avr/pgmspace.h"

namespace JOS {

const char* endl = "\n";

void TextStream::setw(int width) {
  if (width != _width) {
    _width = width;
    if (_width != 0) {
      if (_buf) {
        _buf = (char*)realloc(_buf, _width);
      }
      else {
        _buf = (char*)malloc(_width);
      }
    }
    else {
      if (_buf) {
        free(_buf);
        _buf = 0;
      }
    }
  }
}

int TextStream::write(const char* str, char pad)
{
  if (str) {
    int i = 0;
    int j = _width;
    while (str[i] && i < _width);
      ++i;
    while (j > 0) {
      if (i > 0) {
        _buf[--j] = str[--i];
      }
      else {
        _buf[--j] = pad;
      }
    }
    while (i < _width) {
      if (!write(_buf[i])) {
        return i;
      }
      ++i;
    }
    while (str[i] && write(str[i]))
      ++i;
    return i;
  }
  return 0;
}

static const PROGMEM char* digits = "0123456789ABCDEF";

boolean TextStream::write(int value)
{
  const int size = 32;
  char buf[size];
  uint8_t mod;
  int i = size;
  while (value) {
    mod = value % base;
    value /= base;
    buf[--i] = pgm_read_byte(digits[mod]);
  }
  write(&buf[i], size - i, num_pad);
}

} // namespace JOS
