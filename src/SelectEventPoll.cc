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
#include "SelectEventPoll.h"

#include <cstring>
#include <algorithm>
#include <numeric>
#include <iostream>

#include "Command.h"
#include "LogFactory.h"
#include "Logger.h"

namespace aria2 {

SelectEventPoll::CommandEvent::CommandEvent(Command* command, int events):
  _command(command), _events(events) {}

void SelectEventPoll::CommandEvent::processEvents(int events)
{
  if((_events&events) ||
     ((EventPoll::EVENT_ERROR|EventPoll::EVENT_HUP)&events)) {
    _command->setStatusActive();
  }
  if(EventPoll::EVENT_READ&events) {
    _command->readEventReceived();
  }
  if(EventPoll::EVENT_WRITE&events) {
    _command->writeEventReceived();
  }
  if(EventPoll::EVENT_ERROR&events) {
    _command->errorEventReceived();
  }
  if(EventPoll::EVENT_HUP&events) {
    _command->hupEventReceived();
  }
}

SelectEventPoll::SocketEntry::SocketEntry(sock_t socket):_socket(socket) {}

void SelectEventPoll::SocketEntry::addCommandEvent
(Command* command, int events)
{
  CommandEvent cev(command, events);
  std::deque<CommandEvent>::iterator i = std::find(_commandEvents.begin(),
                                                   _commandEvents.end(),
                                                   cev);
  if(i == _commandEvents.end()) {
    _commandEvents.push_back(cev);
  } else {
    (*i).addEvents(events);
  }
}
void SelectEventPoll::SocketEntry::removeCommandEvent
(Command* command, int events)
{
  CommandEvent cev(command, events);
  std::deque<CommandEvent>::iterator i = std::find(_commandEvents.begin(),
                                                   _commandEvents.end(),
                                                   cev);
  if(i == _commandEvents.end()) {
    // not found
  } else {
    (*i).removeEvents(events);
    if((*i).eventsEmpty()) {
      _commandEvents.erase(i);
    }
  }
}
void SelectEventPoll::SocketEntry::processEvents(int events)
{
  std::for_each(_commandEvents.begin(), _commandEvents.end(),
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
    std::accumulate(_commandEvents.begin(), _commandEvents.end(), 0,
		    accumulateEvent);
}

#ifdef ENABLE_ASYNC_DNS

SelectEventPoll::AsyncNameResolverEntry::AsyncNameResolverEntry
(const SharedHandle<AsyncNameResolver>& nameResolver, Command* command):
  _nameResolver(nameResolver), _command(command) {}

int SelectEventPoll::AsyncNameResolverEntry::getFds
(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  return _nameResolver->getFds(rfdsPtr, wfdsPtr);
}

void SelectEventPoll::AsyncNameResolverEntry::process
(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  _nameResolver->process(rfdsPtr, wfdsPtr);
  switch(_nameResolver->getStatus()) {
  case AsyncNameResolver::STATUS_SUCCESS:
  case AsyncNameResolver::STATUS_ERROR:
    _command->setStatusActive();
    break;
  default:
    break;
  }
}

#endif // ENABLE_ASYNC_DNS

SelectEventPoll::SelectEventPoll():_logger(LogFactory::getInstance())
{
  updateFdSet();
}

SelectEventPoll::~SelectEventPoll() {}

void SelectEventPoll::poll(const struct timeval& tv)
{
  fd_set rfds;
  fd_set wfds;
  
  memcpy(&rfds, &_rfdset, sizeof(fd_set));
  memcpy(&wfds, &_wfdset, sizeof(fd_set));
#ifdef ENABLE_ASYNC_DNS

  for(std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator itr =
        _nameResolverEntries.begin(); itr != _nameResolverEntries.end();
      ++itr) {
    SharedHandle<AsyncNameResolverEntry>& entry = *itr;
    int fd = entry->getFds(&rfds, &wfds);
    // TODO force error if fd == 0
    if(_fdmax < fd) {
      _fdmax = fd;
    }
  }

#endif // ENABLE_ASYNC_DNS
  int retval;
  do {
    struct timeval ttv = tv;
    retval = select(_fdmax+1, &rfds, &wfds, NULL, &ttv);
  } while(retval == -1 && errno == EINTR);
  if(retval > 0) {
    for(std::deque<SharedHandle<SocketEntry> >::iterator i =
          _socketEntries.begin(); i != _socketEntries.end(); ++i) {
      int events = 0;
      if(FD_ISSET((*i)->getSocket(), &rfds)) {
        events |= EventPoll::EVENT_READ;
      }
      if(FD_ISSET((*i)->getSocket(), &wfds)) {
        events |= EventPoll::EVENT_WRITE;
      }
      (*i)->processEvents(events);
    }
  }
#ifdef ENABLE_ASYNC_DNS

  for(std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator i =
        _nameResolverEntries.begin(); i != _nameResolverEntries.end(); ++i) {
    (*i)->process(&rfds, &wfds);
  }

#endif // ENABLE_ASYNC_DNS
}

void SelectEventPoll::updateFdSet()
{
  _fdmax = 0;
  FD_ZERO(&_rfdset);
  FD_ZERO(&_wfdset);
  for(std::deque<SharedHandle<SocketEntry> >::iterator i =
        _socketEntries.begin(); i != _socketEntries.end(); ++i) {
    sock_t fd = (*i)->getSocket();
    int events = (*i)->getEvents();
    if(events&EventPoll::EVENT_READ) {
      FD_SET(fd, &_rfdset);
    }
    if(events&EventPoll::EVENT_WRITE) {
      FD_SET(fd, &_wfdset);
    }
    if(_fdmax < fd) {
      _fdmax = fd;
    }
  }
}

bool SelectEventPoll::addEvents(sock_t socket, Command* command,
				EventPoll::EventType events)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  std::deque<SharedHandle<SocketEntry> >::iterator i =
    std::lower_bound(_socketEntries.begin(), _socketEntries.end(), socketEntry);
  int r = 0;
  if(i != _socketEntries.end() && (*i) == socketEntry) {
    (*i)->addCommandEvent(command, events);
  } else {
    _socketEntries.insert(i, socketEntry);
    socketEntry->addCommandEvent(command, events);
  }
  updateFdSet();
  if(r == -1) {
    _logger->debug("Failed to add socket event %d:%s", socket, strerror(errno));
    return false;
  } else {
    return true;
  }
}

bool SelectEventPoll::deleteEvents(sock_t socket, Command* command,
				   EventPoll::EventType events)
{
  SharedHandle<SocketEntry> socketEntry(new SocketEntry(socket));
  std::deque<SharedHandle<SocketEntry> >::iterator i =
    std::lower_bound(_socketEntries.begin(), _socketEntries.end(), socketEntry);
  if(i != _socketEntries.end() && (*i) == socketEntry) {
    (*i)->removeCommandEvent(command, events);
    int r = 0;
    if((*i)->eventEmpty()) {
      _socketEntries.erase(i);
    }
    updateFdSet();
    if(r == -1) {
      _logger->debug("Failed to delete socket event:%s", strerror(errno));
      return false;
    } else {
      return true;
    }
  } else {
    _logger->debug("Socket %d is not found in SocketEntries.", socket);
    return false;
  }
}

#ifdef ENABLE_ASYNC_DNS
bool SelectEventPoll::addNameResolver
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<AsyncNameResolverEntry> entry
    (new AsyncNameResolverEntry(resolver, command));
  std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator itr =
    std::find(_nameResolverEntries.begin(), _nameResolverEntries.end(), entry);
  if(itr == _nameResolverEntries.end()) {
    _nameResolverEntries.push_back(entry);
    return true;
  } else {
    return false;
  }
}

bool SelectEventPoll::deleteNameResolver
(const SharedHandle<AsyncNameResolver>& resolver, Command* command)
{
  SharedHandle<AsyncNameResolverEntry> entry
    (new AsyncNameResolverEntry(resolver, command));
  std::deque<SharedHandle<AsyncNameResolverEntry> >::iterator itr =
    std::find(_nameResolverEntries.begin(), _nameResolverEntries.end(), entry);
  if(itr == _nameResolverEntries.end()) {
    return false;
  } else {
    _nameResolverEntries.erase(itr);
    return true;
  }
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
