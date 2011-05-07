/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
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
#ifndef D_DHT_ENTRY_POINT_NAME_RESOVE_COMMAND_H
#define D_DHT_ENTRY_POINT_NAME_RESOVE_COMMAND_H

#include "Command.h"

#include <utility>
#include <vector>
#include <deque>
#include <string>

#include "SharedHandle.h"

namespace aria2 {

class DHTTaskQueue;
class DHTTaskFactory;
class DHTRoutingTable;
class DHTNode;
class DownloadEngine;
#ifdef ENABLE_ASYNC_DNS
class AsyncNameResolver;
#endif // ENABLE_ASYNC_DNS

class DHTEntryPointNameResolveCommand:public Command {
private:
  DownloadEngine* e_;

#ifdef ENABLE_ASYNC_DNS
  SharedHandle<AsyncNameResolver> resolver_;
#endif // ENABLE_ASYNC_DNS

  SharedHandle<DHTTaskQueue> taskQueue_;

  SharedHandle<DHTTaskFactory> taskFactory_;

  SharedHandle<DHTRoutingTable> routingTable_;

  SharedHandle<DHTNode> localNode_;

  std::deque<std::pair<std::string, uint16_t> > entryPoints_;

  std::vector<std::pair<std::string, uint16_t> > resolvedEntryPoints_;

  bool bootstrapEnabled_;

  void addPingTask(const std::pair<std::string, uint16_t>& addr);

#ifdef ENABLE_ASYNC_DNS
  bool resolveHostname(const std::string& hostname,
                       const SharedHandle<AsyncNameResolver>& resolver);

  void setNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver);

  void disableNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver);
#endif // ENABLE_ASYNC_DNS

public:
  DHTEntryPointNameResolveCommand
  (cuid_t cuid, DownloadEngine* e,
   const std::vector<std::pair<std:: string, uint16_t> >& entryPoints);

  virtual ~DHTEntryPointNameResolveCommand();

  virtual bool execute();

  void setBootstrapEnabled(bool f);

  void setTaskQueue(const SharedHandle<DHTTaskQueue>& taskQueue);

  void setTaskFactory(const SharedHandle<DHTTaskFactory>& taskFactory);

  void setRoutingTable(const SharedHandle<DHTRoutingTable>& routingTable);

  void setLocalNode(const SharedHandle<DHTNode>& localNode);
};

} // namespace aria2

#endif // D_DHT_ENTRY_POINT_NAME_RESOVE_COMMAND_H
