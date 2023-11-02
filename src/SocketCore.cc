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

#ifdef HAVE_IPHLPAPI_H
#  include <iphlpapi.h>
#endif // HAVE_IPHLPAPI_H

#include <unistd.h>
#ifdef HAVE_IFADDRS_H
#  include <ifaddrs.h>
#endif // HAVE_IFADDRS_H

#include <cerrno>
#include <cstring>
#include <cassert>
#include <sstream>
#include <array>

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
#  include "TLSContext.h"
#  include "TLSSession.h"
#endif // ENABLE_SSL
#ifdef HAVE_LIBSSH2
#  include "SSHSession.h"
#endif // HAVE_LIBSSH2

namespace aria2 {

#ifndef __MINGW32__
#  define SOCKET_ERRNO (errno)
#else
#  define SOCKET_ERRNO (WSAGetLastError())
#endif // __MINGW32__

#ifdef __MINGW32__
#  define A2_EINPROGRESS WSAEWOULDBLOCK
#  define A2_EWOULDBLOCK WSAEWOULDBLOCK
#  define A2_EINTR WSAEINTR
#  define A2_WOULDBLOCK(e) (e == WSAEWOULDBLOCK)
#else // !__MINGW32__
#  define A2_EINPROGRESS EINPROGRESS
#  ifndef EWOULDBLOCK
#    define EWOULDBLOCK EAGAIN
#  endif // EWOULDBLOCK
#  define A2_EWOULDBLOCK EWOULDBLOCK
#  define A2_EINTR EINTR
#  if EWOULDBLOCK == EAGAIN
#    define A2_WOULDBLOCK(e) (e == EWOULDBLOCK)
#  else // EWOULDBLOCK != EAGAIN
#    define A2_WOULDBLOCK(e) (e == EWOULDBLOCK || e == EAGAIN)
#  endif // EWOULDBLOCK != EAGAIN
#endif   // !__MINGW32__

#ifdef __MINGW32__
#  define CLOSE(X) ::closesocket(X)
#else
#  define CLOSE(X) close(X)
#endif // __MINGW32__

namespace {
std::string errorMsg(int errNum)
{
#ifndef __MINGW32__
  return util::safeStrerror(errNum);
#else
  auto msg = util::formatLastError(errNum);
  if (msg.empty()) {
    char buf[256];
    snprintf(buf, sizeof(buf), EX_SOCKET_UNKNOWN_ERROR, errNum, errNum);
    return buf;
  }
  return msg;
#endif // __MINGW32__
}
} // namespace

namespace {
enum TlsState {
  // TLS object is not initialized.
  A2_TLS_NONE = 0,
  // TLS object is now handshaking.
  A2_TLS_HANDSHAKING = 2,
  // TLS object is now connected.
  A2_TLS_CONNECTED = 3
};
} // namespace

int SocketCore::protocolFamily_ = AF_UNSPEC;
int SocketCore::ipDscp_ = 0;

std::vector<SockAddr> SocketCore::bindAddrs_;
std::vector<std::vector<SockAddr>> SocketCore::bindAddrsList_;
std::vector<std::vector<SockAddr>>::iterator SocketCore::bindAddrsListIt_;

int SocketCore::socketRecvBufferSize_ = 0;

#ifdef ENABLE_SSL
std::shared_ptr<TLSContext> SocketCore::clTlsContext_;
std::shared_ptr<TLSContext> SocketCore::svTlsContext_;

void SocketCore::setClientTLSContext(
    const std::shared_ptr<TLSContext>& tlsContext)
{
  clTlsContext_ = tlsContext;
}

void SocketCore::setServerTLSContext(
    const std::shared_ptr<TLSContext>& tlsContext)
{
  svTlsContext_ = tlsContext;
}
#endif // ENABLE_SSL

SocketCore::SocketCore(int sockType) : sockType_(sockType), sockfd_(-1)
{
  init();
}

SocketCore::SocketCore(sock_t sockfd, int sockType)
    : sockType_(sockType), sockfd_(sockfd)
{
  init();
}

void SocketCore::init()
{
  blocking_ = true;
  secure_ = A2_TLS_NONE;

  wantRead_ = false;
  wantWrite_ = false;
}

SocketCore::~SocketCore() { closeConnection(); }

namespace {
void applySocketBufferSize(sock_t fd)
{
  auto recvBufSize = SocketCore::getSocketRecvBufferSize();
  if (recvBufSize == 0) {
    return;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (a2_sockopt_t)&recvBufSize,
                 sizeof(recvBufSize)) < 0) {
    auto errNum = SOCKET_ERRNO;
    A2_LOG_WARN(fmt("Failed to set socket buffer size. Cause: %s",
                    errorMsg(errNum).c_str()));
  }
}
} // namespace

void SocketCore::create(int family, int protocol)
{
  int errNum;
  closeConnection();
  sock_t fd = socket(family, sockType_, protocol);
  errNum = SOCKET_ERRNO;
  if (fd == (sock_t)-1) {
    throw DL_ABORT_EX(
        fmt("Failed to create socket. Cause:%s", errorMsg(errNum).c_str()));
  }
  util::make_fd_cloexec(fd);
  int sockopt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (a2_sockopt_t)&sockopt,
                 sizeof(sockopt)) < 0) {
    errNum = SOCKET_ERRNO;
    CLOSE(fd);
    throw DL_ABORT_EX(
        fmt("Failed to create socket. Cause:%s", errorMsg(errNum).c_str()));
  }

  applySocketBufferSize(fd);

  sockfd_ = fd;
}

static sock_t bindInternal(int family, int socktype, int protocol,
                           const struct sockaddr* addr, socklen_t addrlen,
                           std::string& error)
{
  int errNum;
  sock_t fd = socket(family, socktype, protocol);
  errNum = SOCKET_ERRNO;
  if (fd == (sock_t)-1) {
    error = errorMsg(errNum);
    return -1;
  }
  util::make_fd_cloexec(fd);
  int sockopt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (a2_sockopt_t)&sockopt,
                 sizeof(sockopt)) < 0) {
    errNum = SOCKET_ERRNO;
    error = errorMsg(errNum);
    CLOSE(fd);
    return -1;
  }
#ifdef IPV6_V6ONLY
  if (family == AF_INET6) {
    int sockopt = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (a2_sockopt_t)&sockopt,
                   sizeof(sockopt)) < 0) {
      errNum = SOCKET_ERRNO;
      error = errorMsg(errNum);
      CLOSE(fd);
      return -1;
    }
  }
