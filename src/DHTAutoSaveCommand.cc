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
#include "DHTAutoSaveCommand.h"
#include "DHTRoutingTable.h"
#include "DHTNode.h"
#include "File.h"
#include "DHTRoutingTableSerializer.h"
#include "RecoverableException.h"
#include "DownloadEngine.h"
#include "Util.h"
#include "DHTBucket.h"
#include "RequestGroupMan.h"
#include "prefs.h"
#include "Option.h"
#include <fstream>

DHTAutoSaveCommand::DHTAutoSaveCommand(int32_t cuid, DownloadEngine* e, int32_t interval):
  TimeBasedCommand(cuid, e, interval),
  _localNode(0),
  _routingTable(0) {}

DHTAutoSaveCommand::~DHTAutoSaveCommand() {}

void DHTAutoSaveCommand::preProcess()
{
  if(_e->_requestGroupMan->downloadFinished() || _e->isHaltRequested()) {
    save();
    _exit = true;
  }
}

void DHTAutoSaveCommand::process()
{
  save();
}

void DHTAutoSaveCommand::save()
{
  DHTNodes nodes;
  DHTBuckets buckets = _routingTable->getBuckets();
  for(DHTBuckets::const_iterator i = buckets.begin(); i != buckets.end(); ++i) {
    const DHTBucketHandle& bucket = *i;
    DHTNodes goodNodes = bucket->getGoodNodes();
    nodes.insert(nodes.end(), goodNodes.begin(), goodNodes.end());
  }

  DHTRoutingTableSerializer serializer;
  serializer.setLocalNode(_localNode);
  serializer.setNodes(nodes);

  string dhtFile = _e->option->get(PREF_DHT_FILE_PATH);
  string tempFile = dhtFile+"__temp";
  ofstream o(tempFile.c_str(), ios::out|ios::binary);
  o.exceptions(ios::failbit);
  try {
    serializer.serialize(o);

    if(!File(tempFile).renameTo(dhtFile)) {
      logger->error("Cannot move file from %s to %s.",
		    tempFile.c_str(), dhtFile.c_str());
    }
  } catch(RecoverableException* e) {
    logger->error("Exception caught while saving DHT routing table to %s",
		  e, tempFile.c_str());
    delete e;
  }
}

void DHTAutoSaveCommand::setLocalNode(const DHTNodeHandle& localNode)
{
  _localNode = localNode;
}

void DHTAutoSaveCommand::setRoutingTable(const DHTRoutingTableHandle& routingTable)
{
  _routingTable = routingTable;
}
