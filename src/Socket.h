/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_SOCKET_H_
#define _D_SOCKET_H_

#include <string>
#include "SocketCore.h"
#include "common.h"

using namespace std;

class Socket {
private:
  SocketCore* core;
  /**
   * This method doesn't increment the use count of core.
   */
  Socket(SocketCore* core);
public:
  Socket();
  Socket(const Socket& s);
  ~Socket();

  Socket& operator=(const Socket& s);

  /**
   * Returns socket descriptor of this socket.
   * @returns socket descriptor of this socket.
   */
  int getSockfd() const { return core->sockfd; }

  /**
   * @see SocketCore::beginListen()
   */
  void beginListen() const;
  
  /**
   * @see SocketCore::getAddrInfo()
   */
  void getAddrInfo(pair<string, int>& addrinfo) const;

  /**
   * @see SocketCore::acceptConnection()
   */
  Socket* acceptConnection() const;

  /**
   * @see SocketCore::establishConnection()
   */
  void establishConnection(string host, int port) const;

  /**
   * @see SocketCore::setBlockingMode()
   */
  void setBlockingMode() const;

  /**
   * @see SocketCore::closeConnection()
   */
  void closeConnection() const;

  /**
   * @see SocketCore::isWritable()
   */
  bool isWritable(int timeout) const;

  /**
   * @see SocketCore::isReadable()
   */
  bool isReadable(int timeout) const;

  /**
   * @see SocketCore::writeData()
   */
  void writeData(const char* data, int len, int timeout = 5) const;
  /**
   * A covenient function that can take string class parameter and
   * internally calls SocketCore::writeData().
   */
  void writeData(string str, int timeout = 5) const;

  /**
   * @see SocketCore::readData()
   */
  void readData(char* data, int& len, int timeout = 5) const;

  /**
   * @see SocketCore::peekData()
   */
  void peekData(char* data, int& len, int timeout = 5) const;

  /**
   * @see SocketCore::initiateSecureConnection()
   */
  void initiateSecureConnection() const;
};

#endif // _D_SOCKET_H_
