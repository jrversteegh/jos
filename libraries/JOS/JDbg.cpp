/*
  JDbg.cpp - Debug tools for JOS
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

#define DEBUG
#include "JDbg.h"
#include <wiring_private.h>
 
namespace JOS {

#define DEBUG_BAUD 9600

#ifdef __AVR_ATmega8__
#define _UDR UDR
#define _UBRRH UBRRH
#define _UBRRL UBRRL
#define _UCSRA UCSRA
#define _UCSRB UCSRB
#define _TXEN TXEN
#define _UDRE UDRE
#define _U2X U2X
#else
#define _UDR UDR0
#define _UBRRH UBRR0H
#define _UBRRL UBRR0L
#define _UCSRA UCSR0A
#define _UCSRB UCSR0B
#define _TXEN TXEN0
#define _UDRE UDRE0
#define _U2X U2X0
#endif

void Debug::init_when_required()
{
  if (!(_UCSRB & _BV(_TXEN))) {
    static const uint16_t baud_set = (F_CPU / 8 / DEBUG_BAUD - 1) / 2;
    _UBRRL = baud_set & 0xff; 
    _UBRRH = baud_set >> 8;
    _UCSRA &= ~_BV(_U2X);
    _UCSRB |= _BV(_TXEN);
  }
}

void Debug::write(uint8_t c)
{
  init_when_required();
  while (!(_UCSRA & _BV(_UDRE))) 
    ;  
  _UDR = c;
}

Debug debug;

}  // namespace JOS
