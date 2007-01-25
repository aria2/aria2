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
#include "Peer.h"
#include "AbstractBtMessage.h"
#include "BtRegistry.h"

class DefaultBtMessageFactory : public BtMessageFactory {
private:
  int32_t cuid;
  BtContextHandle btContext;
  PieceStorageHandle pieceStorage;
  PeerHandle peer;

  BtMessageDispatcherWeakHandle dispatcher;

  BtRequestFactoryWeakHandle requestFactory;

  PeerConnectionWeakHandle peerConnection;

  void setCommonProperty(const AbstractBtMessageHandle& msg);
public:
  DefaultBtMessageFactory():cuid(0),
			    btContext(0),
			    pieceStorage(0),
			    peer(0)
  {
    LogFactory::getInstance()->debug("DefaultBtMessageFactory::instantiated");
  }

  virtual ~DefaultBtMessageFactory()
  {
    LogFactory::getInstance()->debug("DefaultBtMessageFactory::deleted");
  }

  virtual BtMessageHandle
  createBtMessage(const unsigned char* msg, int32_t msgLength);

  virtual BtMessageHandle
  createHandshakeMessage(const unsigned char* msg, int32_t msgLength);

  virtual BtMessageHandle
  createHandshakeMessage(const unsigned char* infoHash,
			 const unsigned char* peerId);

  virtual BtMessageHandle
  createRequestMessage(const PieceHandle& piece, int32_t blockIndex);

  virtual BtMessageHandle
  createCancelMessage(int32_t index, int32_t begin, int32_t length);

  virtual BtMessageHandle
  createPieceMessage(int32_t index, int32_t begin, int32_t length);

  virtual BtMessageHandle createHaveMessage(int32_t index);

  virtual BtMessageHandle createChokeMessage();

  virtual BtMessageHandle createUnchokeMessage();
  
  virtual BtMessageHandle createInterestedMessage();

  virtual BtMessageHandle createNotInterestedMessage();

  virtual BtMessageHandle createBitfieldMessage();

  virtual BtMessageHandle createKeepAliveMessage();
  
  virtual BtMessageHandle createHaveAllMessage();

  virtual BtMessageHandle createHaveNoneMessage();

  virtual BtMessageHandle
  createRejectMessage(int32_t index, int32_t begin, int32_t length);

  virtual BtMessageHandle createAllowedFastMessage(int32_t index);

  void setPeer(const PeerHandle& peer) {
    this->peer = peer;
  }

  PeerHandle getPeer() const {
    return peer;
  }

  void setBtContext(const BtContextHandle& btContext) {
    this->btContext = btContext;
    this->pieceStorage = PIECE_STORAGE(btContext);
  }

  BtContextHandle getBtContext() const {
    return btContext;
  }

  void setCuid(int32_t cuid) {
    this->cuid = cuid;
  }

  void setBtMessageDispatcher(const BtMessageDispatcherWeakHandle& dispatcher)
  {
    this->dispatcher = dispatcher;
  }
  
  void setBtRequestFactory(const BtRequestFactoryWeakHandle& factory) {
    this->requestFactory = factory;
  }

  void setPeerConnection(const PeerConnectionWeakHandle& connection) {
    this->peerConnection = connection;
  }
  
};

typedef SharedHandle<DefaultBtMessageFactory> DefaultBtMessageFactoryHandle;

#endif // _D_DEFAULT_BT_MESSAGE_FACTORY_H_
