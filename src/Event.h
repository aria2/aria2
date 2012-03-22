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
#ifndef D_EVENT_H
#define D_EVENT_H

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
  Command* command_;
  int events_;
public:
  CommandEvent(Command* command, int events):
    command_(command), events_(events) {}

  Command* getCommand() const
  {
    return command_;
  }
    
  void addEvents(int events)
  {
    events_ |= events;
  }

  void removeEvents(int events)
  {
    events_ &= (~events); 
  }

  bool eventsEmpty() const
  {
    return events_ == 0;
  }
        
  bool operator==(const CommandEvent& commandEvent) const
  {
    return command_ == commandEvent.command_;
  }

  virtual int getEvents() const
  {
    return events_;
  }

  virtual void processEvents(int events)
  {
    if((events_&events) ||
       ((EventPoll::IEV_ERROR|EventPoll::IEV_HUP)&events)) {
      command_->setStatusActive();
    }
    if(EventPoll::IEV_READ&events) {
      command_->readEventReceived();
    }
    if(EventPoll::IEV_WRITE&events) {
      command_->writeEventReceived();
    }
    if(EventPoll::IEV_ERROR&events) {
      command_->errorEventReceived();
    }
    if(EventPoll::IEV_HUP&events) {
      command_->hupEventReceived();
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
  SharedHandle<AsyncNameResolver> resolver_;
  Command* command_;
  sock_t socket_;
  int events_;
public:
  ADNSEvent(const SharedHandle<AsyncNameResolver>& resolver, Command* command,
            sock_t socket, int events):
    resolver_(resolver), command_(command), socket_(socket), events_(events) {}

  bool operator==(const ADNSEvent& event) const
  {
    return *resolver_ == *event.resolver_;
  }
    
  virtual int getEvents() const
  {
    return events_;
  }

  virtual void processEvents(int events)
  {
    ares_socket_t readfd;
    ares_socket_t writefd;
    if(events&(EventPoll::IEV_READ|EventPoll::IEV_ERROR|
               EventPoll::IEV_HUP)) {
      readfd = socket_;
    } else {
      readfd = ARES_SOCKET_BAD;
    }
    if(events&(EventPoll::IEV_WRITE|EventPoll::IEV_ERROR|
               EventPoll::IEV_HUP)) {
      writefd = socket_;
    } else {
      writefd = ARES_SOCKET_BAD;
    }
    resolver_->process(readfd, writefd);
    command_->setStatusActive();
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
  sock_t socket_;
    
  std::deque<CommandEvent> commandEvents_;
    
#ifdef ENABLE_ASYNC_DNS
    
  std::deque<ADNSEvent> adnsEvents_;

#endif // ENABLE_ASYNC_DNS
public:
  SocketEntry(sock_t socket):socket_(socket) {}

  virtual ~SocketEntry() {}

  bool operator==(const SocketEntry& entry) const
  {
    return socket_ == entry.socket_;
  }

  bool operator<(const SocketEntry& entry) const
  {
    return socket_ < entry.socket_;
  }

  void addCommandEvent(const CommandEvent& cev)
  {
    typename std::deque<CommandEvent>::iterator i =
      std::find(commandEvents_.begin(), commandEvents_.end(), cev);
    if(i == commandEvents_.end()) {
      commandEvents_.push_back(cev);
    } else {
      (*i).addEvents(cev.getEvents());
    }
  }

  void removeCommandEvent(const CommandEvent& cev)
  {
    typename std::deque<CommandEvent>::iterator i =
      std::find(commandEvents_.begin(), commandEvents_.end(), cev);
    if(i == commandEvents_.end()) {
      // not found
    } else {
      (*i).removeEvents(cev.getEvents());
      if((*i).eventsEmpty()) {
        commandEvents_.erase(i);
      }
    }
  }

#ifdef ENABLE_ASYNC_DNS
    
  void addADNSEvent(const ADNSEvent& aev)
  {
    typename std::deque<ADNSEvent>::iterator i =
      std::find(adnsEvents_.begin(), adnsEvents_.end(), aev);
    if(i == adnsEvents_.end()) {
      adnsEvents_.push_back(aev);
    }
  }

  void removeADNSEvent(const ADNSEvent& aev)
  {
    typename std::deque<ADNSEvent>::iterator i =
      std::find(adnsEvents_.begin(), adnsEvents_.end(), aev);
    if(i == adnsEvents_.end()) {
      // not found
    } else {
      adnsEvents_.erase(i);
    }
  }
#endif // ENABLE_ASYNC_DNS

  sock_t getSocket() const
  {
    return socket_;
  }

  void setSocket(sock_t socket)
  {
    socket_ = socket;
  }

  bool eventEmpty() const
  {
#ifdef ENABLE_ASYNC_DNS
    return commandEvents_.empty() && adnsEvents_.empty();
#else // !ENABLE_ASYNC_DNS
    return commandEvents_.empty();
#endif // !ENABLE_ASYNC_DNS)
  }
    
  void processEvents(int events)
  {
    std::for_each(commandEvents_.begin(), commandEvents_.end(),
                  std::bind2nd(std::mem_fun_ref
                               (&CommandEvent::processEvents),
                               events));
#ifdef ENABLE_ASYNC_DNS
    std::for_each(adnsEvents_.begin(), adnsEvents_.end(),
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
  SharedHandle<AsyncNameResolver> nameResolver_;

  Command* command_;

  size_t socketsSize_;
  
  sock_t sockets_[ARES_GETSOCK_MAXNUM];

public:
  AsyncNameResolverEntry(const SharedHandle<AsyncNameResolver>& nameResolver,
                         Command* command):
    nameResolver_(nameResolver), command_(command), socketsSize_(0) {}

  bool operator==(const AsyncNameResolverEntry& entry)
  {
    return *nameResolver_ == *entry.nameResolver_ &&
      command_ == entry.command_;
  }

  bool operator<(const AsyncNameResolverEntry& entry)
  {
    return nameResolver_.get() < entry.nameResolver_.get() ||
      (nameResolver_.get() == entry.nameResolver_.get() &&
       command_ < entry.command_);
  }

  void addSocketEvents(EventPoll* e)
  {
    socketsSize_ = 0;
    int mask = nameResolver_->getsock(sockets_);
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
      e->addEvents(sockets_[i], command_, events, nameResolver_);
    }
    socketsSize_ = i;
  }
    
  void removeSocketEvents(EventPoll* e)
  {
    for(size_t i = 0; i < socketsSize_; ++i) {
      e->deleteEvents(sockets_[i], command_, nameResolver_);
    }
  }

  // Calls AsyncNameResolver::process(ARES_SOCKET_BAD,
  // ARES_SOCKET_BAD).
  void processTimeout()
  {
    nameResolver_->process(ARES_SOCKET_BAD, ARES_SOCKET_BAD);
  }
};
#else // !ENABLE_ASYNC_DNS
template<typename EventPoll>
class AsyncNameResolverEntry {};
#endif // !ENABLE_ASYNC_DNS

} // namespace aria2

#endif // D_EVENT_H
