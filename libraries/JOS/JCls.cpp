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

//#define DEBUG
#include "JCls.h"
#include "JOS.h"
#include <avr/pgmspace.h>
#include <limits.h>
#include <ctype.h>

namespace JOS {

int Input_stream::skip(int size) {
  const int buf_size = 16;
  byte dump[buf_size];
  int rd, ret = 0;
  while (size > buf_size) {
    rd = read(dump, buf_size);
    ret += rd;
    if (rd != buf_size)
      return ret;
    size -= buf_size;
  }
  ret += read(dump, size);
  return ret;
}

const char* endl = "\n";

const char digits[] PROGMEM = "0123456789ABCDEF";

template<typename T> boolean Output_text::write(const Format& fmt, const T& value)
{
  D_JOS("Output_text::write(Format& fmt, const T&)");
  const int size = (sizeof(T) << 3) + 1;
  char buf[size];
  int i = size;
  buf[--i] = 0;
  T val;
  boolean neg = value < 0;
  if (neg)
    val = -value;
  else
    val = value;
  do {
    int mod = val % fmt.base;
    val /= fmt.base;
    buf[--i] = pgm_read_byte(&digits[mod]);
  } while (val);
  if (neg)
    buf[--i] = '-';

  return write_string(&buf[i], true, fmt.num_pad, fmt.width) != 0;
}

// Template instantiations
template boolean Output_text::write<short>(const Format&, const short&);
template boolean Output_text::write<int>(const Format&, const int&);
template boolean Output_text::write<long>(const Format&, const long&);
template boolean Output_text::write<unsigned short>(const Format&, const unsigned short&);
template boolean Output_text::write<unsigned int>(const Format&, const unsigned int&);
template boolean Output_text::write<unsigned long>(const Format&, const unsigned long&);

int Output_text::write_string(const char* str, boolean complete, char pad, uint8_t width)
{
  D_JOS("Output_text::write_string(const char*, boolean complete, char pad, uint8_t width)");
  int wr = writeable();
  if (str && (wr >= width)) {
    int i = 0;
    while (str[i] && i < width)
      ++i;
    if (complete) {
      int j = i;
      while (str[j++])
        if (j > wr)
          return false;
    }
    while (width > i) {
      Output_stream::write(pad); 
      ++i;
    }
    i = 0;
    while (str[i] && Output_stream::write(str[i])) 
      ++i;
    return i;
  }
  return 0;
}

boolean Output_text::write(const Format& fmt, const double& value)
{
  D_JOS("Output_text::write(const double&)");
  const int size = 24;
  char buf[size];
  boolean scientific = fmt.scientific;
  if (value < LONG_MIN || value > LONG_MAX)
    scientific = true;
  if (scientific) {
    dtostre(value, buf, fmt.precision, DTOSTR_UPPERCASE | DTOSTR_ALWAYS_SIGN);
  }
  else {
    dtostrf(value, 0, fmt.precision, buf);
  }
  return write_string(buf, true, fmt.str_pad, fmt.width) != 0;
}

boolean Input_text::skip_to_num(boolean* negative)
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

template <typename T> boolean Input_text::read(T* value)
{
  D_JOS("Input_text::read(T*)");
  boolean neg;
  uint8_t base = 10;
  char c;
  if (!skip_to_num(&neg))
    return false;
  *value = 0;
  while (Input_stream::read(&c)) {
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
  return false;
}

template boolean Input_text::read<short>(short*);
template boolean Input_text::read<int>(int*);
template boolean Input_text::read<long>(long*);

inline boolean isfloatchar(int c)
{
  return (isdigit(c) || c == '.' || c == 'E' || c == 'e'  
      || c == '+' || c == '-');
}

boolean Input_text::read(double* value)
{
  D_JOS("Input_text::read(double*)");
  const int size = 22;
  char buf[size + 1];
  boolean neg;
  int i = 0;
  if (!skip_to_num(&neg)) {
    return false;
  }
  char c = 0;
  while (i < size && Input_stream::read(&c)) {
    buf[i++] = c;
    if (!peek(&c) || !isfloatchar(c))
      break;
  }
  buf[i] = 0;
  *value = strtod(buf, 0);
  if (neg) 
    *value = -*value;
  return true;
}

// There is no garantee that the new size will be accepted!
void Memory_block::resize(int new_size)
{
  if (new_size == _size)
    return;
  if (new_size > max_size)
    new_size = max_size;
  int new_cap = ((new_size >> block_bits) + 1) << block_bits;
  if (new_cap != _capacity) {
    D_JOS("Membloc Alloc: current, capacity, new capacity");
    D_JOS((int)_buf);
    D_JOS((int)_capacity);
    D_JOS((int)new_cap);
    byte* new_buf = (byte*)realloc(_buf, new_cap);
    D_JOS("Membloc Alloc'ed:");
    D_JOS((int)new_buf);

    if (new_buf != 0) {
      _buf = new_buf;
      _size = new_size;
      if (new_cap < _capacity) 
        _capacity = new_cap;
      else while (new_cap > _capacity)
        _buf[_capacity++] = 0x00;
    }
#ifdef DEBUG
    else {
      D_JOS("Out of memory");
    }
#endif
  }
  else {
    while (_size > new_size) {
      _buf[--_size] = 0x00;
    }
    _size = new_size;
  }
}


byte& Array::get_item(const int item) 
{
  contain(item);
  if (item < _size)
    return _buf[item];

  _undef = 0;
  return _undef;
}

byte Array::get_item(const int item) const 
{
  if (item < _size)
    return _buf[item];

  return 0;
}

boolean String::write(const byte* data, int size)
{
  D_JOS("String::write(const byte*, int)");
  if (writeable() >= size) {
    int length = len();
    set_len(length + size);
    if (space() >= size) {
      int i = 0;
      while (i < size) {
        J_ASSERT(_buf[length] == 0, "Expected zero memory")
        _buf[length++] = data[i++];
      }
      return true;
    }
  }
  return false;
}

int String::read(byte* data, int size)
{
  D_JOS("String::read(byte*, int)");
  int ret = 0;
  while (ret < size && _ipos < (unsigned)len()) {
    data[ret++] = _buf[_ipos++];
  }
  return ret;
}

byte& String::get_item(const int index)
{
  D_JOS("String::get_item(const int)");
  if (index >= 0 && index < len()) {
    return _buf[index];
  }
  else
    return Block::get_item(index);
}

byte String::get_item(const int index) const
{
  D_JOS("String::get_item(const int) const");
  if (index >= 0 && index < len()) {
    return _buf[index];
  }
  else
    return 0;
}

} // namespace JOS
