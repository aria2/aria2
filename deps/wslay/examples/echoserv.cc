/*
 * Wslay - The WebSocket Library
 *
 * Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// WebSocket Echo Server
// This is suitable for Autobahn server test.
// g++ -Wall -O2 -g -o echoserv echoserv.cc -L../lib/.libs -I../lib/includes -lwslay -lnettle
// $ export LD_LIBRARY_PATH=../lib/.libs
// $ ./a.out 9000
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>

#include <cassert>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <string>
#include <set>
#include <iomanip>
#include <fstream>

#include <nettle/base64.h>
#include <nettle/sha.h>
#include <wslay/wslay.h>

int create_listen_socket(const char *service)
{
  struct addrinfo hints;
  int sfd = -1;
  int r;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
  struct addrinfo *res;
  r = getaddrinfo(0, service, &hints, &res);
  if(r != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(r) << std::endl;
    return -1;
  }
  for(struct addrinfo *rp = res; rp; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(sfd == -1) {
      continue;
    }
    int val = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val,
                  static_cast<socklen_t>(sizeof(val))) == -1) {
      continue;
    }
    if(bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;
    }
    close(sfd);
  }
  freeaddrinfo(res);
  if(listen(sfd, 16) == -1) {
    perror("listen");
    close(sfd);
    return -1;
  }
  return sfd;
}

int make_non_block(int fd)
{
  int flags, r;
  while((flags = fcntl(fd, F_GETFL, 0)) == -1 && errno == EINTR);
  if(flags == -1) {
    return -1;
  }
  while((r = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1 && errno == EINTR);
  if(r == -1) {
    return -1;
  }
  return 0;
}

std::string sha1(const std::string& src)
{
  sha1_ctx ctx;
  sha1_init(&ctx);
  sha1_update(&ctx, src.size(), reinterpret_cast<const uint8_t*>(src.c_str()));
  uint8_t temp[SHA1_DIGEST_SIZE];
  sha1_digest(&ctx, SHA1_DIGEST_SIZE, temp);
  std::string res(&temp[0], &temp[SHA1_DIGEST_SIZE]);
  return res;
}

std::string base64(const std::string& src)
{
  base64_encode_ctx ctx;
  base64_encode_init(&ctx);
  int dstlen = BASE64_ENCODE_RAW_LENGTH(src.size());
  uint8_t *dst = new uint8_t[dstlen];
  base64_encode_raw(dst, src.size(), reinterpret_cast<const uint8_t*>(src.c_str()));
  std::string res(&dst[0], &dst[dstlen]);
  delete [] dst;
  return res;
}

std::string create_acceptkey(const std::string& clientkey)
{
  std::string s = clientkey+"258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  return base64(sha1(s));
}

class EventHandler {
public:
  virtual ~EventHandler() {}
  virtual int on_read_event() = 0;
  virtual int on_write_event() = 0;
  virtual bool want_read() = 0;
  virtual bool want_write() = 0;
  virtual int fd() const = 0;
  virtual bool finish() = 0;
  virtual EventHandler* next() = 0;
};

ssize_t send_callback(wslay_event_context_ptr ctx,
                      const uint8_t *data, size_t len, int flags,
                      void *user_data);
ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len,
                      int flags, void *user_data);
void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data);

class EchoWebSocketHandler : public EventHandler {
public:
  EchoWebSocketHandler(int fd)
    : fd_(fd)
  {
    struct wslay_event_callbacks callbacks = {
      recv_callback,
      send_callback,
      NULL, /* genmask_callback */
      NULL, /* on_frame_recv_start_callback */
      NULL, /* on_frame_recv_callback */
      NULL, /* on_frame_recv_end_callback */
      on_msg_recv_callback
    };
    wslay_event_context_server_init(&ctx_, &callbacks, this);
  }
  virtual ~EchoWebSocketHandler()
  {
    wslay_event_context_free(ctx_);
    shutdown(fd_, SHUT_WR);
    close(fd_);
  }
  virtual int on_read_event()
  {
    if(wslay_event_recv(ctx_) == 0) {
      return 0;
    } else {
      return -1;
    }
  }
  virtual int on_write_event()
  {
    if(wslay_event_send(ctx_) == 0) {
      return 0;
    } else {
      return -1;
    }
  }
  ssize_t send_data(const uint8_t *data, size_t len, int flags)
  {
    ssize_t r;
    int sflags = 0;
#ifdef MSG_MORE
    if(flags & WSLAY_MSG_MORE) {
      sflags |= MSG_MORE;
    }
#endif // MSG_MORE
    while((r = send(fd_, data, len, sflags)) == -1 && errno == EINTR);
    return r;
  }
  ssize_t recv_data(uint8_t *data, size_t len, int flags)
  {
    ssize_t r;
    while((r = recv(fd_, data, len, 0)) == -1 && errno == EINTR);
    return r;
  }
  virtual bool want_read()
  {
    return wslay_event_want_read(ctx_);
  }
  virtual bool want_write()
  {
    return wslay_event_want_write(ctx_);
  }
  virtual int fd() const
  {
    return fd_;
  }
  virtual bool finish()
  {
    return !want_read() && !want_write();
  }
  virtual EventHandler* next()
  {
    return 0;
  }
