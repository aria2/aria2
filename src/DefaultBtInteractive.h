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
#ifndef D_DEFAULT_BT_INTERACTIVE_H
#define D_DEFAULT_BT_INTERACTIVE_H

#include "BtInteractive.h"

#include  <limits.h>

#include "TimerA2.h"
#include "Command.h"

namespace aria2 {

class DownloadContext;
class BtRuntime;
class PieceStorage;
class PeerStorage;
class Peer;
class BtMessage;
class BtMessageReceiver;
class BtMessageDispatcher;
class BtMessageFactory;
class BtRequestFactory;
class PeerConnection;
class ExtensionMessageFactory;
class ExtensionMessageRegistry;
class DHTNode;
class RequestGroupMan;
class UTMetadataRequestFactory;
class UTMetadataRequestTracker;

class FloodingStat {
private:
  int chokeUnchokeCount;
  int keepAliveCount;
public:
  FloodingStat():chokeUnchokeCount(0), keepAliveCount(0) {}
  
  void incChokeUnchokeCount() {
    if(chokeUnchokeCount < INT_MAX) {
      chokeUnchokeCount++;
    }
  }

  void incKeepAliveCount() {
    if(keepAliveCount < INT_MAX) {
      keepAliveCount++;
    }
  }

  int getChokeUnchokeCount() const {
    return chokeUnchokeCount;
  }

  int getKeepAliveCount() const {
    return keepAliveCount;
  }

  void reset() {
    chokeUnchokeCount = 0;
    keepAliveCount = 0;
  }
};

class DefaultBtInteractive : public BtInteractive {
private:
  cuid_t cuid_;

  SharedHandle<DownloadContext> downloadContext_;

  SharedHandle<BtRuntime> btRuntime_;

  SharedHandle<PieceStorage> pieceStorage_;

  SharedHandle<PeerStorage> peerStorage_;

  SharedHandle<Peer> peer_;

  SharedHandle<BtMessageReceiver> btMessageReceiver_;
  SharedHandle<BtMessageDispatcher> dispatcher_;
  SharedHandle<BtRequestFactory> btRequestFactory_;
  // Although peerStorage_ is not used in this class, this object
  // holds the reference so that peerConnection_ is not deleted.
  SharedHandle<PeerConnection> peerConnection_;
  SharedHandle<BtMessageFactory> messageFactory_;
  SharedHandle<ExtensionMessageFactory> extensionMessageFactory_;
  SharedHandle<ExtensionMessageRegistry> extensionMessageRegistry_;
  SharedHandle<UTMetadataRequestFactory> utMetadataRequestFactory_;
  SharedHandle<UTMetadataRequestTracker> utMetadataRequestTracker_;

  bool metadataGetMode_;

  DHTNode* localNode_;

  size_t allowedFastSetSize_;
  Timer haveTimer_;
  Timer keepAliveTimer_;
  Timer floodingTimer_;
  FloodingStat floodingStat_;
  Timer inactiveTimer_;
  Timer pexTimer_;
  Timer perSecTimer_;
  time_t keepAliveInterval_;
  bool utPexEnabled_;
  bool dhtEnabled_;

  size_t numReceivedMessage_;

  size_t maxOutstandingRequest_;

  RequestGroupMan* requestGroupMan_;

  uint16_t tcpPort_;

  static const time_t FLOODING_CHECK_INTERVAL = 5;

  void addBitfieldMessageToQueue();
  void addAllowedFastMessageToQueue();
  void addHandshakeExtendedMessageToQueue();
  void decideChoking();
  void checkHave();
  void sendKeepAlive();
  void decideInterest();
  void fillPiece(size_t maxMissingBlock);
  void addRequests();
  void detectMessageFlooding();
  void checkActiveInteraction();
  void addPeerExchangeMessage();
  void addPortMessageToQueue();

public:
  DefaultBtInteractive(const SharedHandle<DownloadContext>& downloadContext,
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

  virtual size_t countPendingMessage();
  
  virtual bool isSendingMessageInProgress();

  virtual size_t countReceivedMessageInIteration() const;

  virtual size_t countOutstandingRequest();

  void setCuid(cuid_t cuid)
  {
    cuid_ = cuid;
  }

  void setBtRuntime(const SharedHandle<BtRuntime>& btRuntime);

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  void setPeerStorage(const SharedHandle<PeerStorage>& peerStorage);

  void setPeer(const SharedHandle<Peer>& peer);

  void setBtMessageReceiver(const SharedHandle<BtMessageReceiver>& receiver);

  void setDispatcher(const SharedHandle<BtMessageDispatcher>& dispatcher);

  void setBtRequestFactory(const SharedHandle<BtRequestFactory>& factory);

  void setPeerConnection(const SharedHandle<PeerConnection>& peerConnection);

  void setBtMessageFactory(const SharedHandle<BtMessageFactory>& factory);

  void setExtensionMessageFactory
  (const SharedHandle<ExtensionMessageFactory>& factory);

  void setExtensionMessageRegistry
  (const SharedHandle<ExtensionMessageRegistry>& registry)
  {
    extensionMessageRegistry_ = registry;
  }

  void setKeepAliveInterval(time_t keepAliveInterval) {
    keepAliveInterval_ = keepAliveInterval;
  }

  void setUTPexEnabled(bool f)
  {
    utPexEnabled_ = f;
  }

  void setLocalNode(DHTNode* node);

  void setDHTEnabled(bool f)
  {
    dhtEnabled_ = f;
  }

  void setRequestGroupMan(RequestGroupMan* rgman);

  void setUTMetadataRequestTracker
  (const SharedHandle<UTMetadataRequestTracker>& tracker)
  {
    utMetadataRequestTracker_ = tracker;
  }

  void setUTMetadataRequestFactory
  (const SharedHandle<UTMetadataRequestFactory>& factory)
  {
    utMetadataRequestFactory_ = factory;
  }

  void enableMetadataGetMode()
  {
    metadataGetMode_ = true;
  }

  void setTcpPort(uint16_t port)
  {
    tcpPort_ = port;
  }
};

typedef SharedHandle<DefaultBtInteractive> DefaultBtInteractiveHandle;

} // namespace aria2

#endif // D_DEFAULT_BT_INTERACTIVE_H