#endif // IPV6_V6ONLY

  applySocketBufferSize(fd);

  if (::bind(fd, addr, addrlen) == -1) {
    errNum = SOCKET_ERRNO;
    error = errorMsg(errNum);
    CLOSE(fd);
    return -1;
  }
  return fd;
}

static sock_t bindTo(const char* host, uint16_t port, int family, int sockType,
                     int getaddrinfoFlags, std::string& error)
{
  struct addrinfo* res;
  int s = callGetaddrinfo(&res, host, util::uitos(port).c_str(), family,
                          sockType, getaddrinfoFlags, 0);
  if (s) {
    error = gai_strerror(s);
    return -1;
  }
  std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resDeleter(res,
                                                                freeaddrinfo);
  struct addrinfo* rp;
  for (rp = res; rp; rp = rp->ai_next) {
    sock_t fd = bindInternal(rp->ai_family, rp->ai_socktype, rp->ai_protocol,
                             rp->ai_addr, rp->ai_addrlen, error);
    if (fd != (sock_t)-1) {
      return fd;
    }
  }
  return -1;
}

void SocketCore::bindWithFamily(uint16_t port, int family, int flags)
{
  closeConnection();
  std::string error;
  sock_t fd = bindTo(nullptr, port, family, sockType_, flags, error);
  if (fd == (sock_t)-1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_BIND, error.c_str()));
  }
  sockfd_ = fd;
}

void SocketCore::bind(const char* addr, uint16_t port, int family, int flags)
{
  closeConnection();
  std::string error;
  const char* addrp;
  if (addr && addr[0]) {
    addrp = addr;
  }
  else {
    addrp = nullptr;
  }
  if (addrp || !(flags & AI_PASSIVE) || bindAddrsList_.empty()) {
    sock_t fd = bindTo(addrp, port, family, sockType_, flags, error);
    if (fd == (sock_t)-1) {
      throw DL_ABORT_EX(fmt(EX_SOCKET_BIND, error.c_str()));
    }
    sockfd_ = fd;
    return;
  }

  std::array<char, NI_MAXHOST> host;
  for (const auto& bindAddrs : bindAddrsList_) {
    for (const auto& a : bindAddrs) {
      if (family != AF_UNSPEC && family != a.su.storage.ss_family) {
        continue;
      }
      auto s = getnameinfo(&a.su.sa, a.suLength, host.data(), NI_MAXHOST,
                           nullptr, 0, NI_NUMERICHOST);
      if (s) {
        error = gai_strerror(s);
        continue;
      }
      if (addrp && strcmp(host.data(), addrp) != 0) {
        error = "Given address and resolved address do not match.";
        continue;
      }
      auto fd = bindTo(host.data(), port, family, sockType_, flags, error);
      if (fd != (sock_t)-1) {
        sockfd_ = fd;
        return;
      }
    }
  }

  if (sockfd_ == (sock_t)-1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_BIND, error.c_str()));
  }
}

void SocketCore::bind(uint16_t port, int flags)
{
  bind(nullptr, port, protocolFamily_, flags);
}

void SocketCore::bind(const struct sockaddr* addr, socklen_t addrlen)
{
  closeConnection();
  std::string error;
  sock_t fd = bindInternal(addr->sa_family, sockType_, 0, addr, addrlen, error);
  if (fd == (sock_t)-1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_BIND, error.c_str()));
  }
  sockfd_ = fd;
}

void SocketCore::beginListen()
{
  if (listen(sockfd_, 1024) == -1) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_LISTEN, errorMsg(errNum).c_str()));
  }
  setNonBlockingMode();
}

std::shared_ptr<SocketCore> SocketCore::acceptConnection() const
{
  sockaddr_union sockaddr;
  socklen_t len = sizeof(sockaddr);
  sock_t fd;
  while ((fd = accept(sockfd_, &sockaddr.sa, &len)) == (sock_t)-1 &&
         SOCKET_ERRNO == A2_EINTR)
    ;
  int errNum = SOCKET_ERRNO;
  if (fd == (sock_t)-1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_ACCEPT, errorMsg(errNum).c_str()));
  }

  applySocketBufferSize(fd);

  auto sock = std::make_shared<SocketCore>(fd, sockType_);
  sock->setNonBlockingMode();
  return sock;
}

Endpoint SocketCore::getAddrInfo() const
{
  sockaddr_union sockaddr;
  socklen_t len = sizeof(sockaddr);
  getAddrInfo(sockaddr, len);
  return util::getNumericNameInfo(&sockaddr.sa, len);
}

void SocketCore::getAddrInfo(sockaddr_union& sockaddr, socklen_t& len) const
{
  if (getsockname(sockfd_, &sockaddr.sa, &len) == -1) {
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

Endpoint SocketCore::getPeerInfo() const
{
  sockaddr_union sockaddr;
  socklen_t len = sizeof(sockaddr);
  if (getpeername(sockfd_, &sockaddr.sa, &len) == -1) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_GET_NAME, errorMsg(errNum).c_str()));
  }
  return util::getNumericNameInfo(&sockaddr.sa, len);
}

void SocketCore::establishConnection(const std::string& host, uint16_t port,
                                     bool tcpNodelay)
{
  closeConnection();
  std::string error;
  struct addrinfo* res;
  int s;
  s = callGetaddrinfo(&res, host.c_str(), util::uitos(port).c_str(),
                      protocolFamily_, sockType_, 0, 0);
  if (s) {
    throw DL_ABORT_EX(fmt(EX_RESOLVE_HOSTNAME, host.c_str(), gai_strerror(s)));
  }
  std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resDeleter(res,
                                                                freeaddrinfo);
  struct addrinfo* rp;
  int errNum;
  for (rp = res; rp; rp = rp->ai_next) {
    sock_t fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    errNum = SOCKET_ERRNO;
    if (fd == (sock_t)-1) {
      error = errorMsg(errNum);
      continue;
    }
    util::make_fd_cloexec(fd);
    int sockopt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (a2_sockopt_t)&sockopt,
                   sizeof(sockopt)) < 0) {
      errNum = SOCKET_ERRNO;
      error = errorMsg(errNum);
      CLOSE(fd);
      continue;
    }

    applySocketBufferSize(fd);

    if (!bindAddrs_.empty()) {
      bool bindSuccess = false;
      for (const auto& soaddr : bindAddrs_) {
        if (::bind(fd, &soaddr.su.sa, soaddr.suLength) == -1) {
          errNum = SOCKET_ERRNO;
          error = errorMsg(errNum);
          A2_LOG_DEBUG(fmt(EX_SOCKET_BIND, error.c_str()));
        }
        else {
          bindSuccess = true;
          break;
        }
      }
      if (!bindSuccess) {
        CLOSE(fd);
        continue;
      }
    }
    if (!bindAddrsList_.empty()) {
      ++bindAddrsListIt_;
      if (bindAddrsListIt_ == bindAddrsList_.end()) {
        bindAddrsListIt_ = bindAddrsList_.begin();
      }
      bindAddrs_ = *bindAddrsListIt_;
    }

    sockfd_ = fd;
    // make socket non-blocking mode
    setNonBlockingMode();
    if (tcpNodelay) {
      setTcpNodelay(true);
    }
    if (connect(fd, rp->ai_addr, rp->ai_addrlen) == -1 &&
        SOCKET_ERRNO != A2_EINPROGRESS) {
      errNum = SOCKET_ERRNO;
      error = errorMsg(errNum);
      CLOSE(sockfd_);
      sockfd_ = (sock_t)-1;
      continue;
    }
    // TODO at this point, connection may not be established and it may fail
    // later. In such case, next ai_addr should be tried.
    break;
  }
  if (sockfd_ == (sock_t)-1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_CONNECT, host.c_str(), error.c_str()));
  }
}

