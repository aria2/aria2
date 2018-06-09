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
#  include "AsyncNameResolver.h"
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
  friend int accumulateEvent(int events, const KEvent& event);

  class KSocketEntry : public SocketEntry<KCommandEvent, KADNSEvent> {
  public:
    KSocketEntry(sock_t socket);

    KSocketEntry(const KSocketEntry&) = delete;
    KSocketEntry(KSocketEntry&&) = default;

    int getEvents() const;
  };

  class KPoll {
  private:
    LibuvEventPoll* eventer_;
    KSocketEntry* entry_;
    uv_poll_t handle_;

    static void poll_callback(uv_poll_t* handle, int status, int events)
    {
      auto poll = static_cast<KPoll*>(handle->data);
      poll->eventer_->pollCallback(poll, status, events);
    }
    static void close_callback(uv_handle_t* handle)
    {
      delete static_cast<KPoll*>(handle->data);
    }

  public:
    inline KPoll(LibuvEventPoll* eventer, KSocketEntry* entry, sock_t sock)
        : eventer_(eventer), entry_(entry)
    {
      uv_poll_init_socket(eventer->loop_, &handle_, sock);
      handle_.data = this;
    }
    inline void start()
    {
      uv_poll_start(&handle_, entry_->getEvents() & IEV_RW, poll_callback);
    }
    inline void stop() { uv_poll_stop(&handle_); }
    inline void processEvents(int events) { entry_->processEvents(events); }
    inline void close()
    {
      stop();
      uv_close((uv_handle_t*)&handle_, close_callback);
    }
  };

  typedef std::map<sock_t, KSocketEntry> KSocketEntrySet;

  typedef std::map<sock_t, KPoll*> KPolls;

#ifdef ENABLE_ASYNC_DNS
  typedef std::map<std::pair<AsyncNameResolver*, Command*>,
                   KAsyncNameResolverEntry>
      KAsyncNameResolverEntrySet;
#endif // ENABLE_ASYNC_DNS

  uv_loop_t* loop_;
  KSocketEntrySet socketEntries_;
  KPolls polls_;

#ifdef ENABLE_ASYNC_DNS
  KAsyncNameResolverEntrySet nameResolverEntries_;
#endif // ENABLE_ASYNC_DNS

  bool addEvents(sock_t socket, const KEvent& event);
  bool deleteEvents(sock_t socket, const KEvent& event);
  void pollCallback(KPoll* poll, int status, int events);

#ifdef ENABLE_ASYNC_DNS
  bool addEvents(sock_t socket, Command* command, int events,
                 const std::shared_ptr<AsyncNameResolver>& rs);
  bool deleteEvents(sock_t socket, Command* command,
                    const std::shared_ptr<AsyncNameResolver>& rs);
#endif

  static int translateEvents(EventPoll::EventType events);

public:
  LibuvEventPoll();
  virtual ~LibuvEventPoll();

  bool good() const { return loop_; }

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

  static const int IEV_READ = UV_READABLE;
  static const int IEV_WRITE = UV_WRITABLE;
  static const int IEV_RW = UV_READABLE | UV_WRITABLE;

  // Make sure these do not interfere with the uv_poll API later.
  static const int IEV_ERROR = 128;
  static const int IEV_HUP = 255;
};

} // namespace aria2

#endif // D_LIBUV_EVENT_POLL_H
