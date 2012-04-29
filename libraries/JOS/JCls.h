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

#ifndef __JCLS_H__
#define __JCLS_H__

#include <stdlib.h>
#include <string.h>

// wiring.h disappeared in Arduino 1.0
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "wiring.h"
#endif

//#define DEBUG
#include "JDbg.h"

namespace JOS {

struct Input_stream {
  virtual int available() const = 0;
  virtual boolean peek(byte* b) const = 0;
  virtual int read(byte*, int size) = 0;
  template<typename T> boolean read(T* v) {
    D_JOS("Input_stream generic read");
    if (available() >= sizeof(T)) {
      return read((byte*)v, sizeof(T));
    }
  }
  int skip(int size = 1); 
  void rewind() {
    _ipos = 0;
  }
  Input_stream(): _ipos(0) {
  }
protected:
  unsigned _ipos;
};

struct Output_stream {
  virtual int writeable() const = 0;
  virtual boolean write(const byte*, int size) = 0;
  template <typename T> bool write(const T& v) {
    D_JOS("Generic Output_stream write");
    return write((byte*)&v, sizeof(T));
  }
  Output_stream& operator<< (Input_stream& ist) {
    byte b;
    while (writeable() > 0 && ist.read(&b))
      write(b);
  }
  void reset() {
    _opos = 0;
  }
  void set_pos(unsigned new_pos) {
    _opos = new_pos;
  }
  Output_stream(): _opos(0) {
  }
protected:
  unsigned _opos;
};

struct Stream: public Input_stream, public Output_stream {
  Stream(): Input_stream(), Output_stream() {
  }
};

extern const char* endl; 

struct Format {
  uint8_t width;
  uint8_t base;
  uint8_t precision;
  char num_pad;
  char str_pad;
  boolean scientific;
  Format(uint8_t w = 0, uint8_t b = 10, uint8_t p = 2,
        char np = ' ', char sp = ' ', boolean sc = false): 
      width(w), base(b), precision(p), 
      num_pad(np), str_pad(sp), scientific(sc) {
  }
};

struct Output_text: public Output_stream {
  int write_string(const char* str, boolean complete, char pad, uint8_t width);
  // Life cycle
  Output_text(): Output_stream(), format() {
  }

  // Formatting
  Format format;
  
  // Output_stream extension
  virtual boolean write(const byte*, int size) = 0;
  template<typename T> boolean write(const Format& fmt, const T& value);
  template<typename T> boolean write(const T& value) {
    return write(format, value);
  }
  int write(const char* str, boolean complete = false) {
    return write_string(str, complete, format.str_pad, format.width);
  }
  boolean write(const Format& fmt, const double& value);
  boolean write(const double& value) {
    return write(format, value);
  }
  boolean writeln() {
    return write(endl, true);
  }
};

struct Input_text: public Input_stream {
  Input_text(): Input_stream(), skipall(false) {
  }
  // Parsing
  boolean skipall;

  // Input_stream extension
  using Input_stream::read;
  virtual int read(byte*, int size) = 0;
  template<typename T> boolean read(T* value);
  boolean read(double* value);
  using Input_stream::peek;
  boolean peek(char* c) {
    return peek((byte*)c);
  }
  char peek() {
    char c;
    if (peek(&c)) 
      return c;
    else
      return 0;
  }
private:
  boolean skip_to_num(boolean* negative);
};

struct Text_stream: public Output_text, public Input_text {
  Text_stream(): Output_text(), Input_text() {
  }
};

struct Block {
  virtual int size() const = 0;
  byte& operator[] (const int index) {
    return get_item(index); 
  }
  byte operator[] (const int index) const {
    return get_item(index); 
  }
  Block(): _undef(0) {}
protected:
  byte _undef; // Returned when index is out of range
  virtual byte& get_item(const int index) {
    _undef = 0;
    return _undef;
  }
  virtual byte get_item(const int index) const {
    return 0;
  }
};

struct Matrix {
  virtual Block& operator[] (int) = 0;
};

struct Memory_block: public Block {
  Memory_block(): Block(), _capacity(0), _size(0), _buf(NULL) {
  }
  ~Memory_block() {
    if (_buf != NULL)
      free(_buf);
  }
  virtual int size() const {
    return _size;
  }
  byte* data() {
    return _buf;
  }
  void resize(int new_size);
  void contain(int item) {
    if (item >= _size) {
      resize(item + 1);
    }
  }
protected:
  static const int max_size = 0x400;
  static const int block_bits = 4;
  int _capacity;
  int _size;
  byte* _buf;
};

struct Array: public Memory_block {
protected:
  virtual byte& get_item(const int item); 
  virtual byte get_item(const int item) const;
};

struct String: public Text_stream, public Memory_block {
  String(): Text_stream(), Memory_block() {
    resize(0xF);
  }
  
  // Ostream interface
  virtual boolean write(const byte* data, int size);
  using Text_stream::write;
  virtual int writeable() const {
    return max_size - _size;
  }

  // Input_stream interface
  virtual int read(byte* data, int size);
  using Text_stream::read;
  virtual int available() const {
    return len() - _ipos;
  }
  virtual boolean peek(byte* b) const {
    if (_ipos >= len())
      return false;
    *b = _buf[_ipos];
    return true;
  }
  using Text_stream::peek;

  // String functions
  int len() const {
    return _size - 1;
  }
  void set_len(int new_len) {
    resize(new_len + 1);
  }
  const char* c_str() const {
    return (char*)_buf;
  }
  void clear() {
    rewind();
    set_len(0);
  }

  // Operators
  boolean operator== (const String& str) {
    return strcmp((char*)_buf, str.c_str()) == 0;
  }
  boolean operator== (const char* str) {
    return strcmp((char*)_buf, str) == 0;
  }
  String& operator= (const char* str) {
    clear();
    write(str);
    return *this;
  }
  String& operator= (const String& str) {
    operator=(str.c_str());
    return *this;
  }
  String& operator+= (const char* str) {
    write(str);
    return *this;
  }
  String& operator+= (const String& str) {
    operator+=(str.c_str());
    return *this;
  }
protected:
  // Block interface
  virtual byte& get_item(const int index);
  virtual byte get_item(const int index) const;
private:
  int space() {
    return _capacity - _size;
  }
  void resize(int newsize) {
    Memory_block::resize(newsize);
    _buf[_size] = 0;
  }
};

} // namespace JOS


#endif

