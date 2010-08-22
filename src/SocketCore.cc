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
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "StringFormat.h"
#include "util.h"
#include "TimeA2.h"
#include "a2functional.h"
#include "LogFactory.h"
#include "A2STR.h"
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
# define A2_WOULDBLOCK(e) (e == WSAEWOULDBLOCK)
#else // !__MINGW32__
# define A2_EINPROGRESS EINPROGRESS
# ifndef EWOULDBLOCK
#  define EWOULDBLOCK EAGAIN
# endif // EWOULDBLOCK
# define A2_EWOULDBLOCK EWOULDBLOCK
# define A2_EINTR EINTR
# if EWOULDBLOCK == EAGAIN
#  define A2_WOULDBLOCK(e) (e == EWOULDBLOCK)
# else // EWOULDBLOCK != EAGAIN
#  define A2_WOULDBLOCK(e) (e == EWOULDBLOCK || e == EAGAIN)
# endif // EWOULDBLOCK != EAGAIN
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

int SocketCore::protocolFamily_ = AF_UNSPEC;

std::vector<std::pair<struct sockaddr_storage, socklen_t> >
SocketCore::bindAddrs_;

#ifdef ENABLE_SSL
SharedHandle<TLSContext> SocketCore::tlsContext_;

void SocketCore::setTLSContext(const SharedHandle<TLSContext>& tlsContext)
{
  tlsContext_ = tlsContext;
}
#endif // ENABLE_SSL

SocketCore::SocketCore(int sockType):sockType_(sockType), sockfd_(-1)  {
  init();
}

SocketCore::SocketCore(sock_t sockfd, int sockType):sockType_(sockType), sockfd_(sockfd) {
  init();
}

void SocketCore::init()
{
  blocking_ = true;
  secure_ = 0;

  wantRead_ = false;
  wantWrite_ = false;

#ifdef HAVE_LIBSSL
  // for SSL
  ssl = NULL;
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  sslSession_ = 0;
  peekBufMax_ = 4096;
  peekBuf_ = 0;
  peekBufLength_ = 0;
#endif //HAVE_LIBGNUTLS
}

SocketCore::~SocketCore() {
  closeConnection();
#ifdef HAVE_LIBGNUTLS
  delete [] peekBuf_;
#endif // HAVE_LIBGNUTLS
}

void SocketCore::create(int family, int protocol)
{
  closeConnection();
  sock_t fd = socket(family, sockType_, protocol);
  if(fd == (sock_t) -1) {
    throw DL_ABORT_EX
      (StringFormat("Failed to create socket. Cause:%s", errorMsg()).str());
  }
  int sockopt = 1;
  if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                (a2_sockopt_t) &sockopt, sizeof(sockopt)) < 0) {
    CLOSE(fd);
    throw DL_ABORT_EX
      (StringFormat("Failed to create socket. Cause:%s", errorMsg()).str());
  }
  sockfd_ = fd;
}

static sock_t bindInternal(int family, int socktype, int protocol,
                           const struct sockaddr* addr, socklen_t addrlen,
                           std::string& error)
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
  if(family == AF_INET6) {
    int sockopt = 1;
    if(setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (a2_sockopt_t) &sockopt,
                  sizeof(sockopt)) < 0) {
      CLOSE(fd);
      return -1;
    }
  }
  if(::bind(fd, addr, addrlen) == -1) {
    error = errorMsg();
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
  int s = callGetaddrinfo(&res, host, util::uitos(port).c_str(),
                          family, sockType, getaddrinfoFlags, 0);  
  if(s) {
    error = gai_strerror(s);
    return -1;
  }
  WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
  struct addrinfo* rp;
  for(rp = res; rp; rp = rp->ai_next) {
    sock_t fd = bindInternal(rp->ai_family, rp->ai_socktype, rp->ai_protocol,
                             rp->ai_addr, rp->ai_addrlen, error);
    if(fd != (sock_t)-1) {
      return fd;
    }
  }
  return -1;
}

void SocketCore::bindWithFamily(uint16_t port, int family, int flags)
{
  closeConnection();
  std::string error;
  sock_t fd = bindTo(0, port, family, sockType_, flags, error);
  if(fd == (sock_t) -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_BIND, error.c_str()).str());
  } else {
    sockfd_ = fd;
  }
}

