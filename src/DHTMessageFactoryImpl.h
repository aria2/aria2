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
#ifndef _D_DHT_MESSAGE_FACTORY_IMPL_H_
#define _D_DHT_MESSAGE_FACTORY_IMPL_H_

#include "DHTMessageFactory.h"
#include "DHTRoutingTableDecl.h"
#include "DHTConnectionDecl.h"
#include "DHTMessageDispatcherDecl.h"
#include "DHTPeerAnnounceStorageDecl.h"
#include "DHTTokenTrackerDecl.h"

class Logger;
class Data;
class DHTAbstractMessage;

class DHTMessageFactoryImpl:public DHTMessageFactory {
private:
  DHTNodeHandle _localNode;

  WeakHandle<DHTConnection> _connection;

  WeakHandle<DHTMessageDispatcher> _dispatcher;

  WeakHandle<DHTRoutingTable> _routingTable;
  
  WeakHandle<DHTPeerAnnounceStorage> _peerAnnounceStorage;

  WeakHandle<DHTTokenTracker> _tokenTracker;

  const Logger* _logger;
  
  // search node in routingTable. If it is not found, create new one.
  DHTNodeHandle getRemoteNode(const unsigned char* id, const string& ipaddr, uint16_t port) const;

  void validateID(const Data* id) const;

  void validateIDMatch(const unsigned char* expected, const unsigned char* actual) const;

  void validatePort(const Data* i) const;

  DHTNodes extractNodes(const char* src, size_t length);

  void setCommonProperty(const SharedHandle<DHTAbstractMessage>& m);

public:
  DHTMessageFactoryImpl();

  virtual ~DHTMessageFactoryImpl();

  virtual DHTMessageHandle
  createQueryMessage(const Dictionary* d,
		     const string& ipaddr, uint16_t port);

  virtual DHTMessageHandle
  createResponseMessage(const string& messageType,
			const Dictionary* d,
			const DHTNodeHandle& remoteNode);

  virtual DHTMessageHandle
  createPingMessage(const DHTNodeHandle& remoteNode,
		    const string& transactionID = "");

  virtual DHTMessageHandle
  createPingReplyMessage(const DHTNodeHandle& remoteNode,
			 const unsigned char* id,
			 const string& transactionID);

  virtual DHTMessageHandle
  createFindNodeMessage(const DHTNodeHandle& remoteNode,
			const unsigned char* targetNodeID,
			const string& transactionID = "");

  DHTMessageHandle
  createFindNodeReplyMessage(const DHTNodeHandle& remoteNode,
			     const Dictionary* d,
			     const string& transactionID);


  virtual DHTMessageHandle
  createFindNodeReplyMessage(const DHTNodeHandle& remoteNode,
			     const DHTNodes& closestKNodes,
			     const string& transactionID);

  virtual DHTMessageHandle
  createGetPeersMessage(const DHTNodeHandle& remoteNode,
			const unsigned char* infoHash,
			const string& transactionID = "");

  virtual DHTMessageHandle
  createGetPeersReplyMessage(const DHTNodeHandle& remoteNode,
			     const DHTNodes& closestKNodes,
			     const string& token,
			     const string& transactionID);

  DHTMessageHandle
  createGetPeersReplyMessageWithNodes(const DHTNodeHandle& remoteNode,
				      const Dictionary* d,
				      const string& transactionID);

  virtual DHTMessageHandle
  createGetPeersReplyMessage(const DHTNodeHandle& remoteNode,
			     const Peers& peers,
			     const string& token,
			     const string& transactionID);

  DHTMessageHandle
  createGetPeersReplyMessageWithValues(const DHTNodeHandle& remoteNode,
				       const Dictionary* d,
				       const string& transactionID);

  virtual DHTMessageHandle
  createAnnouncePeerMessage(const DHTNodeHandle& remoteNode,
			    const unsigned char* infoHash,
			    uint16_t tcpPort,
			    const string& token,
			    const string& transactionID = "");

  virtual DHTMessageHandle
  createAnnouncePeerReplyMessage(const DHTNodeHandle& remoteNode,
				 const string& transactionID);

  void setRoutingTable(const DHTRoutingTableHandle& routingTable);

  void setConnection(const DHTConnectionHandle& connection);

  void setMessageDispatcher(const DHTMessageDispatcherHandle& dispatcher);
  
  void setPeerAnnounceStorage(const DHTPeerAnnounceStorageHandle& storage);

  void setTokenTracker(const DHTTokenTrackerHandle& tokenTracker);

  void setLocalNode(const DHTNodeHandle& localNode);
};

#endif // _D_DHT_MESSAGE_FACTORY_IMPL_H_
