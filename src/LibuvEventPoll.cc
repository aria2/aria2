/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
 * Copyright (C) 2013 Tatsuhiro Tsujikawa, Nils Maier
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

#ifdef __MINGW32__
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif // _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif // __MINGW32__

#include "LibuvEventPoll.h"

#include <algorithm>
#include <cstring>
#include <numeric>
#include <stdexcept>

#include <uv.h>

#include "Command.h"
#include "LogFactory.h"
#include "Logger.h"
#include "a2functional.h"
#include "fmt.h"
#include "util.h"

namespace {
  template<typename T>
  static void close_callback(uv_handle_t* handle)
  {
    delete reinterpret_cast<T*>(handle);
  }

  static void timer_callback(uv_timer_t* handle, int status) {}
}

namespace aria2 {

LibuvEventPoll::KSocketEntry::KSocketEntry(sock_t s)
  : SocketEntry<KCommandEvent, KADNSEvent>(s)
{}

int accumulateEvent(int events, const LibuvEventPoll::KEvent& event)
{
  return events|event.getEvents();
}

int LibuvEventPoll::KSocketEntry::getEvents()
{
  int events = 0;
#ifdef ENABLE_ASYNC_DNS
  events = std::accumulate(adnsEvents_.begin(), adnsEvents_.end(),
                           std::accumulate(commandEvents_.begin(),
                                           commandEvents_.end(), 0,
                                           accumulateEvent),
                           accumulateEvent);
#else // !ENABLE_ASYNC_DNS
  events = std::accumulate(commandEvents_.begin(), commandEvents_.end(), 0,
                           accumulateEvent);
#endif // !ENABLE_ASYNC_DNS
  return events;
}

LibuvEventPoll::LibuvEventPoll()
{
  loop = uv_loop_new();
}

LibuvEventPoll::~LibuvEventPoll()
{
  for (KPolls::iterator i = polls_.begin(), e = polls_.end(); i != e; ++i) {
    uv_poll_stop(i->second);
    uv_close((uv_handle_t*)i->second, close_callback<uv_poll_t>);
  }
  // Actually kill the polls, and timers, if any
  uv_run(loop, (uv_run_mode)(UV_RUN_ONCE | UV_RUN_NOWAIT));

  if (loop) {
    uv_loop_delete(loop);
    loop = 0;
  }
  polls_.clear();
}

void LibuvEventPoll::poll(const struct timeval& tv)
{
  int timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  if (timeout > 0) {
    uv_timer_t* timer = new uv_timer_t;
    uv_timer_init(loop, timer);
    uv_timer_start(timer, timer_callback, timeout, timeout);

    uv_run(loop, UV_RUN_ONCE);

    // Remove timer again.
    uv_timer_stop(timer);
    uv_close((uv_handle_t*)timer, close_callback<uv_timer_t>);
  }
  else {
    uv_run(loop, (uv_run_mode)(UV_RUN_ONCE | UV_RUN_NOWAIT));
  }

#ifdef ENABLE_ASYNC_DNS
  // It turns out that we have to call ares_process_fd before ares's
  // own timeout and ares may create new sockets or closes socket in
  // their API. So we call ares_process_fd for all ares_channel and
  // re-register their sockets.
  for(KAsyncNameResolverEntrySet::iterator i = nameResolverEntries_.begin(),
        eoi = nameResolverEntries_.end(); i != eoi; ++i) {
    (*i)->processTimeout();
    (*i)->removeSocketEvents(this);
    (*i)->addSocketEvents(this);
  }
#endif // ENABLE_ASYNC_DNS

  // TODO timeout of name resolver is determined in Command(AbstractCommand,
  // DHTEntryPoint...Command)
}

int LibuvEventPoll::translateEvents(EventPoll::EventType events)
{
  int newEvents = 0;
  if (EventPoll::EVENT_READ & events) {
    newEvents |= IEV_READ;
  }
  if (EventPoll::EVENT_WRITE & events) {
    newEvents |= IEV_WRITE;
  }
  if (EventPoll::EVENT_ERROR & events) {
    newEvents |= IEV_ERROR;
  }
  if (EventPoll::EVENT_HUP & events) {
    newEvents |= IEV_HUP;
  }
  return newEvents;
}

void LibuvEventPoll::poll_callback(uv_poll_t* handle, int status, int events)
{
  if (status == -1) {
    events = 0;
    uv_err_t err = uv_last_error(handle->loop);
    switch (err.code) {
      case UV_EAGAIN:
        return;
      case UV_EOF:
      case UV_ECONNABORTED:
      case UV_ECONNREFUSED:
      case UV_ECONNRESET:
      case UV_ENOTCONN:
      case UV_EPIPE:
      case UV_ESHUTDOWN:
        events = IEV_HUP;
      default:
        events = IEV_ERROR;
    }
  }
  KSocketEntry *p = reinterpret_cast<KSocketEntry*>(handle->data);
  p->processEvents(events);
}

bool LibuvEventPoll::addEvents(sock_t socket,
                               const LibuvEventPoll::KEvent& event)
{
  SharedHandle<KSocketEntry> socketEntry(new KSocketEntry(socket));
  KSocketEntrySet::iterator i = socketEntries_.lower_bound(socketEntry);
  if (i != socketEntries_.end() && **i == *socketEntry) {
    event.addSelf(*i);
    KPolls::iterator poll = polls_.find(socket);
    if (poll == polls_.end()) {
      throw std::logic_error("Invalid socket");
    }
    uv_poll_start(poll->second, (*i)->getEvents() & (IEV_READ | IEV_WRITE),
                  poll_callback);
  }
  else {
    socketEntries_.insert(i, socketEntry);
    event.addSelf(socketEntry);
    uv_poll_t *poll = new uv_poll_t;
    uv_poll_init_socket(loop, poll, socket);
    poll->data = socketEntry.get();
    uv_poll_start(poll, socketEntry->getEvents() & (IEV_READ | IEV_WRITE),
                  poll_callback);
    polls_[socket] = poll;
  }
  return true;
}

bool LibuvEventPoll::addEvents(sock_t socket, Command* command, EventPoll::EventType events)
{
  int pollEvents = translateEvents(events);
  return addEvents(socket, KCommandEvent(command, pollEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool LibuvEventPoll::addEvents(sock_t socket, Command* command, int events,
                               const SharedHandle<AsyncNameResolver>& rs)
{
  return addEvents(socket, KADNSEvent(rs, command, socket, events));
}
#endif // ENABLE_ASYNC_DNS

bool LibuvEventPoll::deleteEvents(sock_t socket,
                                  const LibuvEventPoll::KEvent& event)
{
  SharedHandle<KSocketEntry> socketEntry(new KSocketEntry(socket));
  KSocketEntrySet::iterator i = socketEntries_.find(socketEntry);
  if(i == socketEntries_.end()) {
    A2_LOG_DEBUG(fmt("Socket %d is not found in SocketEntries.", socket));
    return false;
  }

  event.removeSelf(*i);

  if ((*i)->eventEmpty()) {
    KPolls::iterator poll = polls_.find(socket);
    if (poll == polls_.end()) {
      return false;
    }
    uv_poll_stop(poll->second);
    uv_close((uv_handle_t*)poll->second, close_callback<uv_poll_t>);
    polls_.erase(poll);
    socketEntries_.erase(i);
  }
  return true;
}

#ifdef ENABLE_ASYNC_DNS
bool LibuvEventPoll::deleteEvents(sock_t socket, Command* command,
                                  const SharedHandle<AsyncNameResolver>& rs)
{
  return deleteEvents(socket, KADNSEvent(rs, command, socket, 0));
}
#endif // ENABLE_ASYNC_DNS

bool LibuvEventPoll::deleteEvents(sock_t socket, Command* command,
                                  EventPoll::EventType events)
{
  int pollEvents = translateEvents(events);
  return deleteEvents(socket, KCommandEvent(command, pollEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool LibuvEventPoll::addNameResolver(const SharedHandle<AsyncNameResolver>& resolver,
                                     Command* command)
{
  SharedHandle<KAsyncNameResolverEntry> entry(
      new KAsyncNameResolverEntry(resolver, command));
  KAsyncNameResolverEntrySet::iterator itr =
    nameResolverEntries_.lower_bound(entry);
  if (itr != nameResolverEntries_.end() && *(*itr) == *entry) {
    return false;
  }
  nameResolverEntries_.insert(itr, entry);
  entry->addSocketEvents(this);
  return true;
}

bool LibuvEventPoll::deleteNameResolver(const SharedHandle<AsyncNameResolver>& resolver,
                                        Command* command)
{
  SharedHandle<KAsyncNameResolverEntry> entry(
      new KAsyncNameResolverEntry(resolver, command));
  KAsyncNameResolverEntrySet::iterator itr = nameResolverEntries_.find(entry);
  if (itr == nameResolverEntries_.end()) {
    return false;
  }
  (*itr)->removeSocketEvents(this);
  nameResolverEntries_.erase(itr);
  return true;
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
