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
#include "DHTAutoSaveCommand.h"

#include <cstring>

#include "DHTRoutingTable.h"
#include "DHTNode.h"
#include "File.h"
#include "DHTRoutingTableSerializer.h"
#include "RecoverableException.h"
#include "DownloadEngine.h"
#include "DHTBucket.h"
#include "RequestGroupMan.h"
#include "prefs.h"
#include "Option.h"
#include "message.h"
#include "Logger.h"
#include "LogFactory.h"
#include "a2functional.h"
#include "FileEntry.h"
#include "DlAbortEx.h"
#include "fmt.h"

namespace aria2 {

DHTAutoSaveCommand::DHTAutoSaveCommand(cuid_t cuid, DownloadEngine* e,
                                       int family,
                                       std::chrono::seconds interval)
    : TimeBasedCommand{cuid, e, std::move(interval)},
      family_{family},
      routingTable_{nullptr}
{
}

DHTAutoSaveCommand::~DHTAutoSaveCommand() = default;

void DHTAutoSaveCommand::preProcess()
{
  if (getDownloadEngine()->getRequestGroupMan()->downloadFinished() ||
      getDownloadEngine()->isHaltRequested()) {
    save();
    enableExit();
  }
}

void DHTAutoSaveCommand::process() { save(); }

void DHTAutoSaveCommand::save()
{
  std::string dhtFile = getDownloadEngine()->getOption()->get(
      family_ == AF_INET ? PREF_DHT_FILE_PATH : PREF_DHT_FILE_PATH6);
  A2_LOG_INFO(fmt("Saving DHT routing table to %s.", dhtFile.c_str()));

  // Removing tempFile is unnecessary because the file is truncated on
  // open.  But the bug in 1.10.4 creates directory for this path.
  // Because it is directory, opening directory as file fails.  So we
  // first remove it here.
  File tempFile(dhtFile + "__temp");
  tempFile.remove();

  File(File(dhtFile).getDirname()).mkdirs();
  std::vector<std::shared_ptr<DHTNode>> nodes;
  std::vector<std::shared_ptr<DHTBucket>> buckets;
  routingTable_->getBuckets(buckets);
  for (const auto& b : buckets) {
    std::vector<std::shared_ptr<DHTNode>> goodNodes;
    b->getGoodNodes(goodNodes);
    nodes.insert(nodes.end(), goodNodes.begin(), goodNodes.end());
  }

  DHTRoutingTableSerializer serializer(family_);
  serializer.setLocalNode(localNode_);
  serializer.setNodes(nodes);

  try {
    serializer.serialize(dhtFile);
  }
  catch (RecoverableException& e) {
    A2_LOG_ERROR_EX(fmt("Exception caught while saving DHT routing table to %s",
                        dhtFile.c_str()),
                    e);
  }
}

void DHTAutoSaveCommand::setLocalNode(const std::shared_ptr<DHTNode>& localNode)
{
  localNode_ = localNode;
}

void DHTAutoSaveCommand::setRoutingTable(DHTRoutingTable* routingTable)
{
  routingTable_ = routingTable;
}

} // namespace aria2
