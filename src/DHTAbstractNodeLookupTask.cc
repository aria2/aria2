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
#include "DHTAbstractNodeLookupTask.h"
#include "DHTRoutingTable.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageFactory.h"
#include "DHTMessage.h"
#include "DHTNode.h"
#include "DHTNodeLookupEntry.h"
#include "DHTMessageCallbackImpl.h"
#include "DHTBucket.h"
#include "LogFactory.h"
#include "Util.h"
#include "DHTIDCloser.h"

DHTAbstractNodeLookupTask::DHTAbstractNodeLookupTask(const unsigned char* targetID):
  _inFlightMessage(0)
{
  memcpy(_targetID, targetID, DHT_ID_LENGTH);
}

void DHTAbstractNodeLookupTask::onReceived(const DHTMessageHandle& message)
{
  --_inFlightMessage;
  onReceivedInternal(message);
  DHTNodeLookupEntries newEntries = toEntries(getNodesFromMessage(message));
  size_t count = 0;
  for(DHTNodeLookupEntries::const_iterator i = newEntries.begin();
      i != newEntries.end(); ++i) {
    if(memcmp(_localNode->getID(), (*i)->_node->getID(), DHT_ID_LENGTH) != 0) {
      _entries.push_front(*i);
      ++count;
    }
  }
  _logger->debug("%u node lookup entries added.", count);
  stable_sort(_entries.begin(), _entries.end(), DHTIDCloser(_targetID));
  _entries.erase(unique(_entries.begin(), _entries.end()), _entries.end());
  _logger->debug("%u node lookup entries are unique.", _entries.size());
  if(_entries.size() > DHTBucket::K) {
    _entries.erase(_entries.begin()+DHTBucket::K, _entries.end());
  }
  if(needsAdditionalOutgoingMessage()) {
    sendMessage();
  }
  if(_inFlightMessage == 0) {
    _logger->debug("Finished node_lookup for node ID %s",
		   Util::toHex(_targetID, DHT_ID_LENGTH).c_str());
    onFinish();
    updateBucket();
    _finished = true;
  }
}

void DHTAbstractNodeLookupTask::onTimeout(const DHTNodeHandle& node)
{
  _logger->debug("node lookup message timeout for node ID=%s",
		 Util::toHex(node->getID(), DHT_ID_LENGTH).c_str());
  --_inFlightMessage;
  for(DHTNodeLookupEntries::iterator i = _entries.begin(); i != _entries.end(); ++i) {
    if((*i)->_node == node) {
      _entries.erase(i);
      break;
    }
  }
  if(needsAdditionalOutgoingMessage()) {
    sendMessage();
  }
  if(_inFlightMessage == 0) {
    _logger->debug("Finished node_lookup for node ID %s",
		   Util::toHex(_targetID, DHT_ID_LENGTH).c_str());
    onFinish();
    updateBucket();
    _finished = true;
  }  
}

void DHTAbstractNodeLookupTask::sendMessage()
{
  for(DHTNodeLookupEntries::iterator i = _entries.begin(); i != _entries.end() && _inFlightMessage < ALPHA; ++i) {
    if((*i)->_used == false) {
      ++_inFlightMessage;
      (*i)->_used = true;
      DHTMessageHandle m = createMessage((*i)->_node);
      _dispatcher->addMessageToQueue(m, new DHTMessageCallbackImpl(this));
    }
  }
}

void DHTAbstractNodeLookupTask::updateBucket()
{
  // TODO we have to something here?
}

void DHTAbstractNodeLookupTask::startup()
{
  _entries = toEntries(_routingTable->getClosestKNodes(_targetID));
  if(_entries.empty()) {
    _finished = true;
  } else {
    // TODO use RTT here
    _inFlightMessage = 0;
    sendMessage();
    if(_inFlightMessage == 0) {
      _logger->debug("No message was sent in this lookup stage. Finished.");
      _finished = true;
    }
  }
}

DHTNodeLookupEntries DHTAbstractNodeLookupTask::toEntries(const DHTNodes& nodes) const
{
  DHTNodeLookupEntries entries;
  for(DHTNodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i) {
    DHTNodeLookupEntryHandle e = new DHTNodeLookupEntry(*i);
    entries.push_back(e);
  }
  return entries;
}
