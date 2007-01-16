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
#ifndef _D_DEFAULT_BT_MESSAGE_RECEIVER_H_
#define _D_DEFAULT_BT_MESSAGE_RECEIVER_H_

#include "BtMessageReceiver.h"
#include "BtContext.h"
#include "BtRegistry.h"
#include "Peer.h"
#include "PeerConnection.h"
#include "BtMessageDispatcher.h"
#include "Logger.h"
#include "LogFactory.h"

class DefaultBtMessageReceiver : public BtMessageReceiver {
private:
  int32_t cuid;
  bool handshakeSent;
  BtContextHandle btContext;
  PeerHandle peer;
  PeerConnectionWeakHandle peerConnection;
  BtMessageDispatcherWeakHandle dispatcher;
  BtMessageFactoryWeakHandle messageFactory;
  const Logger* logger;

  void sendHandshake();
public:
  DefaultBtMessageReceiver():cuid(0),
			     handshakeSent(false),
			     btContext(0),
			     peer(0),
			     peerConnection(0),
			     dispatcher(0),
			     logger(LogFactory::getInstance())
  {
    logger->debug("DefaultBtMessageReceiver::instantiated");
  }

  virtual ~DefaultBtMessageReceiver()
  {
    logger->debug("DefaultBtMessageReceiver::deleted");
  }

  virtual BtMessageHandle receiveHandshake(bool quickReply = false);

  virtual BtMessageHandle receiveAndSendHandshake();

  virtual BtMessageHandle receiveMessage();

  void setCuid(int32_t cuid) {
    this->cuid = cuid;
  }

  int32_t getCuid() const {
    return cuid;
  }

  void setPeerConnection(const PeerConnectionWeakHandle& peerConnection) {
    this->peerConnection = peerConnection;
  }

  PeerConnectionWeakHandle getPeerConnection() const {
    return peerConnection;
  }

  void setBtContext(const BtContextHandle& btContext) {
    this->btContext = btContext;
  }

  BtContextHandle getBtContext() const {
    return btContext;
  }

  void setPeer(const PeerHandle& peer) {
    this->peer = peer;
  }

  PeerHandle getPeer() const {
    return peer;
  }

  void setDispatcher(const BtMessageDispatcherWeakHandle& dispatcher) {
    this->dispatcher = dispatcher;
  }

  void setBtMessageFactory(const BtMessageFactoryWeakHandle& factory) {
    this->messageFactory = factory;
  }
};

typedef SharedHandle<DefaultBtMessageReceiver> DefaultBtMessageReceiverHandle;

#endif // _D_DEFAULT_BT_MESSAGE_RECEIVER_H_
