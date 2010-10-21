/*
  JLCD.cpp - 4 bit LCD library for JOS
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

#include "JLCD.h"

namespace JLCD {

void LCDTask::write_nibble(const byte value)
{
  // Set enable pin
  digitalWrite(pin_en, HIGH);
  delayMicroseconds(1);
  // Output nibble on data pins
  for (int i = 0; i < d_pins; ++i) {
    digitalWrite(pin_d[i], bitRead(value, i));
  }
  delayMicroseconds(1);
  // Clock out the data with falling edge of enable pin
  digitalWrite(pin_en, LOW);
  delayMicroseconds(1);
}

void LCDTask::write_byte(const byte value)
{
  write_nibble(value >> 4);
  write_nibble(value & 0xF);
}

void LCDTask::write_command(const byte value)
{
  digitalWrite(pin_rs, LOW);
  write_byte(value);
  rest(200);
}

void LCDTask::write_data(const byte value)
{
  digitalWrite(pin_rs, HIGH);
  write_byte(value);
  rest(200);
}

boolean Init::run() 
{
  if (_run_state < 4)
    // Send 4 bit init commands: commands RS -> Low
    digitalWrite(pin_rs, LOW);
  switch (_run_state) {
    case 0:
      D_JOS("Start LCD init");
      // General initialization
      write_nibble(0x3);
      rest(5000);
      return false;
    case 1:
      write_nibble(0x3);
      rest(100);
      return false;
    case 2:
      write_nibble(0x3);
      rest(5000);
      return false;
    case 3:
      // Set to 4 bit mode
      write_nibble(0x2);
      rest(1000);
      return false;
    case 4:
      // From here we can send bytes in 4 bit mode
      write_command(0x28); // 2 lines, 5x8 font
      return false;
    case 5:
      write_command(0x0C); // display on, cursor off, no blink
      return false;
    case 6:
      write_command(cmd_clear);
      rest(2000);
      return false;
    case 7:
      write_command(0x6); // entry mode: autoinc, no shift
      return false;
  }
  D_JOS("Done LCD init");
  // We're done!
  return true;
}

void Display::put_char()
{
  byte addr = _char_index;
  if (addr > 0x0F) {
    addr += 0x30;
  }
  if (addr != _addr) {
    set_address(addr);
  }
  _run_state = run_state_put_char;
}

boolean Display::run() 
{
  switch(_run_state) {
    case 0:
      D_JOS("Start running LCD");
      return false;

    case run_state_clear:
      write_command(cmd_clear);
      rest(2000);
      return false;

    case run_state_home:
      write_command(cmd_home);
      rest(2000);
      return false;

    case run_state_put_char:
      write_data(_data[_char_index]);
      // The address is automatically incremented after write
      ++_addr;
      _actual[_char_index] = _data[_char_index];
      return false;

    default:
      _run_state = run_state_default;
      if (_data[_char_index] != _actual[_char_index]) {
        D_JOS("Put char");
        put_char();
        // Compensate for automatic state increment
        --_run_state;
        // Only one character at a time, so return now
        return false;
      }
      ++_char_index;
      if (_char_index >= char_count)
        _char_index = 0;
      // Display should always remain running so always return false
      return false;
  }
}

LCD::LCD()
{
  // Initialize pins
  pinMode(pin_rs, OUTPUT);
  pinMode(pin_en, OUTPUT);
  for (int i = 0; i < d_pins; ++i) {
    pinMode(pin_d[i], OUTPUT);
  }

  // Create tasks
  Init* init = new Init;
  _display = new Display;

  // Chain tasks..
  _display->set_predecessor(init);
  // ..and add to the OS tasklist for execution
  JOS::tasks.add(init);
}

void LCD::print(const char* str, const int offset)
{
  int j = offset;
  int i = 0;
  while (str[i] != 0) {
    if (j < char_count) {
      _display->_data[j++] = str[i++];
    }
  }
}

void LCD::clear()
{
  for (int i = 0; i < char_count; ++i) {
    _display->_data[i] = ' ';
    _display->_actual[i] = ' ';
    _display->clear();
  }
}


} // namespace JLCD
