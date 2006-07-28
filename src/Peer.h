/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_PEER_H_
#define _D_PEER_H_

#include "common.h"
#include "BitfieldMan.h"
#include "SharedHandle.h"
#include <string.h>
#include <string>

using namespace std;

#define PEER_ID_LENGTH 20
#define DEFAULT_LATENCY 1500

class Peer {
  friend bool operator==(const Peer& p1, const Peer& p2);
  friend bool operator!=(const Peer& p1, const Peer& p2);
public:
  int entryId;
  string ipaddr;
  int port;
  bool amChoking;
  bool amInterested;
  bool peerChoking;
  bool peerInterested;
  int tryCount;
  int error;
  int cuid;
  bool chokingRequired;
  bool optUnchoking;
  bool snubbing;
private:
  char peerId[PEER_ID_LENGTH];
  BitfieldMan* bitfield;
  bool fastExtensionEnabled;
  // allowed fast indexes that peer has sent by Allowed Fast message
  Integers fastSet;
  long long int peerUpload;
  long long int peerDownload;
  int pieceLength;
  int deltaUpload;
  int deltaDownload;
  int latency;
public:
  Peer(string ipaddr, int port, int pieceLength, long long int totalLength)
    :entryId(0), ipaddr(ipaddr), port(port), error(0),
     peerUpload(0), peerDownload(0), pieceLength(pieceLength)
  {
    resetStatus();
    this->bitfield = new BitfieldMan(pieceLength, totalLength);
  }

  Peer():entryId(0), ipaddr(""), port(0), bitfield(0),
	 peerUpload(0), peerDownload(0), pieceLength(0)
  {
    resetStatus();
  }

  ~Peer() {
    if(bitfield != NULL) {
      delete bitfield;
    }
  }

  bool operator==(const Peer& p) {
    return ipaddr == p.ipaddr && port == p.port;
  }
  
  bool operator!=(const Peer& p) {
    return !(*this == p);
  }

  void resetStatus();

  void addDeltaUpload(int length) {
    this->deltaUpload += length;
  }
  void resetDeltaUpload() { this->deltaUpload = 0; }
  int getDeltaUpload() const { return this->deltaUpload; }

  void addDeltaDownload(int length) {
    this->deltaDownload += length;
  }
  void resetDeltaDownload() { this->deltaDownload = 0; }
  int getDeltaDownload() const { return this->deltaDownload; }

  void setPeerId(const char* peerId) {
    memcpy(this->peerId, peerId, PEER_ID_LENGTH);
  }
  const char* getPeerId() const { return this->peerId; }
  
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
  
  void addPeerUpload(int size) {
    peerUpload += size;
    addDeltaUpload(size);
  }
  void setPeerUpload(long long int size) { peerUpload = size; }
  long long int getPeerUpload() const { return peerUpload; }

  void addPeerDownload(int size) {
    peerDownload += size;
    addDeltaDownload(size);
  }
  void setPeerDownload(long long int size) { peerDownload = size; }
  long long int getPeerDownload() const { return peerDownload; }

  void setFastExtensionEnabled(bool enabled) {
    fastExtensionEnabled = enabled;
  }
  bool isFastExtensionEnabled() const { return fastExtensionEnabled; }

  void addFastSetIndex(int index);
  const Integers& getFastSet() const { return fastSet; }
  bool isInFastSet(int index) const;
  int countFastSet() const { return fastSet.size(); }

  bool shouldBeChoking() const;

  bool hasPiece(int index) const;

  bool isSeeder() const;

  void updateLatency(int latency);
  int getLatency() const { return latency; }
};

typedef SharedHandle<Peer> PeerHandle;

#endif // _D_PEER_H_
