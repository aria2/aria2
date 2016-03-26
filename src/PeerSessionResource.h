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
#ifndef D_PEER_SESSION_RESOURCE_H
#define D_PEER_SESSION_RESOURCE_H

#include "common.h"

#include <string>
#include <set>
#include <memory>

#include "BtConstants.h"
#include "NetStat.h"
#include "TimerA2.h"
#include "ExtensionMessageRegistry.h"

namespace aria2 {

class BitfieldMan;
class BtMessageDispatcher;

class PeerSessionResource {
private:
  std::unique_ptr<BitfieldMan> bitfieldMan_;
  // fast index set which a peer has sent to localhost.
  std::set<size_t> peerAllowedIndexSet_;
  // fast index set which localhost has sent to a peer.
  std::set<size_t> amAllowedIndexSet_;
  ExtensionMessageRegistry extreg_;
  NetStat netStat_;

  Timer lastDownloadUpdate_;

  Timer lastAmUnchoking_;

  BtMessageDispatcher* dispatcher_;

  // localhost is choking this peer
  bool amChoking_;
  // localhost is interested in this peer
  bool amInterested_;
  // this peer is choking localhost
  bool peerChoking_;
  // this peer is interested in localhost
  bool peerInterested_;
  // choking this peer is requested
  bool chokingRequired_;
  // this peer is eligible for *optional* unchoking.
  bool optUnchoking_;
  // this peer is snubbing.
  bool snubbing_;
  bool fastExtensionEnabled_;
  bool extendedMessagingEnabled_;
  bool dhtEnabled_;

public:
  PeerSessionResource(int32_t pieceLength, int64_t totalLength);

  ~PeerSessionResource();

  // localhost is choking this peer
  bool amChoking() const { return amChoking_; }

  void amChoking(bool b);

  // localhost is interested in this peer
  bool amInterested() const { return amInterested_; }

  void amInterested(bool b);

  // this peer is choking localhost
  bool peerChoking() const { return peerChoking_; }

  void peerChoking(bool b);

  // this peer is interested in localhost
  bool peerInterested() const { return peerInterested_; }

  void peerInterested(bool b);

  // this peer should be choked
  bool chokingRequired() const { return chokingRequired_; }

  void chokingRequired(bool b);

  // this peer is eligible for unchoking optionally.
  bool optUnchoking() const { return optUnchoking_; }

  void optUnchoking(bool b);

  bool shouldBeChoking() const;

  // this peer is snubbing.
  bool snubbing() const { return snubbing_; }

  void snubbing(bool b);

  bool hasAllPieces() const;

  void updateBitfield(size_t index, int operation);

  void setBitfield(const unsigned char* bitfield, size_t bitfieldLength);

  const unsigned char* getBitfield() const;

  size_t getBitfieldLength() const;

  void reconfigure(int32_t pieceLength, int64_t totalLength);

  bool hasPiece(size_t index) const;

  void markSeeder();

  bool fastExtensionEnabled() const { return fastExtensionEnabled_; }

  void fastExtensionEnabled(bool b);

  // fast index set which a peer has sent to localhost.
  const std::set<size_t>& peerAllowedIndexSet() const;

  void addPeerAllowedIndex(size_t index);

  bool peerAllowedIndexSetContains(size_t index) const;

  // fast index set which localhost has sent to a peer.
  const std::set<size_t>& amAllowedIndexSet() const
  {
    return amAllowedIndexSet_;
  }

  void addAmAllowedIndex(size_t index);

  bool amAllowedIndexSetContains(size_t index) const;

  bool extendedMessagingEnabled() const { return extendedMessagingEnabled_; }

  void extendedMessagingEnabled(bool b);

  uint8_t getExtensionMessageID(int key) const;

  const char* getExtensionName(uint8_t id) const;

  void addExtension(int key, uint8_t id);

  bool dhtEnabled() const { return dhtEnabled_; }

  void dhtEnabled(bool b);

  NetStat& getNetStat() { return netStat_; }

  int64_t uploadLength() const;

  void updateUploadSpeed(int32_t bytes);

  void updateUploadLength(int32_t bytes);

  int64_t downloadLength() const;

  void updateDownload(int32_t bytes);

  const Timer& getLastDownloadUpdate() const { return lastDownloadUpdate_; }

  const Timer& getLastAmUnchoking() const { return lastAmUnchoking_; }

  int64_t getCompletedLength() const;

  void setBtMessageDispatcher(BtMessageDispatcher* dpt);

  size_t countOutstandingUpload() const;
};

} // namespace aria2

#endif // D_PEER_SESSION_RESOURCE_H
