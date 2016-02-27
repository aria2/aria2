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
#include "EpollEventPoll.h"

#include <cerrno>
#include <cstring>
#include <algorithm>
#include <numeric>

#include "Command.h"
#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "a2functional.h"
#include "fmt.h"

namespace aria2 {

EpollEventPoll::KSocketEntry::KSocketEntry(sock_t s)
    : SocketEntry<KCommandEvent, KADNSEvent>(s)
{
}

int accumulateEvent(int events, const EpollEventPoll::KEvent& event)
{
  return events | event.getEvents();
}

struct epoll_event EpollEventPoll::KSocketEntry::getEvents()
{
  struct epoll_event epEvent;
  memset(&epEvent, 0, sizeof(struct epoll_event));
  epEvent.data.ptr = this;

#ifdef ENABLE_ASYNC_DNS

  epEvent.events =
      std::accumulate(adnsEvents_.begin(), adnsEvents_.end(),
                      std::accumulate(commandEvents_.begin(),
                                      commandEvents_.end(), 0, accumulateEvent),
                      accumulateEvent);

#else // !ENABLE_ASYNC_DNS

  epEvent.events = std::accumulate(commandEvents_.begin(), commandEvents_.end(),
                                   0, accumulateEvent);

#endif // !ENABLE_ASYNC_DNS
  return epEvent;
}

EpollEventPoll::EpollEventPoll()
    : epEventsSize_(EPOLL_EVENTS_MAX),
      epEvents_(make_unique<struct epoll_event[]>(epEventsSize_))
{
  epfd_ = epoll_create(EPOLL_EVENTS_MAX);
}

EpollEventPoll::~EpollEventPoll()
{
  if (epfd_ != -1) {
    int r = close(epfd_);
    int errNum = errno;
    if (r == -1) {
      A2_LOG_ERROR(fmt("Error occurred while closing epoll file descriptor"
                       " %d: %s",
                       epfd_, util::safeStrerror(errNum).c_str()));
    }
  }
}

bool EpollEventPoll::good() const { return epfd_ != -1; }

void EpollEventPoll::poll(const struct timeval& tv)
{
  // timeout is millisec
  int timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  int res;
  while ((res = epoll_wait(epfd_, epEvents_.get(), EPOLL_EVENTS_MAX,
                           timeout)) == -1 &&
         errno == EINTR)
    ;

  if (res > 0) {
    for (int i = 0; i < res; ++i) {
      KSocketEntry* p = reinterpret_cast<KSocketEntry*>(epEvents_[i].data.ptr);
      p->processEvents(epEvents_[i].events);
    }
  }
  else if (res == -1) {
    int errNum = errno;
    A2_LOG_INFO(
        fmt("epoll_wait error: %s", util::safeStrerror(errNum).c_str()));
  }
#ifdef ENABLE_ASYNC_DNS
  // It turns out that we have to call ares_process_fd before ares's
  // own timeout and ares may create new sockets or closes socket in
  // their API. So we call ares_process_fd for all ares_channel and
  // re-register their sockets.
  for (auto& i : nameResolverEntries_) {
    auto& ent = i.second;
    ent.processTimeout();
    ent.removeSocketEvents(this);
    ent.addSocketEvents(this);
  }
#endif // ENABLE_ASYNC_DNS

  // TODO timeout of name resolver is determined in Command(AbstractCommand,
  // DHTEntryPoint...Command)
}

namespace {
int translateEvents(EventPoll::EventType events)
{
  int newEvents = 0;
  if (EventPoll::EVENT_READ & events) {
    newEvents |= EPOLLIN;
  }
  if (EventPoll::EVENT_WRITE & events) {
    newEvents |= EPOLLOUT;
  }
  if (EventPoll::EVENT_ERROR & events) {
    newEvents |= EPOLLERR;
  }
  if (EventPoll::EVENT_HUP & events) {
    newEvents |= EPOLLHUP;
  }
  return newEvents;
}
} // namespace

bool EpollEventPoll::addEvents(sock_t socket,
                               const EpollEventPoll::KEvent& event)
{
  auto i = socketEntries_.lower_bound(socket);
  int r = 0;
  int errNum = 0;
  if (i != std::end(socketEntries_) && (*i).first == socket) {
    auto& socketEntry = (*i).second;

    event.addSelf(&socketEntry);

    struct epoll_event epEvent = socketEntry.getEvents();
    r = epoll_ctl(epfd_, EPOLL_CTL_MOD, socketEntry.getSocket(), &epEvent);
    if (r == -1) {
      // try EPOLL_CTL_ADD: There is a chance that previously socket X is
      // added to epoll, but it is closed and is not yet removed from
      // SocketEntries. In this case, EPOLL_CTL_MOD is failed with ENOENT.

      r = epoll_ctl(epfd_, EPOLL_CTL_ADD, socketEntry.getSocket(), &epEvent);
      errNum = errno;
    }
  }
  else {
    i = socketEntries_.insert(i, std::make_pair(socket, KSocketEntry(socket)));
    auto& socketEntry = (*i).second;
    if (socketEntries_.size() > epEventsSize_) {
      epEventsSize_ *= 2;
      epEvents_ = make_unique<struct epoll_event[]>(epEventsSize_);
    }

    event.addSelf(&socketEntry);

    struct epoll_event epEvent = socketEntry.getEvents();
    r = epoll_ctl(epfd_, EPOLL_CTL_ADD, socketEntry.getSocket(), &epEvent);
    errNum = errno;
  }
  if (r == -1) {
    A2_LOG_DEBUG(fmt("Failed to add socket event %d:%s", socket,
                     util::safeStrerror(errNum).c_str()));
    return false;
  }
  else {
    return true;
  }
}

bool EpollEventPoll::addEvents(sock_t socket, Command* command,
                               EventPoll::EventType events)
{
  int epEvents = translateEvents(events);
  return addEvents(socket, KCommandEvent(command, epEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool EpollEventPoll::addEvents(sock_t socket, Command* command, int events,
                               const std::shared_ptr<AsyncNameResolver>& rs)
{
  return addEvents(socket, KADNSEvent(rs, command, socket, events));
}
#endif // ENABLE_ASYNC_DNS

bool EpollEventPoll::deleteEvents(sock_t socket,
                                  const EpollEventPoll::KEvent& event)
{
  auto i = socketEntries_.find(socket);
  if (i == std::end(socketEntries_)) {
    A2_LOG_DEBUG(fmt("Socket %d is not found in SocketEntries.", socket));
    return false;
  }

  auto& socketEntry = (*i).second;
  event.removeSelf(&socketEntry);
  int r = 0;
  int errNum = 0;
  if (socketEntry.eventEmpty()) {
    // In kernel before 2.6.9, epoll_ctl with EPOLL_CTL_DEL requires non-null
    // pointer of epoll_event.
    struct epoll_event ev = {0, {0}};
    r = epoll_ctl(epfd_, EPOLL_CTL_DEL, socketEntry.getSocket(), &ev);
    errNum = errno;
    socketEntries_.erase(i);
  }
  else {
    // If socket is closed, then it seems it is automatically removed from
    // epoll, so following EPOLL_CTL_MOD may fail.
    struct epoll_event epEvent = socketEntry.getEvents();
    r = epoll_ctl(epfd_, EPOLL_CTL_MOD, socketEntry.getSocket(), &epEvent);
    errNum = errno;
    if (r == -1) {
      A2_LOG_DEBUG(fmt("Failed to delete socket event, but may be ignored:%s",
                       util::safeStrerror(errNum).c_str()));
    }
  }
  if (r == -1) {
    A2_LOG_DEBUG(fmt("Failed to delete socket event:%s",
                     util::safeStrerror(errNum).c_str()));
    return false;
  }
  else {
    return true;
  }
}

#ifdef ENABLE_ASYNC_DNS
bool EpollEventPoll::deleteEvents(sock_t socket, Command* command,
                                  const std::shared_ptr<AsyncNameResolver>& rs)
{
  return deleteEvents(socket, KADNSEvent(rs, command, socket, 0));
}
#endif // ENABLE_ASYNC_DNS

bool EpollEventPoll::deleteEvents(sock_t socket, Command* command,
                                  EventPoll::EventType events)
{
  int epEvents = translateEvents(events);
  return deleteEvents(socket, KCommandEvent(command, epEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool EpollEventPoll::addNameResolver(
    const std::shared_ptr<AsyncNameResolver>& resolver, Command* command)
{
  auto key = std::make_pair(resolver.get(), command);
  auto itr = nameResolverEntries_.lower_bound(key);

  if (itr != std::end(nameResolverEntries_) && (*itr).first == key) {
    return false;
  }

  itr = nameResolverEntries_.insert(
      itr, std::make_pair(key, KAsyncNameResolverEntry(resolver, command)));
  (*itr).second.addSocketEvents(this);
  return true;
}

bool EpollEventPoll::deleteNameResolver(
    const std::shared_ptr<AsyncNameResolver>& resolver, Command* command)
{
  auto key = std::make_pair(resolver.get(), command);
  auto itr = nameResolverEntries_.find(key);
  if (itr == std::end(nameResolverEntries_)) {
    return false;
  }

  (*itr).second.removeSocketEvents(this);
  nameResolverEntries_.erase(itr);
  return true;
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
