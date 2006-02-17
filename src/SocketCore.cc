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
#include "SocketCore.h"
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include <errno.h>

SocketCore::SocketCore():sockfd(-1), use(1) {}

SocketCore::~SocketCore() {
  closeConnection();
}

void SocketCore::establishConnection(string host, int port) {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd >= 0) {
    socklen_t sockopt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0) {
      close(sockfd);
      sockfd = -1;
      throw new DlAbortEx(strerror(errno));
    }
  } else {
      throw new DlAbortEx(strerror(errno));
  }

  struct sockaddr_in sockaddr;
  memset((char*)&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  if(inet_aton(host.c_str(), &sockaddr.sin_addr)) {
    // ok
  } else {
    struct addrinfo ai;
    ai.ai_flags = 0;
    ai.ai_family = PF_INET;
    ai.ai_socktype = SOCK_STREAM;
    ai.ai_protocol = 0; 
    ai.ai_addr = (struct sockaddr*)&sockaddr;
    struct addrinfo* res;
    int ec;
    if((ec = getaddrinfo(host.c_str(), NULL, &ai, &res)) != 0) {
      throw new DlAbortEx(gai_strerror(ec));
    }
    sockaddr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    freeaddrinfo(res);
  }
  // make socket non-blocking mode
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);
  if(connect(sockfd, (struct sockaddr*)&sockaddr, (socklen_t)sizeof(sockaddr)) == -1 && errno != EINPROGRESS) {
    throw new DlAbortEx(strerror(errno));
  }
}

void SocketCore::setNonBlockingMode() {
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags&~O_NONBLOCK);
}

void SocketCore::closeConnection() {
  if(sockfd != -1) {
    close(sockfd);
    sockfd = -1;
  }
}

bool SocketCore::isWritable(int timeout) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd, &fds);

  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  int r = select(sockfd+1, NULL, &fds, NULL, &tv);
  if(r == 1) {
    return true;
  } else if(r == 0) {
    // time out
    return false;
  } else {
    throw new DlRetryEx(strerror(errno));
  }
}

bool SocketCore::isReadable(int timeout) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd, &fds);

  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  int r = select(sockfd+1, &fds, NULL, NULL, &tv);
  if(r == 1) {
    return true;
  } else if(r == 0) {
    // time out
    return false;
  } else {
    throw new DlRetryEx(strerror(errno));
  }
}

void SocketCore::writeData(const char* data, int len, int timeout) {
  if(!isWritable(timeout) || send(sockfd, data, (size_t)len, 0) != len) {
    throw new DlRetryEx(strerror(errno));
  }
}

void SocketCore::readData(char* data, int& len, int timeout) {
  if(!isReadable(timeout) || (len = recv(sockfd, data, (size_t)len, 0)) < 0) {
    throw new DlRetryEx(strerror(errno));
  }
}

void SocketCore::peekData(char* data, int& len, int timeout) {
  if(!isReadable(timeout) || (len = recv(sockfd, data, (size_t)len, MSG_PEEK)) < 0) {
    throw new DlRetryEx(strerror(errno));
  }
}
