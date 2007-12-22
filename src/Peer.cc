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
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST

Peer::Peer(string ipaddr, uint16_t port, int32_t pieceLength, int64_t totalLength):
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
#ifdef ENABLE_MESSAGE_DIGEST
  id = MessageDigestHelper::digestString("sha1", idSeed);
#else
  id = idSeed;
#endif // ENABLE_MESSAGE_DIGEST
}

void Peer::reconfigure(int32_t pieceLength, int64_t totalLength)
{
  delete bitfield;
  this->pieceLength = pieceLength;
  this->bitfield = BitfieldManFactory::getFactoryInstance()->
    createBitfieldMan(this->pieceLength, totalLength);  
}

/*
Peer::Peer():entryId(0), ipaddr(""), port(0), bitfield(0),
       sessionUploadLength(0), sessionDownloadLength(0),
       pieceLength(0)
{
  resetStatus();
}
*/

void Peer::updateBitfield(int32_t index, int32_t operation) {
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

bool Peer::hasPiece(int32_t index) const {
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
  _extendedMessagingEnabled = false;
  _extensions.clear();
  latency = DEFAULT_LATENCY;
  peerAllowedIndexSet.clear();
  amAllowedIndexSet.clear();
  peerStat.reset();
}

bool Peer::isInPeerAllowedIndexSet(int32_t index) const {
  return find(peerAllowedIndexSet.begin(), peerAllowedIndexSet.end(),
	      index) != peerAllowedIndexSet.end();
}

void Peer::addPeerAllowedIndex(int32_t index) {
  if(!isInPeerAllowedIndexSet(index)) {
    peerAllowedIndexSet.push_back(index);
  }
}

bool Peer::isInAmAllowedIndexSet(int32_t index) const {
  return find(amAllowedIndexSet.begin(), amAllowedIndexSet.end(),
	      index) != amAllowedIndexSet.end();
}

void Peer::addAmAllowedIndex(int32_t index) {
  if(!isInAmAllowedIndexSet(index)) {
    amAllowedIndexSet.push_back(index);
  }
}

void Peer::setAllBitfield() {
  bitfield->setAllBit();
}

void Peer::updateLatency(int32_t latency) {
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

uint8_t Peer::getExtensionMessageID(const string& name)
{
  Extensions::const_iterator itr = _extensions.find(name);
  if(itr == _extensions.end()) {
    return 0;
  } else {
    return (*itr).second;
  }
}

string Peer::getExtensionName(uint8_t id)
{
  for(Extensions::const_iterator itr = _extensions.begin();
      itr != _extensions.end(); ++itr) {
    const Extensions::value_type& p = *itr;
    if(p.second == id) {
      return p.first;
    }
  }
  return "";
}

void Peer::setExtension(const string& name, uint8_t id)
{
  _extensions[name] = id;
}
