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
#ifndef _D_EVENT_H_
#define _D_EVENT_H_

#include "common.h"

#include <deque>
#include <algorithm>
#include <functional>

#include "SharedHandle.h"
#include "a2netcompat.h"
#include "Command.h"
#ifdef ENABLE_ASYNC_DNS
# include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

template<typename SocketEntry>
class Event {
public:
  virtual ~Event() {}

  virtual void processEvents(int events) = 0;

  virtual int getEvents() const = 0;

  virtual void addSelf(const SharedHandle<SocketEntry>& socketEntry) const = 0;

  virtual void removeSelf
  (const SharedHandle<SocketEntry>& socketEntry) const = 0;
};

template<typename SocketEntry, typename EventPoll>
class CommandEvent : public Event<SocketEntry> {
private:
  Command* _command;
  int _events;
public:
  CommandEvent(Command* command, int events):
    _command(command), _events(events) {}

  Command* getCommand() const
  {
    return _command;
  }
    
  void addEvents(int events)
  {
    _events |= events;
  }

  void removeEvents(int events)
  {
    _events &= (~events); 
  }

  bool eventsEmpty() const
  {
    return _events == 0;
  }
        
  bool operator==(const CommandEvent& commandEvent) const
  {
    return _command == commandEvent._command;
  }

  virtual int getEvents() const
  {
    return _events;
  }

  virtual void processEvents(int events)
  {
    if((_events&events) ||
       ((EventPoll::IEV_ERROR|EventPoll::IEV_HUP)&events)) {
      _command->setStatusActive();
    }
    if(EventPoll::IEV_READ&events) {
      _command->readEventReceived();
    }
    if(EventPoll::IEV_WRITE&events) {
      _command->writeEventReceived();
    }
    if(EventPoll::IEV_ERROR&events) {
      _command->errorEventReceived();
    }
    if(EventPoll::IEV_HUP&events) {
      _command->hupEventReceived();
    }
  }

  virtual void addSelf(const SharedHandle<SocketEntry>& socketEntry) const
  {
    socketEntry->addCommandEvent(*this);
  }

  virtual void removeSelf(const SharedHandle<SocketEntry>& socketEntry) const
  {
    socketEntry->removeCommandEvent(*this);
  }
};
  
#ifdef ENABLE_ASYNC_DNS

template<typename SocketEntry, typename EventPoll>
class ADNSEvent : public Event<SocketEntry> {
private:
  SharedHandle<AsyncNameResolver> _resolver;
  Command* _command;
  sock_t _socket;
  int _events;
public:
  ADNSEvent(const SharedHandle<AsyncNameResolver>& resolver, Command* command,
            sock_t socket, int events):
    _resolver(resolver), _command(command), _socket(socket), _events(events) {}

  bool operator==(const ADNSEvent& event) const
  {
    return _resolver == event._resolver;
  }
    
  virtual int getEvents() const
  {
    return _events;
  }

  virtual void processEvents(int events)
  {
    ares_socket_t readfd;
    ares_socket_t writefd;
    if(events&(EventPoll::IEV_READ|EventPoll::IEV_ERROR|
               EventPoll::IEV_HUP)) {
      readfd = _socket;
    } else {
      readfd = ARES_SOCKET_BAD;
    }
    if(events&(EventPoll::IEV_WRITE|EventPoll::IEV_ERROR|
               EventPoll::IEV_HUP)) {
      writefd = _socket;
    } else {
      writefd = ARES_SOCKET_BAD;
    }
    _resolver->process(readfd, writefd);
    _command->setStatusActive();
  }

  virtual void addSelf(const SharedHandle<SocketEntry>& socketEntry) const
  {
    socketEntry->addADNSEvent(*this);
  }

  virtual void removeSelf(const SharedHandle<SocketEntry>& socketEntry) const
  {
    socketEntry->removeADNSEvent(*this);
  }
};
#else // !ENABLE_ASYNC_DNS
template<typename SocketEntry, typename EventPoll>
class ADNSEvent : public Event<SocketEntry> {};
#endif // !ENABLE_ASYNC_DNS

template<typename CommandEvent, typename ADNSEvent>
class SocketEntry {
protected:
  sock_t _socket;
    
  std::deque<CommandEvent> _commandEvents;
    
#ifdef ENABLE_ASYNC_DNS
    
