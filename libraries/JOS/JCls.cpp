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

const char digits[] PROGMEM = "0123456789ABCDEF";

template<typename T> boolean TextStream::write(const T& value)
{
  D_JOS("TextStream::write(const T&)");
  const int size = sizeof(T) << 3;
  char buf[size];
  char* p = _width > size ? _buf : buf;
  int i = _width > size ? _width : size;
  T val;
  boolean neg = value < 0;
  if (neg)
    val = -value;
  else
    val = value;
  do {
    int mod = val % base;
    val /= base;
    p[--i] = pgm_read_byte(&digits[mod]);
  } while (val);
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

  return write((byte*)&p[i], size - i);
}

template boolean TextStream::write<short>(const short&);
template boolean TextStream::write<int>(const int&);
template boolean TextStream::write<long>(const long&);

int TextStream::write(const char* str, boolean complete)
{
  D_JOS("TextStream::write(const char*)");
  int w = writeable();
  if (str && (w >= _width) && (!complete || (w >= strlen(str)))) {
    int i = 0;
    int k = _width;
    while (str[i] && i < _width)
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
    while (k < _width) 
      OStream::write(_buf[k++]); 
    while (str[i] && OStream::write(str[i])) 
      ++i;
    return i;
  }
  return 0;
}

boolean TextStream::write(const double& value, boolean scientific)
{
  D_JOS("TextStream::write(const double&)");
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

boolean TextStream::skip_to_num(boolean* negative)
{
  *negative = false;
  char c;
  do {
    if (!peek(&c))
      return false;
    if (c == '-') {
      *negative = !*negative;
    }
    else if (c == '+') {
      *negative = false;
    }
    else {
      if (isdigit(c))
        break;
      if (!skipall && !isspace(c))
        return false;
    }
  } while (skip());
  return true;
}

template <typename T> boolean TextStream::read(T* value)
{
  D_JOS("TextStream::read(T*)");
  boolean neg;
  uint8_t base = 10;
  char c;
  if (!skip_to_num(&neg))
    return false;
  *value = 0;
  while (IStream::read(&c)) {
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
        skip();
      }
      else if (neg) {
        *value = -*value;
      }
    }
    else {
      c = toupper(c);
      *value *= base;
      int add = (int)c - (int)'0';
      if (add > 16)
        add -= 7;
      if (neg)
        *value -= add;
      else
        *value += add;
    }
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

template boolean TextStream::read<short>(short*);
template boolean TextStream::read<int>(int*);
template boolean TextStream::read<long>(long*);

inline boolean isfloatchar(int c)
{
  return (isdigit(c) || c == '.' || c == 'E' || c == 'e'  
      || c == '+' || c == '-');
}

boolean TextStream::read(double* value)
{
  D_JOS("TextStream::read(double*)");
  const int size = 22;
  char buf[size + 1];
  boolean neg;
  int i = 0;
  if (!skip_to_num(&neg)) {
    return false;
  }
  char c = 0;
  while (i < size && IStream::read(&c)) {
    buf[i++] = c;
    if (!peek(&c) || !isfloatchar(c))
      break;
  }
  buf[i] = 0;
  *value = strtod(buf, NULL);
  if (neg) 
    *value = -*value;
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
  D_JOS("String::write(const byte*, int)");
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
  D_JOS("String::read(byte*, int)");
  int ret = 0;
  while (ret < len && _ipos < _len) {
    data[ret++] = _buf[_ipos++];
  }
  return ret;
}

byte& String::get_item(int index)
{
  D_JOS("String::get_item(int)");
  if (index >= 0 && index < _len) {
    byte* item = (byte*)&_buf[index];
    return *item;
  }
  else
    return Block::get_item(index);
}

} // namespace JOS
