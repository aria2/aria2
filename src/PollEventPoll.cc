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
#include "PollEventPoll.h"

#include <cstring>
#include <algorithm>
#include <numeric>

#include "Command.h"
#include "LogFactory.h"
#include "Logger.h"
#include "a2functional.h"
#include "fmt.h"
#include "util.h"

namespace aria2 {

PollEventPoll::KSocketEntry::KSocketEntry(sock_t s)
    : SocketEntry<KCommandEvent, KADNSEvent>(s)
{
}

int accumulateEvent(int events, const PollEventPoll::KEvent& event)
{
  return events | event.getEvents();
}

struct pollfd PollEventPoll::KSocketEntry::getEvents()
{
  struct pollfd pollEvent;
  pollEvent.fd = socket_;
#ifdef ENABLE_ASYNC_DNS
  pollEvent.events =
      std::accumulate(adnsEvents_.begin(), adnsEvents_.end(),
                      std::accumulate(commandEvents_.begin(),
                                      commandEvents_.end(), 0, accumulateEvent),
                      accumulateEvent);
#else  // !ENABLE_ASYNC_DNS
  pollEvent.events = std::accumulate(commandEvents_.begin(),
                                     commandEvents_.end(), 0, accumulateEvent);
#endif // !ENABLE_ASYNC_DNS
  pollEvent.revents = 0;
  return pollEvent;
}

PollEventPoll::PollEventPoll()
    : pollfdCapacity_(1024),
      pollfdNum_(0),
      pollfds_(make_unique<struct pollfd[]>(pollfdCapacity_))
{
}

PollEventPoll::~PollEventPoll() = default;

void PollEventPoll::poll(const struct timeval& tv)
{
  // timeout is millisec
  int timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  int res;
  while ((res = ::poll(pollfds_.get(), pollfdNum_, timeout)) == -1 &&
         errno == EINTR)
    ;
  if (res > 0) {
    for (auto first = pollfds_.get(), last = pollfds_.get() + pollfdNum_;
         first != last; ++first) {
      if (first->revents) {
        auto itr = socketEntries_.find(first->fd);
        if (itr == std::end(socketEntries_)) {
          A2_LOG_DEBUG(
              fmt("Socket %d is not found in SocketEntries.", first->fd));
        }
        else {
          (*itr).second.processEvents(first->revents);
        }
      }
    }
  }
  else if (res == -1) {
    int errNum = errno;
    A2_LOG_INFO(fmt("poll error: %s", util::safeStrerror(errNum).c_str()));
  }
#ifdef ENABLE_ASYNC_DNS
  // It turns out that we have to call ares_process_fd before ares's
  // own timeout and ares may create new sockets or closes socket in
  // their API. So we call ares_process_fd for all ares_channel and
  // re-register their sockets.
  for (auto& r : nameResolverEntries_) {
    auto& ent = r.second;
    ent.processTimeout();
    ent.removeSocketEvents(this);
    ent.addSocketEvents(this);
  }
#endif // ENABLE_ASYNC_DNS

  // TODO timeout of name resolver is determined in Command(AbstractCommand,
  // DHTEntryPoint...Command)
}

int PollEventPoll::translateEvents(EventPoll::EventType events)
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

bool PollEventPoll::addEvents(sock_t socket, const PollEventPoll::KEvent& event)
{
  auto i = socketEntries_.lower_bound(socket);
  if (i != std::end(socketEntries_) && (*i).first == socket) {
    auto& socketEntry = (*i).second;
    event.addSelf(&socketEntry);
    for (auto first = pollfds_.get(), last = pollfds_.get() + pollfdNum_;
         first != last; ++first) {
      if (first->fd == socket) {
        *first = socketEntry.getEvents();
        break;
      }
    }
  }
  else {
    i = socketEntries_.insert(i, std::make_pair(socket, KSocketEntry(socket)));
    auto& socketEntry = (*i).second;
    event.addSelf(&socketEntry);
    if (pollfdCapacity_ == pollfdNum_) {
      pollfdCapacity_ *= 2;
      auto newPollfds = make_unique<struct pollfd[]>(pollfdCapacity_);
      memcpy(newPollfds.get(), pollfds_.get(),
             pollfdNum_ * sizeof(struct pollfd));
      pollfds_ = std::move(newPollfds);
    }
    pollfds_[pollfdNum_] = socketEntry.getEvents();
    ++pollfdNum_;
  }
  return true;
}

bool PollEventPoll::addEvents(sock_t socket, Command* command,
                              EventPoll::EventType events)
{
  int pollEvents = translateEvents(events);
  return addEvents(socket, KCommandEvent(command, pollEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::addEvents(sock_t socket, Command* command, int events,
                              const std::shared_ptr<AsyncNameResolver>& rs)
{
  return addEvents(socket, KADNSEvent(rs, command, socket, events));
}
#endif // ENABLE_ASYNC_DNS

bool PollEventPoll::deleteEvents(sock_t socket,
                                 const PollEventPoll::KEvent& event)
{
  auto i = socketEntries_.find(socket);
  if (i == std::end(socketEntries_)) {
    A2_LOG_DEBUG(fmt("Socket %d is not found in SocketEntries.", socket));
    return false;
  }

  auto& socketEntry = (*i).second;
  event.removeSelf(&socketEntry);
  for (auto first = pollfds_.get(), last = pollfds_.get() + pollfdNum_;
       first != last; ++first) {
    if (first->fd == socket) {
      if (socketEntry.eventEmpty()) {
        if (pollfdNum_ >= 2) {
          *first = *(last - 1);
        }
        --pollfdNum_;
        socketEntries_.erase(i);
      }
      else {
        *first = socketEntry.getEvents();
      }
      break;
    }
  }
  return true;
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::deleteEvents(sock_t socket, Command* command,
                                 const std::shared_ptr<AsyncNameResolver>& rs)
{
  return deleteEvents(socket, KADNSEvent(rs, command, socket, 0));
}
#endif // ENABLE_ASYNC_DNS

bool PollEventPoll::deleteEvents(sock_t socket, Command* command,
                                 EventPoll::EventType events)
{
  int pollEvents = translateEvents(events);
  return deleteEvents(socket, KCommandEvent(command, pollEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::addNameResolver(
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

bool PollEventPoll::deleteNameResolver(
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
