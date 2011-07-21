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
#include "fmt.h"
#include "A2STR.h"
#include "bencode2.h"
#include "bittorrent_helper.h"
#include "wallclock.h"
#include "uri.h"

namespace aria2 {

DefaultBtAnnounce::DefaultBtAnnounce
(const SharedHandle<DownloadContext>& downloadContext,
 const Option* option)
  : downloadContext_(downloadContext),
    trackers_(0),
    prevAnnounceTimer_(0),
    interval_(DEFAULT_ANNOUNCE_INTERVAL),
    minInterval_(DEFAULT_ANNOUNCE_INTERVAL),
    userDefinedInterval_(0),
    complete_(0),
    incomplete_(0),
    announceList_(bittorrent::getTorrentAttrs(downloadContext)->announceList),
    option_(option),
    randomizer_(SimpleRandomizer::getInstance()),
    tcpPort_(0)
{}

DefaultBtAnnounce::~DefaultBtAnnounce() {
}

bool DefaultBtAnnounce::isDefaultAnnounceReady() {
  return
    (trackers_ == 0 &&
     prevAnnounceTimer_.
     difference(global::wallclock) >= (userDefinedInterval_==0?
                                       minInterval_:userDefinedInterval_) &&
     !announceList_.allTiersFailed());
}

bool DefaultBtAnnounce::isStoppedAnnounceReady() {
  return (trackers_ == 0 &&
          btRuntime_->isHalt() &&
          announceList_.countStoppedAllowedTier());
}

bool DefaultBtAnnounce::isCompletedAnnounceReady() {
  return (trackers_ == 0 &&
          pieceStorage_->allDownloadFinished() &&
          announceList_.countCompletedAllowedTier());
}

bool DefaultBtAnnounce::isAnnounceReady() {
  return
    isStoppedAnnounceReady() ||
    isCompletedAnnounceReady() ||
    isDefaultAnnounceReady();
}

namespace {
bool uriHasQuery(const std::string& uri)
{
  uri::UriStruct us;
  if(uri::parse(us, uri)) {
    return !us.query.empty();
  } else {
    return false;
  }
}
} // namespace

std::string DefaultBtAnnounce::getAnnounceUrl() {
  if(isStoppedAnnounceReady()) {
    if(!announceList_.currentTierAcceptsStoppedEvent()) {
      announceList_.moveToStoppedAllowedTier();
    }
    announceList_.setEvent(AnnounceTier::STOPPED);
  } else if(isCompletedAnnounceReady()) {
    if(!announceList_.currentTierAcceptsCompletedEvent()) {
      announceList_.moveToCompletedAllowedTier();
    }
    announceList_.setEvent(AnnounceTier::COMPLETED);
  } else if(isDefaultAnnounceReady()) {
    // If download completed before "started" event is sent to a tracker,
    // we change the event to something else to prevent us from
    // sending "completed" event.
    if(pieceStorage_->allDownloadFinished() &&
       announceList_.getEvent() == AnnounceTier::STARTED) {
      announceList_.setEvent(AnnounceTier::STARTED_AFTER_COMPLETION);
    }
  } else {
    return A2STR::NIL;
  }
  unsigned int numWant = 50;
  if(!btRuntime_->lessThanMinPeers() || btRuntime_->isHalt()) {
    numWant = 0;
  }
  TransferStat stat = peerStorage_->calculateStat();
  uint64_t left =
    pieceStorage_->getTotalLength()-pieceStorage_->getCompletedLength();
  std::string uri = announceList_.getAnnounce();
  uri += uriHasQuery(uri) ? "&" : "?";
  uri += "info_hash=";
  uri += util::torrentPercentEncode(bittorrent::getInfoHash(downloadContext_),
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
  if(tcpPort_) {
    uri += "&port=";
    uri += util::uitos(tcpPort_);
  }
  std::string event = announceList_.getEventString();
  if(!event.empty()) {
    uri += "&event=";
    uri += event;
  }
  if(!trackerId_.empty()) {
    uri += "&trackerid="+util::torrentPercentEncode(trackerId_);
  }
  if(option_->getAsBool(PREF_BT_REQUIRE_CRYPTO)) {
    uri += "&requirecrypto=1";
  } else {
    uri += "&supportcrypto=1";
  }
  if(!option_->blank(PREF_BT_EXTERNAL_IP)) {
    uri += "&ip=";
    uri += option_->get(PREF_BT_EXTERNAL_IP);
  }
  return uri;
}

void DefaultBtAnnounce::announceStart() {
  ++trackers_;
}

void DefaultBtAnnounce::announceSuccess() {
  trackers_ = 0;
  announceList_.announceSuccess();
}

void DefaultBtAnnounce::announceFailure() {
  trackers_ = 0;
  announceList_.announceFailure();
}

bool DefaultBtAnnounce::isAllAnnounceFailed() {
  return announceList_.allTiersFailed();
}

void DefaultBtAnnounce::resetAnnounce() {
  prevAnnounceTimer_ = global::wallclock;
  announceList_.resetTier();
}

void
DefaultBtAnnounce::processAnnounceResponse(const unsigned char* trackerResponse,
                                           size_t trackerResponseLength)
{
  A2_LOG_DEBUG("Now processing tracker response.");
  SharedHandle<ValueBase> decodedValue =
    bencode2::decode(trackerResponse, trackerResponseLength);
  const Dict* dict = asDict(decodedValue);
  if(!dict) {
    throw DL_ABORT_EX(MSG_NULL_TRACKER_RESPONSE);
  }
  const String* failure = asString(dict->get(BtAnnounce::FAILURE_REASON));
  if(failure) {
    throw DL_ABORT_EX
      (fmt(EX_TRACKER_FAILURE, failure->s().c_str()));
  }
  const String* warn = asString(dict->get(BtAnnounce::WARNING_MESSAGE));
  if(warn) {
    A2_LOG_WARN(fmt(MSG_TRACKER_WARNING_MESSAGE, warn->s().c_str()));
  }
  const String* tid = asString(dict->get(BtAnnounce::TRACKER_ID));
  if(tid) {
    trackerId_ = tid->s();
    A2_LOG_DEBUG(fmt("Tracker ID:%s", trackerId_.c_str()));
  }
  const Integer* ival = asInteger(dict->get(BtAnnounce::INTERVAL));
  if(ival && ival->i() > 0) {
    interval_ = ival->i();
    A2_LOG_DEBUG(fmt("Interval:%ld", static_cast<long int>(interval_)));
  }
  const Integer* mival = asInteger(dict->get(BtAnnounce::MIN_INTERVAL));
  if(mival && mival->i() > 0) {
    minInterval_ = mival->i();
    A2_LOG_DEBUG(fmt("Min interval:%ld", static_cast<long int>(minInterval_)));
    minInterval_ = std::min(minInterval_, interval_);
  } else {
    // Use interval as a minInterval if minInterval is not supplied.
    minInterval_ = interval_;
  }
  const Integer* comp = asInteger(dict->get(BtAnnounce::COMPLETE));
  if(comp) {
    complete_ = comp->i();
    A2_LOG_DEBUG(fmt("Complete:%d", complete_));
  }
  const Integer* incomp = asInteger(dict->get(BtAnnounce::INCOMPLETE));
  if(incomp) {
    incomplete_ = incomp->i();
    A2_LOG_DEBUG(fmt("Incomplete:%d", incomplete_));
  }
  const SharedHandle<ValueBase>& peerData = dict->get(BtAnnounce::PEERS);
  if(!peerData) {
    A2_LOG_INFO(MSG_NO_PEER_LIST_RECEIVED);
  } else {
    if(!btRuntime_->isHalt() && btRuntime_->lessThanMinPeers()) {
      std::vector<SharedHandle<Peer> > peers;
      bittorrent::extractPeer(peerData, AF_INET, std::back_inserter(peers));
      peerStorage_->addPeer(peers);
    }
  }
  const SharedHandle<ValueBase>& peer6Data = dict->get(BtAnnounce::PEERS6);
  if(!peer6Data) {
    A2_LOG_INFO("No peers6 received.");
  } else {
    if(!btRuntime_->isHalt() && btRuntime_->lessThanMinPeers()) {
      std::vector<SharedHandle<Peer> > peers;
      bittorrent::extractPeer(peer6Data, AF_INET6, std::back_inserter(peers));
      peerStorage_->addPeer(peers);
    }
  }
}

bool DefaultBtAnnounce::noMoreAnnounce() {
  return (trackers_ == 0 &&
          btRuntime_->isHalt() &&
          !announceList_.countStoppedAllowedTier());
}

void DefaultBtAnnounce::shuffleAnnounce() {
  announceList_.shuffle();
}

void DefaultBtAnnounce::setRandomizer(const RandomizerHandle& randomizer)
{
  randomizer_ = randomizer;
}

void DefaultBtAnnounce::setBtRuntime(const BtRuntimeHandle& btRuntime)
{
  btRuntime_ = btRuntime;
}

void DefaultBtAnnounce::setPieceStorage(const PieceStorageHandle& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtAnnounce::setPeerStorage(const PeerStorageHandle& peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtAnnounce::overrideMinInterval(time_t interval)
{
  minInterval_ = interval;
}

} // namespace aria2
