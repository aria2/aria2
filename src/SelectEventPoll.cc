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
#include "SelectEventPoll.h"

#ifdef __MINGW32__
# include <cassert>
#endif // __MINGW32__
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

SelectEventPoll::CommandEvent::CommandEvent(Command* command, int events):
  command_(command), events_(events) {}

void SelectEventPoll::CommandEvent::processEvents(int events)
{
  if((events_&events) ||
     ((EventPoll::EVENT_ERROR|EventPoll::EVENT_HUP)&events)) {
    command_->setStatusActive();
  }
  if(EventPoll::EVENT_READ&events) {
    command_->readEventReceived();
  }
  if(EventPoll::EVENT_WRITE&events) {
    command_->writeEventReceived();
  }
  if(EventPoll::EVENT_ERROR&events) {
    command_->errorEventReceived();
  }
  if(EventPoll::EVENT_HUP&events) {
    command_->hupEventReceived();
  }
}

SelectEventPoll::SocketEntry::SocketEntry(sock_t socket):socket_(socket) {}

void SelectEventPoll::SocketEntry::addCommandEvent
(Command* command, int events)
{
  CommandEvent cev(command, events);
  std::deque<CommandEvent>::iterator i = std::find(commandEvents_.begin(),
                                                   commandEvents_.end(),
                                                   cev);
  if(i == commandEvents_.end()) {
    commandEvents_.push_back(cev);
  } else {
    (*i).addEvents(events);
  }
}
void SelectEventPoll::SocketEntry::removeCommandEvent
(Command* command, int events)
{
  CommandEvent cev(command, events);
  std::deque<CommandEvent>::iterator i = std::find(commandEvents_.begin(),
                                                   commandEvents_.end(),
                                                   cev);
  if(i == commandEvents_.end()) {
    // not found
  } else {
    (*i).removeEvents(events);
    if((*i).eventsEmpty()) {
      commandEvents_.erase(i);
    }
  }
}
void SelectEventPoll::SocketEntry::processEvents(int events)
{
  std::for_each(commandEvents_.begin(), commandEvents_.end(),
                std::bind2nd(std::mem_fun_ref(&CommandEvent::processEvents),
                             events));
}

int accumulateEvent(int events, const SelectEventPoll::CommandEvent& event)
{
  return events|event.getEvents();
}

int SelectEventPoll::SocketEntry::getEvents()
{
  return
    std::accumulate(commandEvents_.begin(), commandEvents_.end(), 0,
                    accumulateEvent);
}

#ifdef ENABLE_ASYNC_DNS

SelectEventPoll::AsyncNameResolverEntry::AsyncNameResolverEntry
(const SharedHandle<AsyncNameResolver>& nameResolver, Command* command):
  nameResolver_(nameResolver), command_(command) {}

int SelectEventPoll::AsyncNameResolverEntry::getFds
(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  return nameResolver_->getFds(rfdsPtr, wfdsPtr);
}

void SelectEventPoll::AsyncNameResolverEntry::process
(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  nameResolver_->process(rfdsPtr, wfdsPtr);
  switch(nameResolver_->getStatus()) {
  case AsyncNameResolver::STATUS_SUCCESS:
  case AsyncNameResolver::STATUS_ERROR:
    command_->setStatusActive();
    break;
  default:
    break;
  }
}

#endif // ENABLE_ASYNC_DNS

SelectEventPoll::SelectEventPoll()
{
#ifdef __MINGW32__
  dummySocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  assert(dummySocket_ != (sock_t)-1);
#endif // __MINGW32__
  updateFdSet();
}

SelectEventPoll::~SelectEventPoll()
{
#ifdef __MINGW32__
  ::closesocket(dummySocket_);
#endif // __MINGW32__
}

