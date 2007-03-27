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
#include "Peer.h"
#include "BitfieldManFactory.h"
#include "Util.h"

Peer::Peer(string ipaddr, int port, int pieceLength, long long int totalLength):
  ipaddr(ipaddr),
  port(port),
  sessionUploadLength(0),
  sessionDownloadLength(0),
  pieceLength(pieceLength),
  active(false),
  _badConditionStartTime(0),
  _badConditionInterval(10)
{
  resetStatus();
  this->bitfield = BitfieldManFactory::getFactoryInstance()->
    createBitfieldMan(pieceLength, totalLength);
  string idSeed = ipaddr+":"+Util::itos(port);
  id = Util::simpleMessageDigest(idSeed);
}

/*
Peer::Peer():entryId(0), ipaddr(""), port(0), bitfield(0),
       sessionUploadLength(0), sessionDownloadLength(0),
       pieceLength(0)
{
  resetStatus();
}
*/

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
  chokingRequired = true;
  optUnchoking = false;
  snubbing = false;
  fastExtensionEnabled = false;
  latency = DEFAULT_LATENCY;
  fastSet.clear();
  peerAllowedIndexSet.clear();
  amAllowedIndexSet.clear();
  peerStat.reset();
}

bool Peer::isInFastSet(int index) const {
  return find(fastSet.begin(), fastSet.end(), index) != fastSet.end();
}

void Peer::addFastSetIndex(int index) {
  if(!isInFastSet(index)) {
    fastSet.push_back(index);
  }
}

bool Peer::isInPeerAllowedIndexSet(int index) const {
  return find(peerAllowedIndexSet.begin(), peerAllowedIndexSet.end(),
	      index) != peerAllowedIndexSet.end();
}

void Peer::addPeerAllowedIndex(int index) {
  if(!isInPeerAllowedIndexSet(index)) {
    peerAllowedIndexSet.push_back(index);
  }
}

bool Peer::isInAmAllowedIndexSet(int index) const {
  return find(amAllowedIndexSet.begin(), amAllowedIndexSet.end(),
	      index) != amAllowedIndexSet.end();
}

void Peer::addAmAllowedIndex(int index) {
  if(!isInAmAllowedIndexSet(index)) {
    amAllowedIndexSet.push_back(index);
  }
}

void Peer::setAllBitfield() {
  bitfield->setAllBit();
}

void Peer::updateLatency(int latency) {
  this->latency = (this->latency*20+latency*80)/200;
}

void Peer::startBadCondition()
{
  _badConditionStartTime.reset();
}

bool Peer::isGood() const
{
  return _badConditionStartTime.elapsed(_badConditionInterval);
}
