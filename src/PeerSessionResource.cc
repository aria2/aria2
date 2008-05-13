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
#include "PeerSessionResource.h"
#include "BitfieldManFactory.h"
#include "BitfieldMan.h"
#include "A2STR.h"
#include <algorithm>

namespace aria2 {

PeerSessionResource::PeerSessionResource(size_t pieceLength, uint64_t totalLength):
  _amChoking(true),
  _amInterested(false),
  _peerChoking(true),
  _peerInterested(false),
  _chokingRequired(true),
  _optUnchoking(false),
  _snubbing(false),
  _bitfieldMan(BitfieldManFactory::getFactoryInstance()->createBitfieldMan(pieceLength, totalLength)),
  _fastExtensionEnabled(false),
  _extendedMessagingEnabled(false),
  _dhtEnabled(false),
  _latency(DEFAULT_LATENCY),
  _uploadLength(0),
  _downloadLength(0),
  _lastDownloadUpdate(0),
  _lastAmUnchoking(0)
{}

PeerSessionResource::~PeerSessionResource()
{
  delete _bitfieldMan;
}

bool PeerSessionResource::amChoking() const
{
  return _amChoking;
}

void PeerSessionResource::amChoking(bool b)
{
  _amChoking = b;
  if(!b) {
    _lastAmUnchoking.reset();
  }
}

bool PeerSessionResource::amInterested() const
{
  return _amInterested;
}

void PeerSessionResource::amInterested(bool b)
{
  _amInterested = b;
}

bool PeerSessionResource::peerChoking() const
{
  return _peerChoking;
}

void PeerSessionResource::peerChoking(bool b)
{
  _peerChoking = b;
}

bool PeerSessionResource::peerInterested() const
{
  return _peerInterested;
}

void PeerSessionResource::peerInterested(bool b)
{
  _peerInterested = b;
}
  
bool PeerSessionResource::chokingRequired() const
{
  return _chokingRequired;
}

void PeerSessionResource::chokingRequired(bool b)
{
  _chokingRequired = b;
}

bool PeerSessionResource::optUnchoking() const
{
  return _optUnchoking;
}

void PeerSessionResource::optUnchoking(bool b)
{
  _optUnchoking = b;
}

bool PeerSessionResource::shouldBeChoking() const
{
  if(_optUnchoking) {
    return false;
  }
  return _chokingRequired;
}

bool PeerSessionResource::snubbing() const
{
  return _snubbing;
}

void PeerSessionResource::snubbing(bool b)
{
  _snubbing = b;
  if(_snubbing) {
    chokingRequired(true);
    optUnchoking(false);
  }
}

bool PeerSessionResource::hasAllPieces() const
{
  return _bitfieldMan->isAllBitSet();
}

void PeerSessionResource::updateBitfield(size_t index, int operation)
{
  if(operation == 1) {
    _bitfieldMan->setBit(index);
  } else if(operation == 0) {
    _bitfieldMan->unsetBit(index);
  }
}

void PeerSessionResource::setBitfield(const unsigned char* bitfield, size_t bitfieldLength)
{
  _bitfieldMan->setBitfield(bitfield, bitfieldLength);
}

const unsigned char* PeerSessionResource::getBitfield() const
{
  return _bitfieldMan->getBitfield();
}

size_t PeerSessionResource::getBitfieldLength() const
{
  return _bitfieldMan->getBitfieldLength();
}

bool PeerSessionResource::hasPiece(size_t index) const
{
  return _bitfieldMan->isBitSet(index);
}

void PeerSessionResource::markSeeder()
{
  _bitfieldMan->setAllBit();
}

bool PeerSessionResource::fastExtensionEnabled() const
{
  return _fastExtensionEnabled;
}

void PeerSessionResource::fastExtensionEnabled(bool b)
{
  _fastExtensionEnabled = b;
}

const std::deque<size_t>& PeerSessionResource::peerAllowedIndexSet() const
{
  return _peerAllowedIndexSet;
}

template<typename T>
bool PeerSessionResource::indexIncluded(const std::deque<T>& c, T index) const
{
  return std::find(c.begin(), c.end(), index) != c.end();
}

void PeerSessionResource::addPeerAllowedIndex(size_t index)
{
  if(!indexIncluded(_peerAllowedIndexSet, index)) {
    _peerAllowedIndexSet.push_back(index);
  }
}

bool PeerSessionResource::peerAllowedIndexSetContains(size_t index) const
{
  return indexIncluded(_peerAllowedIndexSet, index);
}

const std::deque<size_t>& PeerSessionResource::amAllowedIndexSet() const
{
  return _amAllowedIndexSet;
}

void PeerSessionResource::addAmAllowedIndex(size_t index)
{
  if(!indexIncluded(_amAllowedIndexSet, index)) {
    _amAllowedIndexSet.push_back(index);
  }
}

bool PeerSessionResource::amAllowedIndexSetContains(size_t index) const
{
  return indexIncluded(_amAllowedIndexSet, index);
}

bool PeerSessionResource::extendedMessagingEnabled() const
{
  return _extendedMessagingEnabled;
}

void PeerSessionResource::extendedMessagingEnabled(bool b)
{
  _extendedMessagingEnabled = b;
}

uint8_t
PeerSessionResource::getExtensionMessageID(const std::string& name) const
{
  Extensions::const_iterator itr = _extensions.find(name);
  if(itr == _extensions.end()) {
    return 0;
  } else {
    return (*itr).second;
  }
}

std::string PeerSessionResource::getExtensionName(uint8_t id) const
{
  for(Extensions::const_iterator itr = _extensions.begin();
      itr != _extensions.end(); ++itr) {
    const Extensions::value_type& p = *itr;
    if(p.second == id) {
      return p.first;
    }
  }
  return A2STR::NIL;
}

void PeerSessionResource::addExtension(const std::string& name, uint8_t id)
{
  _extensions[name] = id;
}

bool PeerSessionResource::dhtEnabled() const
{
  return _dhtEnabled;
}

void PeerSessionResource::dhtEnabled(bool b)
{
  _dhtEnabled = b;
}

PeerStat& PeerSessionResource::getPeerStat()
{
  return _peerStat;
}

unsigned int PeerSessionResource::latency() const
{
  return _latency;
}

void PeerSessionResource::updateLatency(unsigned int latency)
{
  _latency = _latency*0.2+latency*0.8;
}

uint64_t PeerSessionResource::uploadLength() const
{
  return _uploadLength;
}

void PeerSessionResource::updateUploadLength(size_t bytes)
{
  _peerStat.updateUploadLength(bytes);
  _uploadLength += bytes;
}

uint64_t PeerSessionResource::downloadLength() const
{
  return _downloadLength;
}

void PeerSessionResource::updateDownloadLength(size_t bytes)
{
  _peerStat.updateDownloadLength(bytes);
  _downloadLength += bytes;

  _lastDownloadUpdate.reset();
}

const Time& PeerSessionResource::getLastDownloadUpdate() const
{
  return _lastDownloadUpdate;
}

const Time& PeerSessionResource::getLastAmUnchoking() const
{
  return _lastAmUnchoking;
}

} // namespace aria2
