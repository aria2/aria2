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
#include "BitfieldMan.h"
#include "SharedHandle.h"
#include "PeerStat.h"
#include "TimeA2.h"
#include <string.h>
#include <string>

using namespace std;

#define PEER_ID_LENGTH 20
#define DEFAULT_LATENCY 1500

class Peer {
  friend bool operator==(const Peer& p1, const Peer& p2);
  friend bool operator!=(const Peer& p1, const Peer& p2);
public:
  string ipaddr;
  int32_t port;
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
  BitfieldMan* bitfield;
  bool fastExtensionEnabled;
  // allowed fast indexes that peer has sent by Allowed Fast message
  Integers fastSet;
  // fast index set which a peer has sent to localhost.
  Integers peerAllowedIndexSet;
  // fast index set which localhost has sent to a peer.
  Integers amAllowedIndexSet;
  PeerStat peerStat;
  int64_t sessionUploadLength;
  int64_t sessionDownloadLength;
  int32_t pieceLength;
  int32_t latency;
  bool active;
  string id;
  Time _badConditionStartTime;
  int32_t _badConditionInterval;
public:
  Peer(string ipaddr, int32_t port, int32_t pieceLength, int64_t totalLength);

  ~Peer() {
    delete bitfield;
  }

  bool operator==(const Peer& p) {
    //return ipaddr == p.ipaddr && port == p.port;
    return id == p.id;
  }
  
  bool operator!=(const Peer& p) {
    return !(*this == p);
  }

  void resetStatus();

  void updateUploadLength(int32_t bytes) {
    peerStat.updateUploadLength(bytes);
    sessionUploadLength += bytes;
  }

  void updateDownloadLength(int32_t bytes) {
    peerStat.updateDownloadLength(bytes);
    sessionDownloadLength += bytes;
  }

  /**
   * Returns the transfer rate from localhost to remote host.
   */
  int32_t calculateUploadSpeed() {
    return peerStat.calculateUploadSpeed();
  }

  int32_t calculateUploadSpeed(const struct timeval& now) {
    return peerStat.calculateUploadSpeed(now);
  }

  /**
   * Returns the transfer rate from remote host to localhost.
   */
  int32_t calculateDownloadSpeed() {
    return peerStat.calculateDownloadSpeed();
  }

  int32_t calculateDownloadSpeed(const struct timeval& now) {
    return peerStat.calculateDownloadSpeed(now);
  }

  /**
   * Returns the number of bytes uploaded to the remote host.
   */
  int64_t getSessionUploadLength() const {
    return sessionUploadLength;
  }

  /**
   * Returns the number of bytes downloaded from the remote host.
   */
  int64_t getSessionDownloadLength() const {
    return sessionDownloadLength;
  }

  void activate() {
    peerStat.downloadStart();
    active = true;
  }

  void deactivate() {
    peerStat.downloadStop();
    active = false;
  }

  bool isActive() const {
    return active;
  }

  void setPeerId(const unsigned char* peerId) {
    memcpy(this->peerId, peerId, PEER_ID_LENGTH);
  }
  const unsigned char* getPeerId() const { return this->peerId; }
  
  void setBitfield(const unsigned char* bitfield, int32_t bitfieldLength) {
    this->bitfield->setBitfield(bitfield, bitfieldLength);
  }
  const unsigned char* getBitfield() const { return bitfield->getBitfield(); }
  int32_t getBitfieldLength() const { return bitfield->getBitfieldLength(); }
  void setAllBitfield();

  /**
   * operation = 1: set index-th bit to 1
   * operation = 0: set index-th bit to 0
   */
  void updateBitfield(int32_t index, int32_t operation);
  
  void setFastExtensionEnabled(bool enabled) {
    fastExtensionEnabled = enabled;
  }
  bool isFastExtensionEnabled() const { return fastExtensionEnabled; }

  void addFastSetIndex(int32_t index);
  const Integers& getFastSet() const { return fastSet; }
  bool isInFastSet(int32_t index) const;
  int32_t countFastSet() const { return fastSet.size(); }

  void addPeerAllowedIndex(int32_t index);
  bool isInPeerAllowedIndexSet(int32_t index) const;

  void addAmAllowedIndex(int32_t index);
  bool isInAmAllowedIndexSet(int32_t index) const;

  bool shouldBeChoking() const;

  bool hasPiece(int32_t index) const;

  bool isSeeder() const;

  void updateLatency(int32_t latency);
  int32_t getLatency() const { return latency; }

  const string& getId() const {
    return id;
  }

  void startBadCondition();

  bool isGood() const;

  void reconfigure(int32_t pieceLength, int64_t totalLength);
};

typedef SharedHandle<Peer> PeerHandle;
typedef deque<PeerHandle> Peers;

#endif // _D_PEER_H_