void SelectEventPoll::poll(const struct timeval& tv)
{
  fd_set rfds;
  fd_set wfds;
  
  memcpy(&rfds, &rfdset_, sizeof(fd_set));
  memcpy(&wfds, &wfdset_, sizeof(fd_set));
#ifdef __MINGW32__
  fd_set efds;
  FD_ZERO(&efds);
  FD_SET(dummySocket_, &efds);
#endif // __MINGW32__
#ifdef ENABLE_ASYNC_DNS

  for(AsyncNameResolverEntrySet::iterator itr = nameResolverEntries_.begin(),
        eoi = nameResolverEntries_.end(); itr != eoi; ++itr) {
    const SharedHandle<AsyncNameResolverEntry>& entry = *itr;
    int fd = entry->getFds(&rfds, &wfds);
    // TODO force error if fd == 0
    if(fdmax_ < fd) {
      fdmax_ = fd;
    }
  }

#endif // ENABLE_ASYNC_DNS
  int retval;
  do {
    struct timeval ttv = tv;
#ifdef __MINGW32__
    retval = select(fdmax_+1, &rfds, &wfds, &efds, &ttv);
#else // !__MINGW32__
    retval = select(fdmax_+1, &rfds, &wfds, NULL, &ttv);
#endif // !__MINGW32__
  } while(retval == -1 && errno == EINTR);
  if(retval > 0) {
    for(SocketEntrySet::iterator i = socketEntries_.begin(),
          eoi = socketEntries_.end(); i != eoi; ++i) {
      int events = 0;
      if(FD_ISSET((*i)->getSocket(), &rfds)) {
        events |= EventPoll::EVENT_READ;
      }
      if(FD_ISSET((*i)->getSocket(), &wfds)) {
        events |= EventPoll::EVENT_WRITE;
      }
      (*i)->processEvents(events);
    }
  } else if(retval == -1) {
    int errNum = errno;
    A2_LOG_INFO(fmt("select error: %s", util::safeStrerror(errNum).c_str()));
  }
#ifdef ENABLE_ASYNC_DNS

  for(AsyncNameResolverEntrySet::iterator i = nameResolverEntries_.begin(),
        eoi = nameResolverEntries_.end(); i != eoi; ++i) {
    (*i)->process(&rfds, &wfds);
  }

#endif // ENABLE_ASYNC_DNS
}

#ifdef __MINGW32__
namespace {
void checkFdCountMingw(const fd_set& fdset)
{
  if(fdset.fd_count >= FD_SETSIZE) {
    A2_LOG_WARN("The number of file descriptor exceeded FD_SETSIZE. "
                "Download may slow down or fail.");
  }
}
} // namespace
#endif // __MINGW32__

void SelectEventPoll::updateFdSet()
{
#ifdef __MINGW32__
  fdmax_ = dummySocket_;
#else // !__MINGW32__
  fdmax_ = 0;
#endif // !__MINGW32__
  FD_ZERO(&rfdset_);
  FD_ZERO(&wfdset_);
  for(SocketEntrySet::iterator i = socketEntries_.begin(),
        eoi = socketEntries_.end(); i != eoi; ++i) {
    sock_t fd = (*i)->getSocket();
#ifndef __MINGW32__
    if(fd < 0 || FD_SETSIZE <= fd) {
      A2_LOG_WARN("Detected file descriptor >= FD_SETSIZE or < 0. "
                  "Download may slow down or fail.");
      continue;
    }
#endif // !__MINGW32__
    int events = (*i)->getEvents();
    if(events&EventPoll::EVENT_READ) {
#ifdef __MINGW32__
      checkFdCountMingw(rfdset_);
#endif // __MINGW32__
      FD_SET(fd, &rfdset_);
    }
    if(events&EventPoll::EVENT_WRITE) {
#ifdef __MINGW32__
      checkFdCountMingw(wfdset_);
#endif // __MINGW32__
      FD_SET(fd, &wfdset_);
    }
    if(fdmax_ < fd) {
      fdmax_ = fd;
    }
  }
}

bool SelectEventPoll::addEvents(sock_t socket, Command* command,
                                EventPoll::EventType events)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  SocketEntrySet::iterator i = socketEntries_.lower_bound(socketEntry);
  if(i != socketEntries_.end() && *(*i) == *socketEntry) {
    (*i)->addCommandEvent(command, events);
  } else {
    socketEntries_.insert(i, socketEntry);
    socketEntry->addCommandEvent(command, events);
  }
  updateFdSet();
  return true;
}

bool SelectEventPoll::deleteEvents(sock_t socket, Command* command,
                                   EventPoll::EventType events)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  SocketEntrySet::iterator i = socketEntries_.find(socketEntry);
  if(i == socketEntries_.end()) {
    A2_LOG_DEBUG(fmt("Socket %d is not found in SocketEntries.", socket));
    return false;
  } else {
    (*i)->removeCommandEvent(command, events);
    if((*i)->eventEmpty()) {
      socketEntries_.erase(i);
    }
    updateFdSet();
    return true;
  }
}

#ifdef ENABLE_ASYNC_DNS
bool SelectEventPoll::addNameResolver
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<AsyncNameResolverEntry> entry
    (new AsyncNameResolverEntry(resolver, command));
  AsyncNameResolverEntrySet::iterator itr =
    nameResolverEntries_.lower_bound(entry);
  if(itr != nameResolverEntries_.end() && *(*itr) == *entry) {
    return false;
  } else {
    nameResolverEntries_.insert(itr, entry);
    return true;
  }
}

bool SelectEventPoll::deleteNameResolver
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<AsyncNameResolverEntry> entry
    (new AsyncNameResolverEntry(resolver, command));
  return nameResolverEntries_.erase(entry) == 1;
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
