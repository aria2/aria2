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

#include <limits.h>
#include <vector>

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
  FloodingStat() : chokeUnchokeCount(0), keepAliveCount(0) {}

  void incChokeUnchokeCount()
  {
    if (chokeUnchokeCount < INT_MAX) {
      ++chokeUnchokeCount;
    }
  }

  void incKeepAliveCount()
  {
    if (keepAliveCount < INT_MAX) {
      ++keepAliveCount;
    }
  }

  int getChokeUnchokeCount() const { return chokeUnchokeCount; }

  int getKeepAliveCount() const { return keepAliveCount; }

  void reset()
  {
    chokeUnchokeCount = 0;
    keepAliveCount = 0;
  }
};

class DefaultBtInteractive : public BtInteractive {
private:
  cuid_t cuid_;

  std::shared_ptr<DownloadContext> downloadContext_;

  std::shared_ptr<BtRuntime> btRuntime_;

  std::shared_ptr<PieceStorage> pieceStorage_;

  std::shared_ptr<PeerStorage> peerStorage_;

  std::shared_ptr<Peer> peer_;

  std::unique_ptr<BtMessageReceiver> btMessageReceiver_;
  std::unique_ptr<BtMessageDispatcher> dispatcher_;
  std::unique_ptr<BtRequestFactory> btRequestFactory_;
  std::unique_ptr<PeerConnection> peerConnection_;
  std::unique_ptr<BtMessageFactory> messageFactory_;
  std::unique_ptr<ExtensionMessageFactory> extensionMessageFactory_;
  std::unique_ptr<ExtensionMessageRegistry> extensionMessageRegistry_;
  std::unique_ptr<UTMetadataRequestFactory> utMetadataRequestFactory_;
  std::unique_ptr<UTMetadataRequestTracker> utMetadataRequestTracker_;

  bool metadataGetMode_;

  DHTNode* localNode_;

  // The last haveIndex we have advertised to the peer.
  uint64_t lastHaveIndex_;

  size_t allowedFastSetSize_;
  Timer keepAliveTimer_;
  Timer floodingTimer_;
  FloodingStat floodingStat_;
  Timer inactiveTimer_;
  Timer pexTimer_;
  Timer perSecTimer_;
  std::chrono::seconds keepAliveInterval_;
  bool utPexEnabled_;
  bool dhtEnabled_;

  size_t numReceivedMessage_;

  size_t maxOutstandingRequest_;

  RequestGroupMan* requestGroupMan_;

  uint16_t tcpPort_;

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
  DefaultBtInteractive(const std::shared_ptr<DownloadContext>& downloadContext,
                       const std::shared_ptr<Peer>& peer);

  virtual ~DefaultBtInteractive();

  virtual void initiateHandshake() CXX11_OVERRIDE;

  virtual std::unique_ptr<BtHandshakeMessage>
  receiveHandshake(bool quickReply = false) CXX11_OVERRIDE;

  virtual std::unique_ptr<BtHandshakeMessage>
  receiveAndSendHandshake() CXX11_OVERRIDE;

  virtual void doPostHandshakeProcessing() CXX11_OVERRIDE;

  virtual void doInteractionProcessing() CXX11_OVERRIDE;

  virtual void cancelAllPiece() CXX11_OVERRIDE;

  virtual void sendPendingMessage() CXX11_OVERRIDE;

  size_t receiveMessages();

  virtual size_t countPendingMessage() CXX11_OVERRIDE;

  virtual bool isSendingMessageInProgress() CXX11_OVERRIDE;

  virtual size_t countReceivedMessageInIteration() const CXX11_OVERRIDE;

  virtual size_t countOutstandingRequest() CXX11_OVERRIDE;

  void setCuid(cuid_t cuid) { cuid_ = cuid; }

  void setBtRuntime(const std::shared_ptr<BtRuntime>& btRuntime);

  void setPieceStorage(const std::shared_ptr<PieceStorage>& pieceStorage);

  void setPeerStorage(const std::shared_ptr<PeerStorage>& peerStorage);

  void setPeer(const std::shared_ptr<Peer>& peer);

  void setBtMessageReceiver(std::unique_ptr<BtMessageReceiver> receiver);

  void setDispatcher(std::unique_ptr<BtMessageDispatcher> dispatcher);

  void setBtRequestFactory(std::unique_ptr<BtRequestFactory> factory);

  void setPeerConnection(std::unique_ptr<PeerConnection> peerConnection);

  void setBtMessageFactory(std::unique_ptr<BtMessageFactory> factory);

  void
  setExtensionMessageFactory(std::unique_ptr<ExtensionMessageFactory> factory);

  void setExtensionMessageRegistry(
      std::unique_ptr<ExtensionMessageRegistry> registry);

  void setKeepAliveInterval(std::chrono::seconds keepAliveInterval)
  {
    keepAliveInterval_ = std::move(keepAliveInterval);
  }

  void setUTPexEnabled(bool f) { utPexEnabled_ = f; }

  void setLocalNode(DHTNode* node);

  void setDHTEnabled(bool f) { dhtEnabled_ = f; }

  void setRequestGroupMan(RequestGroupMan* rgman);

  void setUTMetadataRequestTracker(
      std::unique_ptr<UTMetadataRequestTracker> tracker);

  void setUTMetadataRequestFactory(
      std::unique_ptr<UTMetadataRequestFactory> factory);

  void enableMetadataGetMode() { metadataGetMode_ = true; }

  void setTcpPort(uint16_t port) { tcpPort_ = port; }
};

} // namespace aria2

#endif // D_DEFAULT_BT_INTERACTIVE_H
