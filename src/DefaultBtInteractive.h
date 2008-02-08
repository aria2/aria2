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
#ifndef _D_DEFAULT_BT_INTERACTIVE_H_
#define _D_DEFAULT_BT_INTERACTIVE_H_

#include "BtInteractive.h"
#include "BtContextAwareCommand.h"
#include "TimeA2.h"

namespace aria2 {

class Peer;
class BtMessage;
class BtMessageReceiver;
class BtMessageDispatcher;
class BtMessageFactory;
class BtRequestFactory;
class PeerConnection;
class DHTNode;
class Logger;

class FloodingStat {
private:
  int32_t chokeUnchokeCount;
  int32_t keepAliveCount;
public:
  FloodingStat():chokeUnchokeCount(0), keepAliveCount(0) {}
  
  void incChokeUnchokeCount() {
    if(chokeUnchokeCount < INT32_MAX) {
      chokeUnchokeCount++;
    }
  }

  void incKeepAliveCount() {
    if(keepAliveCount < INT32_MAX) {
      keepAliveCount++;
    }
  }

  int32_t getChokeUnchokeCount() const {
    return chokeUnchokeCount;
  }

  int32_t getKeepAliveCount() const {
    return keepAliveCount;
  }

  void reset() {
    chokeUnchokeCount = 0;
    keepAliveCount = 0;
  }
};

class DefaultBtInteractive : public BtInteractive, public BtContextAwareCommand {
private:
  int32_t cuid;
  SharedHandle<Peer> peer;

  WeakHandle<BtMessageReceiver> btMessageReceiver;
  WeakHandle<BtMessageDispatcher> dispatcher;
  WeakHandle<BtRequestFactory> btRequestFactory;
  WeakHandle<PeerConnection> peerConnection;
  WeakHandle<BtMessageFactory> messageFactory;

  WeakHandle<DHTNode> _localNode;

  const Logger* logger;
  int32_t allowedFastSetSize;
  Time haveCheckPoint;
  Time keepAliveCheckPoint;
  Time floodingCheckPoint;
  FloodingStat floodingStat;
  Time inactiveCheckPoint;
  Time _pexCheckPoint;
  int32_t keepAliveInterval;
  int32_t maxDownloadSpeedLimit;
  bool _utPexEnabled;
  bool _dhtEnabled;

  size_t _numReceivedMessage;

  static const int32_t FLOODING_CHECK_INTERVAL = 5;

  void addBitfieldMessageToQueue();
  void addAllowedFastMessageToQueue();
  void addHandshakeExtendedMessageToQueue();
  void decideChoking();
  void checkHave();
  void sendKeepAlive();
  void decideInterest();
  void fillPiece(int maxPieceNum);
  void addRequests();
  void detectMessageFlooding();
  void checkActiveInteraction();
  void addPeerExchangeMessage();
  void addPortMessageToQueue();

public:
  DefaultBtInteractive(const SharedHandle<BtContext>& btContext,
		       const SharedHandle<Peer>& peer);

  virtual ~DefaultBtInteractive();

  virtual void initiateHandshake();

  virtual SharedHandle<BtMessage> receiveHandshake(bool quickReply = false);

  virtual SharedHandle<BtMessage> receiveAndSendHandshake();

  virtual void doPostHandshakeProcessing();

  virtual void doInteractionProcessing();

  virtual void cancelAllPiece();

  virtual void sendPendingMessage();

  size_t receiveMessages();

  virtual int32_t countPendingMessage();
  
  virtual bool isSendingMessageInProgress();

  virtual size_t countReceivedMessageInIteration() const;

  void setCuid(int32_t cuid) {
    this->cuid = cuid;
  }

  void setPeer(const SharedHandle<Peer>& peer);

  void setBtMessageReceiver(const WeakHandle<BtMessageReceiver>& receiver);

  void setDispatcher(const WeakHandle<BtMessageDispatcher>& dispatcher);

  void setBtRequestFactory(const WeakHandle<BtRequestFactory>& factory);

  void setPeerConnection(const WeakHandle<PeerConnection>& peerConnection);

  void setBtMessageFactory(const WeakHandle<BtMessageFactory>& factory);

  void setKeepAliveInterval(int32_t keepAliveInterval) {
    this->keepAliveInterval = keepAliveInterval;
  }

  void setMaxDownloadSpeedLimit(int32_t maxDownloadSpeedLimit) {
    this->maxDownloadSpeedLimit = maxDownloadSpeedLimit;
  }

  void setUTPexEnabled(bool f)
  {
    _utPexEnabled = f;
  }

  void setLocalNode(const WeakHandle<DHTNode>& node);

  void setDHTEnabled(bool f)
  {
    _dhtEnabled = f;
  }
};

typedef SharedHandle<DefaultBtInteractive> DefaultBtInteractiveHandle;

} // namespace aria2

#endif // _D_DEFAULT_BT_INTERACTIVE_H_
