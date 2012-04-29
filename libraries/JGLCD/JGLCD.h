/*
  JGLCD.h - Graphic LCD library (KS0108) for JOS
  Copyright (c) 2012 Jaap Versteegh.  All right reserved.

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

  Much of the code in this library borrowed from the glcd library:
  http://code.google.com/p/glcd-arduino/
*/
#ifndef __JGLCD_H__
#define __JGLCD_H__

//#define DEBUG
#include <JOS.h>
#include <JCls.h>
#include "JGLCD_config.h"

namespace JOS {
  
static const int chip_count = 
      ((display_w + h_px_per_chip - 1) / h_px_per_chip)
    * ((display_h + v_px_per_chip - 1) / v_px_per_chip);

struct GLCDTask: public JOS::Task {
  GLCDTask(): JOS::Task() {
    D_JOS("GLCDTask construction");
  }
protected:
  void write_data(const byte value);
};

struct GInit: public GLCDTask {
protected:
  virtual boolean run();
};


struct GDisplay: public GLCDTask {
  GDisplay(): GLCDTask(), _addr(0), _char_index(0) {
    for (int i = 0; i < char_count; ++i) {
      _data[i] = ' ';
      _actual[i] = ' ';
    }
  }
  friend class GLCD;
protected:
  virtual boolean run();
private:
  void clear() {
    _run_state = run_state_clear;
  }
  void done() {
    _run_state = run_state_done;
  }
};

struct GLCD: public Block, public Output_text {
  GLCD();
  ~GLCD();
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
  // Graphics interface
  void set_fg_color(const uint8_t color);
  void set_bg_color(const uint8_t color);
  void dot(const uint8_t x, const uint8_t y, const boolean clear=false);
  void go_to(const uint8_t x, const uint8_t y);
  void line_to(const uint8_t x, const uint8_t y);
  void line(const uint8_t x1, const uint8_t y1,
            const uint8_t x2, const uint8_t y2);
  void vline(const uint8_t x, const uint8_t y, const uint8_t l);
  void hline(const uint8_t x, const uint8_t y, const uint8_t l);
  void circle(const uint8_t x, const uint8_t y, const uint8_t r, 
              const boolean fill=false);
  void rect(const uint8_t x, const uint8_t y,
            const uint8_t w, const uint8_t h,
            const boolean fill=false);
  void clear(const uint8_t x, const uint8_t y,
             const uint8_t w, const uint8_t h);
  void bitmap(const uint8_t x, const uint8_t y,
              const uint8_t* data);
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
  GDisplay* _display;
};

} // namespace JOS

#endif