void SocketCore::bind
(const std::string& addr, uint16_t port, int family, int flags)
{
  closeConnection();
  std::string error;
  const char* addrp;
  if(addr.empty()) {
    addrp = 0;
  } else {
    addrp = addr.c_str();
  }
  if(!(flags&AI_PASSIVE) || bindAddrs_.empty()) {
    sock_t fd = bindTo(addrp, port, family, sockType_, flags, error);
    if(fd != (sock_t) -1) {
      sockfd_ = fd;
    }
  } else {
    for(std::vector<std::pair<struct sockaddr_storage, socklen_t> >::
          const_iterator i = bindAddrs_.begin(), eoi = bindAddrs_.end();
        i != eoi; ++i) {
      char host[NI_MAXHOST];
      int s;
      s = getnameinfo(reinterpret_cast<const struct sockaddr*>(&(*i).first),
                      (*i).second,
                      host, NI_MAXHOST, 0, 0,
                      NI_NUMERICHOST);
      if(s) {
        error = gai_strerror(s);
        continue;
      }
      if(addrp && strcmp(host, addrp) != 0) {
        // TODO we should assign something to error?
        continue;
      }
      sock_t fd = bindTo(addrp, port, family, sockType_, flags, error);
      if(fd != (sock_t)-1) {
        sockfd_ = fd;
        break;
      }
    }
  }
  if(sockfd_ == (sock_t) -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_BIND, error.c_str()).str());
  }
}

void SocketCore::bind(uint16_t port, int flags)
{
  bind(A2STR::NIL, port, protocolFamily_, flags);
}

void SocketCore::bind(const struct sockaddr* addr, socklen_t addrlen)
{
  closeConnection();
  std::string error;
  sock_t fd = bindInternal(addr->sa_family, sockType_, 0, addr, addrlen, error);
  if(fd != (sock_t)-1) {
    sockfd_ = fd;
  } else {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_BIND, error.c_str()).str());
  }
}

void SocketCore::beginListen()
{
  if(listen(sockfd_, 1) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_LISTEN, errorMsg()).str());
  }
}

SocketCore* SocketCore::acceptConnection() const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  sock_t fd;
  while((fd = accept(sockfd_, reinterpret_cast<struct sockaddr*>(&sockaddr), &len)) == (sock_t) -1 && SOCKET_ERRNO == A2_EINTR);
  if(fd == (sock_t) -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_ACCEPT, errorMsg()).str());
  }
  return new SocketCore(fd, sockType_);
}

void SocketCore::getAddrInfo(std::pair<std::string, uint16_t>& addrinfo) const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  getAddrInfo(sockaddr, len);
  addrinfo = util::getNumericNameInfo
    (reinterpret_cast<const struct sockaddr*>(&sockaddr), len);
}

void SocketCore::getAddrInfo
(struct sockaddr_storage& sockaddr, socklen_t& len) const
{
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  if(getsockname(sockfd_, addrp, &len) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_GET_NAME, errorMsg()).str());
  }
}

int SocketCore::getAddressFamily() const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  getAddrInfo(sockaddr, len);
  return sockaddr.ss_family;
}

void SocketCore::getPeerInfo(std::pair<std::string, uint16_t>& peerinfo) const
{
  struct sockaddr_storage sockaddr;
  socklen_t len = sizeof(sockaddr);
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  if(getpeername(sockfd_, addrp, &len) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_GET_NAME, errorMsg()).str());
  }
  peerinfo = util::getNumericNameInfo(addrp, len);
}

