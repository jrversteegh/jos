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

//#define DEBUG
#include <JOS.h>

namespace JOS {


class IStream {
protected:
  unsigned _ipos;
public:
  virtual int available() const = 0;
  virtual boolean peek(byte* b) const = 0;
  virtual int read(byte*, int size) = 0;
  template<typename T> boolean read(T* v) {
    D_JOS("IStream generic read");
    if (available() >= sizeof(T)) {
      return read((byte*)v, sizeof(T));
    }
  }
  int skip(int size = 1); 
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
  virtual boolean write(const byte*, int size) = 0;
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
  char* _wbuf;
  boolean skip_to_num(boolean* negative);
public:
  // Formatting
  char str_pad;
  char num_pad;
  uint8_t base;
  uint8_t prec;
  boolean skipall;

  void setw(int width); 

  // OStream interface
  virtual boolean write(const byte*, int size) = 0;
  template<typename T> boolean write(const T& value);
  int write(const char* str, boolean complete = false); 
  boolean write(const double& value, boolean scientific = false);
  
  // IStream interface
  using IStream::read;
  virtual int read(byte*, int size) = 0;
  template<typename T> boolean read(T* value);
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
      return 0;
  }
  // Life cycle
  TextStream(): Stream(), _width(0), _wbuf(NULL), str_pad(' '), num_pad(' '),
      skipall(false), base(10), prec(2) {
  }
  ~TextStream() {
    setw(0); // Free 'width' buffer
  }
};


class Block {
protected:
  byte _undef; // Returned when index is out of range
  virtual byte& get_item(int index) {
    _undef = 0;
    return _undef;
  }
public:
  virtual int size() const = 0;
  byte& operator[] (int index) {
    return get_item(index); 
  }
};

struct Matrix {
  virtual Block& operator[] (int) = 0;
};

class MemBlock: public Block {
protected:
  static const int max_size = 0x400;
  static const int block_bits = 4;
  int _capacity;
  int _size;
  byte* _buf;
  void resize(int new_size);
  void contain(int item) {
    if (item >= _size) {
      resize(item + 1);
    }
  }
public:
  virtual int size() const {
    return _size;
  }
  MemBlock(): Block(), _capacity(0), _size(0), _buf(NULL) {
  }
  ~MemBlock() {
    if (_buf != NULL)
      free(_buf);
  }
};

class Array: public MemBlock {
protected:
  virtual byte& get_item(int item); 
};

class String: public TextStream, public MemBlock {
  int space() {
    return _capacity - _size;
  }
  void resize(int newsize) {
    MemBlock::resize(newsize);
    _buf[_size] = 0;
  }
protected:
  // Block interface
  virtual byte& get_item(int index);
public:
  String(): TextStream(), MemBlock() {
    resize(0xF);
  }
  
  // Ostream interface
  virtual boolean write(const byte* data, int size);
  using TextStream::write;
  virtual int writeable() const {
    return max_size - _size;
  }

  // IStream interface
  virtual int read(byte* data, int size);
  using TextStream::read;
  virtual int available() const {
    return len() - _ipos;
  }
  virtual boolean peek(byte* b) const {
    if (_ipos >= len())
      return false;
    *b = _buf[_ipos];
    return true;
  }
  using TextStream::peek;

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
    reset();
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
};

} // namespace JOS


#endif

