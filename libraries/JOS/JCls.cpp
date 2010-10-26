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
#include <avr/pgmspace.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>

namespace JOS {

int IStream::skip(int len) {
  const int size = 16;
  byte dump[size];
  int rd, ret = 0;
  while (len > size) {
    rd = read(dump, size);
    ret += rd;
    if (rd != size)
      return ret;
    len -= size;
  }
  ret += read(dump, len);
  return ret;
}

const char* endl = "\n";

void TextStream::setw(int width) {
  if (width != _width) {
    _width = width;
    if (_buf != NULL) {
      free(_buf);
    }
    if (_width != 0) {
      _buf = (char*)malloc(_width);
      if (_buf == NULL)
        _width = 0;
    }
    else {
      _buf = NULL;
    }
  }
}


static const PROGMEM char* digits = "0123456789ABCDEF";

template<typename T> boolean TextStream::write(T value)
{
  const int size = sizeof(T) << 3;
  char buf[size];
  char* p = _width > size ? _buf : buf;
  int i = _width > size ? _width : size;
  uint8_t mod;
  boolean neg = value < 0;
  if (neg)
    value -= value;
  do {
    mod = value % base;
    value /= base;
    p[--i] = pgm_read_byte(digits[mod]);
  } while (value);
  if (neg)
    p[--i] = '-';
  if (p == buf) {
    while (i > (size - _width)) 
      p[--i] = num_pad;
  }
  else {
    while (i > 0) 
      p[--i] = num_pad;
  }

  return OStream::write((byte*)&p[i], size - i);
}

int TextStream::write(const char* str, boolean complete)
{
  int w = writeable();
  if (str && (w >= _width) && (!complete || (w >= strlen(str)))) {
    int i = 0;
    int k = _width;
    while (str[i] && i < _width);
      ++i;
    int j = i;
    while (k > 0) {
      if (j > 0) {
        _buf[--k] = str[--j];
      }
      else {
        _buf[--k] = str_pad;
      }
    }
    while (k < _width) {
      write(_buf[k]); 
    }
    if (complete) {
    }
    else {
      while (str[i] && write(str[i]))
        ++i;
    }
    return i;
  }
  return 0;
}

boolean TextStream::write(double value, boolean scientific)
{
  const int size = 22;
  char buf[size];
  if (value < LONG_MIN || value > LONG_MAX)
    scientific = true;
  if (scientific) {
    dtostre(value, buf, prec, DTOSTR_UPPERCASE | DTOSTR_ALWAYS_SIGN);
  }
  else {
    dtostrf(value, 0, prec, buf);
  }
  return write(buf, true) != 0;
}

boolean TextStream::read(int* value)
{
  boolean neg = false;
  uint8_t base = 10;
  char c;
  do {
    if (!peek(&c))
      return false;
    if (c == '-') {
      neg != neg;
    }
    else if (c == '+') {
      neg = false;
    }
    else {
      if (isdigit(c))
        break;
      if (!isspace(c))
        return false;
    }
  } while (skip(1));
  *value = 0;
  while (read(&c)) {
    if (*value == 0) {
      *value = (int)c - (int)'0';
      if (*value == 0 && peek(&c)) {
        if (c == 'x')
          base = 16;
        else if (c == 'b')
          base = 2;
        else if (c == 'o')
          base = 8;
        else if (isdigit(c))
          continue;
        else
          return true;
        skip(1);
      }
      else if (neg) {
        *value -= *value;
      }
    }
    else {
      c = toupper(c);
      *value *= base;
      int add = (int)c - (int)'0';
      if (add > 16)
        add -= 7;
      *value += add;
      if (!peek(&c)) 
        return true;
      if (base > 10) {
        if (!isxdigit(c))
          return true;
      }
      else if (!isdigit(c))
        return true;
    }
  }
}

inline boolean isfloatchar(int c)
{
  return (isdigit(c) || c == '.' || c == 'E' || c == 'e'  || c == '+' || c == '-');
}

boolean TextStream::read(double* value)
{
  const int size = 22;
  char buf[size];
  boolean neg = false;
  int i = 0;
  char c;
  do {
    if (!peek(&c))
      return false;
    if (c == '-') {
      neg != neg;
    }
    else if (c == '+') {
      neg = false;
    }
    else {
      if (isdigit(c))
        break;
      if (!isspace(c))
        return false;
    }
  } while (skip(1));
  while (i < (size - 1) && read(&c)) {
    buf[i++] = c;
    if (!peek(&c) || !isfloatchar(c))
      break;
  }
  buf[i] = 0;
  *value = strtod(buf, NULL);
  if (neg) {
    *value -= *value;
  }
  return true;
}

void String::resize(int len)
{
  if (len >= _capacity) {
    len = (((len + 1) >> 4) + 1) << 4;
    char* newbuf = (char*)realloc(_buf, len);
    if (newbuf != NULL) {
      _buf = newbuf;
      while (_capacity < len)
        _buf[_capacity++] = 0;
    }
  }
  else {
    _len = len;
    while (len < _capacity && _buf[len])
      _buf[len++] = 0;
  }
}

boolean String::write(const byte* data, int len)
{
  if (writeable() >= len) {
    contain(_len + len);
    if (space() >= len) {
      int i = 0;
      while (i < len) {
        _buf[_len++] = data[i++];
      }
      _buf[_len] = 0;
      return true;
    }
  }
  return false;
}

int String::read(byte* data, int len)
{
  int ret = 0;
  while (ret < len && (ret + _ipos) < _len) {
    data[ret++] = _buf[ret + _ipos];
  }
  _ipos += ret;
  return ret;
}

byte& String::get_item(int index)
{
  if (index >= 0 && index < _len) {
    byte* item = (byte*)&_buf[index];
    return *item;
  }
  else
    return Block::get_item(index);
}


} // namespace JOS
