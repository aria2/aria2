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

#ifdef HAVE_OPENSSL
# include <openssl/x509.h>
# include <openssl/x509v3.h>
#endif // HAVE_OPENSSL

#ifdef HAVE_LIBGNUTLS
# include <gnutls/x509.h>
#endif // HAVE_LIBGNUTLS

#include "message.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "fmt.h"
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
# define CLOSE(X) close(X)
#endif // __MINGW32__

namespace {
std::string errorMsg(int errNum)
{
#ifndef __MINGW32__
  return util::safeStrerror(errNum);
#else
  static char buf[256];
  if (FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    errNum,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    (LPTSTR) &buf,
                    sizeof(buf),
                    NULL
                    ) == 0) {
    snprintf(buf, sizeof(buf), EX_SOCKET_UNKNOWN_ERROR, errNum, errNum);
  }
  return buf;
#endif // __MINGW32__
}
} // namespace

namespace {
enum TlsState {
  // TLS object is not initialized.
  A2_TLS_NONE = 0,
  // TLS object is initialized. Ready for handshake.
  A2_TLS_INITIALIZED = 1,
  // TLS object is now handshaking.
  A2_TLS_HANDSHAKING = 2,
  // TLS object is now connected.
  A2_TLS_CONNECTED = 3
};
} // namespace

int SocketCore::protocolFamily_ = AF_UNSPEC;

std::vector<std::pair<sockaddr_union, socklen_t> >
SocketCore::bindAddrs_;

#ifdef ENABLE_SSL
SharedHandle<TLSContext> SocketCore::tlsContext_;

void SocketCore::setTLSContext(const SharedHandle<TLSContext>& tlsContext)
{
  tlsContext_ = tlsContext;
}
#endif // ENABLE_SSL

SocketCore::SocketCore(int sockType)
  : sockType_(sockType),
    sockfd_(-1)
{
  init();
}

SocketCore::SocketCore(sock_t sockfd, int sockType)
  : sockType_(sockType),
    sockfd_(sockfd)
{
  init();
}

void SocketCore::init()
{
  blocking_ = true;
  secure_ = A2_TLS_NONE;

  wantRead_ = false;
  wantWrite_ = false;

#ifdef HAVE_OPENSSL
  // for SSL
  ssl = NULL;
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGNUTLS
  sslSession_ = 0;
#endif //HAVE_LIBGNUTLS
}

SocketCore::~SocketCore() {
  closeConnection();
}

void SocketCore::create(int family, int protocol)
{
  int errNum;
  closeConnection();
  sock_t fd = socket(family, sockType_, protocol);
  errNum = SOCKET_ERRNO;
  if(fd == (sock_t) -1) {
    throw DL_ABORT_EX
      (fmt("Failed to create socket. Cause:%s", errorMsg(errNum).c_str()));
  }
  int sockopt = 1;
  if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                (a2_sockopt_t) &sockopt, sizeof(sockopt)) < 0) {
    errNum = SOCKET_ERRNO;
    CLOSE(fd);
    throw DL_ABORT_EX
      (fmt("Failed to create socket. Cause:%s", errorMsg(errNum).c_str()));
  }
  sockfd_ = fd;
}

static sock_t bindInternal
(int family, int socktype, int protocol,
 const struct sockaddr* addr, socklen_t addrlen,
 std::string& error)
{
  int errNum;
  sock_t fd = socket(family, socktype, protocol);
  errNum = SOCKET_ERRNO;
  if(fd == (sock_t) -1) {
    error = errorMsg(errNum);
    return -1;
  }
  int sockopt = 1;
  if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (a2_sockopt_t) &sockopt,
                sizeof(sockopt)) < 0) {
    errNum = SOCKET_ERRNO;
    error = errorMsg(errNum);
    CLOSE(fd);
    return -1;
  }
#ifdef IPV6_V6ONLY
  if(family == AF_INET6) {
    int sockopt = 1;
    if(setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (a2_sockopt_t) &sockopt,
                  sizeof(sockopt)) < 0) {
      errNum = SOCKET_ERRNO;
      error = errorMsg(errNum);
      CLOSE(fd);
      return -1;
    }
  }
