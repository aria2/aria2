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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <unistd.h>
#ifdef HAVE_IFADDRS_H
# include <ifaddrs.h>
#endif // HAVE_IFADDRS_H

#include <cerrno>
#include <cstring>

#ifdef HAVE_LIBGNUTLS
# include <gnutls/x509.h>
#endif // HAVE_LIBGNUTLS

#include "message.h"
#include "a2netcompat.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "StringFormat.h"
#include "util.h"
#include "TimeA2.h"
#include "a2functional.h"
#include "LogFactory.h"
#ifdef ENABLE_SSL
# include "TLSContext.h"
#endif // ENABLE_SSL

namespace aria2 {

#ifndef __MINGW32__
# define SOCKET_ERRNO (errno)
#else
# define SOCKET_ERRNO (WSAGetLastError())
#endif // __MINGW32__

#ifdef __MINGW32__
# define A2_EINPROGRESS WSAEWOULDBLOCK
# define A2_EWOULDBLOCK WSAEWOULDBLOCK
# define A2_EINTR WSAEINTR
#else // !__MINGW32__
# define A2_EINPROGRESS EINPROGRESS
# ifndef EWOULDBLOCK
#  define EWOULDBLOCK EAGAIN
# endif // EWOULDBLOCK
# define A2_EWOULDBLOCK EWOULDBLOCK
# define A2_EINTR EINTR
#endif // !__MINGW32__

#ifdef __MINGW32__
# define CLOSE(X) ::closesocket(X)
#else
# define CLOSE(X) while(close(X) == -1 && errno == EINTR)
#endif // __MINGW32__

static const char *errorMsg(const int err)
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

static const char *errorMsg()
{
  return errorMsg(SOCKET_ERRNO);
}

#ifdef HAVE_EPOLL
SocketCore::PollMethod SocketCore::_pollMethod = SocketCore::POLL_METHOD_EPOLL;
#else // !HAVE_EPOLL
SocketCore::PollMethod SocketCore::_pollMethod = SocketCore::POLL_METHOD_SELECT;
#endif // !HAVE_EPOLL

int SocketCore::_protocolFamily = AF_UNSPEC;

std::vector<std::pair<struct sockaddr_storage, socklen_t> >
SocketCore::_bindAddrs;

#ifdef ENABLE_SSL
SharedHandle<TLSContext> SocketCore::_tlsContext;

void SocketCore::setTLSContext(const SharedHandle<TLSContext>& tlsContext)
{
  _tlsContext = tlsContext;
}
#endif // ENABLE_SSL

SocketCore::SocketCore(int sockType):_sockType(sockType), sockfd(-1)  {
  init();
}

SocketCore::SocketCore(sock_t sockfd, int sockType):_sockType(sockType), sockfd(sockfd) {
  init();
}

void SocketCore::init()
{

#ifdef HAVE_EPOLL

  _epfd = -1;

#endif // HAVE_EPOLL

  blocking = true;
  secure = 0;

  _wantRead = false;
  _wantWrite = false;

#ifdef HAVE_LIBSSL
  // for SSL
  ssl = NULL;
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  sslSession = NULL;
  peekBufMax = 4096;
  peekBuf = 0;
  peekBufLength = 0;
#endif //HAVE_LIBGNUTLS
}

SocketCore::~SocketCore() {
  closeConnection();

#ifdef HAVE_EPOLL

  if(_epfd != -1) {
    CLOSE(_epfd);
  }

#endif // HAVE_EPOLL

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

static sock_t bindInternal(int family, int socktype, int protocol,
                           const struct sockaddr* addr, socklen_t addrlen)
{
  sock_t fd = socket(family, socktype, protocol);
  if(fd == (sock_t) -1) {
    return -1;
  }
  int sockopt = 1;
  if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (a2_sockopt_t) &sockopt,
                sizeof(sockopt)) < 0) {
    CLOSE(fd);
    return -1;
  }
  if(::bind(fd, addr, addrlen) == -1) {
    if(LogFactory::getInstance()->debug()) {
      LogFactory::getInstance()->debug(EX_SOCKET_BIND, errorMsg());
    }
    CLOSE(fd);
    return -1;
  }
  return fd;
}

