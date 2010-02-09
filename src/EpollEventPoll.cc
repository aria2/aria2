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

#include <cstring>
#include <algorithm>
#include <numeric>

#include "Command.h"
#include "LogFactory.h"
#include "Logger.h"

namespace aria2 {

EpollEventPoll::CommandEvent::CommandEvent(Command* command, int events):
  _command(command), _events(events) {}

int EpollEventPoll::CommandEvent::getEvents() const
{
  return _events;
}

void EpollEventPoll::CommandEvent::processEvents(int events)
{
  if((_events&events) ||
     ((EpollEventPoll::EVENT_ERROR|EpollEventPoll::EVENT_HUP)&events)) {
    _command->setStatusActive();
  }
  if(EpollEventPoll::EVENT_READ&events) {
    _command->readEventReceived();
  }
  if(EpollEventPoll::EVENT_WRITE&events) {
    _command->writeEventReceived();
  }
  if(EpollEventPoll::EVENT_ERROR&events) {
    _command->errorEventReceived();
  }
  if(EpollEventPoll::EVENT_HUP&events) {
    _command->hupEventReceived();
  }
}

void EpollEventPoll::CommandEvent::addSelf
(const SharedHandle<SocketEntry>& socketEntry) const
{
  socketEntry->addCommandEvent(*this);
}

void EpollEventPoll::CommandEvent::removeSelf
(const SharedHandle<SocketEntry>& socketEntry) const
{
  socketEntry->removeCommandEvent(*this);
}

#ifdef ENABLE_ASYNC_DNS

EpollEventPoll::ADNSEvent::ADNSEvent
(const SharedHandle<AsyncNameResolver>& resolver,
 Command* command,
 sock_t socket, int events):
  _resolver(resolver), _command(command), _socket(socket), _events(events) {}

int EpollEventPoll::ADNSEvent::getEvents() const
{
  return _events;
}

void EpollEventPoll::ADNSEvent::processEvents(int events)
{
  ares_socket_t readfd;
  ares_socket_t writefd;
  if(events&(EpollEventPoll::EVENT_READ|EpollEventPoll::EVENT_ERROR|
             EpollEventPoll::EVENT_HUP)) {
    readfd = _socket;
  } else {
    readfd = ARES_SOCKET_BAD;
  }
  if(events&(EpollEventPoll::EVENT_WRITE|EpollEventPoll::EVENT_ERROR|
             EpollEventPoll::EVENT_HUP)) {
    writefd = _socket;
  } else {
    writefd = ARES_SOCKET_BAD;
  }
  _resolver->process(readfd, writefd);
  _command->setStatusActive();
}

void EpollEventPoll::ADNSEvent::addSelf
(const SharedHandle<SocketEntry>& socketEntry) const
{
  socketEntry->addADNSEvent(*this);
}

void EpollEventPoll::ADNSEvent::removeSelf
(const SharedHandle<SocketEntry>& socketEntry) const
{
  socketEntry->removeADNSEvent(*this);
}

#endif // ENABLE_ASYNC_DNS

EpollEventPoll::SocketEntry::SocketEntry(sock_t socket):_socket(socket)
{
  memset(&_epEvent, 0, sizeof(struct epoll_event));
}

void EpollEventPoll::SocketEntry::addCommandEvent(const CommandEvent& cev)
{
  std::deque<CommandEvent>::iterator i = std::find(_commandEvents.begin(),
                                                   _commandEvents.end(),
                                                   cev);
  if(i == _commandEvents.end()) {
    _commandEvents.push_back(cev);
  } else {
    (*i).addEvents(cev.getEvents());
  }
}

void EpollEventPoll::SocketEntry::removeCommandEvent(const CommandEvent& cev)
{
  std::deque<CommandEvent>::iterator i = std::find(_commandEvents.begin(),
                                                   _commandEvents.end(),
                                                   cev);
  if(i == _commandEvents.end()) {
    // not found
  } else {
    (*i).removeEvents(cev.getEvents());
    if((*i).eventsEmpty()) {
      _commandEvents.erase(i);
    }
  }
}

#ifdef ENABLE_ASYNC_DNS

void EpollEventPoll::SocketEntry::addADNSEvent(const ADNSEvent& aev)
{
  std::deque<ADNSEvent>::iterator i = std::find(_adnsEvents.begin(),
                                                _adnsEvents.end(),
                                                aev);
  if(i == _adnsEvents.end()) {
    _adnsEvents.push_back(aev);
  }
}

void EpollEventPoll::SocketEntry::removeADNSEvent(const ADNSEvent& aev)
{
  std::deque<ADNSEvent>::iterator i = std::find(_adnsEvents.begin(),
                                                _adnsEvents.end(),
                                                aev);
  if(i == _adnsEvents.end()) {
    // not found
  } else {
    _adnsEvents.erase(i);
  }
}

#endif // ENABLE_ASYNC_DNS

void EpollEventPoll::SocketEntry::processEvents(int events)
{
  std::for_each(_commandEvents.begin(), _commandEvents.end(),
                std::bind2nd(std::mem_fun_ref
                             (&EpollEventPoll::CommandEvent::processEvents),
                             events));
#ifdef ENABLE_ASYNC_DNS

  std::for_each(_adnsEvents.begin(), _adnsEvents.end(),
                std::bind2nd(std::mem_fun_ref
                             (&EpollEventPoll::ADNSEvent::processEvents),
                             events));

#endif // ENABLE_ASYNC_DNS

}

bool EpollEventPoll::SocketEntry::eventEmpty() const
{

#ifdef ENABLE_ASYNC_DNS

  return _commandEvents.empty() && _adnsEvents.empty();

#else // !ENABLE_ASYNC_DNS

  return _commandEvents.empty();

#endif // !ENABLE_ASYNC_DNS)

}

int accumulateEvent(int events, const EpollEventPoll::Event& event)
{
  return events|event.getEvents();
}

struct epoll_event& EpollEventPoll::SocketEntry::getEpEvent()
{
  _epEvent.data.ptr = this;

#ifdef ENABLE_ASYNC_DNS

