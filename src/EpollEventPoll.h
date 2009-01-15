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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#ifndef _D_EPOLL_EVENT_POLL_H_
#define _D_EPLLL_EVENT_POLL_H_

#include "EventPoll.h"

# include <sys/epoll.h>

#include <deque>

#ifdef ENABLE_ASYNC_DNS
# include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

class Logger;

class EpollEventPoll : public EventPoll {

private:

  class SocketEntry;

  class Event {
  public:
    virtual ~Event() {}

    virtual void processEvents(int events) = 0;

    virtual int getEvents() const = 0;

    virtual void addSelf(const SharedHandle<SocketEntry>& socketEntry) const =0;

    virtual void removeSelf
    (const SharedHandle<SocketEntry>& socketEntry) const =0;
  };

  friend int accumulateEvent(int events, const Event& event);

  class CommandEvent : public Event {
  private:
    Command* _command;
    int _events;
  public:
    CommandEvent(Command* command, int events);
    
    Command* getCommand() const;
    
    void addEvents(int events);
    
    void removeEvents(int events);
    
    bool eventsEmpty() const;
        
    bool operator==(const CommandEvent& event) const;

    virtual int getEvents() const;
    
    virtual void processEvents(int events);

    virtual void addSelf(const SharedHandle<SocketEntry>& socketEntry) const;

    virtual void removeSelf(const SharedHandle<SocketEntry>& socketEntry) const;
  };
  
  class ADNSEvent : public Event {
  private:
    SharedHandle<AsyncNameResolver> _resolver;
    Command* _command;
    sock_t _socket;
    int _events;
  public:
    ADNSEvent(const SharedHandle<AsyncNameResolver>& resolver, Command* command,
	      sock_t socket, int events);
    
    bool operator==(const ADNSEvent& event) const;
    
    virtual int getEvents() const;

    virtual void processEvents(int events);

    virtual void addSelf(const SharedHandle<SocketEntry>& socketEntry) const;

    virtual void removeSelf(const SharedHandle<SocketEntry>& socketEntry) const;
  };


  class SocketEntry {
  private:
    sock_t _socket;
    
    std::deque<CommandEvent> _commandEvents;
    
#ifdef ENABLE_ASYNC_DNS
    
    std::deque<ADNSEvent> _adnsEvents;

#endif // ENABLE_ASYNC_DNS

    struct epoll_event _epEvent;

  public:
    SocketEntry(sock_t socket);

    bool operator==(const SocketEntry& entry) const;

    bool operator<(const SocketEntry& entry) const;

    void addCommandEvent(const CommandEvent& cev);

    void removeCommandEvent(const CommandEvent& cev);

#ifdef ENABLE_ASYNC_DNS
    
    void addADNSEvent(const ADNSEvent& aev);
    
    void removeADNSEvent(const ADNSEvent& aev);

#endif // ENABLE_ASYNC_DNS

    struct epoll_event& getEpEvent();
    
    sock_t getSocket() const;
    
    bool eventEmpty() const;
    
    void processEvents(int events);
  };

#ifdef ENABLE_ASYNC_DNS

  class AsyncNameResolverEntry {
  private:
    SharedHandle<AsyncNameResolver> _nameResolver;

    Command* _command;

    size_t _socketsSize;
  
    sock_t _sockets[ARES_GETSOCK_MAXNUM];

  public:
    AsyncNameResolverEntry(const SharedHandle<AsyncNameResolver>& nameResolver,
			   Command* command);

    bool operator==(const AsyncNameResolverEntry& entry);

    void addSocketEvents(EpollEventPoll* socketPoll);
    
    void removeSocketEvents(EpollEventPoll* socketPoll);
  };

#endif // ENABLE_ASYNC_DNS
private:
  enum EventType {
    EVENT_READ = EPOLLIN,
    EVENT_WRITE = EPOLLOUT,
    EVENT_ERROR = EPOLLERR,
    EVENT_HUP = EPOLLHUP,
  };

  std::deque<SharedHandle<SocketEntry> > _socketEntries;
#ifdef ENABLE_ASYNC_DNS
  std::deque<SharedHandle<AsyncNameResolverEntry> > _nameResolverEntries;
#endif // ENABLE_ASYNC_DNS

  int _epfd;

  struct epoll_event* _epEvents;

  static const size_t EPOLL_EVENTS_MAX = 1024;

  Logger* _logger;

  bool addEvents(sock_t socket, const Event& event);

  bool deleteEvents(sock_t socket, const Event& event);

  bool addEvents(sock_t socket, Command* command, int events,
		 const SharedHandle<AsyncNameResolver>& rs);

  bool deleteEvents(sock_t socket, Command* command,
		    const SharedHandle<AsyncNameResolver>& rs);

public:
  EpollEventPoll();

  bool good() const;

  virtual ~EpollEventPoll();

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

};

} // namespace aria2

#endif // _D_EVENT_POLL_H_
