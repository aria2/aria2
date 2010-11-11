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

namespace aria2 {

PollEventPoll::KSocketEntry::KSocketEntry(sock_t s):
  SocketEntry<KCommandEvent, KADNSEvent>(s) {}

int accumulateEvent(int events, const PollEventPoll::KEvent& event)
{
  return events|event.getEvents();
}

struct pollfd PollEventPoll::KSocketEntry::getEvents()
{
  struct pollfd pollEvent;
  pollEvent.fd = socket_;
#ifdef ENABLE_ASYNC_DNS
  pollEvent.events =
    std::accumulate(adnsEvents_.begin(),
                    adnsEvents_.end(),
                    std::accumulate(commandEvents_.begin(),
                                    commandEvents_.end(), 0, accumulateEvent),
                    accumulateEvent);
#else // !ENABLE_ASYNC_DNS
  pollEvent.events =
    std::accumulate(commandEvents_.begin(), commandEvents_.end(), 0,
                    accumulateEvent);
#endif // !ENABLE_ASYNC_DNS
  pollEvent.revents = 0;
  return pollEvent;
}

PollEventPoll::PollEventPoll():
  pollfdCapacity_(1024), pollfdNum_(0), logger_(LogFactory::getInstance())
{
  pollfds_ = new struct pollfd[pollfdCapacity_];
}

PollEventPoll::~PollEventPoll()
{
  delete [] pollfds_;
}

void PollEventPoll::poll(const struct timeval& tv)
{
  // timeout is millisec
  int timeout = tv.tv_sec*1000+tv.tv_usec/1000;
  int res;
  while((res = ::poll(pollfds_, pollfdNum_, timeout)) == -1 &&
        errno == EINTR);
  if(res > 0) {
    SharedHandle<KSocketEntry> se(new KSocketEntry(0));
    for(struct pollfd* first = pollfds_, *last = pollfds_+pollfdNum_;
        first != last; ++first) {
      if(first->revents) {
        se->setSocket(first->fd);
        std::deque<SharedHandle<KSocketEntry> >::iterator itr =
          std::lower_bound(socketEntries_.begin(), socketEntries_.end(), se,
                           DerefLess<SharedHandle<KSocketEntry> >());
        if(itr != socketEntries_.end() && *(*itr) == *se) {
          (*itr)->processEvents(first->revents);
        } else {
          if(logger_->debug()) {
            logger_->debug("Socket %d is not found in SocketEntries.",
                           first->fd);
          }
        }
      }
    }
  }
#ifdef ENABLE_ASYNC_DNS
  // It turns out that we have to call ares_process_fd before ares's
  // own timeout and ares may create new sockets or closes socket in
  // their API. So we call ares_process_fd for all ares_channel and
  // re-register their sockets.
  for(std::deque<SharedHandle<KAsyncNameResolverEntry> >::iterator i =
        nameResolverEntries_.begin(), eoi = nameResolverEntries_.end();
      i != eoi; ++i) {
    (*i)->processTimeout();
    (*i)->removeSocketEvents(this);
    (*i)->addSocketEvents(this);
  }
#endif // ENABLE_ASYNC_DNS

  // TODO timeout of name resolver is determined in Command(AbstractCommand,
  // DHTEntryPoint...Command)
}

int PollEventPoll::translateEvents(EventPoll::EventType events)
{
  int newEvents = 0;
  if(EventPoll::EVENT_READ&events) {
    newEvents |= IEV_READ;
  }
  if(EventPoll::EVENT_WRITE&events) {
    newEvents |= IEV_WRITE;
  }
  if(EventPoll::EVENT_ERROR&events) {
    newEvents |= IEV_ERROR;
  }
  if(EventPoll::EVENT_HUP&events) {
    newEvents |= IEV_HUP;
  }
  return newEvents;
}

bool PollEventPoll::addEvents
(sock_t socket, const PollEventPoll::KEvent& event)
{
  SharedHandle<KSocketEntry> socketEntry(new KSocketEntry(socket));
  std::deque<SharedHandle<KSocketEntry> >::iterator i =
    std::lower_bound(socketEntries_.begin(), socketEntries_.end(), socketEntry,
                     DerefLess<SharedHandle<KSocketEntry> >());
  if(i != socketEntries_.end() && *(*i) == *socketEntry) {
    event.addSelf(*i);
    for(struct pollfd* first = pollfds_, *last = pollfds_+pollfdNum_;
        first != last; ++first) {
      if(first->fd == socket) {
        *first = (*i)->getEvents();
        break;
      }
    }
  } else {    
    socketEntries_.insert(i, socketEntry);
    event.addSelf(socketEntry);
    if(pollfdCapacity_ == pollfdNum_) {
      pollfdCapacity_ *= 2;
      struct pollfd* newPollfds = new struct pollfd[pollfdCapacity_];
      memcpy(newPollfds, pollfds_, pollfdNum_*sizeof(struct pollfd));
      delete [] pollfds_;
      pollfds_ = newPollfds;
    }
    pollfds_[pollfdNum_] = socketEntry->getEvents();
    ++pollfdNum_;
  }
  return true;
}

bool PollEventPoll::addEvents
(sock_t socket, Command* command, EventPoll::EventType events)
{
  int pollEvents = translateEvents(events);
  return addEvents(socket, KCommandEvent(command, pollEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::addEvents
(sock_t socket, Command* command, int events,
 const SharedHandle<AsyncNameResolver>& rs)
{
  return addEvents(socket, KADNSEvent(rs, command, socket, events));
}
#endif // ENABLE_ASYNC_DNS

bool PollEventPoll::deleteEvents
(sock_t socket, const PollEventPoll::KEvent& event)
{
  SharedHandle<KSocketEntry> socketEntry(new KSocketEntry(socket));
  std::deque<SharedHandle<KSocketEntry> >::iterator i =
    std::lower_bound(socketEntries_.begin(), socketEntries_.end(), socketEntry,
                     DerefLess<SharedHandle<KSocketEntry> >());
  if(i != socketEntries_.end() && *(*i) == *socketEntry) {
    event.removeSelf(*i);
    for(struct pollfd* first = pollfds_, *last = pollfds_+pollfdNum_;
        first != last; ++first) {
      if(first->fd == socket) {
        if((*i)->eventEmpty()) {
          if(pollfdNum_ >= 2) {
            *first = *(last-1);
          }
          --pollfdNum_;
          socketEntries_.erase(i);
        } else {
          *first = (*i)->getEvents();
        }
        break;
      }
    }
    return true;
  } else {
    if(logger_->debug()) {
      logger_->debug("Socket %d is not found in SocketEntries.", socket);
    }
    return false;
  }
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::deleteEvents
(sock_t socket, Command* command, const SharedHandle<AsyncNameResolver>& rs)
{
  return deleteEvents(socket, KADNSEvent(rs, command, socket, 0));
}
#endif // ENABLE_ASYNC_DNS

bool PollEventPoll::deleteEvents
(sock_t socket, Command* command, EventPoll::EventType events)
{
  int pollEvents = translateEvents(events);
  return deleteEvents(socket, KCommandEvent(command, pollEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::addNameResolver
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<KAsyncNameResolverEntry> entry
    (new KAsyncNameResolverEntry(resolver, command));
  std::deque<SharedHandle<KAsyncNameResolverEntry> >::iterator itr =
    std::find_if(nameResolverEntries_.begin(), nameResolverEntries_.end(),
                 derefEqual(entry));
  if(itr == nameResolverEntries_.end()) {
    nameResolverEntries_.push_back(entry);
    entry->addSocketEvents(this);
    return true;
  } else {
    return false;
  }
}

bool PollEventPoll::deleteNameResolver
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<KAsyncNameResolverEntry> entry
    (new KAsyncNameResolverEntry(resolver, command));
  std::deque<SharedHandle<KAsyncNameResolverEntry> >::iterator itr =
    std::find_if(nameResolverEntries_.begin(), nameResolverEntries_.end(),
                 derefEqual(entry));
  if(itr == nameResolverEntries_.end()) {
    return false;
  } else {
    (*itr)->removeSocketEvents(this);
    nameResolverEntries_.erase(itr);
    return true;
  }
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