static sock_t bindTo
(const char* host, uint16_t port, int family, int sockType,
 int getaddrinfoFlags, std::string& error)
{
  struct addrinfo* res;
  int s = callGetaddrinfo(&res, host, uitos(port).c_str(), family, sockType,
                          getaddrinfoFlags, 0);  
  if(s) {
    error = gai_strerror(s);
    return -1;
  }
  WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
  struct addrinfo* rp;
  for(rp = res; rp; rp = rp->ai_next) {
    sock_t fd = bindInternal(rp->ai_family, rp->ai_socktype, rp->ai_protocol,
                             rp->ai_addr, rp->ai_addrlen);
    if(fd != (sock_t)-1) {
      return fd;
    }
  }
  error = errorMsg();
  return -1;
}

void SocketCore::bind(uint16_t port, int flags)
{
  closeConnection();
  std::string error;
  if(!(flags&AI_PASSIVE) || _bindAddrs.empty()) {
    sock_t fd = bindTo(0, port, _protocolFamily, _sockType, flags, error);
    if(fd != (sock_t) -1) {
      sockfd = fd;
    }
  } else {
    for(std::vector<std::pair<struct sockaddr_storage, socklen_t> >::
          const_iterator i = _bindAddrs.begin(); i != _bindAddrs.end(); ++i) {
      char host[NI_MAXHOST];
      int s;
      s = getnameinfo(reinterpret_cast<const struct sockaddr*>(&(*i).first),
                      (*i).second,
                      host, NI_MAXHOST, 0, NI_MAXSERV,
                      NI_NUMERICHOST);
      if(s) {
        error = gai_strerror(s);
        continue;
      }
      sock_t fd = bindTo(host, port, _protocolFamily, _sockType, flags, error);
      if(fd != (sock_t)-1) {
        sockfd = fd;
        break;
      }
    }
  }
  if(sockfd == (sock_t) -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_BIND, error.c_str()).str());
  }
}

void SocketCore::bind(const struct sockaddr* addr, socklen_t addrlen)
{
  closeConnection();

  sock_t fd = bindInternal(addr->sa_family, _sockType, 0, addr, addrlen);
  if(fd != (sock_t)-1) {
    sockfd = fd;
  } else {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_BIND, errorMsg()).str());
  }
}

void SocketCore::beginListen()
{
  if(listen(sockfd, 1) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_LISTEN, errorMsg()).str());
  }
}

SocketCore* SocketCore::acceptConnection() const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  sock_t fd;
  while((fd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&sockaddr), &len)) == (sock_t) -1 && SOCKET_ERRNO == A2_EINTR);
  if(fd == (sock_t) -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_ACCEPT, errorMsg()).str());
  }
  return new SocketCore(fd, _sockType);
}

void SocketCore::getAddrInfo(std::pair<std::string, uint16_t>& addrinfo) const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  if(getsockname(sockfd, addrp, &len) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_GET_NAME, errorMsg()).str());
  }
  addrinfo = util::getNumericNameInfo(addrp, len);
}

void SocketCore::getPeerInfo(std::pair<std::string, uint16_t>& peerinfo) const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  if(getpeername(sockfd, addrp, &len) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_GET_NAME, errorMsg()).str());
  }
  peerinfo = util::getNumericNameInfo(addrp, len);
}

void SocketCore::establishConnection(const std::string& host, uint16_t port)
{
  closeConnection();

  struct addrinfo* res;
  int s;
  s = callGetaddrinfo(&res, host.c_str(), uitos(port).c_str(), _protocolFamily,
                      _sockType, 0, 0);
  if(s) {
    throw DL_ABORT_EX(StringFormat(EX_RESOLVE_HOSTNAME,
                                   host.c_str(), gai_strerror(s)).str());
  }
  WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
  struct addrinfo* rp;
  for(rp = res; rp; rp = rp->ai_next) {
    sock_t fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(fd == (sock_t) -1) {
      continue;
    }
    int sockopt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (a2_sockopt_t) &sockopt, sizeof(sockopt)) < 0) {
      CLOSE(fd);
      continue;
    }
    if(!_bindAddrs.empty()) {
      bool bindSuccess = false;
      for(std::vector<std::pair<struct sockaddr_storage, socklen_t> >::
            const_iterator i = _bindAddrs.begin(); i != _bindAddrs.end(); ++i) {
        if(::bind(fd,reinterpret_cast<const struct sockaddr*>(&(*i).first),
                  (*i).second) == -1) {
          if(LogFactory::getInstance()->debug()) {
            LogFactory::getInstance()->debug(EX_SOCKET_BIND, errorMsg());
          }
        } else {
          bindSuccess = true;
          break;
        }
      }
      if(!bindSuccess) {
        CLOSE(fd);
        continue;
      }
    }

    sockfd = fd;
    // make socket non-blocking mode
    setNonBlockingMode();
    if(connect(fd, rp->ai_addr, rp->ai_addrlen) == -1 &&
       SOCKET_ERRNO != A2_EINPROGRESS) {
      CLOSE(sockfd);
      sockfd = (sock_t) -1;
      continue;
    }
    // TODO at this point, connection may not be established and it may fail
    // later. In such case, next ai_addr should be tried.
    break;
  }
  if(sockfd == (sock_t) -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_CONNECT, host.c_str(),
                                   errorMsg()).str());
  }
}