void SocketCore::setSockOpt(int level, int optname, void* optval,
                            socklen_t optlen)
{
  if (setsockopt(sockfd_, level, optname, (a2_sockopt_t)optval, optlen) < 0) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(fmt(EX_SOCKET_SET_OPT, errorMsg(errNum).c_str()));
  }
}

void SocketCore::setMulticastInterface(const std::string& localAddr)
{
  in_addr addr;
  if (localAddr.empty()) {
    addr.s_addr = htonl(INADDR_ANY);
  }
  else if (inetPton(AF_INET, localAddr.c_str(), &addr) != 0) {
    throw DL_ABORT_EX(
        fmt("%s is not valid IPv4 numeric address", localAddr.c_str()));
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

void SocketCore::joinMulticastGroup(const std::string& multicastAddr,
                                    uint16_t multicastPort,
                                    const std::string& localAddr)
{
  in_addr multiAddr;
  if (inetPton(AF_INET, multicastAddr.c_str(), &multiAddr) != 0) {
    throw DL_ABORT_EX(
        fmt("%s is not valid IPv4 numeric address", multicastAddr.c_str()));
  }
  in_addr ifAddr;
  if (localAddr.empty()) {
    ifAddr.s_addr = htonl(INADDR_ANY);
  }
  else if (inetPton(AF_INET, localAddr.c_str(), &ifAddr) != 0) {
    throw DL_ABORT_EX(
        fmt("%s is not valid IPv4 numeric address", localAddr.c_str()));
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

void SocketCore::applyIpDscp()
{
  if (ipDscp_ == 0) {
    return;
  }

  try {
    int family = getAddressFamily();
    if (family == AF_INET) {
      setSockOpt(IPPROTO_IP, IP_TOS, &ipDscp_, sizeof(ipDscp_));
    }
#if defined(IPV6_TCLASS) || defined(__linux__) || defined(__FreeBSD__) ||      \
    defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    else if (family == AF_INET6) {
      setSockOpt(IPPROTO_IPV6, IPV6_TCLASS, &ipDscp_, sizeof(ipDscp_));
    }
#endif
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX("Applying DSCP value failed", e);
  }
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
  while ((flags = fcntl(sockfd_, F_GETFL, 0)) == -1 && errno == EINTR)
    ;
  // TODO add error handling
  while (fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK) == -1 && errno == EINTR)
    ;
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
  while ((flags = fcntl(sockfd_, F_GETFL, 0)) == -1 && errno == EINTR)
    ;
  // TODO add error handling
  while (fcntl(sockfd_, F_SETFL, flags & (~O_NONBLOCK)) == -1 && errno == EINTR)
    ;
#endif // __MINGW32__
  blocking_ = true;
}

void SocketCore::closeConnection()
{
#ifdef ENABLE_SSL
  if (tlsSession_) {
    tlsSession_->closeConnection();
    tlsSession_.reset();
  }
#endif // ENABLE_SSL

#ifdef HAVE_LIBSSH2
  if (sshSession_) {
    sshSession_->closeConnection();
    sshSession_.reset();
  }
#endif // HAVE_LIBSSH2

  if (sockfd_ != (sock_t)-1) {
    shutdown(sockfd_, SHUT_WR);
    CLOSE(sockfd_);
    sockfd_ = -1;
  }
}

#ifndef __MINGW32__
#  define CHECK_FD(fd)                                                         \
    if (fd < 0 || FD_SETSIZE <= fd) {                                          \
      logger_->warn("Detected file descriptor >= FD_SETSIZE or < 0. "          \
                    "Download may slow down or fail.");                        \
      return false;                                                            \
    }
#endif // !__MINGW32__

bool SocketCore::isWritable(time_t timeout)
{
#ifdef HAVE_POLL
  struct pollfd p;
  p.fd = sockfd_;
  p.events = POLLOUT;
  int r;
  while ((r = poll(&p, 1, timeout * 1000)) == -1 && errno == EINTR)
    ;
  int errNum = SOCKET_ERRNO;
  if (r > 0) {
    return p.revents & (POLLOUT | POLLHUP | POLLERR);
  }
  if (r == 0) {
    return false;
  }
  throw DL_RETRY_EX(fmt(EX_SOCKET_CHECK_WRITABLE, errorMsg(errNum).c_str()));
#else // !HAVE_POLL
#  ifndef __MINGW32__
  CHECK_FD(sockfd_);
#  endif // !__MINGW32__
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd_, &fds);

  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  int r = select(sockfd_ + 1, nullptr, &fds, nullptr, &tv);
  int errNum = SOCKET_ERRNO;
  if (r == 1) {
    return true;
  }
  if (r == 0) {
    // time out
    return false;
  }
  if (errNum == A2_EINPROGRESS || errNum == A2_EINTR) {
    return false;
  }
  throw DL_RETRY_EX(fmt(EX_SOCKET_CHECK_WRITABLE, errorMsg(errNum).c_str()));
#endif   // !HAVE_POLL
}

bool SocketCore::isReadable(time_t timeout)
{
#ifdef HAVE_POLL
  struct pollfd p;
  p.fd = sockfd_;
  p.events = POLLIN;
  int r;
  while ((r = poll(&p, 1, timeout * 1000)) == -1 && errno == EINTR)
    ;
  int errNum = SOCKET_ERRNO;
  if (r > 0) {
    return p.revents & (POLLIN | POLLHUP | POLLERR);
  }
  if (r == 0) {
    return false;
  }
  throw DL_RETRY_EX(fmt(EX_SOCKET_CHECK_READABLE, errorMsg(errNum).c_str()));
#else // !HAVE_POLL
#  ifndef __MINGW32__
  CHECK_FD(sockfd_);
#  endif // !__MINGW32__
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd_, &fds);

  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  int r = select(sockfd_ + 1, &fds, nullptr, nullptr, &tv);
  int errNum = SOCKET_ERRNO;
  if (r == 1) {
    return true;
  }
  if (r == 0) {
    // time out
    return false;
  }
  if (errNum == A2_EINPROGRESS || errNum == A2_EINTR) {
    return false;
  }
  throw DL_RETRY_EX(fmt(EX_SOCKET_CHECK_READABLE, errorMsg(errNum).c_str()));
#endif   // !HAVE_POLL
}

ssize_t SocketCore::writeVector(a2iovec* iov, size_t iovcnt)
{
  ssize_t ret = 0;
  wantRead_ = false;
  wantWrite_ = false;
  if (!secure_) {
#ifdef __MINGW32__
    DWORD nsent;
    int rv = WSASend(sockfd_, iov, iovcnt, &nsent, 0, 0, 0);
    if (rv == 0) {
      ret = nsent;
    }
    else {
      ret = -1;
    }
#else  // !__MINGW32__
    while ((ret = writev(sockfd_, iov, iovcnt)) == -1 &&
           SOCKET_ERRNO == A2_EINTR)
      ;
#endif // !__MINGW32__
    int errNum = SOCKET_ERRNO;
    if (ret == -1) {
      if (!A2_WOULDBLOCK(errNum)) {
        throw DL_RETRY_EX(fmt(EX_SOCKET_SEND, errorMsg(errNum).c_str()));
      }
      wantWrite_ = true;
      ret = 0;
    }
  }
  else {
    // For SSL/TLS, we could not use writev, so just iterate vector
    // and write the data in normal way.
    for (size_t i = 0; i < iovcnt; ++i) {
      ssize_t rv = writeData(iov[i].A2IOVEC_BASE, iov[i].A2IOVEC_LEN);
      if (rv == 0) {
        break;
      }
      ret += rv;
    }
  }
  return ret;
}

ssize_t SocketCore::writeData(const void* data, size_t len)
{
  ssize_t ret = 0;
  wantRead_ = false;
  wantWrite_ = false;

  if (!secure_) {
    // Cast for Windows send()
    while ((ret = send(sockfd_, reinterpret_cast<const char*>(data), len, 0)) ==
               -1 &&
           SOCKET_ERRNO == A2_EINTR)
      ;
    int errNum = SOCKET_ERRNO;
    if (ret == -1) {
      if (!A2_WOULDBLOCK(errNum)) {
        throw DL_RETRY_EX(fmt(EX_SOCKET_SEND, errorMsg(errNum).c_str()));
      }
      wantWrite_ = true;
      ret = 0;
    }
  }
  else {
#ifdef ENABLE_SSL
    ret = tlsSession_->writeData(data, len);
    if (ret < 0) {
      if (ret != TLS_ERR_WOULDBLOCK) {
        throw DL_RETRY_EX(
            fmt(EX_SOCKET_SEND, tlsSession_->getLastErrorString().c_str()));
      }
      if (tlsSession_->checkDirection() == TLS_WANT_READ) {
        wantRead_ = true;
      }
      else {
        wantWrite_ = true;
      }
      ret = 0;
    }
#endif // ENABLE_SSL
  }
  return ret;
}

void SocketCore::readData(void* data, size_t& len)
{
  ssize_t ret = 0;
  wantRead_ = false;
  wantWrite_ = false;

#ifdef HAVE_LIBSSH2
  if (sshSession_) {
    ret = sshSession_->readData(data, len);
    if (ret < 0) {
      if (ret != SSH_ERR_WOULDBLOCK) {
        throw DL_RETRY_EX(
            fmt(EX_SOCKET_RECV, sshSession_->getLastErrorString().c_str()));
      }
      if (sshSession_->checkDirection() == SSH_WANT_READ) {
        wantRead_ = true;
      }
      else {
        wantWrite_ = true;
      }
      ret = 0;
    }
  }
  else
#endif // HAVE_LIBSSH2
    if (!secure_) {
      // Cast for Windows recv()
      while ((ret = recv(sockfd_, reinterpret_cast<char*>(data), len, 0)) ==
                 -1 &&
             SOCKET_ERRNO == A2_EINTR)
        ;
      int errNum = SOCKET_ERRNO;
      if (ret == -1) {
        if (!A2_WOULDBLOCK(errNum)) {
          throw DL_RETRY_EX(fmt(EX_SOCKET_RECV, errorMsg(errNum).c_str()));
        }
        wantRead_ = true;
        ret = 0;
      }
    }
    else {
#ifdef ENABLE_SSL
      ret = tlsSession_->readData(data, len);
      if (ret < 0) {
        if (ret != TLS_ERR_WOULDBLOCK) {
          throw DL_RETRY_EX(
              fmt(EX_SOCKET_RECV, tlsSession_->getLastErrorString().c_str()));
        }
        if (tlsSession_->checkDirection() == TLS_WANT_READ) {
          wantRead_ = true;
        }
        else {
          wantWrite_ = true;
        }
        ret = 0;
      }
#endif // ENABLE_SSL
    }

  len = ret;
}

#ifdef ENABLE_SSL

bool SocketCore::tlsAccept()
{
  return tlsHandshake(svTlsContext_.get(), A2STR::NIL);
}

bool SocketCore::tlsConnect(const std::string& hostname)
{
  return tlsHandshake(clTlsContext_.get(), hostname);
}

bool SocketCore::tlsHandshake(TLSContext* tlsctx, const std::string& hostname)
{
  wantRead_ = false;
  wantWrite_ = false;

  if (secure_ == A2_TLS_CONNECTED) {
    // Already connected!
    return true;
  }

  if (secure_ == A2_TLS_NONE) {
    // Do some initial setup
    A2_LOG_DEBUG("Creating TLS session");
    tlsSession_.reset(TLSSession::make(tlsctx));
    auto rv = tlsSession_->init(sockfd_);
    if (rv != TLS_ERR_OK) {
      std::string error = tlsSession_->getLastErrorString();
      tlsSession_.reset();
      throw DL_ABORT_EX(fmt(EX_SSL_INIT_FAILURE, error.c_str()));
    }
    // Check hostname is not numeric and it includes ".". Setting
    // "localhost" will produce TLS alert with GNUTLS.
    if (tlsctx->getSide() == TLS_CLIENT && !util::isNumericHost(hostname) &&
        hostname.find(".") != std::string::npos) {
      rv = tlsSession_->setSNIHostname(hostname);
      if (rv != TLS_ERR_OK) {
        throw DL_ABORT_EX(fmt(EX_SSL_INIT_FAILURE,
                              tlsSession_->getLastErrorString().c_str()));
      }
    }
    // Done with the setup, now let handshaking begin immediately.
    secure_ = A2_TLS_HANDSHAKING;
    A2_LOG_DEBUG("TLS Handshaking");
  }

  if (secure_ == A2_TLS_HANDSHAKING) {
    // Starting handshake after initial setup or still handshaking.
    TLSVersion ver = TLS_PROTO_NONE;
    int rv = 0;
    std::string handshakeError;

    if (tlsctx->getSide() == TLS_CLIENT) {
      rv = tlsSession_->tlsConnect(hostname, ver, handshakeError);
    }
    else {
      rv = tlsSession_->tlsAccept(ver);
    }

    if (rv == TLS_ERR_OK) {
      // We're good, more or less.
      // 1. Construct peerinfo
      std::stringstream ss;
      if (!hostname.empty()) {
        ss << hostname << " (";
      }
      auto peerEndpoint = getPeerInfo();
      ss << peerEndpoint.addr << ":" << peerEndpoint.port;
      if (!hostname.empty()) {
        ss << ")";
      }

      std::string tlsVersion;
      switch (ver) {
      case TLS_PROTO_TLS11:
        tlsVersion = A2_V_TLS11;
        break;
      case TLS_PROTO_TLS12:
        tlsVersion = A2_V_TLS12;
        break;
      case TLS_PROTO_TLS13:
        tlsVersion = A2_V_TLS13;
        break;
      default:
        assert(0);
        abort();
      }

      auto peerInfo = ss.str();

      A2_LOG_DEBUG(fmt("Securely connected to %s with %s", peerInfo.c_str(),
                       tlsVersion.c_str()));

      // 2. We're connected now!
      secure_ = A2_TLS_CONNECTED;
      return true;
    }

    if (rv == TLS_ERR_WOULDBLOCK) {
      // We're not done yet...
      if (tlsSession_->checkDirection() == TLS_WANT_READ) {
        // ... but read buffers are empty.
        wantRead_ = true;
      }
      else {
        // ... but write buffers are full.
        wantWrite_ = true;
      }
      // Returning false (instead of true==success or throwing) will cause this
      // function to be called again once buffering is dealt with
      return false;
    }

    if (rv == TLS_ERR_ERROR) {
      // Damn those error.
      throw DL_ABORT_EX(fmt("SSL/TLS handshake failure: %s",
                            handshakeError.empty()
                                ? tlsSession_->getLastErrorString().c_str()
                                : handshakeError.c_str()));
    }

    // Some implementation passed back an invalid result.
    throw DL_ABORT_EX(fmt(EX_SSL_INIT_FAILURE,
                          "Invalid connect state (this is a bug in the TLS "
                          "backend!)"));
  }

  // We should never get here, i.e. all possible states should have been handled
  // and returned from a branch before! Getting here is a bug, of course!
  throw DL_ABORT_EX(fmt(EX_SSL_INIT_FAILURE, "Invalid state (this is a bug!)"));
}

#endif // ENABLE_SSL

#ifdef HAVE_LIBSSH2

bool SocketCore::sshHandshake(const std::string& hashType,
                              const std::string& digest)
{
  wantRead_ = false;
  wantWrite_ = false;

  if (!sshSession_) {
    sshSession_ = make_unique<SSHSession>();
    if (sshSession_->init(sockfd_) == SSH_ERR_ERROR) {
      throw DL_ABORT_EX("Could not create SSH session");
    }
  }
  auto rv = sshSession_->handshake();
  if (rv == SSH_ERR_WOULDBLOCK) {
    sshCheckDirection();
    return false;
  }
  if (rv == SSH_ERR_ERROR) {
    throw DL_ABORT_EX(fmt("SSH handshake failure: %s",
                          sshSession_->getLastErrorString().c_str()));
  }
  if (!hashType.empty()) {
    auto actualDigest = sshSession_->hostkeyMessageDigest(hashType);
    if (actualDigest.empty()) {
      throw DL_ABORT_EX(fmt("Empty host key fingerprint from SSH layer: "
                            "perhaps hash type %s is not supported?",
                            hashType.c_str()));
    }
    if (digest != actualDigest) {
      throw DL_ABORT_EX(fmt("Unexpected SSH host key: expected %s, actual %s",
                            util::toHex(digest).c_str(),
                            util::toHex(actualDigest).c_str()));
    }
  }
  return true;
}

bool SocketCore::sshAuthPassword(const std::string& user,
                                 const std::string& password)
{
  assert(sshSession_);

  wantRead_ = false;
  wantWrite_ = false;

  auto rv = sshSession_->authPassword(user, password);
  if (rv == SSH_ERR_WOULDBLOCK) {
    sshCheckDirection();
    return false;
  }
  if (rv == SSH_ERR_ERROR) {
    throw DL_ABORT_EX(fmt("SSH authentication failure: %s",
                          sshSession_->getLastErrorString().c_str()));
  }
  return true;
}

bool SocketCore::sshSFTPOpen(const std::string& path)
{
  assert(sshSession_);

  wantRead_ = false;
  wantWrite_ = false;

  auto rv = sshSession_->sftpOpen(path);
  if (rv == SSH_ERR_WOULDBLOCK) {
    sshCheckDirection();
    return false;
  }
  if (rv == SSH_ERR_ERROR) {
    throw DL_ABORT_EX(fmt("SSH opening SFTP path %s failed: %s", path.c_str(),
                          sshSession_->getLastErrorString().c_str()));
  }
  return true;
}

bool SocketCore::sshSFTPClose()
{
  assert(sshSession_);

  wantRead_ = false;
  wantWrite_ = false;

  auto rv = sshSession_->sftpClose();
  if (rv == SSH_ERR_WOULDBLOCK) {
    sshCheckDirection();
    return false;
  }
  if (rv == SSH_ERR_ERROR) {
    throw DL_ABORT_EX(fmt("SSH closing SFTP failed: %s",
                          sshSession_->getLastErrorString().c_str()));
  }
  return true;
}

bool SocketCore::sshSFTPStat(int64_t& totalLength, time_t& mtime,
                             const std::string& path)
{
  assert(sshSession_);

  wantRead_ = false;
  wantWrite_ = false;

  auto rv = sshSession_->sftpStat(totalLength, mtime);
  if (rv == SSH_ERR_WOULDBLOCK) {
    sshCheckDirection();
    return false;
  }
  if (rv == SSH_ERR_ERROR) {
    throw DL_ABORT_EX(fmt("SSH stat SFTP path %s filed: %s", path.c_str(),
                          sshSession_->getLastErrorString().c_str()));
  }
  return true;
}

void SocketCore::sshSFTPSeek(int64_t pos)
{
  assert(sshSession_);

  sshSession_->sftpSeek(pos);
}

bool SocketCore::sshGracefulShutdown()
{
  assert(sshSession_);
  auto rv = sshSession_->gracefulShutdown();
  if (rv == SSH_ERR_WOULDBLOCK) {
    sshCheckDirection();
    return false;
  }
  if (rv == SSH_ERR_ERROR) {
    throw DL_ABORT_EX(fmt("SSH graceful shutdown failed: %s",
                          sshSession_->getLastErrorString().c_str()));
  }
  return true;
}

void SocketCore::sshCheckDirection()
{
  if (sshSession_->checkDirection() == SSH_WANT_READ) {
    wantRead_ = true;
  }
  else {
    wantWrite_ = true;
  }
}

#endif // HAVE_LIBSSH2

ssize_t SocketCore::writeData(const void* data, size_t len,
                              const std::string& host, uint16_t port)
{
  wantRead_ = false;
  wantWrite_ = false;

  struct addrinfo* res;
  int s;
  s = callGetaddrinfo(&res, host.c_str(), util::uitos(port).c_str(),
                      protocolFamily_, sockType_, 0, 0);
  if (s) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_SEND, gai_strerror(s)));
  }
  std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resDeleter(res,
                                                                freeaddrinfo);
  struct addrinfo* rp;
  ssize_t r = -1;
  int errNum = 0;
  for (rp = res; rp; rp = rp->ai_next) {
    // Cast for Windows sendto()
    while ((r = sendto(sockfd_, reinterpret_cast<const char*>(data), len, 0,
                       rp->ai_addr, rp->ai_addrlen)) == -1 &&
           A2_EINTR == SOCKET_ERRNO)
      ;
    errNum = SOCKET_ERRNO;
    if (r == static_cast<ssize_t>(len)) {
      break;
    }
    if (r == -1 && A2_WOULDBLOCK(errNum)) {
      wantWrite_ = true;
      r = 0;
      break;
    }
  }
  if (r == -1) {
    throw DL_ABORT_EX(fmt(EX_SOCKET_SEND, errorMsg(errNum).c_str()));
  }
  return r;
}

