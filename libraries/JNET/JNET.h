/*
  JNET.h - Ethernet library for JOS
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

#ifndef __JNET_H__
#define __JNET_H__

#define DEBUG
#include <JOS.h>

#define USE_WIZ
#ifdef USE_WIZ
#include "contrib/w5100.h"
#elif USE_ENC
#include "contrib/enc28j60.h"
#include "contrib/ip_arp_udp_tcp.h"
#endif

namespace JOS {
  
const byte cmd_none = 0;
const byte cmd_command = 1;
const byte cmd_write = 2;
const byte cmd_read = 3;


class Socket: public JOS::Task {
private:
  SOCKET _sock;
  uint16_t _written;
  byte _command;
protected:
  boolean set_available_socket() {
    for (_sock = 0; _sock < MAX_SOCK_NUM; ++_sock) {
      D_JOS("Sock...");
      if (is_closed()) 
        return true;
    }
    return false;
  }
  boolean init(uint8_t protocol, uint16_t port, uint8_t flag);
  void send_command(uint8_t command);
  void close();
  virtual boolean suspended();
  virtual boolean run();
  void set_destination(uint8_t* addr, uint16_t port); 
public:
  Socket(): JOS::Task(), _sock(255), _written(0), _command(cmd_none) {
    D_JOS("JSocket construction");
    JOS::tasks.add(this);
  }
  ~Socket() {
    D_JOS("JSocket destruction");
  }
  byte status() {
    return W5100.readSnSR(_sock);
  }
  boolean is_closed() {
    return _sock == 255 || status() == SnSR::CLOSED;
  }
  uint16_t available() {
    if (is_closed()) 
      return 0;
    return W5100.readSnRX_RSR(_sock);
  }
  uint16_t read(byte* buf, word len, word timeout);
  uint16_t write(byte* buf, word len);
  void send() {
    if (is_closed()) return;
    send_command(Sock_SEND);
  }
  void recv() {
    if (is_closed()) return;
    send_command(Sock_RECV);
  }
};

class TCPSocket: public Socket {
protected:
  boolean init(uint16_t port) {
    return Socket::init(SnMR::TCP, port, 0);
  }
public:
  TCPSocket(uint16_t port): Socket() {
    init(port);
  }
  TCPSocket(): Socket() {
    init(0);
  }
  boolean listen();
  boolean connect(uint8_t* addr, uint16_t port);
  void disconnect();
  boolean is_connected() {
    if (is_closed()) {
      return false;
    }
    else {
      uint8_t s = status();
      return (s == SnSR::ESTABLISHED || (s == SnSR::CLOSE_WAIT && available()));
    }
  }
  uint16_t send(byte* buf, word len) {
    if (!is_connected()) 
      return 0;
    uint16_t result = write(buf, len);
    Socket::send();
    return result;
  }
  uint16_t recv(byte* buf, word len) {
    if (!is_connected()) 
      return 0;
    uint16_t result = read(buf, len, 0);
    Socket::recv();
    return result;
  }
};

/*
class UDPSocket: public Socket {
protected:
  boolean init(uint16_t port) {
    init(Sn_MR_UDP, port, 0);
  }
public:
  uint16_t send(const byte* buf, word len, uint8_t* addr, uint16_t port) {
    if (is_closed()) 
      return 0;
    set_destination(addr, port);
    uint16_t result = write(buf, len);
    send();
    return result;
  }
  uint16_t recv(byte* buf, word len, uint8_t* addr, uint16_t* port, );
};

class DHCP: public UDPSocket {
};

class DNS: public UDPSocket {
};
*/

struct Ethernet {
  Ethernet(uint8_t* mac) {
    // Initialize network interface chip
    W5100.init();
    // Set the MAC address
    W5100.setMACAddress(mac);
  }
  void set_ip(uint8_t* ip) {
    W5100.setIPAddress(ip);
  }
  void set_subnet(uint8_t* subnet, uint8_t* gateway) {
    W5100.setSubnetMask(subnet);
    W5100.setGatewayIp(gateway);
  }
};

} // namespace JOS

#endif
