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
#include "DefaultBtAnnounce.h"
#include "BtRegistry.h"
#include "LogFactory.h"
#include "MetaFileUtil.h"
#include "Dictionary.h"
#include "List.h"
#include "Data.h"
#include "DelegatingPeerListProcessor.h"
#include "Util.h"
#include "prefs.h"
#include "DlAbortEx.h"
#include "message.h"

DefaultBtAnnounce::DefaultBtAnnounce(BtContextHandle btContext,
				     const Option* option):
  btContext(btContext),
  trackers(0),
  interval(DEFAULT_ANNOUNCE_INTERVAL),
  minInterval(DEFAULT_ANNOUNCE_INTERVAL),
  complete(0),
  incomplete(0),
  announceList(btContext->getAnnounceTiers()),
  trackerNumTry(0),
  option(option),
  btRuntime(BT_RUNTIME(btContext)),
  pieceStorage(PIECE_STORAGE(btContext)),
  peerStorage(PEER_STORAGE(btContext))
{
  prevAnnounceTime.setTimeInSec(0);
  key = generateKey();
  peerId = generatePeerId();
  logger = LogFactory::getInstance();
}

DefaultBtAnnounce::~DefaultBtAnnounce() {
}

string DefaultBtAnnounce::generateKey() const {
  return Util::randomAlpha(8);
}

string DefaultBtAnnounce::generatePeerId() const {
  string peerId = "-aria2-";
  peerId += Util::randomAlpha(20-peerId.size());
  return peerId;
}

bool DefaultBtAnnounce::isDefaultAnnounceReady() {
  return (trackers == 0 && prevAnnounceTime.elapsed(minInterval));
}

bool DefaultBtAnnounce::isStoppedAnnounceReady() {
  return (trackers == 0 &&
	  btRuntime->isHalt() &&
	  announceList.countStoppedAllowedTier());
}

bool DefaultBtAnnounce::isCompletedAnnounceReady() {
  return (trackers == 0 &&
	  pieceStorage->downloadFinished() &&
	  announceList.countCompletedAllowedTier());
}

bool DefaultBtAnnounce::isAnnounceReady() {
  return
    isStoppedAnnounceReady() ||
    isCompletedAnnounceReady() ||
    isDefaultAnnounceReady();
}

string DefaultBtAnnounce::getAnnounceUrl() {
  if(isStoppedAnnounceReady()) {
    announceList.moveToStoppedAllowedTier();
    announceList.setEvent(AnnounceTier::STOPPED);
  } else if(isCompletedAnnounceReady()) {
    announceList.moveToCompletedAllowedTier();
    announceList.setEvent(AnnounceTier::COMPLETED);
  } else if(isDefaultAnnounceReady()) {
    // If download completed before "started" event is sent to a tracker,
    // we change the event to something else to prevent us from
    // sending "completed" event.
    if(pieceStorage->downloadFinished() &&
       announceList.getEvent() == AnnounceTier::STARTED) {
      announceList.setEvent(AnnounceTier::STARTED_AFTER_COMPLETION);
    }
  }
  int numWant = 50;
  if(!btRuntime->lessThanEqMinPeer() ||
     btRuntime->isHalt()) {
    numWant = 0;
  }
  TransferStat stat = peerStorage->calculateStat();
  long long int left = pieceStorage->getTotalLength()-pieceStorage->getCompletedLength();
  if(left < 0) {
    left = 0;
  }
  string url = announceList.getAnnounce()+"?"+
    "info_hash="+Util::torrentUrlencode(btContext->getInfoHash(),
					btContext->getInfoHashLength())+"&"+
    "peer_id="+peerId+"&"+
    "port="+Util::itos(btRuntime->getListenPort())+"&"+
    "uploaded="+Util::llitos(stat.getSessionUploadLength())+"&"+
    "downloaded="+Util::llitos(stat.getSessionDownloadLength())+"&"+
    "left="+Util::llitos(left)+"&"+
    "compact=1"+"&"+
    "key="+key+"&"+
    "numwant="+Util::itos(numWant)+"&"+
    "no_peer_id=1";
  string event = announceList.getEventString();
  if(!event.empty()) {
    url += string("&")+"event="+event;
  }
  if(!trackerId.empty()) {
    url += string("&")+"trackerid="+Util::torrentUrlencode((const unsigned char*)trackerId.c_str(),
							   trackerId.size());
  }
  return url;
}

void DefaultBtAnnounce::announceStart() {
  trackers++;
}

void DefaultBtAnnounce::announceSuccess() {
  trackers = 0;
  announceList.announceSuccess();
}

void DefaultBtAnnounce::announceFailure() {
  trackers = 0;
  trackerNumTry++;
  announceList.announceFailure();
}

bool DefaultBtAnnounce::isAllAnnounceFailed() {
  return 
    trackerNumTry >= option->getAsInt(PREF_TRACKER_MAX_TRIES);
}

void DefaultBtAnnounce::resetAnnounce() {
  prevAnnounceTime.reset();
  trackerNumTry = 0;
}

void
DefaultBtAnnounce::processAnnounceResponse(const char* trackerResponse,
						  size_t trackerResponseLength)
{
  SharedHandle<MetaEntry> entry(MetaFileUtil::bdecoding(trackerResponse,
							trackerResponseLength));
  Dictionary* response = (Dictionary*)entry.get();
  Data* failureReasonData = (Data*)response->get("failure reason");
  if(failureReasonData) {
    throw new DlAbortEx("Tracker returned failure reason: %s",
			failureReasonData->toString().c_str());
  }
  Data* warningMessageData = (Data*)response->get("warning message");
  if(warningMessageData) {
    logger->warn(MSG_TRACKER_WARNING_MESSAGE,
		 warningMessageData->toString().c_str());
  }
  Data* trackerIdData = (Data*)response->get("tracker id");
  if(trackerIdData) {
    trackerId = trackerIdData->toString();
    logger->debug("Tracker ID:%s", trackerId.c_str());
  }
  Data* intervalData = (Data*)response->get("interval");
  if(intervalData) {
    interval = intervalData->toInt();
    logger->debug("Interval:%d", interval);
  }
  Data* minIntervalData = (Data*)response->get("min interval");
  if(minIntervalData) {
    minInterval = minIntervalData->toInt();
    logger->debug("Min interval:%d", minInterval);
  }
  if(minInterval > interval) {
    minInterval = interval;
  }
  Data* completeData = (Data*)response->get("complete");
  if(completeData) {
    complete = completeData->toInt();
    logger->debug("Complete:%d", complete);
  }
  Data* incompleteData = (Data*)response->get("incomplete");
  if(incompleteData) {
    incomplete = incompleteData->toInt();
    logger->debug("Incomplete:%d", incomplete);
  }
  const MetaEntry* peersEntry = response->get("peers");
  if(peersEntry &&
     !btRuntime->isHalt() &&
     btRuntime->lessThanMinPeer()) {
    DelegatingPeerListProcessor proc(btContext->getPieceLength(),
				     btContext->getTotalLength());
    Peers peers = proc.extractPeer(peersEntry);
    peerStorage->addPeer(peers);
  }
  if(!peersEntry) {
    logger->info("No peer list received.");
  }
}

bool DefaultBtAnnounce::noMoreAnnounce() {
  return (trackers == 0 &&
	  btRuntime->isHalt() &&
	  !announceList.countStoppedAllowedTier());
}

void DefaultBtAnnounce::shuffleAnnounce() {
  announceList.shuffle();
}