void SocketCore::establishConnection(const std::string& host, uint16_t port)
{
  closeConnection();
  std::string error;
  struct addrinfo* res;
  int s;
  s = callGetaddrinfo(&res, host.c_str(), util::uitos(port).c_str(),
                      protocolFamily_, sockType_, 0, 0);
  if(s) {
    throw DL_ABORT_EX(StringFormat(EX_RESOLVE_HOSTNAME,
                                   host.c_str(), gai_strerror(s)).str());
  }
  WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
  struct addrinfo* rp;
  for(rp = res; rp; rp = rp->ai_next) {
    sock_t fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(fd == (sock_t) -1) {
      error = errorMsg();
      continue;
    }
    int sockopt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (a2_sockopt_t) &sockopt, sizeof(sockopt)) < 0) {
      error = errorMsg();
      CLOSE(fd);
      continue;
    }
    if(!bindAddrs_.empty()) {
      bool bindSuccess = false;
      for(std::vector<std::pair<struct sockaddr_storage, socklen_t> >::
            const_iterator i = bindAddrs_.begin(), eoi = bindAddrs_.end();
          i != eoi; ++i) {
        if(::bind(fd,reinterpret_cast<const struct sockaddr*>(&(*i).first),
                  (*i).second) == -1) {
          error = errorMsg();
          if(LogFactory::getInstance()->debug()) {
            LogFactory::getInstance()->debug(EX_SOCKET_BIND, error.c_str());
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

    sockfd_ = fd;
    // make socket non-blocking mode
    setNonBlockingMode();
    if(connect(fd, rp->ai_addr, rp->ai_addrlen) == -1 &&
       SOCKET_ERRNO != A2_EINPROGRESS) {
      error = errorMsg();
      CLOSE(sockfd_);
      sockfd_ = (sock_t) -1;
      continue;
    }
    // TODO at this point, connection may not be established and it may fail
    // later. In such case, next ai_addr should be tried.
    break;
  }
  if(sockfd_ == (sock_t) -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_CONNECT, host.c_str(),
                                   error.c_str()).str());
  }
}

void SocketCore::setSockOpt
(int level, int optname, void* optval, socklen_t optlen)
{
  if(setsockopt(sockfd_, level, optname, (a2_sockopt_t)optval, optlen) < 0) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_SET_OPT, errorMsg()).str());
  }
}   

void SocketCore::setMulticastInterface(const std::string& localAddr)
{
  in_addr addr;
  if(localAddr.empty()) {
    addr.s_addr = htonl(INADDR_ANY);
  } else {
    if(inet_aton(localAddr.c_str(), &addr) == 0) {
      throw DL_ABORT_EX
        (StringFormat("inet_aton failed for %s", localAddr.c_str()).str());
    }
  }
  setSockOpt(IPPROTO_IP, IP_MULTICAST_IF, &addr, sizeof(addr));
}

void SocketCore::setMulticastTtl(unsigned char ttl)
{
  setSockOpt(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

void SocketCore::setMulticastLoop(unsigned char loop)
{
  setSockOpt(IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
}

void SocketCore::joinMulticastGroup
(const std::string& multicastAddr, uint16_t multicastPort,
 const std::string& localAddr)
{
  in_addr multiAddr;
  if(inet_aton(multicastAddr.c_str(), &multiAddr) == 0) {
    throw DL_ABORT_EX
      (StringFormat("inet_aton failed for %s", multicastAddr.c_str()).str());
  }
  in_addr ifAddr;
  if(localAddr.empty()) {
    ifAddr.s_addr = htonl(INADDR_ANY);
  } else {
    if(inet_aton(localAddr.c_str(), &ifAddr) == 0) {
      throw DL_ABORT_EX
        (StringFormat("inet_aton failed for %s", localAddr.c_str()).str());
    }
  }
  struct ip_mreq mreq;
  memset(&mreq, 0, sizeof(mreq));
  mreq.imr_multiaddr = multiAddr;
  mreq.imr_interface = ifAddr;
  setSockOpt(IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

void SocketCore::setNonBlockingMode()
{
#ifdef __MINGW32__
  static u_long flag = 1;
  if (::ioctlsocket(sockfd_, FIONBIO, &flag) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_NONBLOCKING, errorMsg()).str());
  }
#else
  int flags;
  while((flags = fcntl(sockfd_, F_GETFL, 0)) == -1 && errno == EINTR);
  // TODO add error handling
  while(fcntl(sockfd_, F_SETFL, flags|O_NONBLOCK) == -1 && errno == EINTR);
#endif // __MINGW32__
  blocking_ = false;
}

void SocketCore::setBlockingMode()
{
#ifdef __MINGW32__
  static u_long flag = 0;
  if (::ioctlsocket(sockfd_, FIONBIO, &flag) == -1) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_BLOCKING, errorMsg()).str());
  }
#else
  int flags;
  while((flags = fcntl(sockfd_, F_GETFL, 0)) == -1 && errno == EINTR);
  // TODO add error handling
  while(fcntl(sockfd_, F_SETFL, flags&(~O_NONBLOCK)) == -1 && errno == EINTR);
#endif // __MINGW32__
  blocking_ = true;
}