void SocketCore::setNonBlockingMode()
{
#ifdef __MINGW32__
  static u_long flag = 1;
  if (::ioctlsocket(sockfd, FIONBIO, &flag) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_NONBLOCKING, errorMsg()).str());
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
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_BLOCKING, errorMsg()).str());
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
  if(sockfd != (sock_t) -1) {
    CLOSE(sockfd);
    sockfd = -1;
  }
#ifdef HAVE_LIBSSL
  // for SSL
  if(secure) {
    SSL_free(ssl);
  }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  if(secure) {
    gnutls_deinit(sslSession);
  }
#endif // HAVE_LIBGNUTLS
}

#ifdef HAVE_EPOLL

void SocketCore::initEPOLL()
{
  if((_epfd = epoll_create(1)) == -1) {
    throw DL_RETRY_EX(StringFormat("epoll_create failed:%s", errorMsg()).str());
  }

  memset(&_epEvent, 0, sizeof(struct epoll_event));
  _epEvent.events = EPOLLIN|EPOLLOUT;
  _epEvent.data.fd = sockfd;

  if(epoll_ctl(_epfd, EPOLL_CTL_ADD, sockfd, &_epEvent) == -1) {
    throw DL_RETRY_EX(StringFormat("epoll_ctl failed:%s", errorMsg()).str());
  }
}

#endif // HAVE_EPOLL

bool SocketCore::isWritable(time_t timeout)
{
#ifdef HAVE_EPOLL
  if(_pollMethod == SocketCore::POLL_METHOD_EPOLL) {
    if(_epfd == -1) {
      initEPOLL();
    }
    struct epoll_event epEvents[1];
    int r;
    while((r = epoll_wait(_epfd, epEvents, 1, 0)) == -1 && errno == EINTR);
    if(r > 0) {
      return epEvents[0].events&(EPOLLOUT|EPOLLHUP|EPOLLERR);
    } else if(r == 0) {
      return false;
    } else {
      throw DL_RETRY_EX(StringFormat(EX_SOCKET_CHECK_WRITABLE, errorMsg()).str());
    }
  } else
#endif // HAVE_EPOLL
    if(_pollMethod == SocketCore::POLL_METHOD_SELECT) {
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
        if(SOCKET_ERRNO == A2_EINPROGRESS || SOCKET_ERRNO == A2_EINTR) {
          return false;
        } else {
          throw DL_RETRY_EX(StringFormat(EX_SOCKET_CHECK_WRITABLE, errorMsg()).str());
        }
      }
    } else {
      abort();
    }
}

bool SocketCore::isReadable(time_t timeout)
{
#ifdef HAVE_LIBGNUTLS
  if(secure && peekBufLength > 0) {
    return true;
  }
#endif // HAVE_LIBGNUTLS

#ifdef HAVE_EPOLL
  if(_pollMethod == SocketCore::POLL_METHOD_EPOLL) {
    if(_epfd == -1) {
      initEPOLL();
    }
    struct epoll_event epEvents[1];
    int r;
    while((r = epoll_wait(_epfd, epEvents, 1, 0)) == -1 && errno == EINTR);

    if(r > 0) {
      return epEvents[0].events&(EPOLLIN|EPOLLHUP|EPOLLERR);
    } else if(r == 0) {
      return false;
    } else {
      throw DL_RETRY_EX(StringFormat(EX_SOCKET_CHECK_READABLE, errorMsg()).str());
    }
  } else
#endif // HAVE_EPOLL
    if(_pollMethod == SocketCore::POLL_METHOD_SELECT) {
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
        if(SOCKET_ERRNO == A2_EINPROGRESS || SOCKET_ERRNO == A2_EINTR) {
          return false;
        } else {
          throw DL_RETRY_EX(StringFormat(EX_SOCKET_CHECK_READABLE, errorMsg()).str());
        }
      }
    } else {
      abort();
    }
}

