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
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include <unistd.h>
#include <cerrno>
#include <cstring>
#ifndef __MINGW32__
# define SOCKET_ERRNO (errno)
#else
# define SOCKET_ERRNO (WSAGetLastError())
#endif // __MINGW32__

#ifdef __MINGW32__
# define A2_EINPROGRESS WSAEWOULDBLOCK
#else
# define A2_EINPROGRESS EINPROGRESS
#endif // __MINGW32__

#ifdef __MINGW32__
# define CLOSE(X) ::closesocket(sockfd)
#else
# define CLOSE(X) while(close(X) == -1 && errno == EINTR)
#endif // __MINGW32__

namespace aria2 {

SocketCore::SocketCore(int sockType):_sockType(sockType), sockfd(-1) {
  init();
}

SocketCore::SocketCore(int sockfd, int sockType):_sockType(sockType), sockfd(sockfd) {
  init();
}

void SocketCore::init()
{
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
  peekBuf = 0;
  peekBufLength = 0;
#endif //HAVE_LIBGNUTLS
}

SocketCore::~SocketCore() {
  closeConnection();
#ifdef HAVE_LIBGNUTLS
  delete [] peekBuf;
#endif // HAVE_LIBGNUTLS
}

template<typename T>
std::string uitos(T value)
{
  std::string str;
  if(value == 0) {
    str = "0";
    return str;
  }
  while(value) {
    char digit = value%10+'0';
    str.insert(str.begin(), digit);
    value /= 10;
  }
  return str;
}

void SocketCore::bind(uint16_t port)
{
  closeConnection();

  struct addrinfo hints;
  struct addrinfo* res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = _sockType;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  int s;
  s = getaddrinfo(0, uitos(port).c_str(), &hints, &res);
  if(s) {
    throw new DlAbortEx(EX_SOCKET_BIND, gai_strerror(s));
  }
  struct addrinfo* rp;
  for(rp = res; rp; rp = rp->ai_next) {
    int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(fd == -1) {
      continue;
    }
    SOCKOPT_T sockopt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(socklen_t)) < 0) {
      CLOSE(fd);
      continue;
    }
    if(::bind(fd, rp->ai_addr, rp->ai_addrlen) == -1) {
      CLOSE(fd);
      continue;
    }
    sockfd = fd;
    break;
  }
  freeaddrinfo(res);
  if(sockfd == -1) {
    throw new DlAbortEx(EX_SOCKET_BIND, "all addresses failed");
  }
}

void SocketCore::beginListen()
{
  if(listen(sockfd, 1) == -1) {
    throw new DlAbortEx(EX_SOCKET_LISTEN, errorMsg());
  }
}

SocketCore* SocketCore::acceptConnection() const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  int fd;
  while((fd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&sockaddr), &len)) == -1 && errno == EINTR);
  if(fd == -1) {
    throw new DlAbortEx(EX_SOCKET_ACCEPT, errorMsg());
  }
  return new SocketCore(fd, _sockType);
}

std::pair<std::string, uint16_t>
SocketCore::getNameInfoInNumeric(const struct sockaddr* sockaddr, socklen_t len)
{
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];
  int s = getnameinfo(sockaddr, len, host, NI_MAXHOST, service, NI_MAXSERV,
		      NI_NUMERICHOST|NI_NUMERICSERV);
  if(s != 0) {
    throw new DlAbortEx("Failed to get hostname and port. cause: %s",
			gai_strerror(s));
  }
  return std::pair<std::string, uint16_t>(host, atoi(service)); // TODO
}

void SocketCore::getAddrInfo(std::pair<std::string, uint16_t>& addrinfo) const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  if(getsockname(sockfd, addrp, &len) == -1) {
    throw new DlAbortEx(EX_SOCKET_GET_NAME, errorMsg());
  }
  addrinfo = SocketCore::getNameInfoInNumeric(addrp, len);
}

