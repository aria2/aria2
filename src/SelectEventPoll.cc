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
#  include <cassert>
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

SelectEventPoll::CommandEvent::CommandEvent(Command* command, int events)
    : command_(command), events_(events)
{
}

void SelectEventPoll::CommandEvent::processEvents(int events)
{
  if ((events_ & events) ||
      ((EventPoll::EVENT_ERROR | EventPoll::EVENT_HUP) & events)) {
    command_->setStatusActive();
  }
  if (EventPoll::EVENT_READ & events) {
    command_->readEventReceived();
  }
  if (EventPoll::EVENT_WRITE & events) {
    command_->writeEventReceived();
  }
  if (EventPoll::EVENT_ERROR & events) {
    command_->errorEventReceived();
  }
  if (EventPoll::EVENT_HUP & events) {
    command_->hupEventReceived();
  }
}

SelectEventPoll::SocketEntry::SocketEntry(sock_t socket) : socket_(socket) {}

void SelectEventPoll::SocketEntry::addCommandEvent(Command* command, int events)
{
  CommandEvent cev(command, events);
  auto i = std::find(commandEvents_.begin(), commandEvents_.end(), cev);
  if (i == commandEvents_.end()) {
    commandEvents_.push_back(cev);
  }
  else {
    (*i).addEvents(events);
  }
}
void SelectEventPoll::SocketEntry::removeCommandEvent(Command* command,
                                                      int events)
{
  CommandEvent cev(command, events);
  auto i = std::find(commandEvents_.begin(), commandEvents_.end(), cev);
  if (i == commandEvents_.end()) {
    // not found
  }
  else {
    (*i).removeEvents(events);
    if ((*i).eventsEmpty()) {
      commandEvents_.erase(i);
    }
  }
}
void SelectEventPoll::SocketEntry::processEvents(int events)
{
  using namespace std::placeholders;
  std::for_each(commandEvents_.begin(), commandEvents_.end(),
                std::bind(&CommandEvent::processEvents, _1, events));
}

int accumulateEvent(int events, const SelectEventPoll::CommandEvent& event)
{
  return events | event.getEvents();
}

int SelectEventPoll::SocketEntry::getEvents()
{
  return std::accumulate(commandEvents_.begin(), commandEvents_.end(), 0,
                         accumulateEvent);
}

#ifdef ENABLE_ASYNC_DNS

SelectEventPoll::AsyncNameResolverEntry::AsyncNameResolverEntry(
    const std::shared_ptr<AsyncNameResolver>& nameResolver, Command* command)
    : nameResolver_(nameResolver), command_(command)
{
}

int SelectEventPoll::AsyncNameResolverEntry::getFds(fd_set* rfdsPtr,
                                                    fd_set* wfdsPtr)
{
  return nameResolver_->getFds(rfdsPtr, wfdsPtr);
}

