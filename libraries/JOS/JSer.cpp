/*
  JSer.cpp - Hardware serial library for JOS
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
  
  Adapted from original code by Nicholas Zambetti and David A. Mellis
*/


#include "JSer.h"
#include <wiring_private.h>

namespace JOS {

static Rx_buffer rx_buffer0;
static Tx_buffer tx_buffer0;

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
static Rx_buffer rx_buffer1;
static Tx_buffer tx_buffer1;
static Rx_buffer rx_buffer2;
static Tx_buffer tx_buffer2;
static Rx_buffer rx_buffer3;
static Tx_buffer tx_buffer3;
#endif

// Some evil macro stuff to reduce repetition
#define RX_HANDLER(sign, reg, buf) SIGNAL(sign) \
{ \
  byte data = reg; \
  buf.put(data); \
}

#define TX_HANDLER(sign, reg, buf) SIGNAL(sign) \
{ \
  int data = buf.get(); \
  if (data >= 0) { \
    reg = (byte)data; \
  } \
} 

// Declare interrupt service routines
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

RX_HANDLER(SIG_USART0_RECV, UDR0, rx_buffer0);
RX_HANDLER(SIG_USART1_RECV, UDR1, rx_buffer1);
RX_HANDLER(SIG_USART2_RECV, UDR2, rx_buffer2);
RX_HANDLER(SIG_USART3_RECV, UDR3, rx_buffer3);
TX_HANDLER(SIG_USART0_DATA, UDR0, tx_buffer0);
TX_HANDLER(SIG_USART1_DATA, UDR1, tx_buffer1);
TX_HANDLER(SIG_USART2_DATA, UDR2, tx_buffer2);
TX_HANDLER(SIG_USART3_DATA, UDR3, tx_buffer3);

#else 

#if defined(__AVR_ATmega8__)
RX_HANDLER(SIG_UART_RECV, UDR, rx_buffer0);
TX_HANDLER(SIG_UART_DATA, UDR, tx_buffer0);
#else
RX_HANDLER(USART_RX_vect, UDR0, rx_buffer0);
TX_HANDLER(USART_UDRE_vect, UDR0, tx_buffer0);
#endif

#endif


Serial::Serial(long baud, int port)
{
  switch(port) {
    case 0:
#if defined(__AVR_ATmega8__)
      init(&rx_buffer0, &tx_buffer0, &UBRRH, &UBRRL, &UCSRA, &UCSRB, 
          &UDR, UDRE, RXEN, TXEN, RXCIE, UDRIE, U2X, baud);
#else
      init(&rx_buffer0, &tx_buffer0, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, 
          &UDR0, UDRE0, RXEN0, TXEN0, RXCIE0, UDRIE0, U2X0, baud);
#endif
      break;
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    case 1:
      init(&rx_buffer1, &tx_buffer1, &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, 
          &UDR1, UDRE1, RXEN1, TXEN1, RXCIE1, UDRIE1, U2X1, baud);
      break;
    case 2:
      init(&rx_buffer2, &tx_buffer2, &UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, 
          &UDR2, UDRE2, RXEN2, TXEN2, RXCIE2, UDRIE2, U2X2, baud);
      break;
    case 3:
      init(&rx_buffer3, &tx_buffer3, &UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, 
          &UDR3, UDRE3, RXEN3, TXEN3, RXCIE3, UDRIE3, U2X3, baud);
      break;
#endif
  }
}

void Serial::set_baud(volatile uint8_t* ubrrh, volatile uint8_t* ubrrl, 
       uint8_t u2x, long baud)
{
  uint16_t baud_setting;
  bool use_u2x;

  // U2X mode is needed for baud rates higher than (CPU Hz / 16)
  if (baud > F_CPU / 16) {
    use_u2x = true;
  } else {
    // figure out if U2X mode would allow for a better connection
    
    // calculate the percent difference between the baud-rate specified and
    // the real baud rate for both U2X and non-U2X mode (0-255 error percent)
    uint8_t nonu2x_baud_error = abs((int)(255 - 
        ((F_CPU / (16 * (((F_CPU / 8 / baud - 1) / 2) + 1)) * 255) / baud)));
    uint8_t u2x_baud_error = abs((int)(255 - 
        ((F_CPU / (8 * (((F_CPU / 4 / baud - 1) / 2) + 1)) * 255) / baud)));
    
    // prefer non-U2X mode because it handles clock skew better
    use_u2x = (nonu2x_baud_error > u2x_baud_error);
  }
  
  if (use_u2x) {
    *_ucsra |= _BV(u2x);
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    *_ucsra &= ~_BV(u2x);
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  *ubrrl = baud_setting & 0xff;
  *ubrrh = baud_setting >> 8;
}

int Serial::available()
{
  return _rx_buffer->len();
}

int Serial::peek()
{
  return _rx_buffer->peek();
}

int Serial::read()
{
  return _rx_buffer->get();
}

void Serial::flush()
{
  _rx_buffer->flush();
  _tx_buffer->flush();
}

void Serial::write(uint8_t c)
{
  if (!_tx_buffer->put(c)) {
    if (!put_xtr(c)) {
      // Write buffers are full... character will be discarded
      D_JOS("Serial write buffer full");
    }
  }
}

boolean Serial::run()
{
  if (!_tx_buffer->full()) {
    // Transfer byte from extra buffer to tx buffer (when there is one)
    int c = get_xtr();
    if (c >= 0) {
      _tx_buffer->put((byte)c);
    }
  }
  // Kick start data transmission when required 
  if (!_tx_buffer->empty() && (*_ucsra & _udre_mask)) {
    // Disable the data register empty interrupt before checking
    *_ucsrb &= ~_udrie_mask;
    int c = _tx_buffer->get();
    // Recheck buffer and data register status after interrupt has 
    // been disabled
    if ((c >= 0) && (*_ucsra & _udre_mask)) {
      *_udr = c;
    }
    // Re-enabling the interrupt: buffer should now be sucked empty
    // by ISR
    *_ucsrb |= _udrie_mask;
  }
  // Never completed -> return false
  return false;
}


} // namespace JOS
