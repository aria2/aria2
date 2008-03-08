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
#include "Util.h"
#include "PeerSessionResource.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <cstring>
#include <cassert>

namespace aria2 {

#define BAD_CONDITION_INTERVAL 10

Peer::Peer(std::string ipaddr, uint16_t port):
  ipaddr(ipaddr),
  port(port),
  _badConditionStartTime(0),
  _seeder(false),
  _res(0)
{
  resetStatus();
  std::string idSeed = ipaddr+":"+Util::uitos(port);
#ifdef ENABLE_MESSAGE_DIGEST
  id = MessageDigestHelper::digestString("sha1", idSeed);
#else
  id = idSeed;
#endif // ENABLE_MESSAGE_DIGEST
}

Peer::~Peer()
{
  releaseSessionResource();
}

bool Peer::operator==(const Peer& p)
{
  return id == p.id;
}
  
bool Peer::operator!=(const Peer& p)
{
  return !(*this == p);
}

const std::string& Peer::getID() const
{
  return id;
}

void Peer::usedBy(int32_t cuid)
{
  _cuid = cuid;
}

int32_t Peer::usedBy() const
{
  return _cuid;
}

bool Peer::unused() const
{
  return _cuid == 0;
}

void Peer::allocateSessionResource(size_t pieceLength, uint64_t totalLength)
{
  delete _res;
  _res = new PeerSessionResource(pieceLength, totalLength);
  _res->getPeerStat().downloadStart();
}

void Peer::releaseSessionResource()
{
  delete _res;
  _res = 0;
}

bool Peer::isActive() const
{
  return _res != 0;
}

void Peer::setPeerId(const unsigned char* peerId)
{
  memcpy(_peerId, peerId, PEER_ID_LENGTH);
}

const unsigned char* Peer::getPeerId() const
{
  return _peerId;
}

void Peer::resetStatus() {
  _cuid = 0;
}

bool Peer::amChoking() const
{
  assert(_res);
  return _res->amChoking();
}

void Peer::amChoking(bool b) const
{
  assert(_res);
  _res->amChoking(b);
}

// localhost is interested in this peer
bool Peer::amInterested() const
{
  assert(_res);
  return _res->amInterested();
}

void Peer::amInterested(bool b) const
{
  assert(_res);
  _res->amInterested(b);
}

// this peer is choking localhost
bool Peer::peerChoking() const
{
  assert(_res);
  return _res->peerChoking();
}

void Peer::peerChoking(bool b) const
{
  assert(_res);
  _res->peerChoking(b);
}

// this peer is interested in localhost
bool Peer::peerInterested() const
{
  assert(_res);
  return _res->peerInterested();
}

void Peer::peerInterested(bool b)
{
  assert(_res);
  _res->peerInterested(b);
}
  
