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
#ifndef _D_DEFAULT_BT_MESSAGE_FACTORY_H_
#define _D_DEFAULT_BT_MESSAGE_FACTORY_H_

#include "BtMessageFactory.h"

namespace aria2 {

class BtContext;
class Peer;
class AbstractBtMessage;
class BtMessageDispatcher;
class BtRequestFactory;
class PeerConnection;
class PieceStorage;
class DHTNode;
class DHTRoutingTable;
class DHTTaskQueue;
class DHTTaskFactory;

class DefaultBtMessageFactory : public BtMessageFactory {
private:
  int32_t cuid;
  SharedHandle<BtContext> btContext;
  SharedHandle<PieceStorage> pieceStorage;
  SharedHandle<Peer> peer;

  bool _dhtEnabled;

  WeakHandle<BtMessageDispatcher> dispatcher;

  WeakHandle<BtRequestFactory> requestFactory;

  WeakHandle<PeerConnection> peerConnection;

  WeakHandle<DHTNode> _localNode;

  WeakHandle<DHTRoutingTable> _routingTable;

  WeakHandle<DHTTaskQueue> _taskQueue;

  WeakHandle<DHTTaskFactory> _taskFactory;

  void setCommonProperty(const SharedHandle<AbstractBtMessage>& msg);
public:
  DefaultBtMessageFactory();

  virtual ~DefaultBtMessageFactory();

  virtual SharedHandle<BtMessage>
  createBtMessage(const unsigned char* msg, int32_t msgLength);

  virtual SharedHandle<BtMessage>
  createHandshakeMessage(const unsigned char* msg, int32_t msgLength);

  virtual SharedHandle<BtMessage>
  createHandshakeMessage(const unsigned char* infoHash,
			 const unsigned char* peerId);

  virtual SharedHandle<BtMessage>
  createRequestMessage(const SharedHandle<Piece>& piece, int32_t blockIndex);

  virtual SharedHandle<BtMessage>
  createCancelMessage(int32_t index, int32_t begin, int32_t length);

  virtual SharedHandle<BtMessage>
  createPieceMessage(int32_t index, int32_t begin, int32_t length);

  virtual SharedHandle<BtMessage> createHaveMessage(int32_t index);

  virtual SharedHandle<BtMessage> createChokeMessage();

  virtual SharedHandle<BtMessage> createUnchokeMessage();
  
  virtual SharedHandle<BtMessage> createInterestedMessage();

  virtual SharedHandle<BtMessage> createNotInterestedMessage();

  virtual SharedHandle<BtMessage> createBitfieldMessage();

  virtual SharedHandle<BtMessage> createKeepAliveMessage();
  
  virtual SharedHandle<BtMessage> createHaveAllMessage();

  virtual SharedHandle<BtMessage> createHaveNoneMessage();

  virtual SharedHandle<BtMessage>
  createRejectMessage(int32_t index, int32_t begin, int32_t length);

  virtual SharedHandle<BtMessage> createAllowedFastMessage(int32_t index);

  virtual SharedHandle<BtMessage> createPortMessage(uint16_t port);

  virtual SharedHandle<BtMessage>
  createBtExtendedMessage(const SharedHandle<ExtensionMessage>& msg);

  void setPeer(const SharedHandle<Peer>& peer);

  void setBtContext(const SharedHandle<BtContext>& btContext);

  void setCuid(int32_t cuid) {
    this->cuid = cuid;
  }

  void setDHTEnabled(bool enabled) {
    _dhtEnabled = enabled;
  }

  void setBtMessageDispatcher(const WeakHandle<BtMessageDispatcher>& dispatcher);
  
  void setBtRequestFactory(const WeakHandle<BtRequestFactory>& factory);

  void setPeerConnection(const WeakHandle<PeerConnection>& connection);

  void setLocalNode(const WeakHandle<DHTNode>& localNode);

  void setRoutingTable(const WeakHandle<DHTRoutingTable>& routingTable);
  
  void setTaskQueue(const WeakHandle<DHTTaskQueue>& taskQueue);

  void setTaskFactory(const WeakHandle<DHTTaskFactory>& taskFactory);
};

typedef SharedHandle<DefaultBtMessageFactory> DefaultBtMessageFactoryHandle;

} // namespace aria2

#endif // _D_DEFAULT_BT_MESSAGE_FACTORY_H_