private:
  int fd_;
  wslay_event_context_ptr ctx_;
};

ssize_t send_callback(wslay_event_context_ptr ctx,
                      const uint8_t *data, size_t len, int flags,
                      void *user_data)
{
  EchoWebSocketHandler *sv = (EchoWebSocketHandler*)user_data;
  ssize_t r = sv->send_data(data, len, flags);
  if(r == -1) {
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }
  }
  return r;
}

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len,
                      int flags, void *user_data)
{
  EchoWebSocketHandler *sv = (EchoWebSocketHandler*)user_data;
  ssize_t r = sv->recv_data(data, len, flags);
  if(r == -1) {
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }
  } else if(r == 0) {
    wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    r = -1;
  }
  return r;
}

void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data)
{
  if(!wslay_is_ctrl_frame(arg->opcode)) {
    struct wslay_event_msg msgarg = {
      arg->opcode, arg->msg, arg->msg_length
    };
    wslay_event_queue_msg(ctx, &msgarg);
  }
}

class HttpHandshakeSendHandler : public EventHandler {
public:
  HttpHandshakeSendHandler(int fd, const std::string& accept_key)
    : fd_(fd),
      resheaders_("HTTP/1.1 101 Switching Protocols\r\n"
                  "Upgrade: websocket\r\n"
                  "Connection: Upgrade\r\n"
                  "Sec-WebSocket-Accept: "+accept_key+"\r\n"
                  "\r\n"),
      off_(0)
  {}
  virtual ~HttpHandshakeSendHandler()
  {
    if(fd_ != -1) {
      shutdown(fd_, SHUT_WR);
      close(fd_);
    }
  }
  virtual int on_read_event()
  {
    return 0;
  }
  virtual int on_write_event()
  {
    while(1) {
      size_t len = resheaders_.size()-off_;
      if(len == 0) {
        break;
      }
      ssize_t r;
      while((r = write(fd_, resheaders_.c_str()+off_, len)) == -1 &&
            errno == EINTR);
      if(r == -1) {
        if(errno == EAGAIN || errno == EWOULDBLOCK) {
          break;
        } else {
          perror("write");
          return -1;
        }
      } else {
        off_ += r;
      }
    }
    return 0;
  }
  virtual bool want_read()
  {
    return false;
  }
  virtual bool want_write()
  {
    return true;
  }
  virtual int fd() const
  {
    return fd_;
  }
  virtual bool finish()
  {
    return off_ == resheaders_.size();
  }
  virtual EventHandler* next()
  {
    if(finish()) {
      int fd = fd_;
      fd_ = -1;
      return new EchoWebSocketHandler(fd);
    } else {
      return 0;
    }
  }
private:
  int fd_;
  std::string headers_;
  std::string resheaders_;
  size_t off_;
};

class HttpHandshakeRecvHandler : public EventHandler {
public:
  HttpHandshakeRecvHandler(int fd)
    : fd_(fd)
  {}
  virtual ~HttpHandshakeRecvHandler()
  {
    if(fd_ != -1) {
      close(fd_);
    }
  }
  virtual int on_read_event()
  {
    char buf[4096];
    ssize_t r;
    std::string client_key;
    while(1) {
      while((r = read(fd_, buf, sizeof(buf))) == -1 && errno == EINTR);
      if(r == -1) {
        if(errno == EWOULDBLOCK || errno == EAGAIN) {
          break;
        } else {
          perror("read");
          return -1;
        }
      } else if(r == 0) {
        std::cerr << "http_upgrade: Got EOF" << std::endl;
        return -1;
      } else {
        headers_.append(buf, buf+r);
        if(headers_.size() > 8192) {
          std::cerr << "Too large http header" << std::endl;
          return -1;
        }
      }
    }
    if(headers_.find("\r\n\r\n") != std::string::npos) {
      std::string::size_type keyhdstart;
      if(headers_.find("Upgrade: websocket\r\n") == std::string::npos ||
         headers_.find("Connection: Upgrade\r\n") == std::string::npos ||
         (keyhdstart = headers_.find("Sec-WebSocket-Key: ")) ==
         std::string::npos) {
        std::cerr << "http_upgrade: missing required headers" << std::endl;
        return -1;
      }
      keyhdstart += 19;
      std::string::size_type keyhdend = headers_.find("\r\n", keyhdstart);
      client_key = headers_.substr(keyhdstart, keyhdend-keyhdstart);
      accept_key_ = create_acceptkey(client_key);
    }
    return 0;
  }
  virtual int on_write_event()
  {
    return 0;
  }
  virtual bool want_read()
  {
    return true;
  }
  virtual bool want_write()
  {
    return false;
  }
  virtual int fd() const
  {
    return fd_;
  }
  virtual bool finish()
  {
    return !accept_key_.empty();
  }
  virtual EventHandler* next()
  {
    if(finish()) {
      int fd = fd_;
      fd_ = -1;
      return new HttpHandshakeSendHandler(fd, accept_key_);
    } else {
      return 0;
    }
  }
private:
  int fd_;
  std::string headers_;
  std::string accept_key_;
};