ssize_t SocketCore::readDataFrom(void* data, size_t len, Endpoint& sender)
{
  wantRead_ = false;
  wantWrite_ = false;
  sockaddr_union sockaddr;
  socklen_t sockaddrlen = sizeof(sockaddr);
  ssize_t r;
  // Cast for Windows recvfrom()
  while ((r = recvfrom(sockfd_, reinterpret_cast<char*>(data), len, 0,
                       &sockaddr.sa, &sockaddrlen)) == -1 &&
         A2_EINTR == SOCKET_ERRNO)
    ;
  int errNum = SOCKET_ERRNO;
  if (r == -1) {
    if (!A2_WOULDBLOCK(errNum)) {
      throw DL_RETRY_EX(fmt(EX_SOCKET_RECV, errorMsg(errNum).c_str()));
    }
    wantRead_ = true;
    r = 0;
  }
  else {
    sender = util::getNumericNameInfo(&sockaddr.sa, sockaddrlen);
  }

  return r;
}

std::string SocketCore::getSocketError() const
{
  int error;
  socklen_t optlen = sizeof(error);

  if (getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, (a2_sockopt_t)&error,
                 &optlen) == -1) {
    int errNum = SOCKET_ERRNO;
    throw DL_ABORT_EX(
        fmt("Failed to get socket error: %s", errorMsg(errNum).c_str()));
  }
  if (error != 0) {
    return errorMsg(error);
  }
  return "";
}

