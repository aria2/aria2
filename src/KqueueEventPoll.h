/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#ifndef D_KQUEUE_EVENT_POLL_H
#define D_KQUEUE_EVENT_POLL_H

#include "EventPoll.h"

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <set>

#include "Event.h"
#include "a2functional.h"
#ifdef ENABLE_ASYNC_DNS
# include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

class KqueueEventPoll : public EventPoll {
private:
  class KSocketEntry;

  typedef Event<KSocketEntry> KEvent;

  typedef CommandEvent<KSocketEntry, KqueueEventPoll> KCommandEvent;
  typedef ADNSEvent<KSocketEntry, KqueueEventPoll> KADNSEvent;
  typedef AsyncNameResolverEntry<KqueueEventPoll> KAsyncNameResolverEntry;
  friend class AsyncNameResolverEntry<KqueueEventPoll>;

  class KSocketEntry:
    public SocketEntry<KCommandEvent, KADNSEvent> {
  public:
    KSocketEntry(sock_t socket);

    // eventlist should be at least size 2.  This function returns the
    // number of filled struct kevent in eventlist.
    size_t getEvents(struct kevent* eventlist);
  };

  friend int accumulateEvent(int events, const KEvent& event);

private:
  typedef std::set<SharedHandle<KSocketEntry>,
                   DerefLess<SharedHandle<KSocketEntry> > > KSocketEntrySet;
  KSocketEntrySet socketEntries_;
#ifdef ENABLE_ASYNC_DNS
  typedef std::set<SharedHandle<KAsyncNameResolverEntry>,
                   DerefLess<SharedHandle<KAsyncNameResolverEntry> > >
  KAsyncNameResolverEntrySet;
  KAsyncNameResolverEntrySet nameResolverEntries_;
#endif // ENABLE_ASYNC_DNS

  int kqfd_;

  size_t kqEventsSize_;

  struct kevent* kqEvents_;

  static const size_t KQUEUE_EVENTS_MAX = 1024;

  bool addEvents(sock_t socket, const KEvent& event);

  bool deleteEvents(sock_t socket, const KEvent& event);

  bool addEvents(sock_t socket, Command* command, int events,
                 const SharedHandle<AsyncNameResolver>& rs);

  bool deleteEvents(sock_t socket, Command* command,
                    const SharedHandle<AsyncNameResolver>& rs);

public:
  KqueueEventPoll();

  bool good() const;

  virtual ~KqueueEventPoll();

  virtual void poll(const struct timeval& tv);

  virtual bool addEvents(sock_t socket,
                         Command* command, EventPoll::EventType events);

  virtual bool deleteEvents(sock_t socket,
                            Command* command, EventPoll::EventType events);
#ifdef ENABLE_ASYNC_DNS

  virtual bool addNameResolver(const SharedHandle<AsyncNameResolver>& resolver,
                               Command* command);
  virtual bool deleteNameResolver
  (const SharedHandle<AsyncNameResolver>& resolver, Command* command);
#endif // ENABLE_ASYNC_DNS

  static const int IEV_READ = POLLIN;
  static const int IEV_WRITE = POLLOUT;
  static const int IEV_ERROR = POLLERR;
  static const int IEV_HUP = POLLHUP;
};

} // namespace aria2

#endif // D_KQUEUE_EVENT_POLL_H
