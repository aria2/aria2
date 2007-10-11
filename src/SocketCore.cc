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
#include "SocketCore.h"
#include "message.h"
#include "a2netcompat.h"
#include "a2time.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifndef __MINGW32__
# define SOCKET_ERRNO (errno)
#else
# define SOCKET_ERRNO (WSAGetLastError())
#endif // __MINGW32__

SocketCore::SocketCore():sockfd(-1) {
  init();
}

SocketCore::SocketCore(int32_t sockfd):sockfd(sockfd) {
  init();
}

void SocketCore::init()
{
  use = 1;
  blocking = true;
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

void SocketCore::beginListen(int32_t port)
  throw(DlAbortEx*)
{
  closeConnection();
  //sockfd = socket(AF_UNSPEC, SOCK_STREAM, PF_UNSPEC);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1) {
    throw new DlAbortEx(EX_SOCKET_OPEN, errorMsg());
  }
  SOCKOPT_T sockopt = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(socklen_t)) < 0) {
#ifdef __MINGW32__
    ::closesocket(sockfd);
#else
    close(sockfd);
#endif // __MINGW32__
    sockfd = -1;
    throw new DlAbortEx(EX_SOCKET_SET_OPT, errorMsg());
  }

  struct sockaddr_in sockaddr;
  memset((char*)&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(port);
  
  if(bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
    throw new DlAbortEx(EX_SOCKET_BIND, errorMsg());
  }

  if(listen(sockfd, 1) == -1) {
    throw new DlAbortEx(EX_SOCKET_LISTEN, errorMsg());
  }

  setNonBlockingMode();
}

SocketCore* SocketCore::acceptConnection() const
  throw(DlAbortEx*)
{
  struct sockaddr_in sockaddr;
  socklen_t len = sizeof(sockaddr);
  memset((char*)&sockaddr, 0, sizeof(sockaddr));
  int32_t fd;
  if((fd = accept(sockfd, (struct sockaddr*)&sockaddr, &len)) == -1) {
    throw new DlAbortEx(EX_SOCKET_ACCEPT, errorMsg());
  }

  SocketCore* s = new SocketCore(fd);
  return s;
}

void SocketCore::getAddrInfo(pair<string, int32_t>& addrinfo) const
  throw(DlAbortEx*)
{
  struct sockaddr_in listenaddr;
  memset((char*)&listenaddr, 0, sizeof(listenaddr));
  socklen_t len = sizeof(listenaddr);
  if(getsockname(sockfd, (struct sockaddr*)&listenaddr, &len) == -1) {
    throw new DlAbortEx(EX_SOCKET_GET_NAME, errorMsg());
  }
  addrinfo.first = inet_ntoa(listenaddr.sin_addr);
  addrinfo.second = ntohs(listenaddr.sin_port);
}

void SocketCore::getPeerInfo(pair<string, int32_t>& peerinfo) const
  throw(DlAbortEx*)
{
  struct sockaddr_in peerin;
  memset(&peerin, 0, sizeof(peerin));
  int32_t len = sizeof(peerin);
  if(getpeername(sockfd, (struct sockaddr*)&peerin, (socklen_t*)&len) < 0) {
    throw new DlAbortEx(EX_SOCKET_GET_PEER, errorMsg());
  }
  peerinfo.first = inet_ntoa(peerin.sin_addr);
  peerinfo.second = ntohs(peerin.sin_port);
}

void SocketCore::establishConnection(const string& host, int32_t port)
  throw(DlAbortEx*)
{
  closeConnection();
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1) {
      throw new DlAbortEx(EX_SOCKET_OPEN, errorMsg());
  }
  SOCKOPT_T sockopt = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(socklen_t)) < 0) {
    close(sockfd);
    sockfd = -1;
    throw new DlAbortEx(EX_SOCKET_SET_OPT, errorMsg());
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
    int32_t ec;
    if((ec = getaddrinfo(host.c_str(), NULL, &ai, &res)) != 0) {
      throw new DlAbortEx(EX_RESOLVE_HOSTNAME,
			  host.c_str(), gai_strerror(ec));
    }
    sockaddr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    freeaddrinfo(res);
  }
  // make socket non-blocking mode
  setNonBlockingMode();
  if(connect(sockfd, (struct sockaddr*)&sockaddr, (socklen_t)sizeof(sockaddr)) == -1 && SOCKET_ERRNO != 
#ifndef __MINGW32__
EINPROGRESS
#else
WSAEWOULDBLOCK
#endif // __MINGW32__
  ) {
    throw new DlAbortEx(EX_SOCKET_CONNECT, host.c_str(), errorMsg());
  }
}