bool SocketCore::wantRead() const { return wantRead_; }

bool SocketCore::wantWrite() const { return wantWrite_; }

void SocketCore::bindAddress(const std::string& iface)
{
  auto bindAddrs = getInterfaceAddress(iface, protocolFamily_);
  if (bindAddrs.empty()) {
    throw DL_ABORT_EX(
        fmt(MSG_INTERFACE_NOT_FOUND, iface.c_str(), "not available"));
  }
  bindAddrs_.swap(bindAddrs);
  for (const auto& a : bindAddrs_) {
    char host[NI_MAXHOST];
    int s;
    s = getnameinfo(&a.su.sa, a.suLength, host, NI_MAXHOST, nullptr, 0,
                    NI_NUMERICHOST);
    if (s == 0) {
      A2_LOG_DEBUG(fmt("Sockets will bind to %s", host));
    }
  }
  bindAddrsList_.push_back(bindAddrs_);
  bindAddrsListIt_ = std::begin(bindAddrsList_);
}

void SocketCore::bindAllAddress(const std::string& ifaces)
{
  std::vector<std::vector<SockAddr>> bindAddrsList;
  std::vector<std::string> ifaceList;
  util::split(ifaces.begin(), ifaces.end(), std::back_inserter(ifaceList), ',',
              true);
  if (ifaceList.empty()) {
    throw DL_ABORT_EX(
        "List of interfaces is empty, one or more interfaces is required");
  }
  for (auto& iface : ifaceList) {
    auto bindAddrs = getInterfaceAddress(iface, protocolFamily_);
    if (bindAddrs.empty()) {
      throw DL_ABORT_EX(
          fmt(MSG_INTERFACE_NOT_FOUND, iface.c_str(), "not available"));
    }
    bindAddrsList.push_back(bindAddrs);
    for (const auto& a : bindAddrs) {
      char host[NI_MAXHOST];
      int s;
      s = getnameinfo(&a.su.sa, a.suLength, host, NI_MAXHOST, nullptr, 0,
                      NI_NUMERICHOST);
      if (s == 0) {
        A2_LOG_DEBUG(fmt("Sockets will bind to %s", host));
      }
    }
  }
  bindAddrsList_.swap(bindAddrsList);
  bindAddrsListIt_ = bindAddrsList_.begin();
  bindAddrs_ = *bindAddrsListIt_;
}

