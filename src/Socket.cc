/* <!-- copyright */
/*
 * aria2 - The high speed download utility
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
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

Socket::Socket(SocketCore* core) {
  this->core = core;
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

void Socket::beginListen(int32_t port) const {
  core->beginListen(port);
}

void Socket::getAddrInfo(pair<string, int32_t>& addrinfo) const {
  core->getAddrInfo(addrinfo);
}

void Socket::getPeerInfo(pair<string, int32_t>& peerinfo) const {
  core->getPeerInfo(peerinfo);
}

Socket* Socket::acceptConnection() const {
  return new Socket(core->acceptConnection());
}

void Socket::establishConnection(const string& host, int32_t port) const {
  core->establishConnection(host, port);
}

void Socket::setBlockingMode() const {
  core->setBlockingMode();
}

void Socket::closeConnection() const {
  core->closeConnection();
}

bool Socket::isWritable(int32_t timeout) const {
  return core->isWritable(timeout);
}

bool Socket::isReadable(int32_t timeout) const {
  return core->isReadable(timeout);
}

void Socket::writeData(const char* data, int32_t len) const {
  core->writeData(data, len);
}

void Socket::writeData(const string& str) const {
  core->writeData(str.c_str(), str.size());
}

void Socket::readData(char* data, int32_t& len) const {
  core->readData(data, len);
}

void Socket::peekData(char* data, int32_t& len) const {
  core->peekData(data, len);
}

void Socket::initiateSecureConnection() const {
  core->initiateSecureConnection();
}
