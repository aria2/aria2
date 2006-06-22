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
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "message.h"
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>

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
#ifdef HAVE_LIBGNUTLS
  sslSession = NULL;
  sslXcred = NULL;
  peekBufMax = 4096;
  peekBuf = new char[peekBufMax];
  peekBufLength = 0;
#endif //HAVE_LIBGNUTLS
}

SocketCore::~SocketCore() {
  closeConnection();
#ifdef HAVE_LIBGNUTLS
  delete [] peekBuf;
#endif // HAVE_LIBGNUTLS
}

void SocketCore::beginListen(int port) {
  closeConnection();
  //sockfd = socket(AF_UNSPEC, SOCK_STREAM, PF_UNSPEC);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1) {
    throw new DlAbortEx(EX_SOCKET_OPEN, strerror(errno));
  }
  socklen_t sockopt = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(socklen_t)) < 0) {
    close(sockfd);
    sockfd = -1;
    throw new DlAbortEx(EX_SOCKET_SET_OPT, strerror(errno));
  }

  struct sockaddr_in sockaddr;
  memset((char*)&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(port);
  
  if(bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
    throw new DlAbortEx(EX_SOCKET_BIND, strerror(errno));
  }

  if(listen(sockfd, 1) == -1) {
    throw new DlAbortEx(EX_SOCKET_LISTEN, strerror(errno));
  }

  setNonBlockingMode();
}

SocketCore* SocketCore::acceptConnection() const {
  struct sockaddr_in sockaddr;
  socklen_t len = sizeof(sockaddr);
  memset((char*)&sockaddr, 0, sizeof(sockaddr));
  int fd;
  if((fd = accept(sockfd, (struct sockaddr*)&sockaddr, &len)) == -1) {
    throw new DlAbortEx(EX_SOCKET_ACCEPT, strerror(errno));
  }

  SocketCore* s = new SocketCore(fd);
  return s;
}

void SocketCore::getAddrInfo(pair<string, int>& addrinfo) const {
  struct sockaddr_in listenaddr;
  memset((char*)&listenaddr, 0, sizeof(listenaddr));
  socklen_t len = sizeof(listenaddr);
  if(getsockname(sockfd, (struct sockaddr*)&listenaddr, &len) == -1) {
    throw new DlAbortEx(EX_SOCKET_GET_NAME, strerror(errno));
  }
  addrinfo.first = inet_ntoa(listenaddr.sin_addr);
  addrinfo.second = ntohs(listenaddr.sin_port);
}

void SocketCore::getPeerInfo(pair<string, int>& peerinfo) const {
  struct sockaddr_in peerin;
  memset(&peerin, 0, sizeof(peerin));
  int len = sizeof(peerin);
  if(getpeername(sockfd, (struct sockaddr*)&peerin, (socklen_t*)&len) < 0) {
    throw new DlAbortEx(EX_SOCKET_GET_PEER, strerror(errno));
  }
  peerinfo.first = inet_ntoa(peerin.sin_addr);
  peerinfo.second = ntohs(peerin.sin_port);
}

void SocketCore::establishConnection(const string& host, int port) {
  closeConnection();
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1) {
      throw new DlAbortEx(EX_SOCKET_OPEN, strerror(errno));
  }
  socklen_t sockopt = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(socklen_t)) < 0) {
    close(sockfd);
    sockfd = -1;
    throw new DlAbortEx(EX_SOCKET_SET_OPT, strerror(errno));
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
      throw new DlAbortEx(EX_RESOLVE_HOSTNAME,
			  host.c_str(), gai_strerror(ec));
    }
    sockaddr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    freeaddrinfo(res);
  }
  // make socket non-blocking mode
  setNonBlockingMode();
  if(connect(sockfd, (struct sockaddr*)&sockaddr, (socklen_t)sizeof(sockaddr)) == -1 && errno != EINPROGRESS) {
    throw new DlAbortEx(EX_SOCKET_CONNECT, host.c_str(), strerror(errno));
  }
}

void SocketCore::setNonBlockingMode() const {
  int flags = fcntl(sockfd, F_GETFL, 0);
  // TODO add error handling
  fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);
}

void SocketCore::setBlockingMode() const {
  int flags = fcntl(sockfd, F_GETFL, 0);
  // TODO add error handling
  fcntl(sockfd, F_SETFL, flags&(~O_NONBLOCK));
}

void SocketCore::closeConnection() {
#ifdef HAVE_LIBSSL
  // for SSL
  if(secure) {
    SSL_shutdown(ssl);
  }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  if(secure) {
    gnutls_bye(sslSession, GNUTLS_SHUT_RDWR);
  }
#endif // HAVE_LIBGNUTLS
  if(sockfd != -1) {
    close(sockfd);
    sockfd = -1;
  }
#ifdef HAVE_LIBSSL
  // for SSL
  if(secure) {
    SSL_free(ssl);
    SSL_CTX_free(sslCtx);
  }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  if(secure) {
    gnutls_deinit(sslSession);
    gnutls_certificate_free_credentials(sslXcred);
  }
#endif // HAVE_LIBGNUTLS
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
    if(errno == EINPROGRESS || errno == EINTR) {
      return false;
    } else {
      throw new DlRetryEx(EX_SOCKET_CHECK_WRITABLE, strerror(errno));
    }
  }
}

bool SocketCore::isReadable(int timeout) const {
#ifdef HAVE_LIBGNUTLS
  if(secure && peekBufLength > 0) {
    return true;
  }
#endif // HAVE_LIBGNUTLS
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
    if(errno == EINPROGRESS || errno == EINTR) {
      return false;
    } else {
      throw new DlRetryEx(EX_SOCKET_CHECK_READABLE, strerror(errno));
    }
  }
}