#endif // IPV6_V6ONLY
  if(::bind(fd, addr, addrlen) == -1) {
    errNum = SOCKET_ERRNO;
    error = errorMsg(errNum);
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
    throw DL_ABORT_EX(fmt(EX_SOCKET_BIND, error.c_str()));
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
    for(std::vector<std::pair<sockaddr_union, socklen_t> >::
          const_iterator i = bindAddrs_.begin(), eoi = bindAddrs_.end();
        i != eoi; ++i) {
      char host[NI_MAXHOST];
      int s;
      s = getnameinfo(&(*i).first.sa, (*i).second, host, NI_MAXHOST, 0, 0,
                      NI_NUMERICHOST);
      if(s) {
        error = gai_strerror(s);
        continue;
      }
      if(addrp && strcmp(host, addrp) != 0) {
        error = "Given address and resolved address do not match.";
        continue;
      }
      sock_t fd = bindTo(host, port, family, sockType_, flags, error);
      if(fd != (sock_t)-1) {
        sockfd_ = fd;
        break;
      }
    }
  }
  if(sockfd_ == (sock_t) -1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_BIND, error.c_str()));
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
    throw DL_ABORT_EX(fmt(EX_SOCKET_BIND, error.c_str()));
  }
}

void SocketCore::beginListen()
{
  if(listen(sockfd_, 1) == -1) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_LISTEN, errorMsg(errNum).c_str()));
  }
}

SocketCore* SocketCore::acceptConnection() const
{
  sockaddr_union sockaddr;
  socklen_t len = sizeof(sockaddr);
  sock_t fd;
  while((fd = accept(sockfd_, &sockaddr.sa, &len)) == (sock_t) -1 &&
        SOCKET_ERRNO == A2_EINTR);
  int errNum = SOCKET_ERRNO;
  if(fd == (sock_t) -1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_ACCEPT, errorMsg(errNum).c_str()));
  }
  return new SocketCore(fd, sockType_);
}

int SocketCore::getAddrInfo(std::pair<std::string, uint16_t>& addrinfo) const
{
  sockaddr_union sockaddr;
  socklen_t len = sizeof(sockaddr);
  getAddrInfo(sockaddr, len);
  addrinfo = util::getNumericNameInfo(&sockaddr.sa, len);
  return sockaddr.storage.ss_family;
}

void SocketCore::getAddrInfo(sockaddr_union& sockaddr, socklen_t& len) const
{
  if(getsockname(sockfd_, &sockaddr.sa, &len) == -1) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_GET_NAME, errorMsg(errNum).c_str()));
  }
}

int SocketCore::getAddressFamily() const
{
  sockaddr_union sockaddr;
  socklen_t len = sizeof(sockaddr);
  getAddrInfo(sockaddr, len);
  return sockaddr.storage.ss_family;
}

int SocketCore::getPeerInfo(std::pair<std::string, uint16_t>& peerinfo) const
{
  sockaddr_union sockaddr;
  socklen_t len = sizeof(sockaddr);
  if(getpeername(sockfd_, &sockaddr.sa, &len) == -1) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_GET_NAME, errorMsg(errNum).c_str()));
  }
  peerinfo = util::getNumericNameInfo(&sockaddr.sa, len);
  return sockaddr.storage.ss_family;
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
    throw DL_ABORT_EX(fmt(EX_RESOLVE_HOSTNAME, host.c_str(), gai_strerror(s)));
  }
  WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
  struct addrinfo* rp;
  int errNum;
  for(rp = res; rp; rp = rp->ai_next) {
    sock_t fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    errNum = SOCKET_ERRNO;
    if(fd == (sock_t) -1) {
      error = errorMsg(errNum);
      continue;
    }
    int sockopt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (a2_sockopt_t) &sockopt,
                  sizeof(sockopt)) < 0) {
      errNum = SOCKET_ERRNO;
      error = errorMsg(errNum);
      CLOSE(fd);
      continue;
    }
    if(!bindAddrs_.empty()) {
      bool bindSuccess = false;
      for(std::vector<std::pair<sockaddr_union, socklen_t> >::
            const_iterator i = bindAddrs_.begin(), eoi = bindAddrs_.end();
          i != eoi; ++i) {
        if(::bind(fd, &(*i).first.sa, (*i).second) == -1) {
          errNum = SOCKET_ERRNO;
          error = errorMsg(errNum);
          A2_LOG_DEBUG(fmt(EX_SOCKET_BIND, error.c_str()));
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
      errNum = SOCKET_ERRNO;
      error = errorMsg(errNum);
      CLOSE(sockfd_);
      sockfd_ = (sock_t) -1;
      continue;
    }
    // TODO at this point, connection may not be established and it may fail
    // later. In such case, next ai_addr should be tried.
    break;
  }
  if(sockfd_ == (sock_t) -1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_CONNECT, host.c_str(), error.c_str()));
  }
}