void SocketCore::setSocketRecvBufferSize(int size)
{
  socketRecvBufferSize_ = size;
}

int SocketCore::getSocketRecvBufferSize() { return socketRecvBufferSize_; }

size_t SocketCore::getRecvBufferedLength() const
{
#ifdef ENABLE_SSL
  if (!tlsSession_) {
    return 0;
  }

  return tlsSession_->getRecvBufferedLength();
#else  // !ENABLE_SSL
  return 0;
#endif // !ENABLE_SSL
}

std::vector<SockAddr> SocketCore::getInterfaceAddress(const std::string& iface,
                                                      int family, int aiFlags)
{
  A2_LOG_DEBUG(fmt("Finding interface %s", iface.c_str()));
  std::vector<SockAddr> ifAddrs;
#ifdef HAVE_GETIFADDRS
  // First find interface in interface addresses
  struct ifaddrs* ifaddr = nullptr;
  if (getifaddrs(&ifaddr) == -1) {
    int errNum = SOCKET_ERRNO;
    A2_LOG_INFO(
        fmt(MSG_INTERFACE_NOT_FOUND, iface.c_str(), errorMsg(errNum).c_str()));
  }
  else {
    std::unique_ptr<ifaddrs, decltype(&freeifaddrs)> ifaddrDeleter(ifaddr,
                                                                   freeifaddrs);
    for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
      if (!ifa->ifa_addr) {
        continue;
      }
      int iffamily = ifa->ifa_addr->sa_family;
      if (family == AF_UNSPEC) {
        if (iffamily != AF_INET && iffamily != AF_INET6) {
          continue;
        }
      }
      else if (family == AF_INET) {
        if (iffamily != AF_INET) {
          continue;
        }
      }
      else if (family == AF_INET6) {
        if (iffamily != AF_INET6) {
          continue;
        }
      }
      else {
        continue;
      }
      if (strcmp(iface.c_str(), ifa->ifa_name) == 0) {
        SockAddr soaddr;
        soaddr.suLength =
            iffamily == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
        memcpy(&soaddr.su, ifa->ifa_addr, soaddr.suLength);
        ifAddrs.push_back(soaddr);
      }
    }
  }