void SocketCore::getPeerInfo(std::pair<std::string, uint16_t>& peerinfo) const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  if(getpeername(sockfd, addrp, &len) == -1) {
    throw new DlAbortEx(EX_SOCKET_GET_NAME, errorMsg());
  }
  peerinfo = SocketCore::getNameInfoInNumeric(addrp, len);
}

void SocketCore::establishConnection(const std::string& host, uint16_t port)
{
  closeConnection();

  struct addrinfo hints;
  struct addrinfo* res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = _sockType;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  int s;
  s = getaddrinfo(host.c_str(), uitos(port).c_str(), &hints, &res);
  if(s) {
    throw new DlAbortEx(EX_RESOLVE_HOSTNAME, host.c_str(), gai_strerror(s));
  }
  struct addrinfo* rp;
  for(rp = res; rp; rp = rp->ai_next) {
    int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(fd == -1) {
      continue;
    }
    SOCKOPT_T sockopt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(socklen_t)) < 0) {
      CLOSE(fd);
      continue;
    }
    sockfd = fd;
    // make socket non-blocking mode
    setNonBlockingMode();
    if(connect(fd, rp->ai_addr, rp->ai_addrlen) == -1 &&
       SOCKET_ERRNO != A2_EINPROGRESS) {
      CLOSE(sockfd);
      sockfd = -1;
      continue;
    }
    // TODO at this point, connection may not be established and it may fail
    // later. In such case, next ai_addr should be tried.
    break;
  }
  freeaddrinfo(res);
  if(sockfd == -1) {
    throw new DlAbortEx(EX_SOCKET_CONNECT, host.c_str(), "all addresses failed");
  }
}

void SocketCore::setNonBlockingMode()
{
#ifdef __MINGW32__
  static u_long flag = 1;
  if (::ioctlsocket(sockfd, FIONBIO, &flag) == -1) {
    throw new DlAbortEx(EX_SOCKET_NONBLOCKING, errorMsg());
  }
#else
  int flags;
  while((flags = fcntl(sockfd, F_GETFL, 0)) == -1 && errno == EINTR);
  // TODO add error handling
  while(fcntl(sockfd, F_SETFL, flags|O_NONBLOCK) == -1 && errno == EINTR);
#endif // __MINGW32__
  blocking = false;
}

void SocketCore::setBlockingMode()
{
#ifdef __MINGW32__
  static u_long flag = 0;
  if (::ioctlsocket(sockfd, FIONBIO, &flag) == -1) {
    throw new DlAbortEx(EX_SOCKET_BLOCKING, errorMsg());
  }
#else
  int flags;
  while((flags = fcntl(sockfd, F_GETFL, 0)) == -1 && errno == EINTR);
  // TODO add error handling
  while(fcntl(sockfd, F_SETFL, flags&(~O_NONBLOCK)) == -1 && errno == EINTR);
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
    CLOSE(sockfd);
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

bool SocketCore::isWritable(time_t timeout) const
{
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
    if(SOCKET_ERRNO == EINPROGRESS || SOCKET_ERRNO == EINTR) {
      return false;
    } else {
      throw new DlRetryEx(EX_SOCKET_CHECK_WRITABLE, errorMsg());
    }
  }
}

bool SocketCore::isReadable(time_t timeout) const
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

  int r = select(sockfd+1, &fds, NULL, NULL, &tv);
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