void SocketCore::setSockOpt
(int level, int optname, void* optval, socklen_t optlen)
{
  if(setsockopt(sockfd_, level, optname, (a2_sockopt_t)optval, optlen) < 0) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_SET_OPT, errorMsg(errNum).c_str()));
  }
}   

void SocketCore::setMulticastInterface(const std::string& localAddr)
{
  in_addr addr;
  if(localAddr.empty()) {
    addr.s_addr = htonl(INADDR_ANY);
  } else {
    if(inetPton(AF_INET, localAddr.c_str(), &addr) != 0) {
      throw DL_ABORT_EX(fmt("%s is not valid IPv4 numeric address",
                            localAddr.c_str()));
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
  if(inetPton(AF_INET, multicastAddr.c_str(), &multiAddr) != 0) {
    throw DL_ABORT_EX(fmt("%s is not valid IPv4 numeric address",
                          multicastAddr.c_str()));
  }
  in_addr ifAddr;
  if(localAddr.empty()) {
    ifAddr.s_addr = htonl(INADDR_ANY);
  } else {
    if(inetPton(AF_INET, localAddr.c_str(), &ifAddr) != 0) {
      throw DL_ABORT_EX(fmt("%s is not valid IPv4 numeric address",
                            localAddr.c_str()));
    }
  }
  struct ip_mreq mreq;
  memset(&mreq, 0, sizeof(mreq));
  mreq.imr_multiaddr = multiAddr;
  mreq.imr_interface = ifAddr;
  setSockOpt(IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

void SocketCore::setTcpNodelay(bool f)
{
  int val = f;
  setSockOpt(IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}

void SocketCore::setNonBlockingMode()
{
#ifdef __MINGW32__
  static u_long flag = 1;
  if (::ioctlsocket(sockfd_, FIONBIO, &flag) == -1) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_NONBLOCKING, errorMsg(errNum).c_str()));
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
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_BLOCKING, errorMsg(errNum).c_str()));
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
#ifdef HAVE_OPENSSL
  // for SSL
  if(secure_) {
    SSL_shutdown(ssl);
  }
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGNUTLS
  if(secure_) {
    gnutls_bye(sslSession_, GNUTLS_SHUT_WR);
  }
#endif // HAVE_LIBGNUTLS
  if(sockfd_ != (sock_t) -1) {
    shutdown(sockfd_, SHUT_WR);
    CLOSE(sockfd_);
    sockfd_ = -1;
  }
#ifdef HAVE_OPENSSL
  // for SSL
  if(secure_) {
    SSL_free(ssl);
  }
#endif // HAVE_OPENSSL
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
  int errNum = SOCKET_ERRNO;
  if(r > 0) {
    return p.revents&(POLLOUT|POLLHUP|POLLERR);
  } else if(r == 0) {
    return false;
  } else {
    throw DL_RETRY_EX(fmt(EX_SOCKET_CHECK_WRITABLE, errorMsg(errNum).c_str()));
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
  int errNum = SOCKET_ERRNO;
  if(r == 1) {
    return true;
  } else if(r == 0) {
    // time out
    return false;
  } else {
    if(errNum == A2_EINPROGRESS || errNum == A2_EINTR) {
      return false;
    } else {
      throw DL_RETRY_EX
        (fmt(EX_SOCKET_CHECK_WRITABLE, errorMsg(errNum).c_str()));
    }
  }
#endif // !HAVE_POLL
}

bool SocketCore::isReadable(time_t timeout)
{
#ifdef HAVE_POLL
  struct pollfd p;
  p.fd = sockfd_;
  p.events = POLLIN;
  int r;
  while((r = poll(&p, 1, timeout*1000)) == -1 && errno == EINTR);
  int errNum = SOCKET_ERRNO;
  if(r > 0) {
    return p.revents&(POLLIN|POLLHUP|POLLERR);
  } else if(r == 0) {
    return false;
  } else {
    throw DL_RETRY_EX(fmt(EX_SOCKET_CHECK_READABLE, errorMsg(errNum).c_str()));
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
  int errNum = SOCKET_ERRNO;
  if(r == 1) {
    return true;
  } else if(r == 0) {
    // time out
    return false;
  } else {
    if(errNum == A2_EINPROGRESS || errNum == A2_EINTR) {
      return false;
    } else {
      throw DL_RETRY_EX
        (fmt(EX_SOCKET_CHECK_READABLE, errorMsg(errNum).c_str()));
    }
  }
#endif // !HAVE_POLL
}

#ifdef HAVE_OPENSSL
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
#endif // HAVE_OPENSSL

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
    int errNum = SOCKET_ERRNO;
    if(ret == -1) {
      if(A2_WOULDBLOCK(errNum)) {
        wantWrite_ = true;
        ret = 0;
      } else {
        throw DL_RETRY_EX(fmt(EX_SOCKET_SEND, errorMsg(errNum).c_str()));
      }
    }
  } else {
#ifdef HAVE_OPENSSL
    ERR_clear_error();
    ret = SSL_write(ssl, data, len);
    if(ret < 0) {
      ret = sslHandleEAGAIN(ret);
    }
    if(ret < 0) {
      throw DL_RETRY_EX
        (fmt(EX_SOCKET_SEND, ERR_error_string(ERR_get_error(), 0)));
    }
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGNUTLS
    while((ret = gnutls_record_send(sslSession_, data, len)) ==
          GNUTLS_E_INTERRUPTED);
    if(ret == GNUTLS_E_AGAIN) {
      gnutlsRecordCheckDirection();
      ret = 0;
    } else if(ret < 0) {
      throw DL_RETRY_EX(fmt(EX_SOCKET_SEND, gnutls_strerror(ret)));
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
    while((ret = recv(sockfd_, data, len, 0)) == -1 &&
          SOCKET_ERRNO == A2_EINTR);
    int errNum = SOCKET_ERRNO;
    if(ret == -1) {
      if(A2_WOULDBLOCK(errNum)) {
        wantRead_ = true;
        ret = 0;
      } else {
        throw DL_RETRY_EX(fmt(EX_SOCKET_RECV, errorMsg(errNum).c_str()));
      }
    }
  } else {
#ifdef HAVE_OPENSSL
    // for SSL
    // TODO handling len == 0 case required
    ERR_clear_error();
    ret = SSL_read(ssl, data, len);
    if(ret < 0) {
      ret = sslHandleEAGAIN(ret);
    }
    if(ret < 0) {
      throw DL_RETRY_EX
        (fmt(EX_SOCKET_RECV, ERR_error_string(ERR_get_error(), 0)));
    }
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGNUTLS
    while((ret = gnutls_record_recv(sslSession_, data, len)) ==
          GNUTLS_E_INTERRUPTED);
    if(ret == GNUTLS_E_AGAIN) {
      gnutlsRecordCheckDirection();
      ret = 0;
    } else if(ret < 0) {
      throw DL_RETRY_EX(fmt(EX_SOCKET_RECV, gnutls_strerror(ret)));
    }
#endif // HAVE_LIBGNUTLS
  }

  len = ret;
}

void SocketCore::prepareSecureConnection()
{
  if(!secure_) {
#ifdef HAVE_OPENSSL
    // for SSL
    ssl = SSL_new(tlsContext_->getSSLCtx());
    if(!ssl) {
      throw DL_ABORT_EX
        (fmt(EX_SSL_INIT_FAILURE, ERR_error_string(ERR_get_error(), 0)));
    }
    if(SSL_set_fd(ssl, sockfd_) == 0) {
      throw DL_ABORT_EX
        (fmt(EX_SSL_INIT_FAILURE, ERR_error_string(ERR_get_error(), 0)));
    }
#endif // HAVE_OPENSSL
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
      throw DL_ABORT_EX(fmt(EX_SSL_INIT_FAILURE, gnutls_strerror(r)));
    }
    // put the x509 credentials to the current session
    gnutls_credentials_set(sslSession_, GNUTLS_CRD_CERTIFICATE,
                           tlsContext_->getCertCred());
    gnutls_transport_set_ptr(sslSession_, (gnutls_transport_ptr_t)sockfd_);
#endif // HAVE_LIBGNUTLS
    secure_ = A2_TLS_INITIALIZED;
  }
}

bool SocketCore::initiateSecureConnection(const std::string& hostname)
{
  wantRead_ = false;
  wantWrite_ = false;
#ifdef HAVE_OPENSSL
  switch(secure_) {
  case A2_TLS_INITIALIZED:
    secure_ = A2_TLS_HANDSHAKING;
#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
    if(!util::isNumericHost(hostname)) {
      // TLS extensions: SNI.  There is not documentation about the
      // return code for this function (actually this is macro
      // wrapping SSL_ctrl at the time of this writing).
      SSL_set_tlsext_host_name(ssl, hostname.c_str());
    }
#endif // SSL_CTRL_SET_TLSEXT_HOSTNAME
    // Fall through
  case A2_TLS_HANDSHAKING: {
    ERR_clear_error();
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
          throw DL_ABORT_EX(fmt(EX_SSL_CONNECT_ERROR, ssl_error));
        }
        break;

      case SSL_ERROR_SYSCALL:
        throw DL_ABORT_EX(EX_SSL_IO_ERROR);

      case SSL_ERROR_SSL:
        throw DL_ABORT_EX(EX_SSL_PROTOCOL_ERROR);

      default:
        throw DL_ABORT_EX(fmt(EX_SSL_UNKNOWN_ERROR, ssl_error));
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
          (fmt(MSG_CERT_VERIFICATION_FAILED,
               X509_verify_cert_error_string(verifyResult)));
      }
      std::string commonName;
      std::vector<std::string> dnsNames;
      std::vector<std::string> ipAddrs;
      GENERAL_NAMES* altNames;
      altNames = reinterpret_cast<GENERAL_NAMES*>
        (X509_get_ext_d2i(peerCert, NID_subject_alt_name, NULL, NULL));
      if(altNames) {
        auto_delete<GENERAL_NAMES*> altNamesDeleter
          (altNames, GENERAL_NAMES_free);
        size_t n = sk_GENERAL_NAME_num(altNames);
        for(size_t i = 0; i < n; ++i) {
          const GENERAL_NAME* altName = sk_GENERAL_NAME_value(altNames, i);
          if(altName->type == GEN_DNS) {
            const char* name =
              reinterpret_cast<char*>(ASN1_STRING_data(altName->d.ia5));
            if(!name) {
              continue;
            }
            size_t len = ASN1_STRING_length(altName->d.ia5);
            dnsNames.push_back(std::string(name, len));
          } else if(altName->type == GEN_IPADD) {
            const unsigned char* ipAddr = altName->d.iPAddress->data;
            if(!ipAddr) {
              continue;
            }
            size_t len = altName->d.iPAddress->length;
            ipAddrs.push_back(std::string(reinterpret_cast<const char*>(ipAddr),
                                          len));
          }
        }
      }
      X509_NAME* subjectName = X509_get_subject_name(peerCert);
      if(!subjectName) {
        throw DL_ABORT_EX
          ("Could not get X509 name object from the certificate.");
      }
      int lastpos = -1;
      while(1) {
        lastpos = X509_NAME_get_index_by_NID(subjectName, NID_commonName,
                                             lastpos);
        if(lastpos == -1) {
          break;
        }
        X509_NAME_ENTRY* entry = X509_NAME_get_entry(subjectName, lastpos);
        unsigned char* out;
        int outlen = ASN1_STRING_to_UTF8(&out,
                                         X509_NAME_ENTRY_get_data(entry));
        if(outlen < 0) {
          continue;
        }
        commonName.assign(&out[0], &out[outlen]);
        OPENSSL_free(out);
        break;
      }
      if(!net::verifyHostname(hostname, dnsNames, ipAddrs, commonName)) {
        throw DL_ABORT_EX(MSG_HOSTNAME_NOT_MATCH);
      }
    }
    secure_ = A2_TLS_CONNECTED;
    break;
  }
  default:
    break;
  }
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGNUTLS
  switch(secure_) {
  case A2_TLS_INITIALIZED:
    secure_ = A2_TLS_HANDSHAKING;
    if(!util::isNumericHost(hostname)) {
      // TLS extensions: SNI
      int ret = gnutls_server_name_set(sslSession_, GNUTLS_NAME_DNS,
                                       hostname.c_str(), hostname.size());
      if(ret < 0) {
        A2_LOG_WARN(fmt("Setting hostname in SNI extension failed. Cause: %s",
                        gnutls_strerror(ret)));
      }
    }
    // Fall through
  case A2_TLS_HANDSHAKING: {
    int ret = gnutls_handshake(sslSession_);
    if(ret == GNUTLS_E_AGAIN) {
      gnutlsRecordCheckDirection();
      return false;
    } else if(ret < 0) {
      throw DL_ABORT_EX(fmt(EX_SSL_INIT_FAILURE, gnutls_strerror(ret)));
    }

    if(tlsContext_->peerVerificationEnabled()) {
      // verify peer
      unsigned int status;
      ret = gnutls_certificate_verify_peers2(sslSession_, &status);
      if(ret < 0) {
        throw DL_ABORT_EX
          (fmt("gnutls_certificate_verify_peer2() failed. Cause: %s",
               gnutls_strerror(ret)));
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
          throw DL_ABORT_EX(fmt(MSG_CERT_VERIFICATION_FAILED, errors.c_str()));
        }
      }
      // certificate type: only X509 is allowed.
      if(gnutls_certificate_type_get(sslSession_) != GNUTLS_CRT_X509) {
        throw DL_ABORT_EX("Certificate type is not X509.");
      }

      unsigned int peerCertsLength;
      const gnutls_datum_t* peerCerts = gnutls_certificate_get_peers
        (sslSession_, &peerCertsLength);
      if(!peerCerts || peerCertsLength == 0 ) {
        throw DL_ABORT_EX(MSG_NO_CERT_FOUND);
      }
      Time now;
      for(unsigned int i = 0; i < peerCertsLength; ++i) {
        gnutls_x509_crt_t cert;
        ret = gnutls_x509_crt_init(&cert);
        if(ret < 0) {
          throw DL_ABORT_EX
            (fmt("gnutls_x509_crt_init() failed. Cause: %s",
                 gnutls_strerror(ret)));
        }
        auto_delete<gnutls_x509_crt_t> certDeleter
          (cert, gnutls_x509_crt_deinit);
        ret = gnutls_x509_crt_import(cert, &peerCerts[i], GNUTLS_X509_FMT_DER);
        if(ret < 0) {
          throw DL_ABORT_EX
            (fmt("gnutls_x509_crt_import() failed. Cause: %s",
                 gnutls_strerror(ret)));
        }
        if(i == 0) {
          std::string commonName;
          std::vector<std::string> dnsNames;
          std::vector<std::string> ipAddrs;
          int ret = 0;
          char altName[256];
          size_t altNameLen;
          for(int j = 0; !(ret < 0); ++j) {
            altNameLen = sizeof(altName);
            ret = gnutls_x509_crt_get_subject_alt_name(cert, j, altName,
                                                       &altNameLen, 0);
            if(ret == GNUTLS_SAN_DNSNAME) {
              dnsNames.push_back(std::string(altName, altNameLen));
            } else if(ret == GNUTLS_SAN_IPADDRESS) {
              ipAddrs.push_back(std::string(altName, altNameLen));
            }
          }
          altNameLen = sizeof(altName);
          ret = gnutls_x509_crt_get_dn_by_oid(cert,
                                              GNUTLS_OID_X520_COMMON_NAME, 0, 0,
                                              altName, &altNameLen);
          if(ret == 0) {
            commonName.assign(altName, altNameLen);
          }
          if(!net::verifyHostname(hostname, dnsNames, ipAddrs, commonName)) {
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
    secure_ = A2_TLS_CONNECTED;
    break;
  }
  default:
    break;
  }
#endif // HAVE_LIBGNUTLS
  return true;
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
    throw DL_ABORT_EX(fmt(EX_SOCKET_SEND, gai_strerror(s)));
  }
  WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
  struct addrinfo* rp;
  ssize_t r = -1;
  int errNum = 0;
  for(rp = res; rp; rp = rp->ai_next) {
    while((r = sendto(sockfd_, data, len, 0, rp->ai_addr, rp->ai_addrlen)) == -1
          && A2_EINTR == SOCKET_ERRNO);
    errNum = SOCKET_ERRNO;
    if(r == static_cast<ssize_t>(len)) {
      break;
    }
    if(r == -1 && A2_WOULDBLOCK(errNum)) {
      wantWrite_ = true;
      r = 0;
      break;
    }
  }
  if(r == -1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_SEND, errorMsg(errNum).c_str()));
  }
  return r;
}