void SocketCore::closeConnection()
{
#ifdef HAVE_LIBSSL
  // for SSL
  if(secure_) {
    SSL_shutdown(ssl);
  }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  if(secure_) {
    gnutls_bye(sslSession_, GNUTLS_SHUT_RDWR);
  }
#endif // HAVE_LIBGNUTLS
  if(sockfd_ != (sock_t) -1) {
    CLOSE(sockfd_);
    sockfd_ = -1;
  }
#ifdef HAVE_LIBSSL
  // for SSL
  if(secure_) {
    SSL_free(ssl);
  }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  if(secure_) {
    gnutls_deinit(sslSession_);
  }
#endif // HAVE_LIBGNUTLS
}

#ifndef __MINGW32__
# define CHECK_FD(fd)                                                   \
  if(fd < 0 || FD_SETSIZE <= fd) {                                      \
    logger_->warn("Detected file descriptor >= FD_SETSIZE or < 0. "     \
                  "Download may slow down or fail.");                   \
    return false;                                                       \
  }
#endif // !__MINGW32__

bool SocketCore::isWritable(time_t timeout)
{
#ifdef HAVE_POLL
  struct pollfd p;
  p.fd = sockfd_;
  p.events = POLLOUT;
  int r;
  while((r = poll(&p, 1, timeout*1000)) == -1 && errno == EINTR);
  if(r > 0) {
    return p.revents&(POLLOUT|POLLHUP|POLLERR);
  } else if(r == 0) {
    return false;
  } else {
    throw DL_RETRY_EX
      (StringFormat(EX_SOCKET_CHECK_WRITABLE, errorMsg()).str());
  }
#else // !HAVE_POLL
# ifndef __MINGW32__
  CHECK_FD(sockfd_);
# endif // !__MINGW32__
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd_, &fds);

  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  int r = select(sockfd_+1, NULL, &fds, NULL, &tv);
  if(r == 1) {
    return true;
  } else if(r == 0) {
    // time out
    return false;
  } else {
    if(SOCKET_ERRNO == A2_EINPROGRESS || SOCKET_ERRNO == A2_EINTR) {
      return false;
    } else {
      throw DL_RETRY_EX
        (StringFormat(EX_SOCKET_CHECK_WRITABLE, errorMsg()).str());
    }
  }
#endif // !HAVE_POLL
}

bool SocketCore::isReadable(time_t timeout)
{
#ifdef HAVE_LIBGNUTLS
  if(secure_ && peekBufLength_ > 0) {
    return true;
  }
#endif // HAVE_LIBGNUTLS
#ifdef HAVE_POLL
  struct pollfd p;
  p.fd = sockfd_;
  p.events = POLLIN;
  int r;
  while((r = poll(&p, 1, timeout*1000)) == -1 && errno == EINTR);
  if(r > 0) {
    return p.revents&(POLLIN|POLLHUP|POLLERR);
  } else if(r == 0) {
    return false;
  } else {
    throw DL_RETRY_EX
      (StringFormat(EX_SOCKET_CHECK_READABLE, errorMsg()).str());
  }
#else // !HAVE_POLL
# ifndef __MINGW32__
  CHECK_FD(sockfd_);
# endif // !__MINGW32__
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd_, &fds);

  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  int r = select(sockfd_+1, &fds, NULL, NULL, &tv);
  if(r == 1) {
    return true;
  } else if(r == 0) {
    // time out
    return false;
  } else {
    if(SOCKET_ERRNO == A2_EINPROGRESS || SOCKET_ERRNO == A2_EINTR) {
      return false;
    } else {
      throw DL_RETRY_EX
        (StringFormat(EX_SOCKET_CHECK_READABLE, errorMsg()).str());
    }
  }
