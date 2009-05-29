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
#ifndef _D_SELECT_EVENT_POLL_H_
#define _D_SELECT_EVENT_POLL_H_

#include "EventPoll.h"

#include <deque>

#ifdef ENABLE_ASYNC_DNS
# include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

class Logger;

class SelectEventPoll : public EventPoll {
private:
  class CommandEvent {
  private:
    Command* _command;
    int _events;
  public:
    CommandEvent(Command* command, int events);
    
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

    int getEvents() const
    {
      return _events;
    }
    
    void processEvents(int events);
  };

  friend int accumulateEvent
  (int events, const SelectEventPoll::CommandEvent& event);

  class SocketEntry {
  private:
    sock_t _socket;

    std::deque<CommandEvent> _commandEvents;
  public: 
    SocketEntry(sock_t socket);

    bool operator==(const SocketEntry& entry) const
    {
      return _socket == entry._socket;
    }

    bool operator<(const SocketEntry& entry) const
    {
      return _socket < entry._socket;
    }
    
    void addCommandEvent(Command* command, int events);
    
    void removeCommandEvent(Command* command, int events);

    int getEvents();

    sock_t getSocket() const
    {
      return _socket;
    }
    
    bool eventEmpty() const
    {
      return _commandEvents.empty();
    }
    
    void processEvents(int events);
  };

#ifdef ENABLE_ASYNC_DNS

  class AsyncNameResolverEntry {
  private:
    SharedHandle<AsyncNameResolver> _nameResolver;
    
    Command* _command;
    
  public:
    AsyncNameResolverEntry(const SharedHandle<AsyncNameResolver>& nameResolver,
			   Command* command);

    bool operator==(const AsyncNameResolverEntry& entry)
    {
      return _nameResolver == entry._nameResolver &&
	_command == entry._command;
    }

    int getFds(fd_set* rfdsPtr, fd_set* wfdsPtr);

    void process(fd_set* rfdsPtr, fd_set* wfdsPtr);
  };
#endif // ENABLE_ASYNC_DNS

  fd_set _rfdset;
  fd_set _wfdset;
  int _fdmax;

  std::deque<SharedHandle<SocketEntry> > _socketEntries;
#ifdef ENABLE_ASYNC_DNS
  std::deque<SharedHandle<AsyncNameResolverEntry> > _nameResolverEntries;
#endif // ENABLE_ASYNC_DNS

  Logger* _logger;

  void updateFdSet();
public:
  SelectEventPoll();

  virtual ~SelectEventPoll();

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

#endif // _D_SELECT_EVENT_POLL_H_