  _epEvent.events =
    std::accumulate(_adnsEvents.begin(),
                    _adnsEvents.end(),
                    std::accumulate(_commandEvents.begin(),
                                    _commandEvents.end(), 0, accumulateEvent),
                    accumulateEvent);

#else // !ENABLE_ASYNC_DNS

  _epEvent.events =
    std::accumulate(_commandEvents.begin(), _commandEvents.end(), 0,
                    accumulateEvent);

#endif // !ENABLE_ASYNC_DNS
  return _epEvent;
}

#ifdef ENABLE_ASYNC_DNS

EpollEventPoll::AsyncNameResolverEntry::AsyncNameResolverEntry
(const SharedHandle<AsyncNameResolver>& nameResolver, Command* command):
  _nameResolver(nameResolver), _command(command), _socketsSize(0)

{}

void EpollEventPoll::AsyncNameResolverEntry::addSocketEvents
(EpollEventPoll* e)
{
  _socketsSize = 0;
  int mask = _nameResolver->getsock(_sockets);
  if(mask == 0) {
    return;
  }
  size_t i;
  for(i = 0; i < ARES_GETSOCK_MAXNUM; ++i) {
    //epoll_event_t* epEventPtr = &_epEvents[_socketsSize];
    int events = 0;
    if(ARES_GETSOCK_READABLE(mask, i)) {
      events |= EPOLLIN;
    }
    if(ARES_GETSOCK_WRITABLE(mask, i)) {
      events |= EPOLLOUT;
    }
    if(events == 0) {
      // assume no further sockets are returned.
      break;
    }
    e->addEvents(_sockets[i], _command, events, _nameResolver);
  }
  _socketsSize = i;
}
  
void EpollEventPoll::AsyncNameResolverEntry::removeSocketEvents
(EpollEventPoll* e)
{
  for(size_t i = 0; i < _socketsSize; ++i) {
    e->deleteEvents(_sockets[i], _command, _nameResolver);
  }
}

void EpollEventPoll::AsyncNameResolverEntry::processTimeout()
{
  _nameResolver->process(ARES_SOCKET_BAD, ARES_SOCKET_BAD);
}

#endif // ENABLE_ASYNC_DNS

EpollEventPoll::EpollEventPoll():_logger(LogFactory::getInstance())
{
  _epfd = epoll_create(EPOLL_EVENTS_MAX);

  _epEvents = new struct epoll_event[EPOLL_EVENTS_MAX];
}

EpollEventPoll::~EpollEventPoll()
{
  if(_epfd != -1) {
    int r;
    while((r = close(_epfd)) == -1 && errno == EINTR);
    if(r == -1) {
      _logger->error("Error occurred while closing epoll file descriptor"
                     " %d: %s",
                     _epfd, strerror(errno));
    }
  }
  delete [] _epEvents;
}

bool EpollEventPoll::good() const
{
  return _epfd != -1;
}

void EpollEventPoll::poll(const struct timeval& tv)
{
  // timeout is millisec
  int timeout = tv.tv_sec*1000+tv.tv_usec/1000;

  int res;
  while((res = epoll_wait(_epfd, _epEvents, EPOLL_EVENTS_MAX, timeout)) == -1 &&
        errno == EINTR);

  if(res > 0) {
    for(int i = 0; i < res; ++i) {
      SocketEntry* p = reinterpret_cast<SocketEntry*>(_epEvents[i].data.ptr);
      p->processEvents(_epEvents[i].events);
    }
  }

#ifdef ENABLE_ASYNC_DNS
  // It turns out that we have to call ares_process_fd before ares's
  // own timeout and ares may create new sockets or closes socket in
  // their API. So we call ares_process_fd for all ares_channel and
  // re-register their sockets.
  for(std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator i =
        _nameResolverEntries.begin(); i != _nameResolverEntries.end(); ++i) {
    (*i)->processTimeout();
    (*i)->removeSocketEvents(this);
    (*i)->addSocketEvents(this);
  }
#endif // ENABLE_ASYNC_DNS

  // TODO timeout of name resolver is determined in Command(AbstractCommand,
  // DHTEntryPoint...Command)
}

static int translateEvents(EventPoll::EventType events)
{
  int newEvents = 0;
  if(EventPoll::EVENT_READ&events) {
    newEvents |= EPOLLIN;
  }
  if(EventPoll::EVENT_WRITE&events) {
    newEvents |= EPOLLOUT;
  }
  if(EventPoll::EVENT_ERROR&events) {
    newEvents |= EPOLLERR;
  }
  if(EventPoll::EVENT_HUP&events) {
    newEvents |= EPOLLHUP;    
  }
  return newEvents;
}

bool EpollEventPoll::addEvents(sock_t socket,
                               const EpollEventPoll::Event& event)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  std::deque<SharedHandle<SocketEntry> >::iterator i =
    std::lower_bound(_socketEntries.begin(), _socketEntries.end(), socketEntry);
  int r = 0;
  if(i != _socketEntries.end() && (*i) == socketEntry) {

    event.addSelf(*i);

    r = epoll_ctl(_epfd, EPOLL_CTL_MOD, (*i)->getSocket(), &(*i)->getEpEvent());
    if(r == -1) {
      // try EPOLL_CTL_ADD: There is a chance that previously socket X is
      // added to epoll, but it is closed and is not yet removed from
      // SocketEntries. In this case, EPOLL_CTL_MOD is failed with ENOENT.

      r = epoll_ctl(_epfd, EPOLL_CTL_ADD, (*i)->getSocket(),
                    &(*i)->getEpEvent());
    }
  } else {
    _socketEntries.insert(i, socketEntry);

    event.addSelf(socketEntry);

    r = epoll_ctl(_epfd, EPOLL_CTL_ADD, socketEntry->getSocket(),
                  &socketEntry->getEpEvent());
  }
  if(r == -1) {
    if(_logger->debug()) {
      _logger->debug("Failed to add socket event %d:%s",
                     socket, strerror(errno));
    }
    return false;
  } else {
    return true;
  }
}

