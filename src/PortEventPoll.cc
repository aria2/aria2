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
#include "PortEventPoll.h"

#include <cerrno>
#include <cstring>
#include <algorithm>
#include <numeric>

#include "Command.h"
#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

PortEventPoll::KSocketEntry::KSocketEntry(sock_t s)
    : SocketEntry<KCommandEvent, KADNSEvent>(s)
{
}

int accumulateEvent(int events, const PortEventPoll::KEvent& event)
{
  return events | event.getEvents();
}

PortEventPoll::A2PortEvent PortEventPoll::KSocketEntry::getEvents()
{
  A2PortEvent portEvent;
  portEvent.socketEntry = this;
#ifdef ENABLE_ASYNC_DNS
  portEvent.events =
      std::accumulate(adnsEvents_.begin(), adnsEvents_.end(),
                      std::accumulate(commandEvents_.begin(),
                                      commandEvents_.end(), 0, accumulateEvent),
                      accumulateEvent);
#else // !ENABLE_ASYNC_DNS
  portEvent.events = std::accumulate(commandEvents_.begin(),
                                     commandEvents_.end(), 0, accumulateEvent);

#endif // !ENABLE_ASYNC_DNS
  return portEvent;
}

PortEventPoll::PortEventPoll()
    : portEventsSize_(PORT_EVENTS_SIZE),
      portEvents_(new port_event_t[portEventsSize_])
{
  port_ = port_create();
}

PortEventPoll::~PortEventPoll()
{
  if (port_ != -1) {
    int r = close(port_);
    int errNum = errno;
    if (r == -1) {
      A2_LOG_ERROR(fmt("Error occurred while closing port %d: %s", port_,
                       util::safeStrerror(errNum).c_str()));
    }
  }
  delete[] portEvents_;
}

bool PortEventPoll::good() const { return port_ != -1; }

void PortEventPoll::poll(const struct timeval& tv)
{
  struct timespec timeout = {tv.tv_sec, tv.tv_usec * 1000};
  int res;
  uint_t nget = 1;
  // If port_getn was interrupted by signal, it can consume events but
  // not updat nget!. For this very annoying bug, we have to check
  // actually event is filled or not.
  portEvents_[0].portev_user = (void*)-1;
  res = port_getn(port_, portEvents_, portEventsSize_, &nget, &timeout);
  if (res == 0 || (res == -1 && (errno == ETIME || errno == EINTR) &&
                   portEvents_[0].portev_user != (void*)-1)) {
    A2_LOG_DEBUG(fmt("nget=%u", nget));
    for (uint_t i = 0; i < nget; ++i) {
      const port_event_t& pev = portEvents_[i];
      KSocketEntry* p = reinterpret_cast<KSocketEntry*>(pev.portev_user);
      p->processEvents(pev.portev_events);
      int r = port_associate(port_, PORT_SOURCE_FD, pev.portev_object,
                             p->getEvents().events, p);
      int errNum = errno;
      if (r == -1) {
        A2_LOG_INFO(fmt("port_associate failed for file descriptor %d:"
                        " cause %s",
                        pev.portev_object, util::safeStrerror(errNum).c_str()));
      }
    }
  }
  else if (res == -1) {
    int errNum = errno;
    A2_LOG_INFO(fmt("port_getn error: %s", util::safeStrerror(errNum).c_str()));
  }
#ifdef ENABLE_ASYNC_DNS
  // It turns out that we have to call ares_process_fd before ares's
  // own timeout and ares may create new sockets or closes socket in
  // their API. So we call ares_process_fd for all ares_channel and
  // re-register their sockets.
  for (KAsyncNameResolverEntrySet::iterator i = nameResolverEntries_.begin(),
                                            eoi = nameResolverEntries_.end();
       i != eoi; ++i) {
    (*i)->processTimeout();
    (*i)->removeSocketEvents(this);
    (*i)->addSocketEvents(this);
  }
#endif // ENABLE_ASYNC_DNS

  // TODO timeout of name resolver is determined in Command(AbstractCommand,
  // DHTEntryPoint...Command)
}

namespace {
int translateEvents(EventPoll::EventType events)
{
  int newEvents = 0;
  if (EventPoll::EVENT_READ & events) {
    newEvents |= PortEventPoll::IEV_READ;
  }
  if (EventPoll::EVENT_WRITE & events) {
    newEvents |= PortEventPoll::IEV_WRITE;
  }
  if (EventPoll::EVENT_ERROR & events) {
    newEvents |= PortEventPoll::IEV_ERROR;
  }
  if (EventPoll::EVENT_HUP & events) {
    newEvents |= PortEventPoll::IEV_HUP;
  }
  return newEvents;
}
} // namespace

