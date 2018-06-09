/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#ifndef D_EPOLL_EVENT_POLL_H
#define D_EPOLL_EVENT_POLL_H

#include "EventPoll.h"

#include <sys/epoll.h>

#include <map>

#include "Event.h"
#include "a2functional.h"
#ifdef ENABLE_ASYNC_DNS
#  include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

class EpollEventPoll : public EventPoll {
private:
  class KSocketEntry;

  typedef Event<KSocketEntry> KEvent;
  typedef CommandEvent<KSocketEntry, EpollEventPoll> KCommandEvent;
  typedef ADNSEvent<KSocketEntry, EpollEventPoll> KADNSEvent;
  typedef AsyncNameResolverEntry<EpollEventPoll> KAsyncNameResolverEntry;
  friend class AsyncNameResolverEntry<EpollEventPoll>;

  class KSocketEntry : public SocketEntry<KCommandEvent, KADNSEvent> {
  public:
    KSocketEntry(sock_t socket);

    KSocketEntry(const KSocketEntry&) = delete;
    KSocketEntry(KSocketEntry&&) = default;

    struct epoll_event getEvents();
  };

  friend int accumulateEvent(int events, const KEvent& event);

private:
  typedef std::map<sock_t, KSocketEntry> KSocketEntrySet;
  KSocketEntrySet socketEntries_;
#ifdef ENABLE_ASYNC_DNS
  typedef std::map<std::pair<AsyncNameResolver*, Command*>,
                   KAsyncNameResolverEntry>
      KAsyncNameResolverEntrySet;
  KAsyncNameResolverEntrySet nameResolverEntries_;
#endif // ENABLE_ASYNC_DNS

  int epfd_;

  size_t epEventsSize_;

  std::unique_ptr<struct epoll_event[]> epEvents_;

  static const size_t EPOLL_EVENTS_MAX = 1024;

  bool addEvents(sock_t socket, const KEvent& event);

  bool deleteEvents(sock_t socket, const KEvent& event);

  bool addEvents(sock_t socket, Command* command, int events,
                 const std::shared_ptr<AsyncNameResolver>& rs);

  bool deleteEvents(sock_t socket, Command* command,
                    const std::shared_ptr<AsyncNameResolver>& rs);

public:
  EpollEventPoll();

  bool good() const;

  virtual ~EpollEventPoll();

  virtual void poll(const struct timeval& tv) CXX11_OVERRIDE;

  virtual bool addEvents(sock_t socket, Command* command,
                         EventPoll::EventType events) CXX11_OVERRIDE;

  virtual bool deleteEvents(sock_t socket, Command* command,
                            EventPoll::EventType events) CXX11_OVERRIDE;
#ifdef ENABLE_ASYNC_DNS

  virtual bool
  addNameResolver(const std::shared_ptr<AsyncNameResolver>& resolver,
                  Command* command) CXX11_OVERRIDE;
  virtual bool
  deleteNameResolver(const std::shared_ptr<AsyncNameResolver>& resolver,
                     Command* command) CXX11_OVERRIDE;
#endif // ENABLE_ASYNC_DNS

  static const int IEV_READ = EPOLLIN;
  static const int IEV_WRITE = EPOLLOUT;
  static const int IEV_ERROR = EPOLLERR;
  static const int IEV_HUP = EPOLLHUP;
};

} // namespace aria2

#endif // D_EPOLL_EVENT_POLL_H
