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
#ifndef D_DHT_MESSAGE_FACTORY_IMPL_H
#define D_DHT_MESSAGE_FACTORY_IMPL_H

#include "DHTMessageFactory.h"
#include "A2STR.h"

namespace aria2 {

class DHTConnection;
class DHTMessageDispatcher;
class DHTRoutingTable;
class DHTPeerAnnounceStorage;
class DHTTokenTracker;
class DHTMessage;
class DHTAbstractMessage;
class BtRegistry;

class DHTMessageFactoryImpl : public DHTMessageFactory {
private:
  int family_;

  std::shared_ptr<DHTNode> localNode_;

  DHTConnection* connection_;

  DHTMessageDispatcher* dispatcher_;

  DHTRoutingTable* routingTable_;

  DHTPeerAnnounceStorage* peerAnnounceStorage_;

  DHTTokenTracker* tokenTracker_;

  BtRegistry* btRegistry_;

  // search node in routingTable. If it is not found, create new one.
  std::shared_ptr<DHTNode> getRemoteNode(const unsigned char* id,
                                         const std::string& ipaddr,
                                         uint16_t port) const;

  void validateID(const String* id) const;

  void validatePort(const Integer* i) const;

  void extractNodes(std::vector<std::shared_ptr<DHTNode>>& nodes,
                    const unsigned char* src, size_t length);

  void setCommonProperty(DHTAbstractMessage* m);

public:
  DHTMessageFactoryImpl(int family);

  virtual std::unique_ptr<DHTQueryMessage>
  createQueryMessage(const Dict* dict, const std::string& ipaddr,
                     uint16_t port) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTResponseMessage>
  createResponseMessage(const std::string& messageType, const Dict* dict,
                        const std::string& ipaddr,
                        uint16_t port) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTPingMessage> createPingMessage(
      const std::shared_ptr<DHTNode>& remoteNode,
      const std::string& transactionID = A2STR::NIL) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTPingReplyMessage>
  createPingReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                         const unsigned char* id,
                         const std::string& transactionID) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTFindNodeMessage> createFindNodeMessage(
      const std::shared_ptr<DHTNode>& remoteNode,
      const unsigned char* targetNodeID,
      const std::string& transactionID = A2STR::NIL) CXX11_OVERRIDE;

  std::unique_ptr<DHTFindNodeReplyMessage>
  createFindNodeReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                             const Dict* dict,
                             const std::string& transactionID);

  virtual std::unique_ptr<DHTFindNodeReplyMessage> createFindNodeReplyMessage(
      const std::shared_ptr<DHTNode>& remoteNode,
      std::vector<std::shared_ptr<DHTNode>> closestKNodes,
      const std::string& transactionID) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTGetPeersMessage> createGetPeersMessage(
      const std::shared_ptr<DHTNode>& remoteNode, const unsigned char* infoHash,
      const std::string& transactionID = A2STR::NIL) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTGetPeersReplyMessage> createGetPeersReplyMessage(
      const std::shared_ptr<DHTNode>& remoteNode,
      std::vector<std::shared_ptr<DHTNode>> closestKNodes,
      std::vector<std::shared_ptr<Peer>> peers, const std::string& token,
      const std::string& transactionID) CXX11_OVERRIDE;

  std::unique_ptr<DHTGetPeersReplyMessage>
  createGetPeersReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                             const Dict* dict,
                             const std::string& transactionID);

  virtual std::unique_ptr<DHTAnnouncePeerMessage> createAnnouncePeerMessage(
      const std::shared_ptr<DHTNode>& remoteNode, const unsigned char* infoHash,
      uint16_t tcpPort, const std::string& token,
      const std::string& transactionID = A2STR::NIL) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTAnnouncePeerReplyMessage>
  createAnnouncePeerReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                                 const std::string& transactionID)
      CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTUnknownMessage>
  createUnknownMessage(const unsigned char* data, size_t length,
                       const std::string& ipaddr, uint16_t port) CXX11_OVERRIDE;

  void setRoutingTable(DHTRoutingTable* routingTable);

  void setConnection(DHTConnection* connection);

  void setMessageDispatcher(DHTMessageDispatcher* dispatcher);

  void setPeerAnnounceStorage(DHTPeerAnnounceStorage* storage);

  void setTokenTracker(DHTTokenTracker* tokenTracker);

  void setLocalNode(const std::shared_ptr<DHTNode>& localNode);

  void setBtRegistry(BtRegistry* btRegistry);
};

} // namespace aria2

#endif // D_DHT_MESSAGE_FACTORY_IMPL_H
