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
#include "Peer.h"

Peer* Peer::nullPeer = new Peer("", 0, 0, 0);

void Peer::updateBitfield(int index, int operation) {
  if(operation == 1) {
    bitfield->setBit(index);
  } else if(operation == 0) {
    bitfield->unsetBit(index);
  }
}

#define THRESHOLD 1024*1024*2

bool Peer::shouldBeChoking() const {
  if(optUnchoking) {
    return false;
  }
  return chokingRequired;
}

bool Peer::hasPiece(int index) const {
  return bitfield->isBitSet(index);
}

bool Peer::isSeeder() const {
  return bitfield->isAllBitSet();
}

void Peer::resetStatus() {
  tryCount = 0;
  cuid = 0;
  amChoking = true;
  amInterested = false;
  peerChoking = true;
  peerInterested = false;
  resetDeltaUpload();
  resetDeltaDownload();
  chokingRequired = true;
  optUnchoking = false;
  snubbing = false;
  fastExtensionEnabled = false;
  latency = DEFAULT_LATENCY;
  fastSet.clear();
}

bool Peer::isInFastSet(int index) const {
  return find(fastSet.begin(), fastSet.end(), index) != fastSet.end();
}

void Peer::addFastSetIndex(int index) {
  if(!isInFastSet(index)) {
    fastSet.push_back(index);
  }
}

void Peer::setAllBitfield() {
  bitfield->setAllBit();
}

void Peer::updateLatency(int latency) {
  this->latency = (this->latency*20+latency*80)/200;
}
