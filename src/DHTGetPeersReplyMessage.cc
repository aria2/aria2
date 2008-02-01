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
#include "DHTGetPeersReplyMessage.h"
#include "DHTNode.h"
#include "DHTBucket.h"
#include "Data.h"
#include "Dictionary.h"
#include "List.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "PeerMessageUtil.h"
#include "Peer.h"
#include "DHTUtil.h"

DHTGetPeersReplyMessage::DHTGetPeersReplyMessage(const DHTNodeHandle& localNode,
						 const DHTNodeHandle& remoteNode,
						 const string& token,
						 const string& transactionID):
  DHTResponseMessage(localNode, remoteNode, transactionID),
  _token(token) {}

DHTGetPeersReplyMessage::~DHTGetPeersReplyMessage() {}

void DHTGetPeersReplyMessage::doReceivedAction()
{
  // Returned peers and nodes are handled in DHTPeerLookupTask.
}

Dictionary* DHTGetPeersReplyMessage::getResponse()
{
  Dictionary* r = new Dictionary();
  r->put("id", new Data(reinterpret_cast<const char*>(_localNode->getID()),
			DHT_ID_LENGTH));
  r->put("token", new Data(_token));
  if(_values.size()) {
    List* valuesList = new List();
    r->put("values", valuesList);
    for(Peers::const_iterator i = _values.begin(); i != _values.end(); ++i) {
      PeerHandle peer = *i;
      char buffer[6];
      if(PeerMessageUtil::createcompact(buffer, peer->ipaddr, peer->port)) {
	valuesList->add(new Data(buffer, sizeof(buffer)));
      }
    }
  } else {
    size_t offset = 0;
    char buffer[DHTBucket::K*26];
    for(DHTNodes::const_iterator i = _closestKNodes.begin(); i != _closestKNodes.end(); ++i) {
      DHTNodeHandle node = *i;
      memcpy(buffer+offset, node->getID(), DHT_ID_LENGTH);
      if(PeerMessageUtil::createcompact(buffer+20+offset, node->getIPAddress(), node->getPort())) {
	offset += 26;
      }
    }
    r->put("nodes", new Data(buffer, offset));
  }
  return r;
}

string DHTGetPeersReplyMessage::getMessageType() const
{
  return "get_peers";
}

void DHTGetPeersReplyMessage::validate() const {}

const DHTNodes& DHTGetPeersReplyMessage::getClosestKNodes() const
{
  return _closestKNodes;
}

void DHTGetPeersReplyMessage::setClosestKNodes(const DHTNodes& closestKNodes)
{
  _closestKNodes = closestKNodes;
}

const Peers& DHTGetPeersReplyMessage::getValues() const
{
  return _values;
}

void DHTGetPeersReplyMessage::setValues(const Peers& peers)
{
  _values = peers;
}
