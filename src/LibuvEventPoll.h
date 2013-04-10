/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Nils Maier
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
#ifndef D_LIBUV_EVENT_POLL_H
#define D_LIBUV_EVENT_POLL_H

#include "EventPoll.h"

#include <map>
#include <set>

#include <uv.h>

#include "Event.h"
#include "a2functional.h"

#ifdef ENABLE_ASYNC_DNS
#include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

class LibuvEventPoll : public EventPoll {
private:
  class KSocketEntry;

  typedef Event<KSocketEntry> KEvent;
  typedef CommandEvent<KSocketEntry, LibuvEventPoll> KCommandEvent;
  typedef ADNSEvent<KSocketEntry, LibuvEventPoll> KADNSEvent;
  typedef AsyncNameResolverEntry<LibuvEventPoll> KAsyncNameResolverEntry;
  friend class AsyncNameResolverEntry<LibuvEventPoll>;

  class KSocketEntry:
    public SocketEntry<KCommandEvent, KADNSEvent> {
  public:
    KSocketEntry(sock_t socket);
    int getEvents();
  };

  friend int accumulateEvent(int events, const KEvent& event);

private:
  uv_loop_t* loop_;

  typedef std::set<SharedHandle<KSocketEntry>,
                   DerefLess<SharedHandle<KSocketEntry> > > KSocketEntrySet;
  KSocketEntrySet socketEntries_;

  typedef struct {
    uv_poll_t p;
    KSocketEntry *entry;
    LibuvEventPoll *eventer;
    int events;
  } poll_t;

  typedef std::map<sock_t, poll_t*> KPolls;
  KPolls polls_;

#ifdef ENABLE_ASYNC_DNS
  typedef std::set<SharedHandle<KAsyncNameResolverEntry>,
                   DerefLess<SharedHandle<KAsyncNameResolverEntry> > >
  KAsyncNameResolverEntrySet;
  KAsyncNameResolverEntrySet nameResolverEntries_;
#endif // ENABLE_ASYNC_DNS

  bool addEvents(sock_t socket, const KEvent& event);
  bool deleteEvents(sock_t socket, const KEvent& event);

#ifdef ENABLE_ASYNC_DNS
  bool addEvents(sock_t socket, Command* command, int events,
                 const SharedHandle<AsyncNameResolver>& rs);
  bool deleteEvents(sock_t socket, Command* command,
                    const SharedHandle<AsyncNameResolver>& rs);
#endif

  static int translateEvents(EventPoll::EventType events);
  static void close_poll_callback(uv_handle_t* handle) {
    delete static_cast<poll_t*>(handle->data);
  }
  static void poll_callback(uv_poll_t* handle, int status, int events) {
    poll_t* poll = static_cast<poll_t*>(handle->data);
    poll->eventer->pollCallback(handle, poll, status, events);
  }
  void pollCallback(uv_poll_t* handle, poll_t *poll, int status, int events);

public:
  LibuvEventPoll();

  virtual ~LibuvEventPoll();

  bool good() const { return loop_; }

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

  static const int IEV_READ = UV_READABLE;
  static const int IEV_WRITE = UV_WRITABLE;
  static const int IEV_ERROR = 128;
  static const int IEV_HUP = 255;
};

} // namespace aria2

#endif // D_LIBUV_EVENT_POLL_H