#endif // HAVE_GETIFADDRS
  if (ifAddrs.empty()) {
    addrinfo* res;
    int s;
    s = callGetaddrinfo(&res, iface.c_str(), nullptr, family, SOCK_STREAM,
                        aiFlags, 0);
    if (s) {
      A2_LOG_INFO(fmt(MSG_INTERFACE_NOT_FOUND, iface.c_str(), gai_strerror(s)));
    }
    else {
      std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resDeleter(
          res, freeaddrinfo);
      addrinfo* rp;
      for (rp = res; rp; rp = rp->ai_next) {
        // Try to bind socket with this address. If it fails, the
        // address is not for this machine.
        try {
          SocketCore socket;
          socket.bind(rp->ai_addr, rp->ai_addrlen);
          SockAddr soaddr;
          memcpy(&soaddr.su, rp->ai_addr, rp->ai_addrlen);
          soaddr.suLength = rp->ai_addrlen;
          ifAddrs.push_back(soaddr);
        }
        catch (RecoverableException& e) {
          continue;
        }
      }
    }
  }

  return ifAddrs;
}

namespace {

int defaultAIFlags = DEFAULT_AI_FLAGS;

int getDefaultAIFlags() { return defaultAIFlags; }

} // namespace

void setDefaultAIFlags(int flags) { defaultAIFlags = flags; }

int callGetaddrinfo(struct addrinfo** resPtr, const char* host,
                    const char* service, int family, int sockType, int flags,
                    int protocol)
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
  sockaddr_union su;
  memset(&su, 0, sizeof(su));
  if (af == AF_INET) {
    su.in.sin_family = AF_INET;
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
    su.in.sin_len = sizeof(su.in);
#endif // HAVE_SOCKADDR_IN_SIN_LEN
    memcpy(&su.in.sin_addr, src, sizeof(su.in.sin_addr));
    return getnameinfo(&su.sa, sizeof(su.in), dst, size, nullptr, 0,
                       NI_NUMERICHOST);
  }
  if (af == AF_INET6) {
    su.in6.sin6_family = AF_INET6;
#ifdef HAVE_SOCKADDR_IN6_SIN6_LEN
    su.in6.sin6_len = sizeof(su.in6);
#endif // HAVE_SOCKADDR_IN6_SIN6_LEN
    memcpy(&su.in6.sin6_addr, src, sizeof(su.in6.sin6_addr));
    return getnameinfo(&su.sa, sizeof(su.in6), dst, size, nullptr, 0,
                       NI_NUMERICHOST);
  }
  return EAI_FAMILY;
}

int inetPton(int af, const char* src, void* dst)
{
  union {
    uint32_t ipv4_addr;
    unsigned char ipv6_addr[16];
  } binaddr;
  size_t len = net::getBinAddr(binaddr.ipv6_addr, src);
  if (af == AF_INET) {
    if (len != 4) {
      return -1;
    }
    in_addr* addr = reinterpret_cast<in_addr*>(dst);
    addr->s_addr = binaddr.ipv4_addr;
    return 0;
  }
  if (af == AF_INET6) {
    if (len != 16) {
      return -1;
    }
    in6_addr* addr = reinterpret_cast<in6_addr*>(dst);
    memcpy(addr->s6_addr, binaddr.ipv6_addr, sizeof(addr->s6_addr));
    return 0;
  }
  return -1;
}