ssize_t SocketCore::readDataFrom(char* data, size_t len,
                                 std::pair<std::string /* numerichost */,
                                 uint16_t /* port */>& sender)
{
  wantRead_ = false;
  wantWrite_ = false;
  sockaddr_union sockaddr;
  socklen_t sockaddrlen = sizeof(sockaddr);
  ssize_t r;
  while((r = recvfrom(sockfd_, data, len, 0, &sockaddr.sa, &sockaddrlen)) == -1
        && A2_EINTR == SOCKET_ERRNO);
  int errNum = SOCKET_ERRNO;
  if(r == -1) {
    if(A2_WOULDBLOCK(errNum)) {
      wantRead_ = true;
      r = 0;
    } else {
      throw DL_RETRY_EX(fmt(EX_SOCKET_RECV, errorMsg(errNum).c_str()));
    }
  } else {
    sender = util::getNumericNameInfo(&sockaddr.sa, sockaddrlen);
  }

  return r;
}

std::string SocketCore::getSocketError() const
{
  int error;
  socklen_t optlen = sizeof(error);

  if(getsockopt(sockfd_, SOL_SOCKET, SO_ERROR,
                (a2_sockopt_t) &error, &optlen) == -1) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX
      (fmt("Failed to get socket error: %s", errorMsg(errNum).c_str()));
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
  std::vector<std::pair<sockaddr_union, socklen_t> > bindAddrs;
  getInterfaceAddress(bindAddrs, iface, protocolFamily_);
  if(bindAddrs.empty()) {
    throw DL_ABORT_EX
      (fmt(MSG_INTERFACE_NOT_FOUND, iface.c_str(), "not available"));
  } else {
    bindAddrs_.swap(bindAddrs);
    for(std::vector<std::pair<sockaddr_union, socklen_t> >::
          const_iterator i = bindAddrs_.begin(), eoi = bindAddrs_.end();
        i != eoi; ++i) {
      char host[NI_MAXHOST];
      int s;
      s = getnameinfo(&(*i).first.sa, (*i).second, host, NI_MAXHOST, 0, 0,
                      NI_NUMERICHOST);
      if(s == 0) {
        A2_LOG_DEBUG(fmt("Sockets will bind to %s", host));
      }
    }
  }
}

