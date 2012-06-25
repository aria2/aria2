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
#include "Peer.h"

#include <cstring>
#include <cassert>

#include "util.h"
#include "a2functional.h"
#include "PeerSessionResource.h"
#include "BtMessageDispatcher.h"
#include "wallclock.h"

namespace aria2 {

#define BAD_CONDITION_INTERVAL 10

Peer::Peer(std::string ipaddr, uint16_t port, bool incoming):
  ipaddr_(ipaddr),
  port_(port),
  id_(fmt("%s(%u)", ipaddr_.c_str(), port_)),
  firstContactTime_(global::wallclock()),
  badConditionStartTime_(0),
  seeder_(false),
  res_(0),
  incoming_(incoming),
  localPeer_(false),
  disconnectedGracefully_(false)
{
  memset(peerId_, 0, PEER_ID_LENGTH);
  resetStatus();
}

Peer::~Peer()
{
  releaseSessionResource();
}

void Peer::usedBy(cuid_t cuid)
{
  cuid_ = cuid;
}

void Peer::allocateSessionResource(int32_t pieceLength, int64_t totalLength)
{
  delete res_;
  res_ = new PeerSessionResource(pieceLength, totalLength);
  res_->getPeerStat().downloadStart();
  updateSeeder();
}

void Peer::reconfigureSessionResource(int32_t pieceLength, int64_t totalLength)
{
  assert(res_);
  res_->reconfigure(pieceLength, totalLength);
}

void Peer::releaseSessionResource()
{
  delete res_;
  res_ = 0;
}

void Peer::setPeerId(const unsigned char* peerId)
{
  memcpy(peerId_, peerId, PEER_ID_LENGTH);
}

void Peer::resetStatus() {
  cuid_ = 0;
}

bool Peer::amChoking() const
{
  assert(res_);
  return res_->amChoking();
}

void Peer::amChoking(bool b) const
{
  assert(res_);
  res_->amChoking(b);
}

// localhost is interested in this peer
bool Peer::amInterested() const
{
  assert(res_);
  return res_->amInterested();
}

void Peer::amInterested(bool b) const
{
  assert(res_);
  res_->amInterested(b);
}

// this peer is choking localhost
bool Peer::peerChoking() const
{
  assert(res_);
  return res_->peerChoking();
}

void Peer::peerChoking(bool b) const
{
  assert(res_);
  res_->peerChoking(b);
}

// this peer is interested in localhost
bool Peer::peerInterested() const
{
  assert(res_);
  return res_->peerInterested();
}

void Peer::peerInterested(bool b)
{
  assert(res_);
  res_->peerInterested(b);
}
  
// this peer should be choked
bool Peer::chokingRequired() const
{
  assert(res_);
  return res_->chokingRequired();
}

void Peer::chokingRequired(bool b)
{
  assert(res_);
  res_->chokingRequired(b);
}

// this peer is eligible for unchoking optionally.
bool Peer::optUnchoking() const
{
  assert(res_);
  return res_->optUnchoking();
}

void Peer::optUnchoking(bool b)
{
  assert(res_);
  res_->optUnchoking(b);
}

// this peer is snubbing.
bool Peer::snubbing() const
{
  assert(res_);
  return res_->snubbing();
}

void Peer::snubbing(bool b)
{
  assert(res_);
  res_->snubbing(b);
}

void Peer::updateUploadLength(int32_t bytes)
{
  assert(res_);
  res_->updateUploadLength(bytes);
}

void Peer::updateDownloadLength(int32_t bytes)
{
  assert(res_);
  res_->updateDownloadLength(bytes);
}

void Peer::updateSeeder()
{
  assert(res_);
  seeder_ = res_->hasAllPieces();
}

void Peer::updateBitfield(size_t index, int operation) {
  assert(res_);
  res_->updateBitfield(index, operation);
  updateSeeder();
}

int Peer::calculateUploadSpeed()
{
  assert(res_);
  return res_->getPeerStat().calculateUploadSpeed();
}

int Peer::calculateDownloadSpeed()
{
  assert(res_);
  return res_->getPeerStat().calculateDownloadSpeed();
}

int64_t Peer::getSessionUploadLength() const
{
  assert(res_);
  return res_->uploadLength();
}

int64_t Peer::getSessionDownloadLength() const
{
  assert(res_);
  return res_->downloadLength();
}

void Peer::setBitfield(const unsigned char* bitfield, size_t bitfieldLength)
{
  assert(res_);
  res_->setBitfield(bitfield, bitfieldLength);
  updateSeeder();
}

const unsigned char* Peer::getBitfield() const
{
  assert(res_);
  return res_->getBitfield();
}

size_t Peer::getBitfieldLength() const
{
  assert(res_);
  return res_->getBitfieldLength();
}

bool Peer::shouldBeChoking() const {
  assert(res_);
  return res_->shouldBeChoking();
}

bool Peer::hasPiece(size_t index) const {
  assert(res_);
  return res_->hasPiece(index);
}

void Peer::setFastExtensionEnabled(bool enabled)
{
  assert(res_);
  return res_->fastExtensionEnabled(enabled);
}

bool Peer::isFastExtensionEnabled() const
{
  assert(res_);
  return res_->fastExtensionEnabled();
}

size_t Peer::countPeerAllowedIndexSet() const
{
  assert(res_);
  return res_->peerAllowedIndexSet().size();
}

const std::set<size_t>& Peer::getPeerAllowedIndexSet() const
{
  assert(res_);
  return res_->peerAllowedIndexSet();
}

bool Peer::isInPeerAllowedIndexSet(size_t index) const
{
  assert(res_);
  return res_->peerAllowedIndexSetContains(index);
}

void Peer::addPeerAllowedIndex(size_t index)
{
  assert(res_);
  res_->addPeerAllowedIndex(index);
}

bool Peer::isInAmAllowedIndexSet(size_t index) const
{
  assert(res_);
  return res_->amAllowedIndexSetContains(index);
}

void Peer::addAmAllowedIndex(size_t index)
{
  assert(res_);
  res_->addAmAllowedIndex(index);
}

void Peer::setAllBitfield() {
  assert(res_);
  res_->markSeeder();
  updateSeeder();
}

void Peer::startBadCondition()
{
  badConditionStartTime_ = global::wallclock();
}

bool Peer::isGood() const
{
  return badConditionStartTime_.
    difference(global::wallclock()) >= BAD_CONDITION_INTERVAL;
}

uint8_t Peer::getExtensionMessageID(const std::string& name) const
{
  assert(res_);
  return res_->getExtensionMessageID(name);
}

std::string Peer::getExtensionName(uint8_t id) const
{
  assert(res_);
  return res_->getExtensionName(id);
}

void Peer::setExtension(const std::string& name, uint8_t id)
{
  assert(res_);
  res_->addExtension(name, id);
}

void Peer::setExtendedMessagingEnabled(bool enabled)
{
  assert(res_);
  res_->extendedMessagingEnabled(enabled);
}

bool Peer::isExtendedMessagingEnabled() const
{
  assert(res_);
  return res_->extendedMessagingEnabled();
}

void Peer::setDHTEnabled(bool enabled)
{
  assert(res_);
  res_->dhtEnabled(enabled);
}

bool Peer::isDHTEnabled() const
{
  assert(res_);
  return res_->dhtEnabled();
}

const Timer& Peer::getLastDownloadUpdate() const
{
  assert(res_);
  return res_->getLastDownloadUpdate();
}

const Timer& Peer::getLastAmUnchoking() const
{
  assert(res_);
  return res_->getLastAmUnchoking();
}

int64_t Peer::getCompletedLength() const
{
  assert(res_);
  return res_->getCompletedLength();
}

void Peer::setIncomingPeer(bool incoming)
{
  incoming_ = incoming;
}

void Peer::setFirstContactTime(const Timer& time)
{
  firstContactTime_ = time;
}

void Peer::setBtMessageDispatcher(BtMessageDispatcher* dpt)
{
  assert(res_);
  res_->setBtMessageDispatcher(dpt);
}

size_t Peer::countOutstandingUpload() const
{
  assert(res_);
  return res_->countOutstandingUpload();
}

} // namespace aria2