  // this peer should be choked
bool Peer::chokingRequired() const
{
  assert(_res);
  return _res->chokingRequired();
}

void Peer::chokingRequired(bool b)
{
  assert(_res);
  _res->chokingRequired(b);
}

// this peer is eligible for unchoking optionally.
bool Peer::optUnchoking() const
{
  assert(_res);
  return _res->optUnchoking();
}

void Peer::optUnchoking(bool b)
{
  assert(_res);
  _res->optUnchoking(b);
}

// this peer is snubbing.
bool Peer::snubbing() const
{
  assert(_res);
  return _res->snubbing();
}

void Peer::snubbing(bool b)
{
  assert(_res);
  _res->snubbing(b);
}

void Peer::updateUploadLength(size_t bytes)
{
  assert(_res);
  _res->updateUploadLength(bytes);
}

void Peer::updateDownloadLength(size_t bytes)
{
  assert(_res);
  _res->updateDownloadLength(bytes);
}

void Peer::updateSeeder()
{
  assert(_res);
  if(_res->hasAllPieces()) {
    _seeder = true;
  }  
}

void Peer::updateBitfield(size_t index, int operation) {
  assert(_res);
  _res->updateBitfield(index, operation);
  updateSeeder();
}

unsigned int Peer::calculateUploadSpeed()
{
  assert(_res);
  return _res->getPeerStat().calculateUploadSpeed();
}

unsigned int Peer::calculateUploadSpeed(const struct timeval& now)
{
  assert(_res);
  return _res->getPeerStat().calculateUploadSpeed(now);
}

unsigned int Peer::calculateDownloadSpeed()
{
  assert(_res);
  return _res->getPeerStat().calculateDownloadSpeed();
}

unsigned int Peer::calculateDownloadSpeed(const struct timeval& now)
{
  assert(_res);
  return _res->getPeerStat().calculateDownloadSpeed(now);
}

uint64_t Peer::getSessionUploadLength() const
{
  assert(_res);
  return _res->uploadLength();
}

uint64_t Peer::getSessionDownloadLength() const
{
  assert(_res);
  return _res->downloadLength();
}

void Peer::setBitfield(const unsigned char* bitfield, size_t bitfieldLength)
{
  assert(_res);
  _res->setBitfield(bitfield, bitfieldLength);
  updateSeeder();
}

const unsigned char* Peer::getBitfield() const
{
  assert(_res);
  return _res->getBitfield();
}

size_t Peer::getBitfieldLength() const
{
  assert(_res);
  return _res->getBitfieldLength();
}

bool Peer::shouldBeChoking() const {
  assert(_res);
  return _res->shouldBeChoking();
}

bool Peer::hasPiece(size_t index) const {
  assert(_res);
  return _res->hasPiece(index);
}

void Peer::setFastExtensionEnabled(bool enabled)
{
  assert(_res);
  return _res->fastExtensionEnabled(enabled);
}

bool Peer::isFastExtensionEnabled() const
{
  assert(_res);
  return _res->fastExtensionEnabled();
}

size_t Peer::countPeerAllowedIndexSet() const
{
  assert(_res);
  return _res->peerAllowedIndexSet().size();
}

const std::deque<size_t>& Peer::getPeerAllowedIndexSet() const
{
  assert(_res);
  return _res->peerAllowedIndexSet();
}

bool Peer::isInPeerAllowedIndexSet(size_t index) const
{
  assert(_res);
  return _res->peerAllowedIndexSetContains(index);
}

void Peer::addPeerAllowedIndex(size_t index)
{
  assert(_res);
  _res->addPeerAllowedIndex(index);
}

bool Peer::isInAmAllowedIndexSet(size_t index) const
{
  assert(_res);
  return _res->amAllowedIndexSetContains(index);
}

void Peer::addAmAllowedIndex(size_t index)
{
  assert(_res);
  _res->addAmAllowedIndex(index);
}

void Peer::setAllBitfield() {
  assert(_res);
  _res->markSeeder();
  _seeder = true;
}

void Peer::updateLatency(unsigned int latency)
{
  assert(_res);
  _res->updateLatency(latency);
}

unsigned int Peer::getLatency() const
{
  assert(_res);
  return _res->latency();
}

void Peer::startBadCondition()
{
  _badConditionStartTime.reset();
}

bool Peer::isGood() const
{
  return _badConditionStartTime.elapsed(BAD_CONDITION_INTERVAL);
}

uint8_t Peer::getExtensionMessageID(const std::string& name) const
{
  assert(_res);
  return _res->getExtensionMessageID(name);
}

std::string Peer::getExtensionName(uint8_t id) const
{
  assert(_res);
  return _res->getExtensionName(id);
}

void Peer::setExtension(const std::string& name, uint8_t id)
{
  assert(_res);
  _res->addExtension(name, id);
}

void Peer::setExtendedMessagingEnabled(bool enabled)
{
  assert(_res);
  _res->extendedMessagingEnabled(enabled);
}

bool Peer::isExtendedMessagingEnabled() const
{
  assert(_res);
  return _res->extendedMessagingEnabled();
}

void Peer::setDHTEnabled(bool enabled)
{
  assert(_res);
  _res->dhtEnabled(enabled);
}

bool Peer::isDHTEnabled() const
{
  assert(_res);
  return _res->dhtEnabled();
}

bool Peer::isSeeder() const
{
  return _seeder;
}

const Time& Peer::getFirstContactTime() const
{
  return _firstContactTime;
}

const Time& Peer::getBadConditionStartTime() const
{
  return _badConditionStartTime;
}

} // namespace aria2