void SocketCore::writeData(const char* data, int len) {
  int ret = 0;
  if(!secure && (ret = send(sockfd, data, (size_t)len, 0)) != len
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
     || secure && (ret = SSL_write(ssl, data, len)) != len
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
     || secure && (ret = gnutls_record_send(sslSession, data, len)) != len
#endif // HAVE_LIBGNUTLS
     ) {
    const char* errorMsg;
#ifdef HAVE_LIBGNUTLS
    if(secure) {
      errorMsg = gnutls_strerror(ret);
    } else {
      errorMsg = strerror(errno);
    }
#else // HAVE_LIBGNUTLS
    errorMsg = strerror(errno);
#endif
    throw new DlRetryEx(EX_SOCKET_SEND, errorMsg);
  }
}

void SocketCore::readData(char* data, int& len) {
  int ret = 0;
  if(!secure && (ret = recv(sockfd, data, (size_t)len, 0)) < 0
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
     || secure && (ret = SSL_read(ssl, data, len)) < 0
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
     || secure && (ret = gnutlsRecv(data, len)) < 0
#endif // HAVE_LIBGNUTLS
     ) {
    const char* errorMsg;
#ifdef HAVE_LIBGNUTLS
    if(secure) {
      errorMsg = gnutls_strerror(ret);
    } else {
      errorMsg = strerror(errno);
    }
#else // HAVE_LIBGNUTLS
    errorMsg = strerror(errno);
#endif
    throw new DlRetryEx(EX_SOCKET_RECV, errorMsg);
  }
  len = ret;
}

void SocketCore::peekData(char* data, int& len) {
  int ret = 0;
  if(!secure && (ret = recv(sockfd, data, (size_t)len, MSG_PEEK)) < 0
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
     || secure && (ret = SSL_peek(ssl, data, len)) < 0
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
     || secure && (ret = gnutlsPeek(data, len)) < 0
#endif // HAVE_LIBGNUTLS
     ) {
    const char* errorMsg;
#ifdef HAVE_LIBGNUTLS
    if(secure) {
      errorMsg = gnutls_strerror(ret);
    } else {
      errorMsg = strerror(errno);
    }
#else // HAVE_LIBGNUTLS
    errorMsg = strerror(errno);
#endif
    throw new DlRetryEx(EX_SOCKET_PEEK, errorMsg);
  }
  len = ret;
}

#ifdef HAVE_LIBGNUTLS
int SocketCore::shiftPeekData(char* data, int len) {
  if(peekBufLength <= len) {
    memcpy(data, peekBuf, peekBufLength);
    int ret = peekBufLength;
    peekBufLength = 0;
    return ret;
  } else {
    memcpy(data, peekBuf, len);
    char* temp = new char[peekBufMax];
    memcpy(temp, peekBuf+len, peekBufLength-len);
    delete [] peekBuf;
    peekBuf = temp;
    peekBufLength -= len;
    return len;
  }

}

void SocketCore::addPeekData(char* data, int len) {
  if(peekBufLength+len > peekBufMax) {
    char* temp = new char[peekBufMax+len];
    memcpy(temp, peekBuf, peekBufLength);
    delete [] peekBuf;
    peekBuf = temp;
    peekBufMax = peekBufLength+len;
  }
  memcpy(peekBuf+peekBufLength, data, len);
  peekBufLength += len;
}

int SocketCore::gnutlsRecv(char* data, int len) {
  int plen = shiftPeekData(data, len);
  if(plen < len) {
    int ret = gnutls_record_recv(sslSession, data+plen, len-plen);
    if(ret < 0) {
      throw new DlRetryEx(EX_SOCKET_RECV, gnutls_strerror(ret));
    }
    return plen+ret;
  } else {
    return plen;
  }
}

int SocketCore::gnutlsPeek(char* data, int len) {
  if(peekBufLength >= len) {
    memcpy(data, peekBuf, len);
    return len;
  } else {
    memcpy(data, peekBuf, peekBufLength);
    int ret = gnutls_record_recv(sslSession, data+peekBufLength, len-peekBufLength);
    if(ret < 0) {
      throw new DlRetryEx(EX_SOCKET_PEEK, gnutls_strerror(ret));
    }
    addPeekData(data+peekBufLength, ret);
    return peekBufLength;
  }
}
#endif // HAVE_LIBGNUTLS

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
#ifdef HAVE_LIBGNUTLS
  if(!secure) {
    const int cert_type_priority[3] = { GNUTLS_CRT_X509,
					GNUTLS_CRT_OPENPGP, 0
    };
    // while we do not support X509 certificate, most web servers require
    // X509 stuff.
    gnutls_certificate_allocate_credentials (&sslXcred);
    gnutls_init(&sslSession, GNUTLS_CLIENT);
    gnutls_set_default_priority(sslSession);
    gnutls_kx_set_priority(sslSession, cert_type_priority);
    // put the x509 credentials to the current session
    gnutls_credentials_set(sslSession, GNUTLS_CRD_CERTIFICATE, sslXcred);
    gnutls_transport_set_ptr(sslSession, (gnutls_transport_ptr_t)sockfd);
    int ret = gnutls_handshake(sslSession);
    if(ret < 0) {
      throw new DlAbortEx(gnutls_strerror(ret));
    }
    secure = true;
  }
#endif // HAVE_LIBGNUTLS
}

