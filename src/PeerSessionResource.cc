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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <cassert>
#include <algorithm>

#include "BitfieldMan.h"
#include "A2STR.h"
#include "BtMessageDispatcher.h"
#include "wallclock.h"

namespace aria2 {

PeerSessionResource::PeerSessionResource(int32_t pieceLength, int64_t totalLength)
  :
  amChoking_(true),
  amInterested_(false),
  peerChoking_(true),
  peerInterested_(false),
  chokingRequired_(true),
  optUnchoking_(false),
  snubbing_(false),
  bitfieldMan_(new BitfieldMan(pieceLength, totalLength)),
  fastExtensionEnabled_(false),
  extendedMessagingEnabled_(false),
  dhtEnabled_(false),
  lastDownloadUpdate_(0),
  lastAmUnchoking_(0),
  dispatcher_(0)
{}

PeerSessionResource::~PeerSessionResource()
{
  delete bitfieldMan_;
}

void PeerSessionResource::amChoking(bool b)
{
  amChoking_ = b;
  if(!b) {
    lastAmUnchoking_ = global::wallclock();
  }
}

void PeerSessionResource::amInterested(bool b)
{
  amInterested_ = b;
}

void PeerSessionResource::peerChoking(bool b)
{
  peerChoking_ = b;
}

void PeerSessionResource::peerInterested(bool b)
{
  peerInterested_ = b;
}
  
void PeerSessionResource::chokingRequired(bool b)
{
  chokingRequired_ = b;
}

void PeerSessionResource::optUnchoking(bool b)
{
  optUnchoking_ = b;
}

bool PeerSessionResource::shouldBeChoking() const
{
  if(optUnchoking_) {
    return false;
  }
  return chokingRequired_;
}

void PeerSessionResource::snubbing(bool b)
{
  snubbing_ = b;
  if(snubbing_) {
    chokingRequired(true);
    optUnchoking(false);
  }
}

bool PeerSessionResource::hasAllPieces() const
{
  return bitfieldMan_->isAllBitSet();
}

void PeerSessionResource::updateBitfield(size_t index, int operation)
{
  if(operation == 1) {
    bitfieldMan_->setBit(index);
  } else if(operation == 0) {
    bitfieldMan_->unsetBit(index);
  }
}

void PeerSessionResource::setBitfield
(const unsigned char* bitfield, size_t bitfieldLength)
{
  bitfieldMan_->setBitfield(bitfield, bitfieldLength);
}

const unsigned char* PeerSessionResource::getBitfield() const
{
  return bitfieldMan_->getBitfield();
}

size_t PeerSessionResource::getBitfieldLength() const
{
  return bitfieldMan_->getBitfieldLength();
}

bool PeerSessionResource::hasPiece(size_t index) const
{
  return bitfieldMan_->isBitSet(index);
}

void PeerSessionResource::markSeeder()
{
  bitfieldMan_->setAllBit();
}

void PeerSessionResource::fastExtensionEnabled(bool b)
{
  fastExtensionEnabled_ = b;
}

const std::set<size_t>& PeerSessionResource::peerAllowedIndexSet() const
{
  return peerAllowedIndexSet_;
}

void PeerSessionResource::addPeerAllowedIndex(size_t index)
{
  peerAllowedIndexSet_.insert(index);
}

bool PeerSessionResource::peerAllowedIndexSetContains(size_t index) const
{
  return peerAllowedIndexSet_.count(index) == 1;
}

void PeerSessionResource::addAmAllowedIndex(size_t index)
{
  amAllowedIndexSet_.insert(index);
}

bool PeerSessionResource::amAllowedIndexSetContains(size_t index) const
{
  return amAllowedIndexSet_.count(index) == 1;
}

void PeerSessionResource::extendedMessagingEnabled(bool b)
{
  extendedMessagingEnabled_ = b;
}

uint8_t
PeerSessionResource::getExtensionMessageID(const std::string& name) const
{
  Extensions::const_iterator itr = extensions_.find(name);
  if(itr == extensions_.end()) {
    return 0;
  } else {
    return (*itr).second;
  }
}

std::string PeerSessionResource::getExtensionName(uint8_t id) const
{
  for(Extensions::const_iterator itr = extensions_.begin(),
        eoi = extensions_.end(); itr != eoi; ++itr) {
    const Extensions::value_type& p = *itr;
    if(p.second == id) {
      return p.first;
    }
  }
  return A2STR::NIL;
}

void PeerSessionResource::addExtension(const std::string& name, uint8_t id)
{
  extensions_[name] = id;
}

void PeerSessionResource::dhtEnabled(bool b)
{
  dhtEnabled_ = b;
}

int64_t PeerSessionResource::uploadLength() const
{
  return peerStat_.getSessionUploadLength();
}

void PeerSessionResource::updateUploadLength(int32_t bytes)
{
  peerStat_.updateUploadLength(bytes);
}

int64_t PeerSessionResource::downloadLength() const
{
  return peerStat_.getSessionDownloadLength();
}

void PeerSessionResource::updateDownloadLength(int32_t bytes)
{
  peerStat_.updateDownloadLength(bytes);

  lastDownloadUpdate_ = global::wallclock();
}

int64_t PeerSessionResource::getCompletedLength() const
{
  return bitfieldMan_->getCompletedLength();
}

void PeerSessionResource::setBtMessageDispatcher(BtMessageDispatcher* dpt)
{
  dispatcher_ = dpt;
}

size_t PeerSessionResource::countOutstandingUpload() const
{
  assert(dispatcher_);
  return dispatcher_->countOutstandingUpload();
}

void PeerSessionResource::reconfigure(int32_t pieceLength, int64_t totalLenth)
{
  delete bitfieldMan_;
  bitfieldMan_ = new BitfieldMan(pieceLength, totalLenth);
}

} // namespace aria2
