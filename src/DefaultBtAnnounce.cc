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
#include "UDPTrackerRequest.h"
#include "SocketCore.h"

namespace aria2 {

DefaultBtAnnounce::DefaultBtAnnounce(DownloadContext* downloadContext,
                                     const Option* option)
    : downloadContext_{downloadContext},
      trackers_(0),
      prevAnnounceTimer_(Timer::zero()),
      interval_(DEFAULT_ANNOUNCE_INTERVAL),
      minInterval_(DEFAULT_ANNOUNCE_INTERVAL),
      userDefinedInterval_(0_s),
      complete_(0),
      incomplete_(0),
      announceList_(bittorrent::getTorrentAttrs(downloadContext)->announceList),
      option_(option),
      randomizer_(SimpleRandomizer::getInstance().get()),
      tcpPort_(0)
{
}

DefaultBtAnnounce::~DefaultBtAnnounce() = default;

bool DefaultBtAnnounce::isDefaultAnnounceReady()
{
  return (trackers_ == 0 &&
          prevAnnounceTimer_.difference(global::wallclock()) >=
              (userDefinedInterval_.count() == 0 ? minInterval_
                                                 : userDefinedInterval_) &&
          !announceList_.allTiersFailed());
}

bool DefaultBtAnnounce::isStoppedAnnounceReady()
{
  return (trackers_ == 0 && btRuntime_->isHalt() &&
          announceList_.countStoppedAllowedTier());
}

bool DefaultBtAnnounce::isCompletedAnnounceReady()
{
  return (trackers_ == 0 && pieceStorage_->allDownloadFinished() &&
          announceList_.countCompletedAllowedTier());
}

bool DefaultBtAnnounce::isAnnounceReady()
{
  return isStoppedAnnounceReady() || isCompletedAnnounceReady() ||
         isDefaultAnnounceReady();
}

namespace {
bool uriHasQuery(const std::string& uri)
{
  uri_split_result us;
  if (uri_split(&us, uri.c_str()) == 0) {
    return (us.field_set & (1 << USR_QUERY)) && us.fields[USR_QUERY].len > 0;
  }
  else {
    return false;
  }
}
} // namespace

bool DefaultBtAnnounce::adjustAnnounceList()
{
  if (isStoppedAnnounceReady()) {
    if (!announceList_.currentTierAcceptsStoppedEvent()) {
      announceList_.moveToStoppedAllowedTier();
    }
    announceList_.setEvent(AnnounceTier::STOPPED);
  }
  else if (isCompletedAnnounceReady()) {
    if (!announceList_.currentTierAcceptsCompletedEvent()) {
      announceList_.moveToCompletedAllowedTier();
    }
    announceList_.setEvent(AnnounceTier::COMPLETED);
  }
  else if (isDefaultAnnounceReady()) {
    // If download completed before "started" event is sent to a tracker,
    // we change the event to something else to prevent us from
    // sending "completed" event.
    if (pieceStorage_->allDownloadFinished() &&
        announceList_.getEvent() == AnnounceTier::STARTED) {
      announceList_.setEvent(AnnounceTier::STARTED_AFTER_COMPLETION);
    }
  }
  else {
    return false;
  }
  return true;
}

std::string DefaultBtAnnounce::getAnnounceUrl()
{
  if (!adjustAnnounceList()) {
    return A2STR::NIL;
  }
  int numWant = 50;
  if (!btRuntime_->lessThanMinPeers() || btRuntime_->isHalt()) {
    numWant = 0;
  }
  NetStat& stat = downloadContext_->getNetStat();
  int64_t left =
      pieceStorage_->getTotalLength() - pieceStorage_->getCompletedLength();
  // Use last 8 bytes of peer ID as a key
  const size_t keyLen = 8;
  std::string uri = announceList_.getAnnounce();
  uri += uriHasQuery(uri) ? "&" : "?";
  uri +=
      fmt("info_hash=%s&"
          "peer_id=%s&"
          "uploaded=%" PRId64 "&"
          "downloaded=%" PRId64 "&"
          "left=%" PRId64 "&"
          "compact=1&"
          "key=%s&"
          "numwant=%d&"
          "no_peer_id=1",
          util::percentEncode(bittorrent::getInfoHash(downloadContext_),
                              INFO_HASH_LENGTH)
              .c_str(),
          util::percentEncode(bittorrent::getStaticPeerId(), PEER_ID_LENGTH)
              .c_str(),
          stat.getSessionUploadLength(), stat.getSessionDownloadLength(), left,
          util::percentEncode(
              bittorrent::getStaticPeerId() + PEER_ID_LENGTH - keyLen, keyLen)
              .c_str(),
          numWant);
  if (tcpPort_) {
    uri += fmt("&port=%u", tcpPort_);
  }
  const char* event = announceList_.getEventString();
  if (event[0]) {
    uri += "&event=";
    uri += event;
  }
  if (!trackerId_.empty()) {
    uri += "&trackerid=";
    uri += util::percentEncode(trackerId_);
  }
  if (option_->getAsBool(PREF_BT_FORCE_ENCRYPTION) ||
      option_->getAsBool(PREF_BT_REQUIRE_CRYPTO)) {
    uri += "&requirecrypto=1";
  }
  else {
    uri += "&supportcrypto=1";
  }
  if (!option_->blank(PREF_BT_EXTERNAL_IP)) {
    uri += "&ip=";
    uri += option_->get(PREF_BT_EXTERNAL_IP);
  }
  return uri;
}

std::shared_ptr<UDPTrackerRequest> DefaultBtAnnounce::createUDPTrackerRequest(
    const std::string& remoteAddr, uint16_t remotePort, uint16_t localPort)
{
  if (!adjustAnnounceList()) {
    return nullptr;
  }
  NetStat& stat = downloadContext_->getNetStat();
  int64_t left =
      pieceStorage_->getTotalLength() - pieceStorage_->getCompletedLength();
  auto req = std::make_shared<UDPTrackerRequest>();
  req->remoteAddr = remoteAddr;
  req->remotePort = remotePort;
  req->action = UDPT_ACT_ANNOUNCE;
  req->infohash = bittorrent::getTorrentAttrs(downloadContext_)->infoHash;
  const unsigned char* peerId = bittorrent::getStaticPeerId();
  req->peerId.assign(peerId, peerId + PEER_ID_LENGTH);
  req->downloaded = stat.getSessionDownloadLength();
  req->left = left;
  req->uploaded = stat.getSessionUploadLength();
  switch (announceList_.getEvent()) {
  case AnnounceTier::STARTED:
  case AnnounceTier::STARTED_AFTER_COMPLETION:
    req->event = UDPT_EVT_STARTED;
    break;
  case AnnounceTier::STOPPED:
    req->event = UDPT_EVT_STOPPED;
    break;
  case AnnounceTier::COMPLETED:
    req->event = UDPT_EVT_COMPLETED;
    break;
  default:
    req->event = 0;
  }
  if (!option_->blank(PREF_BT_EXTERNAL_IP)) {
    unsigned char dest[16];
    if (net::getBinAddr(dest, option_->get(PREF_BT_EXTERNAL_IP)) == 4) {
      memcpy(&req->ip, dest, 4);
    }
    else {
      req->ip = 0;
    }
  }
  else {
    req->ip = 0;
  }
  req->key = randomizer_->getRandomNumber(INT32_MAX);
  int numWant = 50;
  if (!btRuntime_->lessThanMinPeers() || btRuntime_->isHalt()) {
    numWant = 0;
  }
  req->numWant = numWant;
  req->port = localPort;
  req->extensions = 0;
  return req;
}

void DefaultBtAnnounce::announceStart() { ++trackers_; }

void DefaultBtAnnounce::announceSuccess()
{
  trackers_ = 0;
  announceList_.announceSuccess();
}

void DefaultBtAnnounce::announceFailure()
{
  trackers_ = 0;
  announceList_.announceFailure();
}

bool DefaultBtAnnounce::isAllAnnounceFailed()
{
  return announceList_.allTiersFailed();
}

void DefaultBtAnnounce::resetAnnounce()
{
  prevAnnounceTimer_ = global::wallclock();
  announceList_.resetTier();
}

void DefaultBtAnnounce::processAnnounceResponse(
    const unsigned char* trackerResponse, size_t trackerResponseLength)
{
  A2_LOG_DEBUG("Now processing tracker response.");
  auto decodedValue = bencode2::decode(trackerResponse, trackerResponseLength);
  const Dict* dict = downcast<Dict>(decodedValue);
  if (!dict) {
    throw DL_ABORT_EX(MSG_NULL_TRACKER_RESPONSE);
  }
  const String* failure =
      downcast<String>(dict->get(BtAnnounce::FAILURE_REASON));
  if (failure) {
    throw DL_ABORT_EX(fmt(EX_TRACKER_FAILURE, failure->s().c_str()));
  }
  const String* warn = downcast<String>(dict->get(BtAnnounce::WARNING_MESSAGE));
  if (warn) {
    A2_LOG_WARN(fmt(MSG_TRACKER_WARNING_MESSAGE, warn->s().c_str()));
  }
  const String* tid = downcast<String>(dict->get(BtAnnounce::TRACKER_ID));
  if (tid) {
    trackerId_ = tid->s();
    A2_LOG_DEBUG(fmt("Tracker ID:%s", trackerId_.c_str()));
  }
  const Integer* ival = downcast<Integer>(dict->get(BtAnnounce::INTERVAL));
  if (ival && ival->i() > 0) {
    interval_ = std::chrono::seconds(ival->i());
    A2_LOG_DEBUG(fmt("Interval:%ld", static_cast<long int>(interval_.count())));
  }
  const Integer* mival = downcast<Integer>(dict->get(BtAnnounce::MIN_INTERVAL));
  if (mival && mival->i() > 0) {
    minInterval_ = std::chrono::seconds(mival->i());
    A2_LOG_DEBUG(
        fmt("Min interval:%ld", static_cast<long int>(minInterval_.count())));
    minInterval_ = std::min(minInterval_, interval_);
  }
  else {
    // Use interval as a minInterval if minInterval is not supplied.
    minInterval_ = interval_;
  }
  const Integer* comp = downcast<Integer>(dict->get(BtAnnounce::COMPLETE));
  if (comp && comp->i() >= 0) {
    complete_ = comp->i();
    A2_LOG_DEBUG(fmt("Complete:%d", complete_));
  }
  const Integer* incomp = downcast<Integer>(dict->get(BtAnnounce::INCOMPLETE));
  if (incomp && incomp->i() >= 0) {
    incomplete_ = incomp->i();
    A2_LOG_DEBUG(fmt("Incomplete:%d", incomplete_));
  }
  auto peerData = dict->get(BtAnnounce::PEERS);
  if (!peerData) {
    A2_LOG_INFO(MSG_NO_PEER_LIST_RECEIVED);
  }
  else {
    if (!btRuntime_->isHalt() && btRuntime_->lessThanMinPeers()) {
      std::vector<std::shared_ptr<Peer>> peers;
      bittorrent::extractPeer(peerData, AF_INET, std::back_inserter(peers));
      peerStorage_->addPeer(peers);
    }
  }
  auto peer6Data = dict->get(BtAnnounce::PEERS6);
  if (!peer6Data) {
    A2_LOG_INFO("No peers6 received.");
  }
  else {
    if (!btRuntime_->isHalt() && btRuntime_->lessThanMinPeers()) {
      std::vector<std::shared_ptr<Peer>> peers;
      bittorrent::extractPeer(peer6Data, AF_INET6, std::back_inserter(peers));
      peerStorage_->addPeer(peers);
    }
  }
}

void DefaultBtAnnounce::processUDPTrackerResponse(
    const std::shared_ptr<UDPTrackerRequest>& req)
{
  const std::shared_ptr<UDPTrackerReply>& reply = req->reply;
  A2_LOG_DEBUG("Now processing UDP tracker response.");
  if (reply->interval > 0) {
    minInterval_ = std::chrono::seconds(reply->interval);
    A2_LOG_DEBUG(
        fmt("Min interval:%ld", static_cast<long int>(minInterval_.count())));
    interval_ = minInterval_;
  }
  complete_ = reply->seeders;
  A2_LOG_DEBUG(fmt("Complete:%d", reply->seeders));
  incomplete_ = reply->leechers;
  A2_LOG_DEBUG(fmt("Incomplete:%d", reply->leechers));
  if (!btRuntime_->isHalt() && btRuntime_->lessThanMinPeers()) {
    for (auto& elem : reply->peers) {
      peerStorage_->addPeer(std::make_shared<Peer>(elem.first, elem.second));
    }
  }
}

bool DefaultBtAnnounce::noMoreAnnounce()
{
  return (trackers_ == 0 && btRuntime_->isHalt() &&
          !announceList_.countStoppedAllowedTier());
}

void DefaultBtAnnounce::shuffleAnnounce() { announceList_.shuffle(); }

void DefaultBtAnnounce::setRandomizer(Randomizer* randomizer)
{
  randomizer_ = randomizer;
}

void DefaultBtAnnounce::setBtRuntime(
    const std::shared_ptr<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

void DefaultBtAnnounce::setPieceStorage(
    const std::shared_ptr<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtAnnounce::setPeerStorage(
    const std::shared_ptr<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtAnnounce::overrideMinInterval(std::chrono::seconds interval)
{
  minInterval_ = std::move(interval);
}

} // namespace aria2