void SocketCore::setNonBlockingMode()
  throw(DlAbortEx*)
{
#ifdef __MINGW32__
  static u_long flag = 1;
  if (::ioctlsocket(sockfd, FIONBIO, &flag) == -1) {
    throw new DlAbortEx(EX_SOCKET_NONBLOCKING, errorMsg());
  }
#else
  int32_t flags = fcntl(sockfd, F_GETFL, 0);
  // TODO add error handling
  fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);
#endif // __MINGW32__
  blocking = false;
}

void SocketCore::setBlockingMode()
  throw(DlAbortEx*)
{
#ifdef __MINGW32__
  static u_long flag = 0;
  if (::ioctlsocket(sockfd, FIONBIO, &flag) == -1) {
    throw new DlAbortEx(EX_SOCKET_BLOCKING, errorMsg());
  }
#else
  int32_t flags = fcntl(sockfd, F_GETFL, 0);
  // TODO add error handling
  fcntl(sockfd, F_SETFL, flags&(~O_NONBLOCK));
#endif // __MINGW32__
  blocking = true;
}

void SocketCore::closeConnection()
{
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
#ifdef __MINGW32__
  ::closesocket(sockfd);
#else
    close(sockfd);
#endif // __MINGW32__
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

bool SocketCore::isWritable(int32_t timeout) const
  throw(DlRetryEx*)
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd, &fds);

  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  int32_t r = select(sockfd+1, NULL, &fds, NULL, &tv);
  if(r == 1) {
    return true;
  } else if(r == 0) {
    // time out
    return false;
  } else {
    if(SOCKET_ERRNO == EINPROGRESS || SOCKET_ERRNO == EINTR) {
      return false;
    } else {
      throw new DlRetryEx(EX_SOCKET_CHECK_WRITABLE, errorMsg());
    }
  }
}

bool SocketCore::isReadable(int32_t timeout) const
  throw(DlRetryEx*)
{
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

  int32_t r = select(sockfd+1, &fds, NULL, NULL, &tv);
  if(r == 1) {
    return true;
  } else if(r == 0) {
    // time out
    return false;
  } else {
    if(SOCKET_ERRNO == EINPROGRESS || SOCKET_ERRNO == EINTR) {
      return false;
    } else {
      throw new DlRetryEx(EX_SOCKET_CHECK_READABLE, errorMsg());
    }
  }
}

