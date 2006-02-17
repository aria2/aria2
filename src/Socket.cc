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
#include "Socket.h"

Socket::Socket() {
  core = new SocketCore();
}

Socket::Socket(const Socket& s) {
  core = s.core;
  core->use++;
}

Socket::~Socket() {
  core->use--;
  if(core->use == 0) {
    delete core;
  }
}

Socket& Socket::operator=(const Socket& s) {
  if(this != &s) {
    core->use--;
    if(core->use == 0) {
      delete core;
    }
    core = s.core;
    core->use++;
  }
  return *this;
}

void Socket::establishConnection(string host, int port) {
  core->establishConnection(host, port);
}

void Socket::setNonBlockingMode() {
  core->setNonBlockingMode();
}

void Socket::closeConnection() {
  core->closeConnection();
}

bool Socket::isWritable(int timeout) {
  return core->isWritable(timeout);
}

bool Socket::isReadable(int timeout) {
  return core->isReadable(timeout);
}

void Socket::writeData(const char* data, int len, int timeout) {
  core->writeData(data, len, timeout);
}

void Socket::readData(char* data, int& len, int timeout) {
  core->readData(data, len, timeout);
}

void Socket::peekData(char* data, int& len, int timeout) {
  core->peekData(data, len, timeout);
}

#ifdef HAVE_LIBSSL
// for SSL
void Socket::initiateSecureConnection() {
  core->initiateSecureConnection();
}
#endif // HAVE_LIBSSL
