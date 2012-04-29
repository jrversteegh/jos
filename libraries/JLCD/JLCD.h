/*
  JLCD.h - 4 bit LCD library (HD44780) for JOS
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
#include <JCls.h>
#include "JLCD_config.h"

namespace JOS {
  
//command bytes for LCD
static const byte cmd_clear = 0x01;
static const byte cmd_right = 0x1C;
static const byte cmd_left = 0x18;
static const byte cmd_home = 0x02;


struct LCDTask: public JOS::Task {
  LCDTask(): JOS::Task() {
    D_JOS("LCDTask construction");
  }
protected:
  void write_nibble(const byte value);
  void write_byte(const byte value);
  void write_command(const byte value);
  void write_data(const byte value);
};

struct Init: public LCDTask {
protected:
  virtual boolean run();
};


struct Display: public LCDTask {
  Display(): LCDTask(), _addr(0), _char_index(0) {
    for (int i = 0; i < char_count; ++i) {
      _data[i] = ' ';
      _actual[i] = ' ';
    }
  }
  friend class LCD;
protected:
  virtual boolean run();
private:
  static const byte run_state_clear = 100;
  static const byte run_state_home = 102;
  static const byte run_state_put_char = 104;
  static const byte run_state_done = 120;
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
  void done() {
    _run_state = run_state_done;
  }
};

struct LCD: public Block, public Output_text {
  LCD();
  ~LCD();
  // Output stream interface
  virtual int writeable() const {
    return char_count - _opos;
  }
  virtual boolean write(const byte* data, int size);
  using Output_text::write;
  // Block interface
  virtual int size() const {
    return char_count;
  }
  using Output_text::set_pos;
  void set_pos(int line, unsigned new_pos) {
    set_pos(line * line_chars + new_pos);
  }
  void set_line(unsigned line) {
    set_pos(line * line_chars);
  }
  void clear();
protected:
  // Block interface
  byte& get_item(const int index) {
    if (index >= 0 && index < char_count) 
      return *((byte*)&_display->_data[index]);
    else
      return Block::get_item(index);
  }
  byte get_item(const int index) const {
    if (index >= 0 && index < char_count) 
      return _display->_data[index];
    else
      return 0;
  }
private:
  Display* _display;
};

} // namespace JOS

#endif
