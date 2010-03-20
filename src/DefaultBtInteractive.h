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
#ifndef _D_DEFAULT_BT_INTERACTIVE_H_
#define _D_DEFAULT_BT_INTERACTIVE_H_

#include "BtInteractive.h"

#include  <limits.h>

#include "TimeA2.h"
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
class Logger;
class RequestGroupMan;
class UTMetadataRequestFactory;
class UTMetadataRequestTracker;

class FloodingStat {
private:
  unsigned int chokeUnchokeCount;
  unsigned int keepAliveCount;
public:
  FloodingStat():chokeUnchokeCount(0), keepAliveCount(0) {}
  
  void incChokeUnchokeCount() {
    if(chokeUnchokeCount < UINT_MAX) {
      chokeUnchokeCount++;
    }
  }

  void incKeepAliveCount() {
    if(keepAliveCount < UINT_MAX) {
      keepAliveCount++;
    }
  }

  unsigned int getChokeUnchokeCount() const {
    return chokeUnchokeCount;
  }

  unsigned int getKeepAliveCount() const {
    return keepAliveCount;
  }

  void reset() {
    chokeUnchokeCount = 0;
    keepAliveCount = 0;
  }
};

class DefaultBtInteractive : public BtInteractive {
private:
  cuid_t cuid;

  SharedHandle<DownloadContext> _downloadContext;

  SharedHandle<BtRuntime> _btRuntime;

  SharedHandle<PieceStorage> _pieceStorage;

  SharedHandle<PeerStorage> _peerStorage;

  SharedHandle<Peer> peer;

  SharedHandle<BtMessageReceiver> btMessageReceiver;
  SharedHandle<BtMessageDispatcher> dispatcher;
  SharedHandle<BtRequestFactory> btRequestFactory;
  SharedHandle<PeerConnection> peerConnection;
  SharedHandle<BtMessageFactory> messageFactory;
  SharedHandle<ExtensionMessageFactory> _extensionMessageFactory;
  SharedHandle<ExtensionMessageRegistry> _extensionMessageRegistry;
  SharedHandle<UTMetadataRequestFactory> _utMetadataRequestFactory;
  SharedHandle<UTMetadataRequestTracker> _utMetadataRequestTracker;

  bool _metadataGetMode;

  WeakHandle<DHTNode> _localNode;

  Logger* logger;
  size_t allowedFastSetSize;
  Time haveCheckPoint;
  Time keepAliveCheckPoint;
  Time floodingCheckPoint;
  FloodingStat floodingStat;
  Time inactiveCheckPoint;
  Time _pexCheckPoint;
  Time _perSecCheckPoint;
  time_t keepAliveInterval;
  bool _utPexEnabled;
  bool _dhtEnabled;

  size_t _numReceivedMessage;

  size_t _maxOutstandingRequest;

  WeakHandle<RequestGroupMan> _requestGroupMan;

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
    this->cuid = cuid;
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
    _extensionMessageRegistry = registry;
  }

  void setKeepAliveInterval(time_t keepAliveInterval) {
    this->keepAliveInterval = keepAliveInterval;
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

  void setRequestGroupMan(const WeakHandle<RequestGroupMan>& rgman);

  void setUTMetadataRequestTracker
  (const SharedHandle<UTMetadataRequestTracker>& tracker)
  {
    _utMetadataRequestTracker = tracker;
  }

  void setUTMetadataRequestFactory
  (const SharedHandle<UTMetadataRequestFactory>& factory)
  {
    _utMetadataRequestFactory = factory;
  }

  void enableMetadataGetMode()
  {
    _metadataGetMode = true;
  }
};

typedef SharedHandle<DefaultBtInteractive> DefaultBtInteractiveHandle;

} // namespace aria2

#endif // _D_DEFAULT_BT_INTERACTIVE_H_