void SocketCore::writeData(const char* data, size_t len)
{
  ssize_t ret = 0;

  if(!secure) {
    while((ret = send(sockfd, data, len, 0)) == -1 && errno == EINTR);
    // TODO assuming Blocking mode.
    if(ret == -1 || (size_t)ret != len) {
      throw new DlRetryEx(EX_SOCKET_SEND, errorMsg());
    }
  } else {
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
    ret = SSL_write(ssl, data, len);
    if(ret <= 0 || (size_t)ret != len) {
      throw new DlRetryEx(EX_SOCKET_SEND, ERR_error_string(ERR_get_error(), NULL));
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    ret = gnutls_record_send(sslSession, data, len);
    if(ret < 0 || (size_t)ret != len) {
      throw new DlRetryEx(EX_SOCKET_SEND, gnutls_strerror(ret));
    }
#endif // HAVE_LIBGNUTLS
  }
}

void SocketCore::readData(char* data, size_t& len)
{
  ssize_t ret = 0;

  if(!secure) {    
    while((ret = recv(sockfd, data, len, 0)) == -1 && errno == EINTR);
    if(ret == -1) {
      throw new DlRetryEx(EX_SOCKET_RECV, errorMsg());
    }
  } else {
#ifdef HAVE_LIBSSL
     // for SSL
     // TODO handling len == 0 case required
    if ((ret = SSL_read(ssl, data, len)) <= 0) {
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

void SocketCore::peekData(char* data, size_t& len)
{
  ssize_t ret = 0;

  if(!secure) {
    while((ret = recv(sockfd, data, len, MSG_PEEK)) == -1 && errno == EINTR);
    if(ret == -1) {
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
size_t SocketCore::shiftPeekData(char* data, size_t len)
{
  if(peekBufLength <= len) {
    memcpy(data, peekBuf, peekBufLength);
    size_t ret = peekBufLength;
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

void SocketCore::addPeekData(char* data, size_t len)
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

ssize_t SocketCore::gnutlsRecv(char* data, size_t len)
{
  size_t plen = shiftPeekData(data, len);
  if(plen < len) {
    ssize_t ret = gnutls_record_recv(sslSession, data+plen, len-plen);
    if(ret < 0) {
      throw new DlRetryEx(EX_SOCKET_RECV, gnutls_strerror(ret));
    }
    return plen+ret;
  } else {
    return plen;
  }
}

ssize_t SocketCore::gnutlsPeek(char* data, size_t len)
{
  if(peekBufLength >= len) {
    memcpy(data, peekBuf, len);
    return len;
  } else {
    memcpy(data, peekBuf, peekBufLength);
    ssize_t ret = gnutls_record_recv(sslSession, data+peekBufLength, len-peekBufLength);
    if(ret < 0) {
      throw new DlRetryEx(EX_SOCKET_PEEK, gnutls_strerror(ret));
    }
    addPeekData(data+peekBufLength, ret);
    return peekBufLength;
  }
}
#endif // HAVE_LIBGNUTLS

void SocketCore::initiateSecureConnection()
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
      throw new DlAbortEx(EX_SSL_INIT_FAILURE, gnutls_strerror(ret));
    }
    peekBuf = new char[peekBufMax];
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

void SocketCore::writeData(const char* data, size_t len, const std::string& host, uint16_t port)
{

  struct addrinfo hints;
  struct addrinfo* res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = _sockType;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  int s;
  s = getaddrinfo(host.c_str(), uitos(port).c_str(), &hints, &res);
  if(s) {
    throw new DlAbortEx(EX_SOCKET_SEND, gai_strerror(s));
  }
  struct addrinfo* rp;
  ssize_t r = -1;
  for(rp = res; rp; rp = rp->ai_next) {
    while((r = sendto(sockfd, data, len, 0, rp->ai_addr, rp->ai_addrlen)) == -1 && EINTR == errno);
    if(r == static_cast<ssize_t>(len)) {
      break;
    }
  }
  freeaddrinfo(res);
  if(r == -1) {
    throw new DlAbortEx(EX_SOCKET_SEND, errorMsg());
  }
}

ssize_t SocketCore::readDataFrom(char* data, size_t len,
				 std::pair<std::string /* numerichost */,
				 uint16_t /* port */>& sender)
{
  struct sockaddr_storage sockaddr;
  socklen_t sockaddrlen = sizeof(struct sockaddr_storage);
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  ssize_t r;
  while((r = recvfrom(sockfd, data, len, 0, addrp, &sockaddrlen)) == -1 &&
	EINTR == errno);
  if(r == -1) {
    throw new DlAbortEx(EX_SOCKET_RECV, errorMsg());
  }
  sender = SocketCore::getNameInfoInNumeric(addrp, sockaddrlen);

  return r;
}

} // namespace aria2