namespace net {

size_t getBinAddr(void* dest, const std::string& ip)
{
  size_t len = 0;
  addrinfo* res;
  if (callGetaddrinfo(&res, ip.c_str(), nullptr, AF_UNSPEC, 0, AI_NUMERICHOST,
                      0) != 0) {
    return len;
  }
  std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resDeleter(res,
                                                                freeaddrinfo);
  for (addrinfo* rp = res; rp; rp = rp->ai_next) {
    sockaddr_union su;
    memcpy(&su, rp->ai_addr, rp->ai_addrlen);
    if (rp->ai_family == AF_INET) {
      len = sizeof(in_addr);
      memcpy(dest, &(su.in.sin_addr), len);
      break;
    }
    else if (rp->ai_family == AF_INET6) {
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
  if (util::isNumericHost(hostname)) {
    if (ipAddrs.empty()) {
      return commonName == hostname;
    }
    // We need max 16 bytes to store IPv6 address.
    unsigned char binAddr[16];
    size_t addrLen = getBinAddr(binAddr, hostname);
    if (addrLen == 0) {
      return false;
    }
    for (auto& ipAddr : ipAddrs) {
      if (addrLen == ipAddr.size() &&
          memcmp(binAddr, ipAddr.c_str(), addrLen) == 0) {
        return true;
      }
    }
    return false;
  }

  if (dnsNames.empty()) {
    return util::tlsHostnameMatch(commonName, hostname);
  }
  for (auto& dnsName : dnsNames) {
    if (util::tlsHostnameMatch(dnsName, hostname)) {
      return true;
    }
  }
  return false;
}

namespace {
bool ipv4AddrConfigured = true;
bool ipv6AddrConfigured = true;
} // namespace

#ifdef __MINGW32__
namespace {
const uint32_t APIPA_IPV4_BEGIN = 2851995649u; // 169.254.0.1
const uint32_t APIPA_IPV4_END = 2852061183u;   // 169.254.255.255
} // namespace
#endif // __MINGW32__

void checkAddrconfig()
{
#ifdef HAVE_IPHLPAPI_H
  A2_LOG_INFO("Checking configured addresses");
  ULONG bufsize = 15_k;
  ULONG retval = 0;
  IP_ADAPTER_ADDRESSES* buf = 0;
  int numTry = 0;
  const int MAX_TRY = 3;
  do {
    buf = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(malloc(bufsize));
    retval = GetAdaptersAddresses(AF_UNSPEC, 0, 0, buf, &bufsize);
    if (retval != ERROR_BUFFER_OVERFLOW) {
      break;
    }
    free(buf);
    buf = 0;
  } while (retval == ERROR_BUFFER_OVERFLOW && numTry < MAX_TRY);
  if (retval != NO_ERROR) {
    A2_LOG_INFO("GetAdaptersAddresses failed. Assume both IPv4 and IPv6 "
                " addresses are configured.");
    return;
  }
  ipv4AddrConfigured = false;
  ipv6AddrConfigured = false;
  char host[NI_MAXHOST];
  sockaddr_union ad;
  int rv;
  for (IP_ADAPTER_ADDRESSES* p = buf; p; p = p->Next) {
    if (p->IfType == IF_TYPE_TUNNEL) {
      // Skip tunnel interface because Windows7 automatically setup
      // this for IPv6.
      continue;
    }
    PIP_ADAPTER_UNICAST_ADDRESS ucaddr = p->FirstUnicastAddress;
    if (!ucaddr) {
      continue;
    }
    for (PIP_ADAPTER_UNICAST_ADDRESS i = ucaddr; i; i = i->Next) {
      bool found = false;
      switch (i->Address.iSockaddrLength) {
      case sizeof(sockaddr_in): {
        memcpy(&ad.storage, i->Address.lpSockaddr, i->Address.iSockaddrLength);
        uint32_t haddr = ntohl(ad.in.sin_addr.s_addr);
        if (haddr != INADDR_LOOPBACK &&
            (haddr < APIPA_IPV4_BEGIN || APIPA_IPV4_END <= haddr)) {
          ipv4AddrConfigured = true;
          found = true;
        }
        break;
      }
      case sizeof(sockaddr_in6):
        memcpy(&ad.storage, i->Address.lpSockaddr, i->Address.iSockaddrLength);
        if (!IN6_IS_ADDR_LOOPBACK(&ad.in6.sin6_addr) &&
            !IN6_IS_ADDR_LINKLOCAL(&ad.in6.sin6_addr)) {
          ipv6AddrConfigured = true;
          found = true;
        }
        break;
      }
      rv = getnameinfo(i->Address.lpSockaddr, i->Address.iSockaddrLength, host,
                       NI_MAXHOST, 0, 0, NI_NUMERICHOST);
      if (rv == 0) {
        if (found) {
          A2_LOG_INFO(fmt("Found configured address: %s", host));
        }
        else {
          A2_LOG_INFO(fmt("Not considered: %s", host));
        }
      }
    }
  }
  free(buf);

  A2_LOG_INFO(fmt("IPv4 configured=%d, IPv6 configured=%d", ipv4AddrConfigured,
                  ipv6AddrConfigured));
#elif defined(HAVE_GETIFADDRS)
  A2_LOG_INFO("Checking configured addresses");
  ipv4AddrConfigured = false;
  ipv6AddrConfigured = false;
  ifaddrs* ifaddr = nullptr;
  int rv;
  rv = getifaddrs(&ifaddr);
  if (rv == -1) {
    int errNum = SOCKET_ERRNO;
    A2_LOG_INFO(fmt("getifaddrs failed. Cause: %s", errorMsg(errNum).c_str()));
    return;
  }
  std::unique_ptr<ifaddrs, decltype(&freeifaddrs)> ifaddrDeleter(ifaddr,
                                                                 freeifaddrs);
  char host[NI_MAXHOST];
  sockaddr_union ad;
  for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    bool found = false;
    size_t addrlen = 0;
    switch (ifa->ifa_addr->sa_family) {
    case AF_INET: {
      addrlen = sizeof(sockaddr_in);
      memcpy(&ad.storage, ifa->ifa_addr, addrlen);
      if (ad.in.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
        ipv4AddrConfigured = true;
        found = true;
      }
      break;
    }
    case AF_INET6: {
      addrlen = sizeof(sockaddr_in6);
      memcpy(&ad.storage, ifa->ifa_addr, addrlen);
      if (!IN6_IS_ADDR_LOOPBACK(&ad.in6.sin6_addr) &&
          !IN6_IS_ADDR_LINKLOCAL(&ad.in6.sin6_addr)) {
        ipv6AddrConfigured = true;
        found = true;
      }
      break;
    }
    default:
      continue;
    }
    rv = getnameinfo(ifa->ifa_addr, addrlen, host, NI_MAXHOST, nullptr, 0,
                     NI_NUMERICHOST);
    if (rv == 0) {
      if (found) {
        A2_LOG_INFO(fmt("Found configured address: %s", host));
      }
      else {
        A2_LOG_INFO(fmt("Not considered: %s", host));
      }
    }
  }
  A2_LOG_INFO(fmt("IPv4 configured=%d, IPv6 configured=%d", ipv4AddrConfigured,
                  ipv6AddrConfigured));
#else  // !HAVE_GETIFADDRS
  A2_LOG_INFO("getifaddrs is not available. Assume IPv4 and IPv6 addresses"
              " are configured.");
#endif // !HAVE_GETIFADDRS
}

bool getIPv4AddrConfigured() { return ipv4AddrConfigured; }

bool getIPv6AddrConfigured() { return ipv6AddrConfigured; }

} // namespace net

} // namespace aria2
