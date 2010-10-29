/*
  JNET.cpp - Ethernet library for JOS
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

#include "JNet.h"

namespace JOS {

boolean Socket::init(uint8_t protocol, uint16_t port, uint8_t flag)
{
  D_JOS("init(protocol, port, flag)");

  // Get available socket
  if (!set_available_socket()) {
    D_JOS("No socket available");
    return false;
  }
 
  // Assert a valid protocol
  J_ASSERT((protocol == SnMR::TCP) || (protocol == SnMR::UDP) 
      || (protocol == SnMR::IPRAW) || (protocol == SnMR::MACRAW) 
      || (protocol == SnMR::PPPOE), "Invalid protocol")

  // Assert the provided port is not in the automatic port range
  J_ASSERT(port < 0x8000 || port >= (0x8000 + MAX_SOCK_NUM),
      "Port in invalid automatic port range"); 
  // Determine port automatically when requested
  if (port == 0) {
    port = 0x8000 + _sock;
  }

  D_JOS("Send iinchip open");
  W5100.writeSnMR(_sock, protocol | flag);
  W5100.writeSnPORT(_sock, port);

  D_JOS("Send open command");
  send_command(Sock_OPEN);
  D_JOS("Done open command");
  return true;
}

void Socket::close()
{
  D_JOS("Closing socket");
  send_command(Sock_CLOSE);
  W5100.writeSnIR(_sock, 0xFF);
}

void Socket::send_command(uint8_t command)
{
  _command = cmd_command;
  W5100.writeSnCR(_sock, command);
  wait(); // wait for the command to complete
  _command = cmd_none;
}

boolean Socket::run()
{
  return false;
}

boolean Socket::suspended()
{
  switch (_command) {
    case cmd_command:
      // Suspend when the chip is processing a command
      return W5100.readSnCR(_sock) != 0;
    case cmd_write:
    case cmd_read:
    default:
      return Task::suspended();
  }
}

void Socket::set_destination(uint8_t* addr, uint16_t port) 
{
  // set destination IP ...
  W5100.writeSnDIPR(_sock, addr);
  // ... and port
  W5100.writeSnDPORT(_sock, port);
}

uint16_t Socket::read(byte* buf, word len, word timeout) 
{
  _command = cmd_read;
  while (timeout > 0) {
    if (available() >= len) {
      W5100.recv_data_processing(_sock, buf, len);
      return len;
    }
    sleep(1000);
    --timeout;
  }
  uint16_t avail = available();
  if (avail < len) {
    len = avail;
  }
  W5100.recv_data_processing(_sock, buf, len);
  return len;
  _command = cmd_none;
}

uint16_t Socket::write(byte* buf, word len) 
{
  _command = cmd_write;
  uint16_t available = W5100.SSIZE - _written;
  if (len > available) {
    len = available;
  }
  uint16_t free = W5100.readSnTX_FSR(_sock);
  while (len > free) {
    sleep(500);
    free = W5100.readSnTX_FSR(_sock);
  }
  W5100.send_data_processing(_sock, buf, len);
  _written += len;
  _command = cmd_none;
  return len;
}

boolean TCPSocket::listen()
{
  if (status() == SnSR::INIT) {
    D_JOS("Sending listen command");
    send_command(Sock_LISTEN);
    D_JOS("Done listen command");
    return true;
  }
  else {
    return false;
  }
}

boolean TCPSocket::connect(uint8_t* addr, uint16_t port)
{
  if (is_closed() || is_connected()) {
    return false;
  }

  set_destination(addr, port);

  send_command(Sock_CONNECT);
  return is_connected();
}

void TCPSocket::disconnect()
{
  if (!is_closed()) {
    send_command(Sock_DISCON);
  }
}

} // namespace JOS