void getInterfaceAddress
(std::vector<std::pair<sockaddr_union, socklen_t> >& ifAddrs,
 const std::string& iface, int family, int aiFlags)
{
  A2_LOG_DEBUG(fmt("Finding interface %s", iface.c_str()));
#ifdef HAVE_GETIFADDRS
  // First find interface in interface addresses
  struct ifaddrs* ifaddr = 0;
  if(getifaddrs(&ifaddr) == -1) {
    int errNum = SOCKET_ERRNO;
    A2_LOG_INFO(fmt(MSG_INTERFACE_NOT_FOUND,
                    iface.c_str(), errorMsg(errNum).c_str()));
  } else {
    auto_delete<ifaddrs*> ifaddrDeleter(ifaddr, freeifaddrs);
    for(ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
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
      if(strcmp(iface.c_str(), ifa->ifa_name) == 0) {
        socklen_t bindAddrLen =
          iffamily == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
        sockaddr_union bindAddr;
        memset(&bindAddr, 0, sizeof(bindAddr));
        memcpy(&bindAddr.storage, ifa->ifa_addr, bindAddrLen);
        ifAddrs.push_back(std::make_pair(bindAddr, bindAddrLen));
      }
    }
  }
#endif // HAVE_GETIFADDRS
  if(ifAddrs.empty()) {
    addrinfo* res;
    int s;
    s = callGetaddrinfo(&res, iface.c_str(), 0, family, SOCK_STREAM, aiFlags,0);
    if(s) {
      A2_LOG_INFO(fmt(MSG_INTERFACE_NOT_FOUND, iface.c_str(), gai_strerror(s)));
    } else {
      WSAAPI_AUTO_DELETE<addrinfo*> resDeleter(res, freeaddrinfo);
      addrinfo* rp;
      for(rp = res; rp; rp = rp->ai_next) {
        // Try to bind socket with this address. If it fails, the
        // address is not for this machine.
        try {
          SocketCore socket;
          socket.bind(rp->ai_addr, rp->ai_addrlen);          
          sockaddr_union bindAddr;
          memset(&bindAddr, 0, sizeof(bindAddr));
          memcpy(&bindAddr.storage, rp->ai_addr, rp->ai_addrlen);
          ifAddrs.push_back(std::make_pair(bindAddr, rp->ai_addrlen));
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

} // namespace

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

int inetNtop(int af, const void* src, char* dst, socklen_t size)
{
  int s;
  sockaddr_union su;
  memset(&su, 0, sizeof(su));
  if(af == AF_INET) {
    su.in.sin_family = AF_INET;
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
    su.in.sin_len = sizeof(su.in);
#endif // HAVE_SOCKADDR_IN_SIN_LEN
    memcpy(&su.in.sin_addr, src, sizeof(su.in.sin_addr));
    s = getnameinfo(&su.sa, sizeof(su.in),
                    dst, size, 0, 0, NI_NUMERICHOST);
  } else if(af == AF_INET6) {
    su.in6.sin6_family = AF_INET6;
#ifdef HAVE_SOCKADDR_IN6_SIN6_LEN
    su.in6.sin6_len = sizeof(su.in6);
#endif // HAVE_SOCKADDR_IN6_SIN6_LEN
    memcpy(&su.in6.sin6_addr, src, sizeof(su.in6.sin6_addr));
    s = getnameinfo(&su.sa, sizeof(su.in6),
                    dst, size, 0, 0, NI_NUMERICHOST);
  } else {
    s = EAI_FAMILY;
  }
  return s;
}

int inetPton(int af, const char* src, void* dst)
{
  union {
    uint32_t ipv4_addr;
    unsigned char ipv6_addr[16];
  } binaddr;
  size_t len = net::getBinAddr(binaddr.ipv6_addr, src);
  if(af == AF_INET) {
    if(len != 4) {
      return -1;
    }
    in_addr* addr = reinterpret_cast<in_addr*>(dst);
    addr->s_addr = binaddr.ipv4_addr;
  } else if(af == AF_INET6) {
    if(len != 16) {
      return -1;
    }
    in6_addr* addr = reinterpret_cast<in6_addr*>(dst);
    memcpy(addr->s6_addr, binaddr.ipv6_addr, sizeof(addr->s6_addr));
  } else {
    return -1;
  }
  return 0;
}

namespace net {

size_t getBinAddr(void* dest, const std::string& ip)
{
  size_t len = 0;
  addrinfo* res;
  if(callGetaddrinfo(&res, ip.c_str(), 0, AF_UNSPEC,
                     0, AI_NUMERICHOST, 0) != 0) {
    return len;
  }
  WSAAPI_AUTO_DELETE<addrinfo*> resDeleter(res, freeaddrinfo);
  for(addrinfo* rp = res; rp; rp = rp->ai_next) {
    sockaddr_union su;
    memcpy(&su, rp->ai_addr, rp->ai_addrlen);
    if(rp->ai_family == AF_INET) {
      len = sizeof(in_addr);
      memcpy(dest, &(su.in.sin_addr), len);
      break;
    } else if(rp->ai_family == AF_INET6) {
      len = sizeof(in6_addr);
      memcpy(dest, &(su.in6.sin6_addr), len);
      break;
    }
  }
  return len;
}

bool verifyHostname(const std::string& hostname,
                    const std::vector<std::string>& dnsNames,
                    const std::vector<std::string>& ipAddrs,
                    const std::string& commonName)
{
  if(util::isNumericHost(hostname)) {
    if(ipAddrs.empty()) {
      return commonName == hostname;
    }
    // We need max 16 bytes to store IPv6 address.
    unsigned char binAddr[16];
    size_t addrLen = getBinAddr(binAddr, hostname);
    if(addrLen == 0) {
      return false;
    }
    for(std::vector<std::string>::const_iterator i = ipAddrs.begin(),
          eoi = ipAddrs.end(); i != eoi; ++i) {
      if(addrLen == (*i).size() &&
         memcmp(binAddr, (*i).c_str(), addrLen) == 0) {
        return true;
      }
    }
  } else {
    if(dnsNames.empty()) {
      return util::tlsHostnameMatch(commonName, hostname);
    }
    for(std::vector<std::string>::const_iterator i = dnsNames.begin(),
          eoi = dnsNames.end(); i != eoi; ++i) {
      if(util::tlsHostnameMatch(*i, hostname)) {
        return true;
      }
    }
  }
  return false;
}

} // namespace net

} // namespace aria2