bool PortEventPoll::addEvents(sock_t socket, const PortEventPoll::KEvent& event)
{
  auto socketEntry = std::make_shared<KSocketEntry>(socket);
  KSocketEntrySet::iterator i = socketEntries_.lower_bound(socketEntry);
  int r = 0;
  int errNum = 0;
  if (i != socketEntries_.end() && *(*i) == *socketEntry) {
    event.addSelf((*i).get());
    A2PortEvent pv = (*i)->getEvents();
    r = port_associate(port_, PORT_SOURCE_FD, (*i)->getSocket(), pv.events,
                       pv.socketEntry);
    errNum = r;
  }
  else {
    socketEntries_.insert(i, socketEntry);
    if (socketEntries_.size() > portEventsSize_) {
      portEventsSize_ *= 2;
      delete[] portEvents_;
      portEvents_ = new port_event_t[portEventsSize_];
    }
    event.addSelf(socketEntry.get());
    A2PortEvent pv = socketEntry->getEvents();
    r = port_associate(port_, PORT_SOURCE_FD, socketEntry->getSocket(),
                       pv.events, pv.socketEntry);
    errNum = r;
  }
  if (r == -1) {
    A2_LOG_DEBUG(fmt("Failed to add socket event %d:%s", socket,
                     util::safeStrerror(errNum).c_str()));
    return false;
  }
  else {
    return true;
  }
}

bool PortEventPoll::addEvents(sock_t socket, Command* command,
                              EventPoll::EventType events)
{
  int portEvents = translateEvents(events);
  return addEvents(socket, KCommandEvent(command, portEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool PortEventPoll::addEvents(sock_t socket, Command* command, int events,
                              const std::shared_ptr<AsyncNameResolver>& rs)
{
  return addEvents(socket, KADNSEvent(rs, command, socket, events));
}
#endif // ENABLE_ASYNC_DNS

bool PortEventPoll::deleteEvents(sock_t socket,
                                 const PortEventPoll::KEvent& event)
{
  auto socketEntry = std::make_shared<KSocketEntry>(socket);
  KSocketEntrySet::iterator i = socketEntries_.find(socketEntry);
  if (i == socketEntries_.end()) {
    A2_LOG_DEBUG(fmt("Socket %d is not found in SocketEntries.", socket));
    return false;
  }
  else {
    event.removeSelf((*i).get());
    int r = 0;
    int errNum = 0;
    if ((*i)->eventEmpty()) {
      r = port_dissociate(port_, PORT_SOURCE_FD, (*i)->getSocket());
      errNum = errno;
      socketEntries_.erase(i);
    }
    else {
      A2PortEvent pv = (*i)->getEvents();
      r = port_associate(port_, PORT_SOURCE_FD, (*i)->getSocket(), pv.events,
                         pv.socketEntry);
      errNum = errno;
    }
    if (r == -1) {
      A2_LOG_DEBUG(fmt("Failed to delete socket event:%s",
                       util::safeStrerror(errNum).c_str()));
      return false;
    }
    else {
      return true;
    }
  }
}

#ifdef ENABLE_ASYNC_DNS
bool PortEventPoll::deleteEvents(sock_t socket, Command* command,
                                 const std::shared_ptr<AsyncNameResolver>& rs)
{
  return deleteEvents(socket, KADNSEvent(rs, command, socket, 0));
}
#endif // ENABLE_ASYNC_DNS

bool PortEventPoll::deleteEvents(sock_t socket, Command* command,
                                 EventPoll::EventType events)
{
  int portEvents = translateEvents(events);
  return deleteEvents(socket, KCommandEvent(command, portEvents));
}

#ifdef ENABLE_ASYNC_DNS
bool PortEventPoll::addNameResolver(
    const std::shared_ptr<AsyncNameResolver>& resolver, Command* command)
{
  auto entry = std::make_shared<KAsyncNameResolverEntry>(resolver, command);
  KAsyncNameResolverEntrySet::iterator itr = nameResolverEntries_.find(entry);
  if (itr == nameResolverEntries_.end()) {
    nameResolverEntries_.insert(entry);
    entry->addSocketEvents(this);
    return true;
  }
  else {
    return false;
  }
}

bool PortEventPoll::deleteNameResolver(
    const std::shared_ptr<AsyncNameResolver>& resolver, Command* command)
{
  auto entry = std::make_shared<KAsyncNameResolverEntry>(resolver, command);
  KAsyncNameResolverEntrySet::iterator itr = nameResolverEntries_.find(entry);
  if (itr == nameResolverEntries_.end()) {
    return false;
  }
  else {
    (*itr)->removeSocketEvents(this);
    nameResolverEntries_.erase(itr);
    return true;
  }
}
#endif // ENABLE_ASYNC_DNS

} // namespace aria2