#ifdef HAVE_LIBSSL
int SocketCore::sslHandleEAGAIN(int ret)
{
  int error = SSL_get_error(ssl, ret);
  if(error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
    ret = 0;
    if(error == SSL_ERROR_WANT_READ) {
      _wantRead = true;
    } else {
      _wantWrite = true;
    }
  }
  return ret;
}
#endif // HAVE_LIBSSL

#ifdef HAVE_LIBGNUTLS
void SocketCore::gnutlsRecordCheckDirection()
{
  int direction = gnutls_record_get_direction(sslSession);
  if(direction == 0) {
    _wantRead = true;
  } else { // if(direction == 1) {
    _wantWrite = true;
  }
}
#endif // HAVE_LIBGNUTLS

ssize_t SocketCore::writeData(const char* data, size_t len)
{
  ssize_t ret = 0;
  _wantRead = false;
  _wantWrite = false;

  if(!secure) {
    while((ret = send(sockfd, data, len, 0)) == -1 && SOCKET_ERRNO == A2_EINTR);
    if(ret == -1) {
      if(SOCKET_ERRNO == A2_EWOULDBLOCK) {
        _wantWrite = true;
        ret = 0;
      } else {
        throw DL_RETRY_EX(StringFormat(EX_SOCKET_SEND, errorMsg()).str());
      }
    }
  } else {
#ifdef HAVE_LIBSSL
    ret = SSL_write(ssl, data, len);
    if(ret == 0) {
      throw DL_RETRY_EX
        (StringFormat
         (EX_SOCKET_SEND, ERR_error_string(SSL_get_error(ssl, ret), 0)).str());
    }
    if(ret < 0) {
      ret = sslHandleEAGAIN(ret);
    }
    if(ret < 0) {
      throw DL_RETRY_EX
        (StringFormat
         (EX_SOCKET_SEND, ERR_error_string(SSL_get_error(ssl, ret), 0)).str());
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    while((ret = gnutls_record_send(sslSession, data, len)) ==
          GNUTLS_E_INTERRUPTED);
    if(ret == GNUTLS_E_AGAIN) {
      gnutlsRecordCheckDirection();
      ret = 0;
    } else if(ret < 0) {
      throw DL_RETRY_EX(StringFormat(EX_SOCKET_SEND, gnutls_strerror(ret)).str());
    }
#endif // HAVE_LIBGNUTLS
  }

  return ret;
}

void SocketCore::readData(char* data, size_t& len)
{
  ssize_t ret = 0;
  _wantRead = false;
  _wantWrite = false;

  if(!secure) {    
    while((ret = recv(sockfd, data, len, 0)) == -1 && SOCKET_ERRNO == A2_EINTR);
    
    if(ret == -1) {
      if(SOCKET_ERRNO == A2_EWOULDBLOCK) {
        _wantRead = true;
        ret = 0;
      } else {
        throw DL_RETRY_EX(StringFormat(EX_SOCKET_RECV, errorMsg()).str());
      }
    }
  } else {
#ifdef HAVE_LIBSSL
    // for SSL
    // TODO handling len == 0 case required
    ret = SSL_read(ssl, data, len);
    if(ret == 0) {
      // TODO Check this is really an error with SSL_get_error().
      // Or not throw exception and just return 0
      throw DL_RETRY_EX
        (StringFormat
         (EX_SOCKET_RECV, ERR_error_string(SSL_get_error(ssl, ret), 0)).str());
    }
    if(ret < 0) {
      ret = sslHandleEAGAIN(ret);
    }
    if(ret < 0) {
      throw DL_RETRY_EX
        (StringFormat
         (EX_SOCKET_RECV, ERR_error_string(SSL_get_error(ssl, ret), 0)).str());
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    ret = gnutlsRecv(data, len);
    if(ret == GNUTLS_E_AGAIN) {
      gnutlsRecordCheckDirection();
      ret = 0;
    } else if(ret < 0) {
      throw DL_RETRY_EX
        (StringFormat(EX_SOCKET_RECV, gnutls_strerror(ret)).str());
    }
#endif // HAVE_LIBGNUTLS
  }

  len = ret;
}

void SocketCore::peekData(char* data, size_t& len)
{
  ssize_t ret = 0;
  _wantRead = false;
  _wantWrite = false;

  if(!secure) {
    while((ret = recv(sockfd, data, len, MSG_PEEK)) == -1 &&
          SOCKET_ERRNO == A2_EINTR);
    if(ret == -1) {
      if(SOCKET_ERRNO == A2_EWOULDBLOCK) {
        _wantRead = true;
        ret = 0;
      } else {
        throw DL_RETRY_EX(StringFormat(EX_SOCKET_PEEK, errorMsg()).str());
      }
    }
  } else {
#ifdef HAVE_LIBSSL
    // for SSL
    // TODO handling len == 0 case required
    ret = SSL_peek(ssl, data, len);
    if(ret == 0) {
      throw DL_RETRY_EX
        (StringFormat(EX_SOCKET_PEEK,
                      ERR_error_string(SSL_get_error(ssl, ret), 0)).str());
    }
    if(ret < 0) {
      ret = sslHandleEAGAIN(ret);
    }
    if(ret < 0) {
      throw DL_RETRY_EX
        (StringFormat(EX_SOCKET_PEEK,
                      ERR_error_string(SSL_get_error(ssl, ret), 0)).str());
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    ret = gnutlsPeek(data, len);
    if(ret == GNUTLS_E_AGAIN) {
      gnutlsRecordCheckDirection();
      ret = 0;
    } else if(ret < 0) {
      throw DL_RETRY_EX(StringFormat(EX_SOCKET_PEEK,
                                     gnutls_strerror(ret)).str());
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

static ssize_t GNUTLS_RECORD_RECV_NO_INTERRUPT
(gnutls_session_t sslSession, char* data, size_t len)
{
  int ret;
  while((ret = gnutls_record_recv(sslSession, data, len)) ==
        GNUTLS_E_INTERRUPTED);
  if(ret < 0 && ret != GNUTLS_E_AGAIN) {
    throw DL_RETRY_EX
      (StringFormat(EX_SOCKET_RECV, gnutls_strerror(ret)).str());
  }
  return ret;
}

ssize_t SocketCore::gnutlsRecv(char* data, size_t len)
{
  size_t plen = shiftPeekData(data, len);
  if(plen < len) {
    ssize_t ret = GNUTLS_RECORD_RECV_NO_INTERRUPT
      (sslSession, data+plen, len-plen);
    if(ret == GNUTLS_E_AGAIN) {
      return GNUTLS_E_AGAIN;
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
    ssize_t ret = GNUTLS_RECORD_RECV_NO_INTERRUPT
      (sslSession, data+peekBufLength, len-peekBufLength);
    if(ret == GNUTLS_E_AGAIN) {
      return GNUTLS_E_AGAIN;
    }
    addPeekData(data+peekBufLength, ret);
    return peekBufLength;
  }
}
#endif // HAVE_LIBGNUTLS

void SocketCore::prepareSecureConnection()
{
  if(!secure) {
#ifdef HAVE_LIBSSL
    // for SSL
    ssl = SSL_new(_tlsContext->getSSLCtx());
    if(!ssl) {
      throw DL_ABORT_EX
        (StringFormat(EX_SSL_INIT_FAILURE,
                      ERR_error_string(ERR_get_error(), 0)).str());
    }
    if(SSL_set_fd(ssl, sockfd) == 0) {
      throw DL_ABORT_EX
        (StringFormat(EX_SSL_INIT_FAILURE,
                      ERR_error_string(ERR_get_error(), 0)).str());
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    int r;
    gnutls_init(&sslSession, GNUTLS_CLIENT);
    // It seems err is not error message, but the argument string
    // which causes syntax error.
    const char* err;
    // Disables TLS1.1 here because there are servers that don't
    // understand TLS1.1.
    r = gnutls_priority_set_direct(sslSession, "NORMAL:!VERS-TLS1.1", &err);
    if(r != GNUTLS_E_SUCCESS) {
      throw DL_ABORT_EX
        (StringFormat(EX_SSL_INIT_FAILURE, gnutls_strerror(r)).str());
    }
    // put the x509 credentials to the current session
    gnutls_credentials_set(sslSession, GNUTLS_CRD_CERTIFICATE,
                           _tlsContext->getCertCred());
    gnutls_transport_set_ptr(sslSession, (gnutls_transport_ptr_t)sockfd);
#endif // HAVE_LIBGNUTLS
    secure = 1;
  }
}

bool SocketCore::initiateSecureConnection(const std::string& hostname)
{
  if(secure == 1) {
    _wantRead = false;
    _wantWrite = false;
#ifdef HAVE_LIBSSL
    int e = SSL_connect(ssl);

    if (e <= 0) {
      int ssl_error = SSL_get_error(ssl, e);
      switch(ssl_error) {
      case SSL_ERROR_NONE:
        break;

      case SSL_ERROR_WANT_READ:
        _wantRead = true;
        return false;
      case SSL_ERROR_WANT_WRITE:
        _wantWrite = true;
        return false;
      case SSL_ERROR_WANT_X509_LOOKUP:
      case SSL_ERROR_ZERO_RETURN:
        if (blocking) {
          throw DL_ABORT_EX
            (StringFormat(EX_SSL_CONNECT_ERROR, ssl_error).str());
        }
        break;

      case SSL_ERROR_SYSCALL:
        throw DL_ABORT_EX(EX_SSL_IO_ERROR);

      case SSL_ERROR_SSL:
        throw DL_ABORT_EX(EX_SSL_PROTOCOL_ERROR);

      default:
        throw DL_ABORT_EX
          (StringFormat(EX_SSL_UNKNOWN_ERROR, ssl_error).str());
      }
    }
    if(_tlsContext->peerVerificationEnabled()) {
      // verify peer
      X509* peerCert = SSL_get_peer_certificate(ssl);
      if(!peerCert) {
        throw DL_ABORT_EX(MSG_NO_CERT_FOUND);
      }
      auto_delete<X509*> certDeleter(peerCert, X509_free);

      long verifyResult = SSL_get_verify_result(ssl);
      if(verifyResult != X509_V_OK) {
        throw DL_ABORT_EX
          (StringFormat(MSG_CERT_VERIFICATION_FAILED,
                        X509_verify_cert_error_string(verifyResult)).str());
      }
      X509_NAME* name = X509_get_subject_name(peerCert);
      if(!name) {
        throw DL_ABORT_EX("Could not get X509 name object from the certificate.");
      }

      bool hostnameOK = false;
      int lastpos = -1;
      while(true) {
        lastpos = X509_NAME_get_index_by_NID(name, NID_commonName, lastpos);
        if(lastpos == -1) {
          break;
        }
        X509_NAME_ENTRY* entry = X509_NAME_get_entry(name, lastpos);
        unsigned char* out;
        int outlen = ASN1_STRING_to_UTF8(&out, X509_NAME_ENTRY_get_data(entry));
        if(outlen < 0) {
          continue;
        }
        std::string commonName(&out[0], &out[outlen]);
        OPENSSL_free(out);
        if(commonName == hostname) {
          hostnameOK = true;
          break;
        }
      }
      if(!hostnameOK) {
        throw DL_ABORT_EX(MSG_HOSTNAME_NOT_MATCH);
      }
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    int ret = gnutls_handshake(sslSession);
    if(ret == GNUTLS_E_AGAIN) {
      gnutlsRecordCheckDirection();
      return false;
    } else if(ret < 0) {
      throw DL_ABORT_EX
        (StringFormat(EX_SSL_INIT_FAILURE, gnutls_strerror(ret)).str());
    }

    if(_tlsContext->peerVerificationEnabled()) {
      // verify peer
      unsigned int status;
      ret = gnutls_certificate_verify_peers2(sslSession, &status);
      if(ret < 0) {
        throw DL_ABORT_EX
          (StringFormat("gnutls_certificate_verify_peer2() failed. Cause: %s",
                        gnutls_strerror(ret)).str());
      }
      if(status) {
        std::string errors;
        if(status & GNUTLS_CERT_INVALID) {
          errors += " `not signed by known authorities or invalid'";
        }
        if(status & GNUTLS_CERT_REVOKED) {
          errors += " `revoked by its CA'";
        }
        if(status & GNUTLS_CERT_SIGNER_NOT_FOUND) {
          errors += " `issuer is not known'";
        }
        if(!errors.empty()) {
          throw DL_ABORT_EX
            (StringFormat(MSG_CERT_VERIFICATION_FAILED, errors.c_str()).str());
        }
      }
      // certificate type: only X509 is allowed.
      if(gnutls_certificate_type_get(sslSession) != GNUTLS_CRT_X509) {
        throw DL_ABORT_EX("Certificate type is not X509.");
      }

      unsigned int peerCertsLength;
      const gnutls_datum_t* peerCerts = gnutls_certificate_get_peers
        (sslSession, &peerCertsLength);
      if(!peerCerts) {
        throw DL_ABORT_EX(MSG_NO_CERT_FOUND);
      }
      Time now;
      for(unsigned int i = 0; i < peerCertsLength; ++i) {
        gnutls_x509_crt_t cert;
        ret = gnutls_x509_crt_init(&cert);
        if(ret < 0) {
          throw DL_ABORT_EX
            (StringFormat("gnutls_x509_crt_init() failed. Cause: %s",
                          gnutls_strerror(ret)).str());
        }
        auto_delete<gnutls_x509_crt_t> certDeleter
          (cert, gnutls_x509_crt_deinit);
        ret = gnutls_x509_crt_import(cert, &peerCerts[i], GNUTLS_X509_FMT_DER);
        if(ret < 0) {
          throw DL_ABORT_EX
            (StringFormat("gnutls_x509_crt_import() failed. Cause: %s",
                          gnutls_strerror(ret)).str());
        }
        if(i == 0) {
          if(!gnutls_x509_crt_check_hostname(cert, hostname.c_str())) {
            throw DL_ABORT_EX(MSG_HOSTNAME_NOT_MATCH);
          }
        }
        time_t activationTime = gnutls_x509_crt_get_activation_time(cert);
        if(activationTime == -1) {
          throw DL_ABORT_EX("Could not get activation time from certificate.");
        }
        if(now.getTime() < activationTime) {
          throw DL_ABORT_EX("Certificate is not activated yet.");
        }
        time_t expirationTime = gnutls_x509_crt_get_expiration_time(cert);
        if(expirationTime == -1) {
          throw DL_ABORT_EX("Could not get expiration time from certificate.");
        }
        if(expirationTime < now.getTime()) {
          throw DL_ABORT_EX("Certificate has expired.");
        }
      }
    }
    peekBuf = new char[peekBufMax];
#endif // HAVE_LIBGNUTLS
    secure = 2;
    return true;
  } else {
    return true;
  }
}

ssize_t SocketCore::writeData(const char* data, size_t len,
                              const std::string& host, uint16_t port)
{
  _wantRead = false;
  _wantWrite = false;

  struct addrinfo* res;
  int s;
  s = callGetaddrinfo(&res, host.c_str(), uitos(port).c_str(), _protocolFamily,
                      _sockType, 0, 0);
  if(s) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_SEND, gai_strerror(s)).str());
  }
  WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
  struct addrinfo* rp;
  ssize_t r = -1;
  for(rp = res; rp; rp = rp->ai_next) {
    while((r = sendto(sockfd, data, len, 0, rp->ai_addr, rp->ai_addrlen)) == -1
          && A2_EINTR == SOCKET_ERRNO);
    if(r == static_cast<ssize_t>(len)) {
      break;
    }
    if(r == -1 && SOCKET_ERRNO == A2_EWOULDBLOCK) {
      _wantWrite = true;
      r = 0;
      break;
    }
  }
  if(r == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_SEND, errorMsg()).str());
  }
  return r;
}

ssize_t SocketCore::readDataFrom(char* data, size_t len,
                                 std::pair<std::string /* numerichost */,
                                 uint16_t /* port */>& sender)
{
  _wantRead = false;
  _wantWrite = false;
  struct sockaddr_storage sockaddr;
  socklen_t sockaddrlen = sizeof(struct sockaddr_storage);
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  ssize_t r;
  while((r = recvfrom(sockfd, data, len, 0, addrp, &sockaddrlen)) == -1 &&
        A2_EINTR == SOCKET_ERRNO);
  if(r == -1) {
    if(SOCKET_ERRNO == A2_EWOULDBLOCK) {
      _wantRead = true;
      r = 0;
    } else {
      throw DL_RETRY_EX(StringFormat(EX_SOCKET_RECV, errorMsg()).str());
    }
  } else {
    sender = util::getNumericNameInfo(addrp, sockaddrlen);
  }

  return r;
}

std::string SocketCore::getSocketError() const
{
  int error;
  socklen_t optlen = sizeof(error);

  if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (a2_sockopt_t) &error, &optlen) == -1) {
    throw DL_ABORT_EX(StringFormat("Failed to get socket error: %s",
                                   errorMsg()).str());
  }
  if(error != 0) {
    return errorMsg(error);
  } else {
    return "";
  }
}

bool SocketCore::wantRead() const
{
  return _wantRead;
}

bool SocketCore::wantWrite() const
{
  return _wantWrite;
}

#ifdef HAVE_EPOLL
void SocketCore::useEpoll()
{
  _pollMethod = SocketCore::POLL_METHOD_EPOLL;
}
#endif // HAVE_EPOLL

void SocketCore::useSelect()
{
  _pollMethod = SocketCore::POLL_METHOD_SELECT;
}

void SocketCore::bindAddress(const std::string& iface)
{
  std::vector<std::pair<struct sockaddr_storage, socklen_t> > bindAddrs;
  if(LogFactory::getInstance()->debug()) {
    LogFactory::getInstance()->debug("Finding interface %s", iface.c_str());
  }
#ifdef HAVE_GETIFADDRS
  // First find interface in interface addresses
  struct ifaddrs* ifaddr = 0;
  if(getifaddrs(&ifaddr) == -1) {
    throw DL_ABORT_EX
      (StringFormat(MSG_INTERFACE_NOT_FOUND,
                    iface.c_str(), errorMsg()).str());
  } else {
    auto_delete<struct ifaddrs*> ifaddrDeleter(ifaddr, freeifaddrs);
    for(struct ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
      if(!ifa->ifa_addr) {
        continue;
      }
      int family = ifa->ifa_addr->sa_family;
      if(_protocolFamily == AF_UNSPEC) {
        if(family != AF_INET && family != AF_INET6) {
          continue;
        }
      } else if(_protocolFamily == AF_INET) {
        if(family != AF_INET) {
          continue;
        }
      } else if(_protocolFamily == AF_INET6) {
        if(family != AF_INET6) {
          continue;
        }
      } else {
        continue;
      }
      if(std::string(ifa->ifa_name) == iface) {
        socklen_t bindAddrLen =
          family == AF_INET?sizeof(struct sockaddr_in):
          sizeof(struct sockaddr_in6);
        struct sockaddr_storage bindAddr;
        memset(&bindAddr, 0, sizeof(bindAddr));
        memcpy(&bindAddr, ifa->ifa_addr, bindAddrLen);
        bindAddrs.push_back(std::make_pair(bindAddr, bindAddrLen));
      }
    }
  }
#endif // HAVE_GETIFADDRS
  if(bindAddrs.empty()) {
    struct addrinfo* res;
    int s;
    s = callGetaddrinfo(&res, iface.c_str(), 0, _protocolFamily,
                        SOCK_STREAM, 0, 0);
    if(s) {
      throw DL_ABORT_EX
        (StringFormat(MSG_INTERFACE_NOT_FOUND,
                      iface.c_str(), gai_strerror(s)).str());
    } else {
      WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
      struct addrinfo* rp;
      for(rp = res; rp; rp = rp->ai_next) {
        socklen_t bindAddrLen = rp->ai_addrlen;
        struct sockaddr_storage bindAddr;
        memset(&bindAddr, 0, sizeof(bindAddr));
        memcpy(&bindAddr, rp->ai_addr, rp->ai_addrlen);
        // Try to bind socket with this address. If it fails, the
        // address is not for this machine.
        try {
          SocketCore socket;
          socket.bind
            (reinterpret_cast<const struct sockaddr*>(&bindAddr), bindAddrLen);
          bindAddrs.push_back(std::make_pair(bindAddr, bindAddrLen));
        } catch(RecoverableException& e) {
          continue;
        }
      }
    }
  }
  if(bindAddrs.empty()) {
    throw DL_ABORT_EX
      (StringFormat(MSG_INTERFACE_NOT_FOUND,
                    iface.c_str(), "not available").str());
  } else {
    _bindAddrs = bindAddrs;
    for(std::vector<std::pair<struct sockaddr_storage, socklen_t> >::
          const_iterator i = _bindAddrs.begin(); i != _bindAddrs.end(); ++i) {
      char host[NI_MAXHOST];
      int s;
      s = getnameinfo(reinterpret_cast<const struct sockaddr*>(&(*i).first),
                      (*i).second,
                      host, NI_MAXHOST, 0, NI_MAXSERV,
                      NI_NUMERICHOST);
      if(s == 0) {
        if(LogFactory::getInstance()->debug()) {
          LogFactory::getInstance()->debug("Sockets will bind to %s", host);
        }
      }
    }
  }
}

namespace {

int defaultAIFlags = DEFAULT_AI_FLAGS;

int getDefaultAIFlags()
{
  return defaultAIFlags;
}

}

void setDefaultAIFlags(int flags)
{
  defaultAIFlags = flags;
}

int callGetaddrinfo
(struct addrinfo** resPtr, const char* host, const char* service, int family,
 int sockType, int flags, int protocol)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = sockType;
  hints.ai_flags = getDefaultAIFlags();
  hints.ai_flags |= flags;
  hints.ai_protocol = protocol;
  return getaddrinfo(host, service, &hints, resPtr);  
}

} // namespace aria2