void SocketCore::writeData(const char* data, int32_t len)
  throw(DlRetryEx*)
{
  int32_t ret = 0;

  if(!secure) {
    if((ret = send(sockfd, data, (size_t)len, 0)) != len) {
      throw new DlRetryEx(EX_SOCKET_SEND, errorMsg());
    }
  } else {
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
    if((ret = SSL_write(ssl, data, len)) != len) {
      throw new DlRetryEx(EX_SOCKET_SEND, ERR_error_string(ERR_get_error(), NULL));
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    if((ret = gnutls_record_send(sslSession, data, len)) == len) {
      throw new DlRetryEx(EX_SOCKET_SEND, gnutls_strerror(ret));
    }
#endif // HAVE_LIBGNUTLS
  }
}

void SocketCore::readData(char* data, int32_t& len)
  throw(DlRetryEx*)
{
  int32_t ret = 0;

  if(!secure) {
    if ((ret = recv(sockfd, data, (size_t)len, 0)) < 0) {
      throw new DlRetryEx(EX_SOCKET_RECV, errorMsg());
    }
  } else {
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
    if ((ret = SSL_read(ssl, data, len)) < 0) {
      throw new DlRetryEx(EX_SOCKET_RECV, ERR_error_string(ERR_get_error(), NULL));
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    if ((ret = gnutlsRecv(data, len)) < 0) {
      throw new DlRetryEx(EX_SOCKET_RECV, gnutls_strerror(ret));
    }
#endif // HAVE_LIBGNUTLS
  }

  len = ret;
}

void SocketCore::peekData(char* data, int32_t& len)
  throw(DlRetryEx*)
{
  int32_t ret = 0;

  if(!secure) {
    if ((ret = recv(sockfd, data, (size_t)len, MSG_PEEK)) < 0) {
      throw new DlRetryEx(EX_SOCKET_PEEK, errorMsg());
    }
  } else {
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
    if ((ret = SSL_peek(ssl, data, len)) < 0) {
      throw new DlRetryEx(EX_SOCKET_PEEK, ERR_error_string(ERR_get_error(), NULL));
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    if ((ret = gnutlsPeek(data, len)) < 0) {
      throw new DlRetryEx(EX_SOCKET_PEEK, gnutls_strerror(ret));
    }
#endif // HAVE_LIBGNUTLS
  }

  len = ret;
}

#ifdef HAVE_LIBGNUTLS
int32_t SocketCore::shiftPeekData(char* data, int32_t len)
{
  if(peekBufLength <= len) {
    memcpy(data, peekBuf, peekBufLength);
    int32_t ret = peekBufLength;
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

void SocketCore::addPeekData(char* data, int32_t len)
{
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

int32_t SocketCore::gnutlsRecv(char* data, int32_t len)
  throw(DlRetryEx*)
{
  int32_t plen = shiftPeekData(data, len);
  if(plen < len) {
    int32_t ret = gnutls_record_recv(sslSession, data+plen, len-plen);
    if(ret < 0) {
      throw new DlRetryEx(EX_SOCKET_RECV, gnutls_strerror(ret));
    }
    return plen+ret;
  } else {
    return plen;
  }
}

int32_t SocketCore::gnutlsPeek(char* data, int32_t len)
  throw(DlRetryEx*)
{
  if(peekBufLength >= len) {
    memcpy(data, peekBuf, len);
    return len;
  } else {
    memcpy(data, peekBuf, peekBufLength);
    int32_t ret = gnutls_record_recv(sslSession, data+peekBufLength, len-peekBufLength);
    if(ret < 0) {
      throw new DlRetryEx(EX_SOCKET_PEEK, gnutls_strerror(ret));
    }
    addPeekData(data+peekBufLength, ret);
    return peekBufLength;
  }
}
#endif // HAVE_LIBGNUTLS

void SocketCore::initiateSecureConnection()
  throw(DlAbortEx*)
{
#ifdef HAVE_LIBSSL
  // for SSL
  if(!secure) {
    sslCtx = SSL_CTX_new(SSLv23_client_method());
    if(sslCtx == NULL) {
      throw new DlAbortEx(EX_SSL_INIT_FAILURE, ERR_error_string(ERR_get_error(), NULL));
    }
    SSL_CTX_set_mode(sslCtx, SSL_MODE_AUTO_RETRY);
    ssl = SSL_new(sslCtx);
    if(ssl == NULL) {
      throw new DlAbortEx(EX_SSL_INIT_FAILURE, ERR_error_string(ERR_get_error(), NULL));
    }
    if(SSL_set_fd(ssl, sockfd) == 0) {
      throw new DlAbortEx(EX_SSL_INIT_FAILURE, ERR_error_string(ERR_get_error(), NULL));
    }
     // TODO handling return value == 0 case required
    int e = SSL_connect(ssl);

    if (e <= 0) {
      int ssl_error = SSL_get_error(ssl, e);
      switch(ssl_error) {
        case SSL_ERROR_NONE:
          break;

        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
        case SSL_ERROR_WANT_X509_LOOKUP:
        case SSL_ERROR_ZERO_RETURN:
          if (blocking) {
            throw new DlAbortEx(EX_SSL_CONNECT_ERROR, ssl_error);
          }
          break;

        case SSL_ERROR_SYSCALL:
          throw new DlAbortEx(EX_SSL_IO_ERROR);

        case SSL_ERROR_SSL:
          throw new DlAbortEx(EX_SSL_PROTOCOL_ERROR);

        default:
          throw new DlAbortEx(EX_SSL_UNKNOWN_ERROR, ssl_error);
      }
    }
  }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  if(!secure) {
    const int32_t cert_type_priority[3] = { GNUTLS_CRT_X509,
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
    int32_t ret = gnutls_handshake(sslSession);
    if(ret < 0) {
      throw new DlAbortEx(EX_SSL_INIT_FAILURE, gnutls_strerror(ret));
    }
  }
#endif // HAVE_LIBGNUTLS

  secure = true;
}

/* static */ int SocketCore::error()
{
  return SOCKET_ERRNO;
}

/* static */ const char *SocketCore::errorMsg()
{
  return errorMsg(SOCKET_ERRNO);
}

/* static */ const char *SocketCore::errorMsg(const int err)
{
#ifndef __MINGW32__
  return strerror(err);
#else
  static char buf[256];
  if (FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    err,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &buf,
    sizeof(buf),
    NULL
  ) == 0) {
    snprintf(buf, sizeof(buf), EX_SOCKET_UNKNOWN_ERROR, err, err);
  }
  return buf;
#endif // __MINGW32__
}
