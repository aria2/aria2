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
#ifndef _D_DHT_MESSAGE_FACTORY_IMPL_H_
#define _D_DHT_MESSAGE_FACTORY_IMPL_H_

#include "DHTMessageFactory.h"
#include "A2STR.h"

namespace aria2 {

class DHTConnection;
class DHTMessageDispatcher;
class DHTRoutingTable;
class DHTPeerAnnounceStorage;
class DHTTokenTracker;
class Logger;
class DHTMessage;
class DHTAbstractMessage;

class DHTMessageFactoryImpl:public DHTMessageFactory {
private:
  SharedHandle<DHTNode> _localNode;

  WeakHandle<DHTConnection> _connection;

  WeakHandle<DHTMessageDispatcher> _dispatcher;

  WeakHandle<DHTRoutingTable> _routingTable;
  
  WeakHandle<DHTPeerAnnounceStorage> _peerAnnounceStorage;

  WeakHandle<DHTTokenTracker> _tokenTracker;

  Logger* _logger;
  
  // search node in routingTable. If it is not found, create new one.
  SharedHandle<DHTNode> getRemoteNode(const unsigned char* id, const std::string& ipaddr, uint16_t port) const;

  void validateID(const BDE& id) const;

  void validatePort(const BDE& i) const;

  std::deque<SharedHandle<DHTNode> > extractNodes(const unsigned char* src, size_t length);

  void setCommonProperty(const SharedHandle<DHTAbstractMessage>& m);

public:
  DHTMessageFactoryImpl();

  virtual ~DHTMessageFactoryImpl();

  virtual SharedHandle<DHTMessage>
  createQueryMessage(const BDE& dict,
                     const std::string& ipaddr, uint16_t port);

  virtual SharedHandle<DHTMessage>
  createResponseMessage(const std::string& messageType,
                        const BDE& dict,
                        const std::string& ipaddr, uint16_t port);

  virtual SharedHandle<DHTMessage>
  createPingMessage(const SharedHandle<DHTNode>& remoteNode,
                    const std::string& transactionID = A2STR::NIL);

  virtual SharedHandle<DHTMessage>
  createPingReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                         const unsigned char* id,
                         const std::string& transactionID);

  virtual SharedHandle<DHTMessage>
  createFindNodeMessage(const SharedHandle<DHTNode>& remoteNode,
                        const unsigned char* targetNodeID,
                        const std::string& transactionID = A2STR::NIL);

  SharedHandle<DHTMessage>
  createFindNodeReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                             const BDE& dict,
                             const std::string& transactionID);


  virtual SharedHandle<DHTMessage>
  createFindNodeReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                             const std::deque<SharedHandle<DHTNode> >& closestKNodes,
                             const std::string& transactionID);

  virtual SharedHandle<DHTMessage>
  createGetPeersMessage(const SharedHandle<DHTNode>& remoteNode,
                        const unsigned char* infoHash,
                        const std::string& transactionID = A2STR::NIL);

  virtual SharedHandle<DHTMessage>
  createGetPeersReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                             const std::deque<SharedHandle<DHTNode> >& closestKNodes,
                             const std::string& token,
                             const std::string& transactionID);

  SharedHandle<DHTMessage>
  createGetPeersReplyMessageWithNodes(const SharedHandle<DHTNode>& remoteNode,
                                      const BDE& dict,
                                      const std::string& transactionID);

  virtual SharedHandle<DHTMessage>
  createGetPeersReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                             const std::deque<SharedHandle<Peer> >& peers,
                             const std::string& token,
                             const std::string& transactionID);

  SharedHandle<DHTMessage>
  createGetPeersReplyMessageWithValues(const SharedHandle<DHTNode>& remoteNode,
                                       const BDE& dict,
                                       const std::string& transactionID);

  virtual SharedHandle<DHTMessage>
  createAnnouncePeerMessage(const SharedHandle<DHTNode>& remoteNode,
                            const unsigned char* infoHash,
                            uint16_t tcpPort,
                            const std::string& token,
                            const std::string& transactionID = A2STR::NIL);

  virtual SharedHandle<DHTMessage>
  createAnnouncePeerReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                                 const std::string& transactionID);

  virtual SharedHandle<DHTMessage>
  createUnknownMessage(const unsigned char* data, size_t length,
                       const std::string& ipaddr, uint16_t port);

  void setRoutingTable(const WeakHandle<DHTRoutingTable>& routingTable);

  void setConnection(const WeakHandle<DHTConnection>& connection);

  void setMessageDispatcher(const WeakHandle<DHTMessageDispatcher>& dispatcher);
  
  void setPeerAnnounceStorage(const WeakHandle<DHTPeerAnnounceStorage>& storage);

  void setTokenTracker(const WeakHandle<DHTTokenTracker>& tokenTracker);

  void setLocalNode(const SharedHandle<DHTNode>& localNode);
};

} // namespace aria2

#endif // _D_DHT_MESSAGE_FACTORY_IMPL_H_
