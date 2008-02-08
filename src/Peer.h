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
#ifndef _D_PEER_H_
#define _D_PEER_H_

#include "common.h"
#include "PeerDecl.h"
#include "TimeA2.h"
#include "BtConstants.h"
#include "PeerStat.h"
#include <string>
#include <deque>

namespace aria2 {

class BitfieldMan;

class Peer {
public:
  std::string ipaddr;
  // TCP port which this peer is listening for incoming connections.
  // If it is unknown, for example, localhost accepted the incoming connection
  // from this peer, set port to 0.
  uint16_t port;
  bool amChoking;
  bool amInterested;
  bool peerChoking;
  bool peerInterested;
  int32_t tryCount;
  int32_t cuid;
  bool chokingRequired;
  bool optUnchoking;
  bool snubbing;
private:
  unsigned char peerId[PEER_ID_LENGTH];
  BitfieldMan* _bitfield;
  bool fastExtensionEnabled;
  // fast index set which a peer has sent to localhost.
  std::deque<int32_t> peerAllowedIndexSet;
  // fast index set which localhost has sent to a peer.
  std::deque<int32_t> amAllowedIndexSet;
  bool _extendedMessagingEnabled;
  Extensions _extensions;
  bool _dhtEnabled;
  PeerStat peerStat;
  int64_t sessionUploadLength;
  int64_t sessionDownloadLength;
  int32_t latency;
  bool active;
  std::string id;
  Time _firstContactTime;
  Time _badConditionStartTime;
  bool _seeder;

  void updateSeeder();
public:
  Peer(std::string ipaddr, uint16_t port);

  ~Peer();

  bool operator==(const Peer& p);
  
  bool operator!=(const Peer& p);

  void resetStatus();

  void updateUploadLength(int32_t bytes);

  void updateDownloadLength(int32_t bytes);

  /**
   * Returns the transfer rate from localhost to remote host.
   */
  int32_t calculateUploadSpeed();

  int32_t calculateUploadSpeed(const struct timeval& now);

  /**
   * Returns the transfer rate from remote host to localhost.
   */
  int32_t calculateDownloadSpeed();

  int32_t calculateDownloadSpeed(const struct timeval& now);

  /**
   * Returns the number of bytes uploaded to the remote host.
   */
  int64_t getSessionUploadLength() const;

  /**
   * Returns the number of bytes downloaded from the remote host.
   */
  int64_t getSessionDownloadLength() const;

  void activate();

  void deactivate();

  bool isActive() const;

  void setPeerId(const unsigned char* peerId);

  const unsigned char* getPeerId() const;
  
  void setBitfield(const unsigned char* bitfield, int32_t bitfieldLength);

  const unsigned char* getBitfield() const;

  int32_t getBitfieldLength() const;

  void setAllBitfield();

  /**
   * operation = 1: set index-th bit to 1
   * operation = 0: set index-th bit to 0
   */
  void updateBitfield(int32_t index, int32_t operation);
  
  void setFastExtensionEnabled(bool enabled);

  bool isFastExtensionEnabled() const;

  void addPeerAllowedIndex(int32_t index);
  bool isInPeerAllowedIndexSet(int32_t index) const;

  int32_t countPeerAllowedIndexSet() const;

  const std::deque<int32_t>& getPeerAllowedIndexSet() const;

  void addAmAllowedIndex(int32_t index);
  bool isInAmAllowedIndexSet(int32_t index) const;

  void setExtendedMessagingEnabled(bool enabled)
  {
    _extendedMessagingEnabled = enabled;
  }

  bool isExtendedMessagingEnabled() const
  {
    return _extendedMessagingEnabled;
  }

  void setDHTEnabled(bool enabled)
  {
    _dhtEnabled = enabled;
  }

  bool isDHTEnabled() const
  {
    return _dhtEnabled;
  }

  bool shouldBeChoking() const;

  bool hasPiece(int32_t index) const;

  bool isSeeder() const
  {
    return _seeder;
  }

  void updateLatency(int32_t latency);

  int32_t getLatency() const { return latency; }

  const std::string& getId() const {
    return id;
  }

  void startBadCondition();

  bool isGood() const;

  void allocateBitfield(int32_t pieceLength, int64_t totalLength);

  void deallocateBitfield();

  Time getFirstContactTime() const
  {
    return _firstContactTime;
  }

  Time getBadConditionStartTime() const
  {
    return _badConditionStartTime;
  }

  uint8_t getExtensionMessageID(const std::string& name);

  std::string getExtensionName(uint8_t id);

  void setExtension(const std::string& name, uint8_t id);
};

} // namespace aria2

#endif // _D_PEER_H_