bool EpollEventPoll::addEvents(sock_t socket, Command* command,
                               EventPoll::EventType events)
{
  int epEvents = translateEvents(events);
  return addEvents(socket, CommandEvent(command, epEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool EpollEventPoll::addEvents(sock_t socket, Command* command, int events,
                               const SharedHandle<AsyncNameResolver>& rs)
{
  return addEvents(socket, ADNSEvent(rs, command, socket, events));
}
#endif // ENABLE_ASYNC_DNS

bool EpollEventPoll::deleteEvents(sock_t socket,
                                  const EpollEventPoll::Event& event)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  std::deque<SharedHandle<SocketEntry> >::iterator i =
    std::lower_bound(_socketEntries.begin(), _socketEntries.end(), socketEntry);
  if(i != _socketEntries.end() && (*i) == socketEntry) {

    event.removeSelf(*i);

    int r = 0;
    if((*i)->eventEmpty()) {
      // In kernel before 2.6.9, epoll_ctl with EPOLL_CTL_DEL requires non-null
      // pointer of epoll_event.
      struct epoll_event ev = {0,{0}};
      r = epoll_ctl(_epfd, EPOLL_CTL_DEL, (*i)->getSocket(), &ev);
      _socketEntries.erase(i);
    } else {
      // If socket is closed, then it seems it is automatically removed from
      // epoll, so following EPOLL_CTL_MOD may fail.
      r = epoll_ctl(_epfd, EPOLL_CTL_MOD, (*i)->getSocket(),
                    &(*i)->getEpEvent());
      if(r == -1) {
        if(_logger->debug()) {
          _logger->debug("Failed to delete socket event, but may be ignored:%s",
                         strerror(errno));
        }
      }
    }
    if(r == -1) {
      if(_logger->debug()) {
        _logger->debug("Failed to delete socket event:%s", strerror(errno));
      }
      return false;
    } else {
      return true;
    }
  } else {
    if(_logger->debug()) {
      _logger->debug("Socket %d is not found in SocketEntries.", socket);
    }
    return false;
  }
}

#ifdef ENABLE_ASYNC_DNS
bool EpollEventPoll::deleteEvents(sock_t socket, Command* command,
                                  const SharedHandle<AsyncNameResolver>& rs)
{
  return deleteEvents(socket, ADNSEvent(rs, command, socket, 0));
}
#endif // ENABLE_ASYNC_DNS

bool EpollEventPoll::deleteEvents(sock_t socket, Command* command,
                                  EventPoll::EventType events)
{
  int epEvents = translateEvents(events);
  return deleteEvents(socket, CommandEvent(command, epEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool EpollEventPoll::addNameResolver
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<AsyncNameResolverEntry> entry
    (new AsyncNameResolverEntry(resolver, command));
  std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator itr =
    std::find(_nameResolverEntries.begin(), _nameResolverEntries.end(), entry);
  if(itr == _nameResolverEntries.end()) {
    _nameResolverEntries.push_back(entry);
    entry->addSocketEvents(this);
    return true;
  } else {
    return false;
  }
}

bool EpollEventPoll::deleteNameResolver
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<AsyncNameResolverEntry> entry
    (new AsyncNameResolverEntry(resolver, command));
  std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator itr =
    std::find(_nameResolverEntries.begin(), _nameResolverEntries.end(), entry);
  if(itr == _nameResolverEntries.end()) {
    return false;
  } else {
    (*itr)->removeSocketEvents(this);
    _nameResolverEntries.erase(itr);
    return true;
  }
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
