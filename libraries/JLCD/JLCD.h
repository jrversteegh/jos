/*
  JLCD.h - 4 bit LCD library for JOS
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
#ifndef __JLCD_H__
#define __JLCD_H__

//#define DEBUG
#include <JOS.h>

namespace JLCD {
  
//command bytes for LCD
const byte cmd_clear = 0x01;
const byte cmd_right = 0x1C;
const byte cmd_left = 0x18;
const byte cmd_home = 0x02;

// Pins:
const int pin_rs = 8;
const int pin_en = 9;

const int d_pins = 4;
const int d_base = 4;
const int pin_d[] = {d_base, d_base + 1, d_base + 2 , d_base + 3};  

class LCDTask: public JOS::Task {
protected:
  void write_nibble(const byte value);
  void write_byte(const byte value);
  void write_command(const byte value);
  void write_data(const byte value);
public:
  LCDTask(): JOS::Task() {
    D_JOS("LCDTask construction");
  }
};

class Init: public LCDTask {
protected:
  virtual boolean run();
};

const int char_count = 32;
const int line_chars = 16;

class Display: public LCDTask {
  static const byte run_state_clear = 100;
  static const byte run_state_home = 102;
  static const byte run_state_put_char = 104;
  byte _addr;
  int _char_index;
  char _actual[char_count];
  char _data[char_count];
  void set_address(byte addr) {
    write_command(0x80 | (addr & 0x7f));
    _addr = addr;
  }
  void put_char();
  void clear() {
    _run_state = run_state_clear;
  }
  void home() {
    _run_state = run_state_home;
  }
protected:
  virtual boolean run();
public:
  Display(): LCDTask(), _addr(0), _char_index(0) {
    for (int i = 0; i < char_count; ++i) {
      _data[i] = ' ';
      _actual[i] = ' ';
    }
  }
  friend class LCD;
};

class LCD {
  Display* _display;
public:
  LCD();
  void print(const char* str, const int offset);
  void print(const int line, const char* str) {
    print(str, line * line_chars);
  }
  void print(const int line, const char* str, const int offset) {
    print(str, line * line_chars  + offset);
  }
  void print(const char* str) {
    print(str, 0);
  }
  char& operator[] (const int index) {
    if (index > 0 && index < char_count) {
      return _display->_data[index];
    }
    else {
      return *_display->_data;
    }
  }
  char operator[] (const int index) const {
    if (index > 0 && index < char_count) {
      return _display->_data[index];
    }
    else {
      return *_display->_data;
    }
  }
  char& operator() (const int index) {
    return this->operator[](index);
  }
  char operator() (const int index) const {
    return this->operator[](index);
  }
  char& operator() (const int line, const int index) {
    return this->operator[](index + line * line_chars);
  }
  char operator() (const int line, const int index) const {
    return this->operator[](index + line * line_chars);
  }
  void clear();
};

} // namespace JLCD

#endif