void SelectEventPoll::AsyncNameResolverEntry::process(fd_set* rfdsPtr,
                                                      fd_set* wfdsPtr)
{
  nameResolver_->process(rfdsPtr, wfdsPtr);
  switch (nameResolver_->getStatus()) {
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
  memcpy(&efds, &wfdset_, sizeof(fd_set));
#endif // __MINGW32__

#ifdef ENABLE_ASYNC_DNS

  for (auto& i : nameResolverEntries_) {
    auto& entry = i.second;
    int fd = entry.getFds(&rfds, &wfds);
    // TODO force error if fd == 0
    if (fdmax_ < fd) {
      fdmax_ = fd;
    }
  }

#endif // ENABLE_ASYNC_DNS
  int retval;
  do {
    struct timeval ttv = tv;
#ifdef __MINGW32__
    // winsock will report non-blocking connect() errors in efds,
    // unlike posix, which will mark such sockets as writable.
    retval = select(fdmax_ + 1, &rfds, &wfds, &efds, &ttv);
#else  // !__MINGW32__
    retval = select(fdmax_ + 1, &rfds, &wfds, nullptr, &ttv);
#endif // !__MINGW32__
  } while (retval == -1 && errno == EINTR);
  if (retval > 0) {
    for (auto& i : socketEntries_) {
      auto& e = i.second;
      int events = 0;
      if (FD_ISSET(e.getSocket(), &rfds)) {
        events |= EventPoll::EVENT_READ;
      }
      if (FD_ISSET(e.getSocket(), &wfds)) {
        events |= EventPoll::EVENT_WRITE;
      }
#ifdef __MINGW32__
      if (FD_ISSET(e.getSocket(), &efds)) {
        events |= EventPoll::EVENT_ERROR;
      }
#endif // __MINGW32__
      e.processEvents(events);
    }
  }
  else if (retval == -1) {
    int errNum = errno;
    A2_LOG_INFO(fmt("select error: %s, fdmax: %d",
                    util::safeStrerror(errNum).c_str(), fdmax_));
  }
#ifdef ENABLE_ASYNC_DNS

  for (auto& i : nameResolverEntries_) {
    i.second.process(&rfds, &wfds);
  }

#endif // ENABLE_ASYNC_DNS
}

#ifdef __MINGW32__
namespace {
void checkFdCountMingw(const fd_set& fdset)
{
  if (fdset.fd_count >= FD_SETSIZE) {
    A2_LOG_WARN("The number of file descriptor exceeded FD_SETSIZE. "
                "Download may slow down or fail.");
  }
}
} // namespace
#endif // __MINGW32__

void SelectEventPoll::updateFdSet()
{
  FD_ZERO(&rfdset_);
  FD_ZERO(&wfdset_);
#ifdef __MINGW32__
  FD_SET(dummySocket_, &rfdset_);
  FD_SET(dummySocket_, &wfdset_);
  fdmax_ = dummySocket_;
#else  // !__MINGW32__
  fdmax_ = 0;
#endif // !__MINGW32__

  for (auto& i : socketEntries_) {
    auto& e = i.second;
    sock_t fd = e.getSocket();
#ifndef __MINGW32__
    if (fd < 0 || FD_SETSIZE <= fd) {
      A2_LOG_WARN("Detected file descriptor >= FD_SETSIZE or < 0. "
                  "Download may slow down or fail.");
      continue;
    }
#endif // !__MINGW32__
    int events = e.getEvents();
    if (events & EventPoll::EVENT_READ) {
#ifdef __MINGW32__
      checkFdCountMingw(rfdset_);
#endif // __MINGW32__
      FD_SET(fd, &rfdset_);
    }
    if (events & EventPoll::EVENT_WRITE) {
#ifdef __MINGW32__
      checkFdCountMingw(wfdset_);
#endif // __MINGW32__
      FD_SET(fd, &wfdset_);
    }
    if (fdmax_ < fd) {
      fdmax_ = fd;
    }
  }
}

bool SelectEventPoll::addEvents(sock_t socket, Command* command,
                                EventPoll::EventType events)
{
  auto i = socketEntries_.lower_bound(socket);
  if (i != std::end(socketEntries_) && (*i).first == socket) {
    (*i).second.addCommandEvent(command, events);
  }
  else {
    i = socketEntries_.insert(i, std::make_pair(socket, SocketEntry(socket)));
    (*i).second.addCommandEvent(command, events);
  }
  updateFdSet();
  return true;
}

bool SelectEventPoll::deleteEvents(sock_t socket, Command* command,
                                   EventPoll::EventType events)
{
  auto i = socketEntries_.find(socket);
  if (i == std::end(socketEntries_)) {
    A2_LOG_DEBUG(fmt("Socket %d is not found in SocketEntries.", socket));
    return false;
  }

  auto& socketEntry = (*i).second;
  socketEntry.removeCommandEvent(command, events);
  if (socketEntry.eventEmpty()) {
    socketEntries_.erase(i);
  }
  updateFdSet();
  return true;
}

#ifdef ENABLE_ASYNC_DNS
bool SelectEventPoll::addNameResolver(
    const std::shared_ptr<AsyncNameResolver>& resolver, Command* command)
{
  auto key = std::make_pair(resolver.get(), command);
  auto itr = nameResolverEntries_.lower_bound(key);
  if (itr != std::end(nameResolverEntries_) && (*itr).first == key) {
    return false;
  }

  nameResolverEntries_.insert(
      itr, std::make_pair(key, AsyncNameResolverEntry(resolver, command)));

  return true;
}

bool SelectEventPoll::deleteNameResolver(
    const std::shared_ptr<AsyncNameResolver>& resolver, Command* command)
{
  auto key = std::make_pair(resolver.get(), command);
  return nameResolverEntries_.erase(key) == 1;
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
