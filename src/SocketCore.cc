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
#include "message.h"

SocketCore::SocketCore():sockfd(-1) {
  init();
}

SocketCore::SocketCore(int sockfd):sockfd(sockfd) {
  init();
}

void SocketCore::init() {
  use = 1;
  secure = false;
#ifdef HAVE_LIBSSL
  // for SSL
  sslCtx = NULL;
  ssl = NULL;
#endif // HAVE_LIBSSL
}

SocketCore::~SocketCore() {
  closeConnection();
}

void SocketCore::beginListen(int port) {
  closeConnection();
  //sockfd = socket(AF_UNSPEC, SOCK_STREAM, PF_UNSPEC);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1) {
    throw new DlAbortEx(strerror(errno));
  }
  socklen_t sockopt = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(socklen_t)) < 0) {
    close(sockfd);
    sockfd = -1;
    throw new DlAbortEx(strerror(errno));
  }

  struct sockaddr_in sockaddr;
  memset((char*)&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(port);
  
  if(bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
    throw new DlAbortEx(strerror(errno));
  }

  if(listen(sockfd, 1) == -1) {
    throw new DlAbortEx(strerror(errno));
  }
}

SocketCore* SocketCore::acceptConnection() const {
  struct sockaddr_in sockaddr;
  socklen_t len = sizeof(sockaddr);
  memset((char*)&sockaddr, 0, sizeof(sockaddr));
  int fd;
  if((fd = accept(sockfd, (struct sockaddr*)&sockaddr, &len)) == -1) {
    throw new DlAbortEx(strerror(errno));
  }

  SocketCore* s = new SocketCore(fd);
  return s;
}

void SocketCore::getAddrInfo(pair<string, int>& addrinfo) const {
  struct sockaddr_in listenaddr;
  memset((char*)&listenaddr, 0, sizeof(listenaddr));
  socklen_t len = sizeof(listenaddr);
  if(getsockname(sockfd, (struct sockaddr*)&listenaddr, &len) == -1) {
    throw new DlAbortEx(strerror(errno));
  }
  addrinfo.first = inet_ntoa(listenaddr.sin_addr);
  addrinfo.second = ntohs(listenaddr.sin_port);
}

void SocketCore::getPeerInfo(pair<string, int>& peerinfo) const {
  struct sockaddr_in peerin;
  memset(&peerin, 0, sizeof(peerin));
  int len = sizeof(peerin);
  if(getpeername(sockfd, (struct sockaddr*)&peerin, (socklen_t*)&len) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
  peerinfo.first = inet_ntoa(peerin.sin_addr);
  peerinfo.second = ntohs(peerin.sin_port);
}

void SocketCore::establishConnection(string host, int port) {
  closeConnection();
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1) {
      throw new DlAbortEx(strerror(errno));
  }
  socklen_t sockopt = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(socklen_t)) < 0) {
    close(sockfd);
    sockfd = -1;
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
    memset((char*)&ai, 0, sizeof(ai));
    ai.ai_flags = 0;
    ai.ai_family = PF_INET;
    ai.ai_socktype = SOCK_STREAM;
    ai.ai_protocol = 0; 
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

void SocketCore::setBlockingMode() const {
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags&~O_NONBLOCK);
}

void SocketCore::closeConnection() {
#ifdef HAVE_LIBSSL
  // for SSL
  if(secure && ssl != NULL) {
    SSL_shutdown(ssl);
  }
#endif // HAVE_LIBSSL
  if(sockfd != -1) {
    close(sockfd);
    sockfd = -1;
  }
#ifdef HAVE_LIBSSL
  // for SSL
  if(secure && ssl != NULL) {
    SSL_free(ssl);
    SSL_CTX_free(sslCtx);
    ssl = NULL;
    sslCtx = NULL;
  }
#endif // HAVE_LIBSSL
}

bool SocketCore::isWritable(int timeout) const {
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
    if(errno == EINPROGRESS) {
      return false;
    } else {
      throw new DlRetryEx(strerror(errno));
    }
  }
}

bool SocketCore::isReadable(int timeout) const {
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
    if(errno == EINPROGRESS) {
      return false;
    } else {
      throw new DlRetryEx(strerror(errno));
    }
  }
}

void SocketCore::writeData(const char* data, int len, int timeout) const {
  if(!secure && send(sockfd, data, (size_t)len, 0) != len
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
     || secure && SSL_write(ssl, data, len) != len
#endif // HAVE_LIBSSL
     ) {
    throw new DlRetryEx(strerror(errno));
  }
}

void SocketCore::readData(char* data, int& len, int timeout) const {
  if(!secure && (len = recv(sockfd, data, (size_t)len, 0)) < 0
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
     || secure && (len = SSL_read(ssl, data, len)) < 0
#endif // HAVE_LIBSSL
     ) {
    throw new DlRetryEx(strerror(errno));
  }
}

void SocketCore::peekData(char* data, int& len, int timeout) const {
  if(!secure && (len = recv(sockfd, data, (size_t)len, MSG_PEEK)) < 0
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
     || secure && (len == SSL_peek(ssl, data, len)) < 0
#endif // HAVE_LIBSSL
     ) {
    throw new DlRetryEx(strerror(errno));
  }
}

void SocketCore::initiateSecureConnection() {
#ifdef HAVE_LIBSSL
  // for SSL
  if(!secure) {
    sslCtx = SSL_CTX_new(SSLv23_client_method());
    if(sslCtx == NULL) {
      throw new DlAbortEx(EX_SSL_INIT_FAILURE);
    }
    SSL_CTX_set_mode(sslCtx, SSL_MODE_AUTO_RETRY);
    ssl = SSL_new(sslCtx);
    if(ssl == NULL) {
      throw new DlAbortEx(EX_SSL_INIT_FAILURE);
    }
    if(SSL_set_fd(ssl, sockfd) == 0) {
      throw new DlAbortEx(EX_SSL_INIT_FAILURE);
    }
     // TODO handling return value == 0 case required
    if(SSL_connect(ssl) <= 0) {
      throw new DlAbortEx(EX_SSL_INIT_FAILURE);
    }
    secure = true;
  }
#endif // HAVE_LIBSSL
}

