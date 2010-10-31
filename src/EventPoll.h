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
#ifndef D_EVENT_POLL_H
#define D_EVENT_POLL_H

#include "common.h"
#include "SharedHandle.h"
#include "a2time.h"
#include "a2netcompat.h"

namespace aria2 {

class SocketCore;
class Command;
class AsyncNameResolver;

class EventPoll {

public:
  enum EventType {
    EVENT_READ = 1,
    EVENT_WRITE = 1 << 1,
    EVENT_ERROR = 1 << 2,
    EVENT_HUP = 1 << 3,
  };

  virtual ~EventPoll() {}

  virtual void poll(const struct timeval& tv) = 0;

  virtual bool addEvents(sock_t socket, Command* command, EventType events) = 0;

  virtual bool deleteEvents(sock_t socket, Command* command,
                            EventType events) = 0;
#ifdef ENABLE_ASYNC_DNS

  virtual bool addNameResolver(const SharedHandle<AsyncNameResolver>& resolver,
                               Command* command) = 0;
  virtual bool deleteNameResolver
  (const SharedHandle<AsyncNameResolver>& resolver, Command* command) = 0;
#endif // ENABLE_ASYNC_DNS

};

} // namespace aria2

#endif // D_EVENT_POLL_H