#endif // !HAVE_POLL
}

#ifdef HAVE_LIBSSL
int SocketCore::sslHandleEAGAIN(int ret)
{
  int error = SSL_get_error(ssl, ret);
  if(error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
    ret = 0;
    if(error == SSL_ERROR_WANT_READ) {
      wantRead_ = true;
    } else {
      wantWrite_ = true;
    }
  }
  return ret;
}
#endif // HAVE_LIBSSL

#ifdef HAVE_LIBGNUTLS
void SocketCore::gnutlsRecordCheckDirection()
{
  int direction = gnutls_record_get_direction(sslSession_);
  if(direction == 0) {
    wantRead_ = true;
  } else { // if(direction == 1) {
    wantWrite_ = true;
  }
}
#endif // HAVE_LIBGNUTLS

ssize_t SocketCore::writeData(const char* data, size_t len)
{
  ssize_t ret = 0;
  wantRead_ = false;
  wantWrite_ = false;

  if(!secure_) {
    while((ret = send(sockfd_, data, len, 0)) == -1 && SOCKET_ERRNO == A2_EINTR);
    if(ret == -1) {
      if(A2_WOULDBLOCK(SOCKET_ERRNO)) {
        wantWrite_ = true;
        ret = 0;
      } else {
        throw DL_RETRY_EX(StringFormat(EX_SOCKET_SEND, errorMsg()).str());
      }
    }
  } else {
#ifdef HAVE_LIBSSL
    ret = SSL_write(ssl, data, len);
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
    while((ret = gnutls_record_send(sslSession_, data, len)) ==
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
  wantRead_ = false;
  wantWrite_ = false;

  if(!secure_) {    
    while((ret = recv(sockfd_, data, len, 0)) == -1 && SOCKET_ERRNO == A2_EINTR);
    
    if(ret == -1) {
      if(A2_WOULDBLOCK(SOCKET_ERRNO)) {
        wantRead_ = true;
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
  wantRead_ = false;
  wantWrite_ = false;

  if(!secure_) {
    while((ret = recv(sockfd_, data, len, MSG_PEEK)) == -1 &&
          SOCKET_ERRNO == A2_EINTR);
    if(ret == -1) {
      if(A2_WOULDBLOCK(SOCKET_ERRNO)) {
        wantRead_ = true;
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
  if(peekBufLength_ <= len) {
    memcpy(data, peekBuf_, peekBufLength_);
    size_t ret = peekBufLength_;
    peekBufLength_ = 0;
    return ret;
  } else {
    memcpy(data, peekBuf_, len);
    peekBufLength_ -= len;    
    memmove(peekBuf_, peekBuf_+len, peekBufLength_);
    return len;
  }

}

void SocketCore::addPeekData(char* data, size_t len)
{
  if(peekBufLength_+len > peekBufMax_) {
    char* temp = new char[peekBufMax_+len];
    memcpy(temp, peekBuf_, peekBufLength_);
    delete [] peekBuf_;
    peekBuf_ = temp;
    peekBufMax_ = peekBufLength_+len;
  }
  memcpy(peekBuf_+peekBufLength_, data, len);
  peekBufLength_ += len;
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
      (sslSession_, data+plen, len-plen);
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
  if(peekBufLength_ >= len) {
    memcpy(data, peekBuf_, len);
    return len;
  } else {
    memcpy(data, peekBuf_, peekBufLength_);
    ssize_t ret = GNUTLS_RECORD_RECV_NO_INTERRUPT
      (sslSession_, data+peekBufLength_, len-peekBufLength_);
    if(ret == GNUTLS_E_AGAIN) {
      return GNUTLS_E_AGAIN;
    }
    addPeekData(data+peekBufLength_, ret);
    return peekBufLength_;
  }
}
#endif // HAVE_LIBGNUTLS

void SocketCore::prepareSecureConnection()
{
  if(!secure_) {
#ifdef HAVE_LIBSSL
    // for SSL
    ssl = SSL_new(tlsContext_->getSSLCtx());
    if(!ssl) {
      throw DL_ABORT_EX
        (StringFormat(EX_SSL_INIT_FAILURE,
                      ERR_error_string(ERR_get_error(), 0)).str());
    }
    if(SSL_set_fd(ssl, sockfd_) == 0) {
      throw DL_ABORT_EX
        (StringFormat(EX_SSL_INIT_FAILURE,
                      ERR_error_string(ERR_get_error(), 0)).str());
    }
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
    int r;
    gnutls_init(&sslSession_, GNUTLS_CLIENT);
    // It seems err is not error message, but the argument string
    // which causes syntax error.
    const char* err;
    // Disables TLS1.1 here because there are servers that don't
    // understand TLS1.1.
    r = gnutls_priority_set_direct(sslSession_, "NORMAL:!VERS-TLS1.1", &err);
    if(r != GNUTLS_E_SUCCESS) {
      throw DL_ABORT_EX
        (StringFormat(EX_SSL_INIT_FAILURE, gnutls_strerror(r)).str());
    }
    // put the x509 credentials to the current session
    gnutls_credentials_set(sslSession_, GNUTLS_CRD_CERTIFICATE,
                           tlsContext_->getCertCred());
    gnutls_transport_set_ptr(sslSession_, (gnutls_transport_ptr_t)sockfd_);
#endif // HAVE_LIBGNUTLS
    secure_ = 1;
  }
}

bool SocketCore::initiateSecureConnection(const std::string& hostname)
{
  if(secure_ == 1) {
    wantRead_ = false;
    wantWrite_ = false;
#ifdef HAVE_LIBSSL
    int e = SSL_connect(ssl);

    if (e <= 0) {
      int ssl_error = SSL_get_error(ssl, e);
      switch(ssl_error) {
      case SSL_ERROR_NONE:
        break;

      case SSL_ERROR_WANT_READ:
        wantRead_ = true;
        return false;
      case SSL_ERROR_WANT_WRITE:
        wantWrite_ = true;
        return false;
      case SSL_ERROR_WANT_X509_LOOKUP:
      case SSL_ERROR_ZERO_RETURN:
        if (blocking_) {
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
    if(tlsContext_->peerVerificationEnabled()) {
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
    int ret = gnutls_handshake(sslSession_);
    if(ret == GNUTLS_E_AGAIN) {
      gnutlsRecordCheckDirection();
      return false;
    } else if(ret < 0) {
      throw DL_ABORT_EX
        (StringFormat(EX_SSL_INIT_FAILURE, gnutls_strerror(ret)).str());
    }

    if(tlsContext_->peerVerificationEnabled()) {
      // verify peer
      unsigned int status;
      ret = gnutls_certificate_verify_peers2(sslSession_, &status);
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
      if(gnutls_certificate_type_get(sslSession_) != GNUTLS_CRT_X509) {
        throw DL_ABORT_EX("Certificate type is not X509.");
      }

      unsigned int peerCertsLength;
      const gnutls_datum_t* peerCerts = gnutls_certificate_get_peers
        (sslSession_, &peerCertsLength);
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
    peekBuf_ = new char[peekBufMax_];
#endif // HAVE_LIBGNUTLS
    secure_ = 2;
    return true;
  } else {
    return true;
  }
}

ssize_t SocketCore::writeData(const char* data, size_t len,
                              const std::string& host, uint16_t port)
{
  wantRead_ = false;
  wantWrite_ = false;

  struct addrinfo* res;
  int s;
  s = callGetaddrinfo(&res, host.c_str(), util::uitos(port).c_str(),
                      protocolFamily_, sockType_, 0, 0);
  if(s) {
    throw DL_ABORT_EX(StringFormat(EX_SOCKET_SEND, gai_strerror(s)).str());
  }
  WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
  struct addrinfo* rp;
  ssize_t r = -1;
  for(rp = res; rp; rp = rp->ai_next) {
    while((r = sendto(sockfd_, data, len, 0, rp->ai_addr, rp->ai_addrlen)) == -1
          && A2_EINTR == SOCKET_ERRNO);
    if(r == static_cast<ssize_t>(len)) {
      break;
    }
    if(r == -1 && A2_WOULDBLOCK(SOCKET_ERRNO)) {
      wantWrite_ = true;
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
  wantRead_ = false;
  wantWrite_ = false;
  struct sockaddr_storage sockaddr;
  socklen_t sockaddrlen = sizeof(struct sockaddr_storage);
  struct sockaddr* addrp = reinterpret_cast<struct sockaddr*>(&sockaddr);
  ssize_t r;
  while((r = recvfrom(sockfd_, data, len, 0, addrp, &sockaddrlen)) == -1 &&
        A2_EINTR == SOCKET_ERRNO);
  if(r == -1) {
    if(A2_WOULDBLOCK(SOCKET_ERRNO)) {
      wantRead_ = true;
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

  if(getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, (a2_sockopt_t) &error, &optlen) == -1) {
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
  return wantRead_;
}

bool SocketCore::wantWrite() const
{
  return wantWrite_;
}

void SocketCore::bindAddress(const std::string& iface)
{
  std::vector<std::pair<struct sockaddr_storage, socklen_t> > bindAddrs;
  getInterfaceAddress(bindAddrs, iface, protocolFamily_);
  if(bindAddrs.empty()) {
    throw DL_ABORT_EX
      (StringFormat(MSG_INTERFACE_NOT_FOUND,
                    iface.c_str(), "not available").str());
  } else {
    bindAddrs_ = bindAddrs;
    for(std::vector<std::pair<struct sockaddr_storage, socklen_t> >::
          const_iterator i = bindAddrs_.begin(), eoi = bindAddrs_.end();
        i != eoi; ++i) {
      char host[NI_MAXHOST];
      int s;
      s = getnameinfo(reinterpret_cast<const struct sockaddr*>(&(*i).first),
                      (*i).second,
                      host, NI_MAXHOST, 0, 0,
                      NI_NUMERICHOST);
      if(s == 0) {
        if(LogFactory::getInstance()->debug()) {
          LogFactory::getInstance()->debug("Sockets will bind to %s", host);
        }
      }
    }
  }
}

void getInterfaceAddress
(std::vector<std::pair<struct sockaddr_storage, socklen_t> >& ifAddrs,
 const std::string& iface, int family, int aiFlags)
{
  Logger* logger = LogFactory::getInstance();
  if(logger->debug()) {
    logger->debug("Finding interface %s", iface.c_str());
  }
#ifdef HAVE_GETIFADDRS
  // First find interface in interface addresses
  struct ifaddrs* ifaddr = 0;
  if(getifaddrs(&ifaddr) == -1) {
    logger->info(MSG_INTERFACE_NOT_FOUND, iface.c_str(), errorMsg());
  } else {
    auto_delete<struct ifaddrs*> ifaddrDeleter(ifaddr, freeifaddrs);
    for(struct ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
      if(!ifa->ifa_addr) {
        continue;
      }
      int iffamily = ifa->ifa_addr->sa_family;
      if(family == AF_UNSPEC) {
        if(iffamily != AF_INET && iffamily != AF_INET6) {
          continue;
        }
      } else if(family == AF_INET) {
        if(iffamily != AF_INET) {
          continue;
        }
      } else if(family == AF_INET6) {
        if(iffamily != AF_INET6) {
          continue;
        }
      } else {
        continue;
      }
      if(std::string(ifa->ifa_name) == iface) {
        socklen_t bindAddrLen = iffamily == AF_INET?sizeof(struct sockaddr_in):
          sizeof(struct sockaddr_in6);
        struct sockaddr_storage bindAddr;
        memset(&bindAddr, 0, sizeof(bindAddr));
        memcpy(&bindAddr, ifa->ifa_addr, bindAddrLen);
        ifAddrs.push_back(std::make_pair(bindAddr, bindAddrLen));
      }
    }
  }
#endif // HAVE_GETIFADDRS
  if(ifAddrs.empty()) {
    struct addrinfo* res;
    int s;
    s = callGetaddrinfo(&res, iface.c_str(), 0, family, SOCK_STREAM, aiFlags,0);
    if(s) {
      logger->info(MSG_INTERFACE_NOT_FOUND, iface.c_str(), gai_strerror(s));
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
          ifAddrs.push_back(std::make_pair(bindAddr, bindAddrLen));
        } catch(RecoverableException& e) {
          continue;
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