  std::deque<ADNSEvent> _adnsEvents;

#endif // ENABLE_ASYNC_DNS
public:
  SocketEntry(sock_t socket):_socket(socket) {}

  virtual ~SocketEntry() {}

  bool operator==(const SocketEntry& entry) const
  {
    return _socket == entry._socket;
  }

  bool operator<(const SocketEntry& entry) const
  {
    return _socket < entry._socket;
  }

  void addCommandEvent(const CommandEvent& cev)
  {
    typename std::deque<CommandEvent>::iterator i =
      std::find(_commandEvents.begin(), _commandEvents.end(), cev);
    if(i == _commandEvents.end()) {
      _commandEvents.push_back(cev);
    } else {
      (*i).addEvents(cev.getEvents());
    }
  }

  void removeCommandEvent(const CommandEvent& cev)
  {
    typename std::deque<CommandEvent>::iterator i =
      std::find(_commandEvents.begin(), _commandEvents.end(), cev);
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
    
  void addADNSEvent(const ADNSEvent& aev)
  {
    typename std::deque<ADNSEvent>::iterator i =
      std::find(_adnsEvents.begin(), _adnsEvents.end(), aev);
    if(i == _adnsEvents.end()) {
      _adnsEvents.push_back(aev);
    }
  }

  void removeADNSEvent(const ADNSEvent& aev)
  {
    typename std::deque<ADNSEvent>::iterator i =
      std::find(_adnsEvents.begin(), _adnsEvents.end(), aev);
    if(i == _adnsEvents.end()) {
      // not found
    } else {
      _adnsEvents.erase(i);
    }
  }
#endif // ENABLE_ASYNC_DNS

  sock_t getSocket() const
  {
    return _socket;
  }

  void setSocket(sock_t socket)
  {
    _socket = socket;
  }

  bool eventEmpty() const
  {
#ifdef ENABLE_ASYNC_DNS
    return _commandEvents.empty() && _adnsEvents.empty();
#else // !ENABLE_ASYNC_DNS
    return _commandEvents.empty();
#endif // !ENABLE_ASYNC_DNS)
  }
    
  void processEvents(int events)
  {
    std::for_each(_commandEvents.begin(), _commandEvents.end(),
                  std::bind2nd(std::mem_fun_ref
                               (&CommandEvent::processEvents),
                               events));
#ifdef ENABLE_ASYNC_DNS
    std::for_each(_adnsEvents.begin(), _adnsEvents.end(),
                  std::bind2nd(std::mem_fun_ref
                               (&ADNSEvent::processEvents),
                               events));
#endif // ENABLE_ASYNC_DNS
  }
};

#ifdef ENABLE_ASYNC_DNS

template<typename EventPoll>
class AsyncNameResolverEntry {
private:
  SharedHandle<AsyncNameResolver> _nameResolver;

  Command* _command;

  size_t _socketsSize;
  
  sock_t _sockets[ARES_GETSOCK_MAXNUM];

public:
  AsyncNameResolverEntry(const SharedHandle<AsyncNameResolver>& nameResolver,
                         Command* command):
    _nameResolver(nameResolver), _command(command), _socketsSize(0) {}

  bool operator==(const AsyncNameResolverEntry& entry)
  {
    return _nameResolver == entry._nameResolver &&
      _command == entry._command;
  }

  void addSocketEvents(EventPoll* e)
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
        events |= EventPoll::IEV_READ;
      }
      if(ARES_GETSOCK_WRITABLE(mask, i)) {
        events |= EventPoll::IEV_WRITE;
      }
      if(events == 0) {
        // assume no further sockets are returned.
        break;
      }
      e->addEvents(_sockets[i], _command, events, _nameResolver);
    }
    _socketsSize = i;
  }
    
  void removeSocketEvents(EventPoll* e)
  {
    for(size_t i = 0; i < _socketsSize; ++i) {
      e->deleteEvents(_sockets[i], _command, _nameResolver);
    }
  }

  // Calls AsyncNameResolver::process(ARES_SOCKET_BAD,
  // ARES_SOCKET_BAD).
  void processTimeout()
  {
    _nameResolver->process(ARES_SOCKET_BAD, ARES_SOCKET_BAD);
  }
};
#else // !ENABLE_ASYNC_DNS
template<typename EventPoll>
class AsyncNameResolverEntry {};
#endif // !ENABLE_ASYNC_DNS

} // namespace aria2

#endif // _D_EVENT_H_
