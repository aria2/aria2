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
#include <string.h>
#include <string>

using namespace std;

#define PEER_ID_LENGTH 20

class Peer {
public:
  int entryId;
  string ipaddr;
  int port;
  bool amChocking;
  bool amInterested;
  bool peerChoking;
  bool peerInterested;
  int tryCount;
  int error;
  int cuid;
private:
  char peerId[PEER_ID_LENGTH];
  //unsigned char* bitfield;
  BitfieldMan* bitfield;
  //int bitfieldLength;
  long long int peerUpload;
  long long int peerDownload;
  int pieceLength;
  long long int totalLength;
public:
  Peer(string ipaddr, int port, int pieceLength, long long int totalLength):
    entryId(0), ipaddr(ipaddr), port(port),
    amChocking(true), amInterested(false),
    peerChoking(true), peerInterested(false),
    tryCount(0), error(0), cuid(0),
    bitfield(NULL),// bitfieldLength(0),
    peerUpload(0), peerDownload(0),
    pieceLength(pieceLength), totalLength(totalLength) {
    this->bitfield = new BitfieldMan(pieceLength, totalLength);
  }

  ~Peer() {
    if(bitfield != NULL) {
      delete /*[]*/ bitfield;
    }
  }

  void setPeerId(const char* peerId) {
    memcpy(this->peerId, peerId, PEER_ID_LENGTH);
  }
  const char* getPeerId() const { return this->peerId; }
  
  void setBitfield(const unsigned char* bitfield, int bitfieldLength) {
    this->bitfield->setBitfield(bitfield, bitfieldLength);
    /*
    if(this->bitfield != NULL) {
      delete [] this->bitfield;
    }
    this->bitfieldLength = bitfieldLength;
    this->bitfield = new unsigned char[this->bitfieldLength];
    memcpy(this->bitfield, bitfield, this->bitfieldLength);
    */
  }
  const unsigned char* getBitfield() const { return bitfield->getBitfield(); }
  int getBitfieldLength() const { return bitfield->getBitfieldLength(); }

  /*
  void initBitfield(int pieces);
  */

  /**
   * operation = 1: set index-th bit 1
   * operation = 0: set index-th bit 0
   */
  void updateBitfield(int index, int operation);

  void addPeerUpload(int size) { peerUpload += size; }
  void setPeerUpload(long long int size) { peerUpload = size; }
  long long int getPeerUpload() const { return peerUpload; }

  void addPeerDownload(int size) { peerDownload += size; }
  void setPeerDownload(long long int size) { peerDownload = size; }
  long long int getPeerDownload() const { return peerDownload; }

  bool shouldChoke() const;

  bool hasPiece(int index) const;

  bool isSeeder() const;

  static Peer* nullPeer;
};

#endif // _D_PEER_H_
