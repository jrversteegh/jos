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

#include <JOS.h>

namespace JOS {


class IStream {
protected:
  unsigned _ipos;
public:
  virtual int available() const = 0;
  virtual boolean peek(byte* b) const = 0;
  virtual int read(byte*, int len) = 0;
  template<typename T> boolean read(T* v) {
    if (available() >= sizeof(T)) {
      return read((byte*)&v, sizeof(T));
    }
  }
  int skip(int len = 1); 
  void reset() {
    _ipos = 0;
  }
  IStream(): _ipos(0) {
  }
};

class OStream {
protected:
  unsigned _opos;
public:
  virtual int writeable() const = 0;
  virtual boolean write(const byte*, int len) = 0;
  template <typename T> bool write(const T& v) {
    D_JOS("Generic OStream write");
    return write((byte*)&v, sizeof(T));
  }
  OStream& operator<< (IStream& ist) {
    byte b;
    while (writeable() > 0 && ist.read(&b))
      write(b);
  }
  void reset() {
    _opos = 0;
  }
  OStream(): _opos(0) {
  }
};

struct Stream: public IStream, public OStream {
  Stream(): IStream(), OStream() {
  }
  void reset() {
    IStream::reset();
    OStream::reset();
  }
};

extern const char* endl; 

class TextStream: public Stream {
  int _width;
  char* _buf;
public:
  // Formatting
  char str_pad;
  char num_pad;
  uint8_t base;
  uint8_t prec;
  void setw(int width); 

  // OStream interface
  virtual boolean write(const byte*, int len) = 0;
  int write(const char* str, boolean complete = false); 
  boolean write(const double& value, boolean scientific = false);
  template<typename T> boolean write(const T& value);
  
  // IStream interface
  using IStream::read;
  boolean read(int* value);
  boolean read(double* value);
  using IStream::peek;
  boolean peek(char* c) {
    return peek((byte*)c);
  }
  char peek() {
    char c;
    if (peek(&c)) 
      return c;
    else
      return -1;
  }
  // Life cycle
  TextStream(): Stream(), _width(0), _buf(NULL), str_pad(' '), num_pad(' '),
      base(10), prec(2) {
  }
  ~TextStream() {
    setw(0); // Free 'width' buffer
  }
};


class Block {
protected:
  byte _undef; // Returned when index is out of range
  virtual byte& get_item(int) {
    _undef = 0;
    return _undef;
  }
public:
  byte& operator[] (int index) {
    return get_item(index); 
  }
};

struct Array {
  virtual Block& operator[] (int) = 0;
};

class String: public TextStream, public Block {
  static const int maxlen = 1023;
  int _capacity;
  int _len;
  char* _buf;
  int space() {
    return _capacity - _len - 1;
  }
  void resize(int len);
  void contain(int len) {
    if (len >= _capacity) 
      resize(len);
  }
protected:
  virtual byte& get_item(int index);
public:
  String(): TextStream(), Block(), _capacity(0), _len(0), _buf(NULL) {
    resize(0xf);
  }
  ~String() {
    if (_buf != NULL)
      free(_buf);
  }
  // Ostream interface
  virtual boolean write(const byte* data, int len);
  using TextStream::write;
  virtual int writeable() const {
    return maxlen - _len;
  }

  // IStream interface
  virtual int read(byte* data, int len);
  using TextStream::read;
  virtual int available() const {
    return _len - _ipos;
  }
  virtual boolean peek(byte* b) const {
    if (_ipos >= _len)
      return false;
    *b = _buf[_ipos];
    return true;
  }
  using TextStream::peek;

  int len() {
    return _len;
  }
  const char* c_str() const {
    return _buf;
  }
  void clear() {
    reset();
    resize(0);
  }

  // Operators
  boolean operator== (const String& str) {
    return strcmp(_buf, str._buf) == 0;
  }
  boolean operator== (const char* str) {
    return strcmp(_buf, str) == 0;
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
};

} // namespace JOS


#endif

