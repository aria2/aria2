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
#ifndef _D_PEER_SESSION_RESOURCE_H_
#define _D_PEER_SESSION_RESOURCE_H_

#include "common.h"
#include "BtConstants.h"
#include "PeerStat.h"
#include "TimeA2.h"
#include <string>
#include <deque>

namespace aria2 {

class BitfieldMan;
class BtMessageDispatcher;

class PeerSessionResource {
private:
  // localhost is choking this peer
  bool _amChoking;
  // localhost is interested in this peer
  bool _amInterested;
  // this peer is choking localhost
  bool _peerChoking;
  // this peer is interested in localhost
  bool _peerInterested;
  // choking this peer is requested
  bool _chokingRequired;
  // this peer is eligible for *optional* unchoking.
  bool _optUnchoking;
  // this peer is snubbing.
  bool _snubbing;

  BitfieldMan* _bitfieldMan;
  bool _fastExtensionEnabled;
  // fast index set which a peer has sent to localhost.
  std::deque<size_t> _peerAllowedIndexSet;
  // fast index set which localhost has sent to a peer.
  std::deque<size_t> _amAllowedIndexSet;
  bool _extendedMessagingEnabled;
  Extensions _extensions;
  bool _dhtEnabled;
  PeerStat _peerStat;
  unsigned int _latency;

  Time _lastDownloadUpdate;

  Time _lastAmUnchoking;

  WeakHandle<BtMessageDispatcher> _dispatcher;
public:
  PeerSessionResource(size_t pieceLength, uint64_t totalLength);

  ~PeerSessionResource();

  // localhost is choking this peer
  bool amChoking() const;

  void amChoking(bool b);

  // localhost is interested in this peer
  bool amInterested() const;

  void amInterested(bool b);

  // this peer is choking localhost
  bool peerChoking() const;

  void peerChoking(bool b);

  // this peer is interested in localhost
  bool peerInterested() const;

  void peerInterested(bool b);
  
  // this peer should be choked
  bool chokingRequired() const;

  void chokingRequired(bool b);

  // this peer is eligible for unchoking optionally.
  bool optUnchoking() const;

  void optUnchoking(bool b);

  bool shouldBeChoking() const;

  // this peer is snubbing.
  bool snubbing() const;

  void snubbing(bool b);

  bool hasAllPieces() const;

  void updateBitfield(size_t index, int operation);
  
  void setBitfield(const unsigned char* bitfield, size_t bitfieldLength);

  const unsigned char* getBitfield() const;

  size_t getBitfieldLength() const;

  bool hasPiece(size_t index) const;

  void markSeeder();

  bool fastExtensionEnabled() const;

  void fastExtensionEnabled(bool b);

  // fast index set which a peer has sent to localhost.
  const std::deque<size_t>& peerAllowedIndexSet() const;

  void addPeerAllowedIndex(size_t index);

  bool peerAllowedIndexSetContains(size_t index) const;

  // fast index set which localhost has sent to a peer.
  const std::deque<size_t>& amAllowedIndexSet() const;

  void addAmAllowedIndex(size_t index);

  bool amAllowedIndexSetContains(size_t index) const;

  bool extendedMessagingEnabled() const;

  void extendedMessagingEnabled(bool b);

  uint8_t getExtensionMessageID(const std::string& name) const;

  std::string getExtensionName(uint8_t id) const;

  void addExtension(const std::string& name, uint8_t id);

  bool dhtEnabled() const;

  void dhtEnabled(bool b);

  PeerStat& getPeerStat();

  unsigned int latency() const;

  void updateLatency(unsigned int latency);

  uint64_t uploadLength() const;

  void updateUploadLength(size_t bytes);

  uint64_t downloadLength() const;

  void updateDownloadLength(size_t bytes);

  const Time& getLastDownloadUpdate() const;

  const Time& getLastAmUnchoking() const;

  uint64_t getCompletedLength() const;

  void setBtMessageDispatcher(const WeakHandle<BtMessageDispatcher>& dpt);

  size_t countOutstandingUpload() const;
};

} // namespace aria2

#endif // _D_PEER_SESSION_RESOURCE_H_
