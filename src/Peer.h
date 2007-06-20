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
  int port;
  bool amChoking;
  bool amInterested;
  bool peerChoking;
  bool peerInterested;
  int tryCount;
  int cuid;
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
  long long int sessionUploadLength;
  long long int sessionDownloadLength;
  int pieceLength;
  int latency;
  bool active;
  string id;
  Time _badConditionStartTime;
  int _badConditionInterval;
public:
  Peer(string ipaddr, int port, int pieceLength, long long int totalLength);

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

  void updateUploadLength(int bytes) {
    peerStat.updateUploadLength(bytes);
    sessionUploadLength += bytes;
  }

  void updateDownloadLength(int bytes) {
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
  long long int getSessionUploadLength() const {
    return sessionUploadLength;
  }

  /**
   * Returns the number of bytes downloaded from the remote host.
   */
  long long int getSessionDownloadLength() const {
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
  
  void setBitfield(const unsigned char* bitfield, int bitfieldLength) {
    this->bitfield->setBitfield(bitfield, bitfieldLength);
  }
  const unsigned char* getBitfield() const { return bitfield->getBitfield(); }
  int getBitfieldLength() const { return bitfield->getBitfieldLength(); }
  void setAllBitfield();

  /**
   * operation = 1: set index-th bit to 1
   * operation = 0: set index-th bit to 0
   */
  void updateBitfield(int index, int operation);
  
  void setFastExtensionEnabled(bool enabled) {
    fastExtensionEnabled = enabled;
  }
  bool isFastExtensionEnabled() const { return fastExtensionEnabled; }

  void addFastSetIndex(int index);
  const Integers& getFastSet() const { return fastSet; }
  bool isInFastSet(int index) const;
  int countFastSet() const { return fastSet.size(); }

  void addPeerAllowedIndex(int index);
  bool isInPeerAllowedIndexSet(int index) const;

  void addAmAllowedIndex(int index);
  bool isInAmAllowedIndexSet(int index) const;

  bool shouldBeChoking() const;

  bool hasPiece(int index) const;

  bool isSeeder() const;

  void updateLatency(int latency);
  int getLatency() const { return latency; }

  const string& getId() const {
    return id;
  }

  void startBadCondition();

  bool isGood() const;
};

typedef SharedHandle<Peer> PeerHandle;
typedef deque<PeerHandle> Peers;

#endif // _D_PEER_H_
