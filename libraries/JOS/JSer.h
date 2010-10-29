/*
  JSer.h - Hardware serial library for JOS
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

#ifndef __JSER_H__
#define __JSER_H__

#include "JOS.h"
#include "JCls.h"

namespace JOS {

// Check if the buffers size are valid
#if (RX_BUFFER_SIZE < 4 || RX_BUFFER_SIZE > 256)
#error "Invalid RX buffersize"
#endif

#if (TX_BUFFER_SIZE < 4 || TX_BUFFER_SIZE > 256)
#error "Invalid TX buffersize"
#endif

#define RX_BUFFER_MASK (RX_BUFFER_SIZE - 1)
#define TX_BUFFER_MASK (TX_BUFFER_SIZE - 1)

#if ( RX_BUFFER_SIZE & RX_BUFFER_MASK )
#error "RX buffer size is not a power of 2"
#endif
#if ( TX_BUFFER_SIZE & TX_BUFFER_MASK )
#error "TX buffer size is not a power of 2"
#endif

template<int bufsize> class Buffer {
protected:
  static const uint8_t mask = bufsize - 1;
  byte buffer[bufsize];
  volatile uint8_t _head;
  volatile uint8_t _tail;
  uint8_t new_head() {
    return (_head + 1) & mask;
  }
public:
  static const int size = bufsize;
  boolean empty() {
    return _head == _tail;
  }
  boolean full() {
    return new_head() == _tail;
  }
  boolean put(byte b) {
    uint8_t n_head = new_head();
    if (n_head != _tail) {
      buffer[_head] = b;
      _head = n_head;
      return true;
    }
    return false;
  }
  boolean peek(byte* b) {
    if (empty()) 
      return false;
    *b = buffer[_tail];
    return true;
  }
  boolean get(byte* b) {
    if (peek(b)) {
      _tail = (_tail + 1) & mask;
      return true;
    }
    return false;
  }
  uint8_t len() {
    int l = _head + bufsize - _tail;
    return (uint8_t)l & mask;
  }
  Buffer(): _head(0), _tail(0) {}
};

struct Rx_buffer: public Buffer<RX_BUFFER_SIZE> {
  void flush() {
    _head = _tail;
  }
}; 

struct Tx_buffer: public Buffer<TX_BUFFER_SIZE> {
  void flush() {
    _tail = _head;
  }
};

class SerialBase : public Task {
  Rx_buffer *_rx_buffer;
  Tx_buffer *_tx_buffer;
  volatile uint8_t *_ucsra; // UART status register A
  volatile uint8_t *_ucsrb; // UART status register B
  volatile uint8_t *_udr;   // UART data register
  uint8_t _udre_mask;       // Data register empty mask for status register A
  uint8_t _udrie_mask;      // DRE interrupt mask for status register B
  uint8_t _ucsrb_mask;      // Enable bits mask for status register B
  void set_status(volatile uint8_t* reg, uint8_t mask) {
    *reg |= mask;
  }
  void clear_status(volatile uint8_t* reg, uint8_t mask) {
    *reg &= ~mask;
  }
  void set_baud(volatile uint8_t* ubrrh, volatile uint8_t* ubrrl, 
       uint8_t u2x, long baud);
protected:
  virtual boolean run();
  void init(Rx_buffer* rx_buffer, Tx_buffer* tx_buffer, // Buffers
      volatile uint8_t* ubrrh, volatile uint8_t* ubrrl, // Baudrate registers
      volatile uint8_t* ucsra, volatile uint8_t* ucsrb, // Status registers (A and B)
      volatile uint8_t* udr, // Data register
      uint8_t udre,   // Data register empty bit (Status A)
      uint8_t rxen, uint8_t txen, // RX/TX enable bits (Status B)
      uint8_t rxcie,  uint8_t udrie, // RX/DRE interrupt bits (Status B)
      uint8_t u2x,  // Baud rate sampling bit (Status A) 
      long baud); 
  void init(long baud, int port);
public:
  SerialBase(long baud, int port=0): Task() {
    init(baud, port);
  }
  ~SerialBase() {
    // Disable UART...
    clear_status(_ucsrb, _ucsrb_mask);
    // ... and clear buffers
    flush();
  }

  // Stream interface implementation
  int available_data() const {
    return _rx_buffer->len();
  }
  boolean peek_data(byte* b) const {
    return _rx_buffer->peek(b);
  }
  int read_data(byte* data, int size); 
  boolean write_data(const byte* data, int size);
  int writeable_data() const {
    return _tx_buffer->size - _tx_buffer->len();
  }

  // Serial specific
  void flush();
};

template <class ST>
class Serial_template: public SerialBase, public ST {
public:
  Serial_template(long baud, int port = 0): SerialBase(baud, port), ST() {
  }
  // IStream interface
  virtual int available() const {
    return available_data();
  }
  virtual boolean peek(byte* b) const {
    return peek_data(b);
  }
  virtual int read(byte* data, int size) {
    return read_data(data, size);
  }
  using ST::read;

  // OStream interface
  virtual int writeable() const {
    return writeable_data();
  }
  virtual boolean write(const byte* data, int size) {
    return write_data(data, size);
  }
  using ST::write; 
};

typedef Serial_template<Stream> Serial;
typedef Serial_template<Text_stream> Text_serial;

} // Namespace JOS

#endif
