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

namespace aria2 {

PollEventPoll::CommandEvent::CommandEvent(Command* command, int events):
  _command(command), _events(events) {}

int PollEventPoll::CommandEvent::getEvents() const
{
  return _events;
}

void PollEventPoll::CommandEvent::processEvents(int events)
{
  if((_events&events) ||
     ((PollEventPoll::EVENT_ERROR|PollEventPoll::EVENT_HUP)&events)) {
    _command->setStatusActive();
  }
  if(PollEventPoll::EVENT_READ&events) {
    _command->readEventReceived();
  }
  if(PollEventPoll::EVENT_WRITE&events) {
    _command->writeEventReceived();
  }
  if(PollEventPoll::EVENT_ERROR&events) {
    _command->errorEventReceived();
  }
  if(PollEventPoll::EVENT_HUP&events) {
    _command->hupEventReceived();
  }
}

void PollEventPoll::CommandEvent::addSelf
(const SharedHandle<SocketEntry>& socketEntry) const
{
  socketEntry->addCommandEvent(*this);
}

void PollEventPoll::CommandEvent::removeSelf
(const SharedHandle<SocketEntry>& socketEntry) const
{
  socketEntry->removeCommandEvent(*this);
}

#ifdef ENABLE_ASYNC_DNS

PollEventPoll::ADNSEvent::ADNSEvent
(const SharedHandle<AsyncNameResolver>& resolver,
 Command* command,
 sock_t socket, int events):
  _resolver(resolver), _command(command), _socket(socket), _events(events) {}

int PollEventPoll::ADNSEvent::getEvents() const
{
  return _events;
}

void PollEventPoll::ADNSEvent::processEvents(int events)
{
  ares_socket_t readfd;
  ares_socket_t writefd;
  if(events&(PollEventPoll::EVENT_READ|PollEventPoll::EVENT_ERROR|
             PollEventPoll::EVENT_HUP)) {
    readfd = _socket;
  } else {
    readfd = ARES_SOCKET_BAD;
  }
  if(events&(PollEventPoll::EVENT_WRITE|PollEventPoll::EVENT_ERROR|
             PollEventPoll::EVENT_HUP)) {
    writefd = _socket;
  } else {
    writefd = ARES_SOCKET_BAD;
  }
  _resolver->process(readfd, writefd);
  _command->setStatusActive();
}

void PollEventPoll::ADNSEvent::addSelf
(const SharedHandle<SocketEntry>& socketEntry) const
{
  socketEntry->addADNSEvent(*this);
}

void PollEventPoll::ADNSEvent::removeSelf
(const SharedHandle<SocketEntry>& socketEntry) const
{
  socketEntry->removeADNSEvent(*this);
}

#endif // ENABLE_ASYNC_DNS

PollEventPoll::SocketEntry::SocketEntry(sock_t socket):_socket(socket)
{
  memset(&_pollEvent, 0, sizeof(struct pollfd));
}

void PollEventPoll::SocketEntry::addCommandEvent(const CommandEvent& cev)
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

void PollEventPoll::SocketEntry::removeCommandEvent(const CommandEvent& cev)
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

void PollEventPoll::SocketEntry::addADNSEvent(const ADNSEvent& aev)
{
  std::deque<ADNSEvent>::iterator i = std::find(_adnsEvents.begin(),
                                                _adnsEvents.end(),
                                                aev);
  if(i == _adnsEvents.end()) {
    _adnsEvents.push_back(aev);
  }
}

void PollEventPoll::SocketEntry::removeADNSEvent(const ADNSEvent& aev)
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

void PollEventPoll::SocketEntry::processEvents(int events)
{
  std::for_each(_commandEvents.begin(), _commandEvents.end(),
                std::bind2nd(std::mem_fun_ref
                             (&PollEventPoll::CommandEvent::processEvents),
                             events));
#ifdef ENABLE_ASYNC_DNS

  std::for_each(_adnsEvents.begin(), _adnsEvents.end(),
                std::bind2nd(std::mem_fun_ref
                             (&PollEventPoll::ADNSEvent::processEvents),
                             events));

#endif // ENABLE_ASYNC_DNS

}

bool PollEventPoll::SocketEntry::eventEmpty() const
{

#ifdef ENABLE_ASYNC_DNS

  return _commandEvents.empty() && _adnsEvents.empty();

#else // !ENABLE_ASYNC_DNS

  return _commandEvents.empty();

#endif // !ENABLE_ASYNC_DNS)

}

int accumulateEvent(int events, const PollEventPoll::Event& event)
{
  return events|event.getEvents();
}

struct pollfd& PollEventPoll::SocketEntry::getPollEvent()
{
  _pollEvent.fd = _socket;
#ifdef ENABLE_ASYNC_DNS
  _pollEvent.events =
    std::accumulate(_adnsEvents.begin(),
                    _adnsEvents.end(),
                    std::accumulate(_commandEvents.begin(),
                                    _commandEvents.end(), 0, accumulateEvent),
                    accumulateEvent);
#else // !ENABLE_ASYNC_DNS
  _pollEvent.events =
    std::accumulate(_commandEvents.begin(), _commandEvents.end(), 0,
                    accumulateEvent);
#endif // !ENABLE_ASYNC_DNS
  return _pollEvent;
}

#ifdef ENABLE_ASYNC_DNS

PollEventPoll::AsyncNameResolverEntry::AsyncNameResolverEntry
(const SharedHandle<AsyncNameResolver>& nameResolver, Command* command):
  _nameResolver(nameResolver), _command(command), _socketsSize(0)

{}

void PollEventPoll::AsyncNameResolverEntry::addSocketEvents(PollEventPoll* e)
{
  _socketsSize = 0;
  int mask = _nameResolver->getsock(_sockets);
  if(mask == 0) {
    return;
  }
  size_t i;
  for(i = 0; i < ARES_GETSOCK_MAXNUM; ++i) {
    int events = 0;
    if(ARES_GETSOCK_READABLE(mask, i)) {
      events |= PollEventPoll::EVENT_READ;
    }
    if(ARES_GETSOCK_WRITABLE(mask, i)) {
      events |= PollEventPoll::EVENT_WRITE;
    }
    if(events == 0) {
      // assume no further sockets are returned.
      break;
    }
    LogFactory::getInstance()->debug("addSocketEvents, %d", _sockets[i]);
    e->addEvents(_sockets[i], _command, events, _nameResolver);
  }
  _socketsSize = i;
}
  
void PollEventPoll::AsyncNameResolverEntry::removeSocketEvents(PollEventPoll* e)
{
  for(size_t i = 0; i < _socketsSize; ++i) {
    e->deleteEvents(_sockets[i], _command, _nameResolver);
  }
}

void PollEventPoll::AsyncNameResolverEntry::processTimeout()
{
  _nameResolver->process(ARES_SOCKET_BAD, ARES_SOCKET_BAD);
}

#endif // ENABLE_ASYNC_DNS

PollEventPoll::PollEventPoll():
  _pollfdCapacity(1024), _pollfdNum(0), _logger(LogFactory::getInstance())
{
  _pollfds = new struct pollfd[_pollfdCapacity];
}

PollEventPoll::~PollEventPoll()
{
  delete [] _pollfds;
}

void PollEventPoll::poll(const struct timeval& tv)
{
  // timeout is millisec
  int timeout = tv.tv_sec*1000+tv.tv_usec/1000;
  int res;
  while((res = ::poll(_pollfds, _pollfdNum, timeout)) == -1 &&
        errno == EINTR);
  if(res > 0) {
    SharedHandle<SocketEntry> se(new SocketEntry(0));
    for(struct pollfd* first = _pollfds, *last = _pollfds+_pollfdNum;
        first != last; ++first) {
      if(first->revents) {
        se->setSocket(first->fd);
        std::deque<SharedHandle<SocketEntry> >::iterator itr =
          std::lower_bound(_socketEntries.begin(), _socketEntries.end(), se);
        if(itr != _socketEntries.end() && (*itr) == se) {
          (*itr)->processEvents(first->revents);
        } else {
          if(_logger->debug()) {
            _logger->debug("Socket %d is not found in SocketEntries.",
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
  for(std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator i =
        _nameResolverEntries.begin(), eoi = _nameResolverEntries.end();
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
    newEvents |= PollEventPoll::EVENT_READ;
  }
  if(EventPoll::EVENT_WRITE&events) {
    newEvents |= PollEventPoll::EVENT_WRITE;
  }
  if(EventPoll::EVENT_ERROR&events) {
    newEvents |= PollEventPoll::EVENT_ERROR;
  }
  if(EventPoll::EVENT_HUP&events) {
    newEvents |= PollEventPoll::EVENT_HUP;
  }
  return newEvents;
}

bool PollEventPoll::addEvents
(sock_t socket, const PollEventPoll::Event& event)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  std::deque<SharedHandle<SocketEntry> >::iterator i =
    std::lower_bound(_socketEntries.begin(), _socketEntries.end(), socketEntry);
  if(i != _socketEntries.end() && (*i) == socketEntry) {
    event.addSelf(*i);
    for(struct pollfd* first = _pollfds, *last = _pollfds+_pollfdNum;
        first != last; ++first) {
      if(first->fd == socket) {
        *first = (*i)->getPollEvent();
        break;
      }
    }
  } else {    
    _socketEntries.insert(i, socketEntry);
    event.addSelf(socketEntry);
    if(_pollfdCapacity == _pollfdNum) {
      _pollfdCapacity *= 2;
      struct pollfd* newPollfds = new struct pollfd[_pollfdCapacity];
      memcpy(newPollfds, _pollfds, _pollfdNum*sizeof(struct pollfd));
      delete [] _pollfds;
      _pollfds = newPollfds;
    }
    _pollfds[_pollfdNum] = socketEntry->getPollEvent();
    ++_pollfdNum;
  }
  return true;
}

bool PollEventPoll::addEvents
(sock_t socket, Command* command, EventPoll::EventType events)
{
  int pollEvents = translateEvents(events);
  return addEvents(socket, CommandEvent(command, pollEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::addEvents
(sock_t socket, Command* command, int events,
 const SharedHandle<AsyncNameResolver>& rs)
{
  return addEvents(socket, ADNSEvent(rs, command, socket, events));
}
#endif // ENABLE_ASYNC_DNS

bool PollEventPoll::deleteEvents
(sock_t socket, const PollEventPoll::Event& event)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  std::deque<SharedHandle<SocketEntry> >::iterator i =
    std::lower_bound(_socketEntries.begin(), _socketEntries.end(), socketEntry);
  if(i != _socketEntries.end() && (*i) == socketEntry) {
    event.removeSelf(*i);
    for(struct pollfd* first = _pollfds, *last = _pollfds+_pollfdNum;
        first != last; ++first) {
      if(first->fd == socket) {
        if((*i)->eventEmpty()) {
          if(_pollfdNum >= 2) {
            *first = *(last-1);
          }
          --_pollfdNum;
          _socketEntries.erase(i);
        } else {
          *first = (*i)->getPollEvent();
        }
        break;
      }
    }
    return true;
  } else {
    if(_logger->debug()) {
      _logger->debug("Socket %d is not found in SocketEntries.", socket);
    }
    return false;
  }
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::deleteEvents
(sock_t socket, Command* command, const SharedHandle<AsyncNameResolver>& rs)
{
  return deleteEvents(socket, ADNSEvent(rs, command, socket, 0));
}
#endif // ENABLE_ASYNC_DNS

bool PollEventPoll::deleteEvents
(sock_t socket, Command* command, EventPoll::EventType events)
{
  int pollEvents = translateEvents(events);
  return deleteEvents(socket, CommandEvent(command, pollEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool PollEventPoll::addNameResolver
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

bool PollEventPoll::deleteNameResolver
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
