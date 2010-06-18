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
#include "DefaultBtAnnounce.h"
#include "LogFactory.h"
#include "Logger.h"
#include "PeerListProcessor.h"
#include "util.h"
#include "prefs.h"
#include "DlAbortEx.h"
#include "message.h"
#include "SimpleRandomizer.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "BtRuntime.h"
#include "PeerStorage.h"
#include "Peer.h"
#include "Option.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "Request.h"
#include "bencode2.h"
#include "bittorrent_helper.h"
#include "wallclock.h"

namespace aria2 {

DefaultBtAnnounce::DefaultBtAnnounce
(const SharedHandle<DownloadContext>& downloadContext,
 const Option* option):
  _downloadContext(downloadContext),
  _trackers(0),
  _prevAnnounceTimer(0),
  _interval(DEFAULT_ANNOUNCE_INTERVAL),
  _minInterval(DEFAULT_ANNOUNCE_INTERVAL),
  _userDefinedInterval(0),
  _complete(0),
  _incomplete(0),
  _announceList(bittorrent::getTorrentAttrs(downloadContext)->announceList),
  _option(option),
  _logger(LogFactory::getInstance()),
  _randomizer(SimpleRandomizer::getInstance())
{}

DefaultBtAnnounce::~DefaultBtAnnounce() {
}

bool DefaultBtAnnounce::isDefaultAnnounceReady() {
  return
    (_trackers == 0 &&
     _prevAnnounceTimer.
     difference(global::wallclock) >= (_userDefinedInterval==0?
                                       _minInterval:_userDefinedInterval) &&
     !_announceList.allTiersFailed());
}

bool DefaultBtAnnounce::isStoppedAnnounceReady() {
  return (_trackers == 0 &&
          _btRuntime->isHalt() &&
          _announceList.countStoppedAllowedTier());
}

bool DefaultBtAnnounce::isCompletedAnnounceReady() {
  return (_trackers == 0 &&
          _pieceStorage->allDownloadFinished() &&
          _announceList.countCompletedAllowedTier());
}

bool DefaultBtAnnounce::isAnnounceReady() {
  return
    isStoppedAnnounceReady() ||
    isCompletedAnnounceReady() ||
    isDefaultAnnounceReady();
}

static bool uriHasQuery(const std::string& uri)
{
  Request req;
  req.setUri(uri);
  return !req.getQuery().empty();
}

std::string DefaultBtAnnounce::getAnnounceUrl() {
  if(isStoppedAnnounceReady()) {
    if(!_announceList.currentTierAcceptsStoppedEvent()) {
      _announceList.moveToStoppedAllowedTier();
    }
    _announceList.setEvent(AnnounceTier::STOPPED);
  } else if(isCompletedAnnounceReady()) {
    if(!_announceList.currentTierAcceptsCompletedEvent()) {
      _announceList.moveToCompletedAllowedTier();
    }
    _announceList.setEvent(AnnounceTier::COMPLETED);
  } else if(isDefaultAnnounceReady()) {
    // If download completed before "started" event is sent to a tracker,
    // we change the event to something else to prevent us from
    // sending "completed" event.
    if(_pieceStorage->allDownloadFinished() &&
       _announceList.getEvent() == AnnounceTier::STARTED) {
      _announceList.setEvent(AnnounceTier::STARTED_AFTER_COMPLETION);
    }
  } else {
    return A2STR::NIL;
  }
  unsigned int numWant = 50;
  if(!_btRuntime->lessThanMinPeers() || _btRuntime->isHalt()) {
    numWant = 0;
  }
  TransferStat stat = _peerStorage->calculateStat();
  uint64_t left =
    _pieceStorage->getTotalLength()-_pieceStorage->getCompletedLength();
  std::string uri = _announceList.getAnnounce();
  uri += uriHasQuery(uri) ? "&" : "?";
  uri += "info_hash=";
  uri += util::torrentPercentEncode(bittorrent::getInfoHash(_downloadContext),
                                    INFO_HASH_LENGTH);
  uri += "&peer_id=";
  uri += util::torrentPercentEncode(bittorrent::getStaticPeerId(),
                                    PEER_ID_LENGTH);
  uri += "&uploaded=";
  uri += util::uitos(stat.getSessionUploadLength());
  uri += "&downloaded=";
  uri += util::uitos(stat.getSessionDownloadLength());
  uri += "&left=";
  uri += util::uitos(left);
  uri += "&compact=1";
  uri += "&key=";
  // Use last 8 bytes of peer ID as a key
  size_t keyLen = 8;
  uri += util::torrentPercentEncode
    (bittorrent::getStaticPeerId()+PEER_ID_LENGTH-keyLen, keyLen);
  uri += "&numwant=";
  uri += util::uitos(numWant);
  uri += "&no_peer_id=1";
  if(_btRuntime->getListenPort() > 0) {
    uri += "&port=";
    uri += util::uitos(_btRuntime->getListenPort());
  }
  std::string event = _announceList.getEventString();
  if(!event.empty()) {
    uri += "&event=";
    uri += event;
  }
  if(!_trackerId.empty()) {
    uri += "&trackerid="+util::torrentPercentEncode(_trackerId);
  }
  if(_option->getAsBool(PREF_BT_REQUIRE_CRYPTO)) {
    uri += "&requirecrypto=1";
  } else {
    uri += "&supportcrypto=1";
  }
  if(!_option->blank(PREF_BT_EXTERNAL_IP)) {
    uri += "&ip=";
    uri += _option->get(PREF_BT_EXTERNAL_IP);
  }
  return uri;
}

void DefaultBtAnnounce::announceStart() {
  ++_trackers;
}

void DefaultBtAnnounce::announceSuccess() {
  _trackers = 0;
  _announceList.announceSuccess();
}

void DefaultBtAnnounce::announceFailure() {
  _trackers = 0;
  _announceList.announceFailure();
}

bool DefaultBtAnnounce::isAllAnnounceFailed() {
  return _announceList.allTiersFailed();
}

void DefaultBtAnnounce::resetAnnounce() {
  _prevAnnounceTimer = global::wallclock;
  _announceList.resetTier();
}

void
DefaultBtAnnounce::processAnnounceResponse(const unsigned char* trackerResponse,
                                           size_t trackerResponseLength)
{
  if(_logger->debug()) {
    _logger->debug("Now processing tracker response.");
  }
  SharedHandle<ValueBase> decodedValue =
    bencode2::decode(trackerResponse, trackerResponseLength);
  const Dict* dict = asDict(decodedValue);
  if(!dict) {
    throw DL_ABORT_EX(MSG_NULL_TRACKER_RESPONSE);
  }
  const String* failure = asString(dict->get(BtAnnounce::FAILURE_REASON));
  if(failure) {
    throw DL_ABORT_EX
      (StringFormat(EX_TRACKER_FAILURE, failure->s().c_str()).str());
  }
  const String* warn = asString(dict->get(BtAnnounce::WARNING_MESSAGE));
  if(warn) {
    _logger->warn(MSG_TRACKER_WARNING_MESSAGE, warn->s().c_str());
  }
  const String* tid = asString(dict->get(BtAnnounce::TRACKER_ID));
  if(tid) {
    _trackerId = tid->s();
    if(_logger->debug()) {
      _logger->debug("Tracker ID:%s", _trackerId.c_str());
    }
  }
  const Integer* ival = asInteger(dict->get(BtAnnounce::INTERVAL));
  if(ival && ival->i() > 0) {
    _interval = ival->i();
    if(_logger->debug()) {
      _logger->debug("Interval:%d", _interval);
    }
  }
  const Integer* mival = asInteger(dict->get(BtAnnounce::MIN_INTERVAL));
  if(mival && mival->i() > 0) {
    _minInterval = mival->i();
    if(_logger->debug()) {
      _logger->debug("Min interval:%d", _minInterval);
    }
    _minInterval = std::min(_minInterval, _interval);
  } else {
    // Use interval as a minInterval if minInterval is not supplied.
    _minInterval = _interval;
  }
  const Integer* comp = asInteger(dict->get(BtAnnounce::COMPLETE));
  if(comp) {
    _complete = comp->i();
    if(_logger->debug()) {
      _logger->debug("Complete:%d", _complete);
    }
  }
  const Integer* incomp = asInteger(dict->get(BtAnnounce::INCOMPLETE));
  if(incomp) {
    _incomplete = incomp->i();
    if(_logger->debug()) {
      _logger->debug("Incomplete:%d", _incomplete);
    }
  }
  const SharedHandle<ValueBase>& peerData = dict->get(BtAnnounce::PEERS);
  if(peerData.isNull()) {
    _logger->info(MSG_NO_PEER_LIST_RECEIVED);
  } else {
    if(!_btRuntime->isHalt() && _btRuntime->lessThanMinPeers()) {
      std::vector<SharedHandle<Peer> > peers;
      PeerListProcessor().extractPeer(peerData, std::back_inserter(peers));
      _peerStorage->addPeer(peers);
    }
  }
}

bool DefaultBtAnnounce::noMoreAnnounce() {
  return (_trackers == 0 &&
          _btRuntime->isHalt() &&
          !_announceList.countStoppedAllowedTier());
}

void DefaultBtAnnounce::shuffleAnnounce() {
  _announceList.shuffle();
}

void DefaultBtAnnounce::setRandomizer(const RandomizerHandle& randomizer)
{
  _randomizer = randomizer;
}

void DefaultBtAnnounce::setBtRuntime(const BtRuntimeHandle& btRuntime)
{
  _btRuntime = btRuntime;
}

void DefaultBtAnnounce::setPieceStorage(const PieceStorageHandle& pieceStorage)
{
  _pieceStorage = pieceStorage;
}

void DefaultBtAnnounce::setPeerStorage(const PeerStorageHandle& peerStorage)
{
  _peerStorage = peerStorage;
}

void DefaultBtAnnounce::overrideMinInterval(time_t interval)
{
  _minInterval = interval;
}

} // namespace aria2