class ListenEventHandler : public EventHandler {
public:
  ListenEventHandler(int fd)
    : fd_(fd), cfd_(-1)
  {}
  virtual ~ListenEventHandler()
  {
    close(fd_);
    close(cfd_);
  }
  virtual int on_read_event()
  {
    if(cfd_ != -1) {
      close(cfd_);
    }
    while((cfd_ = accept(fd_, 0, 0)) == -1 && errno == EINTR);
    if(cfd_ == -1) {
      perror("accept");
    }
    return 0;
  }
  virtual int on_write_event()
  {
    return 0;
  }
  virtual bool want_read()
  {
    return true;
  }
  virtual bool want_write()
  {
    return false;
  }
  virtual int fd() const
  {
    return fd_;
  }
  virtual bool finish()
  {
    return false;
  }
  virtual EventHandler* next()
  {
    if(cfd_ != -1) {
      int val = 1;
      int fd = cfd_;
      cfd_ = -1;
      if(make_non_block(fd) == -1 ||
         setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t)sizeof(val))
         == -1) {
        close(fd);
        return 0;
      }
      return new HttpHandshakeRecvHandler(fd);
    } else {
      return 0;
    }
  }
private:
  int fd_;
  int cfd_;
};

int ctl_epollev(int epollfd, int op, EventHandler *handler)
{
  epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  int events = 0;
  if(handler->want_read()) {
    events |= EPOLLIN;
  }
  if(handler->want_write()) {
    events |= EPOLLOUT;
  }
  ev.events = events;
  ev.data.ptr = handler;
  return epoll_ctl(epollfd, op, handler->fd(), &ev);
}

void reactor(int sfd)
{
  std::set<EventHandler*> handlers;
  ListenEventHandler* listen_handler = new ListenEventHandler(sfd);
  handlers.insert(listen_handler);
  int epollfd = epoll_create(16);
  if(epollfd == -1) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }
  if(ctl_epollev(epollfd, EPOLL_CTL_ADD, listen_handler) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }
  static const size_t MAX_EVENTS = 64;
  epoll_event events[MAX_EVENTS];
  while(1) {
    int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if(nfds == -1) {
      perror("epoll_wait");
      return;
    }
    for(int n = 0; n < nfds; ++n) {
      EventHandler* eh = (EventHandler*)events[n].data.ptr;
      if(((events[n].events & EPOLLIN) && eh->on_read_event() == -1) ||
         ((events[n].events & EPOLLOUT) && eh->on_write_event() == -1) ||
         (events[n].events & (EPOLLERR | EPOLLHUP))) {
        handlers.erase(eh);
        delete eh;
      } else {
        EventHandler* next = eh->next();
        if(next) {
          handlers.insert(next);
          if(ctl_epollev(epollfd, EPOLL_CTL_ADD, next) == -1) {
            if(errno == EEXIST) {
              if(ctl_epollev(epollfd, EPOLL_CTL_MOD, next) == -1) {
                perror("epoll_ctl");
                delete next;
              }
            } else {
              perror("epoll_ctl");
              delete next;
            }
          }
        }
        if(eh->finish()) {
          handlers.erase(eh);
          delete eh;
        } else {
          if(ctl_epollev(epollfd, EPOLL_CTL_MOD, eh) == -1) {
            perror("epoll_ctl");
          }
        }
      }
    }
  }
}

int main(int argc, char **argv)
{
  if(argc < 2) {
    std::cerr << "Usage: " << argv[0] << " PORT" << std::endl;
    exit(EXIT_FAILURE);
  }
  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, 0);
  int sfd = create_listen_socket(argv[1]);
  if(sfd == -1) {
    std::cerr << "Failed to create server socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "WebSocket echo server, listening on " << argv[1] << std::endl;
  reactor(sfd);
}
