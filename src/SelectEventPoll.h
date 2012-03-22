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
#ifndef D_SELECT_EVENT_POLL_H
#define D_SELECT_EVENT_POLL_H

#include "EventPoll.h"

#include <deque>
#include <set>

#include "a2functional.h"
#ifdef ENABLE_ASYNC_DNS
# include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

class SelectEventPoll : public EventPoll {
private:
  class CommandEvent {
  private:
    Command* command_;
    int events_;
  public:
    CommandEvent(Command* command, int events);
    
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

    int getEvents() const
    {
      return events_;
    }
    
    void processEvents(int events);
  };

  friend int accumulateEvent
  (int events, const SelectEventPoll::CommandEvent& event);

  class SocketEntry {
  private:
    sock_t socket_;

    std::deque<CommandEvent> commandEvents_;
  public: 
    SocketEntry(sock_t socket);

    bool operator==(const SocketEntry& entry) const
    {
      return socket_ == entry.socket_;
    }

    bool operator<(const SocketEntry& entry) const
    {
      return socket_ < entry.socket_;
    }
    
    void addCommandEvent(Command* command, int events);
    
    void removeCommandEvent(Command* command, int events);

    int getEvents();

    sock_t getSocket() const
    {
      return socket_;
    }
    
    bool eventEmpty() const
    {
      return commandEvents_.empty();
    }
    
    void processEvents(int events);
  };

#ifdef ENABLE_ASYNC_DNS

  class AsyncNameResolverEntry {
  private:
    SharedHandle<AsyncNameResolver> nameResolver_;
    
    Command* command_;
    
  public:
    AsyncNameResolverEntry(const SharedHandle<AsyncNameResolver>& nameResolver,
                           Command* command);

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

    int getFds(fd_set* rfdsPtr, fd_set* wfdsPtr);

    void process(fd_set* rfdsPtr, fd_set* wfdsPtr);
  };
#endif // ENABLE_ASYNC_DNS

  fd_set rfdset_;
  fd_set wfdset_;
  sock_t fdmax_;

  typedef std::set<SharedHandle<SocketEntry>,
                   DerefLess<SharedHandle<SocketEntry> > > SocketEntrySet;
  SocketEntrySet socketEntries_;
#ifdef ENABLE_ASYNC_DNS
  typedef std::set<SharedHandle<AsyncNameResolverEntry>,
                   DerefLess<SharedHandle<AsyncNameResolverEntry> > >
  AsyncNameResolverEntrySet;
  AsyncNameResolverEntrySet nameResolverEntries_;
#endif // ENABLE_ASYNC_DNS

#ifdef __MINGW32__
  // Winsock select() doesn't work if no socket is in FD_SET. We add
  // this dummy socket to work around this problem
  sock_t dummySocket_;
#endif // __MINGW32__

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

#endif // D_SELECT_EVENT_POLL_H
