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
#include "TrackerUpdateCommand.h"
#include "LogFactory.h"
#include "DlAbortEx.h"
#include "message.h"
#include "PeerInitiateConnectionCommand.h"
#include "SleepCommand.h"
#include "Util.h"
#include "CUIDCounter.h"
#include <sstream>

TrackerUpdateCommand::TrackerUpdateCommand(int32_t cuid,
					   TorrentDownloadEngine* e,
					   const BtContextHandle& btContext):
  BtContextAwareCommand(cuid, btContext), e(e)
{
  logger = LogFactory::getInstance();
}

TrackerUpdateCommand::~TrackerUpdateCommand() {}

bool TrackerUpdateCommand::prepareForRetry() {
  e->commands.push_back(this);
  return false;
}

string TrackerUpdateCommand::getTrackerResponse() {
  stringstream strm;
  char data[2048];
  try {
    while(1) {
      int32_t dataLength = e->_requestGroupMan->getRequestGroup(0)->getSegmentMan()->diskWriter->readData(data, sizeof(data), strm.tellp());
      strm.write(data, dataLength);
      if(dataLength != sizeof(data)) {
	break;
      }
    }
    return strm.str();
  } catch(RecoverableException* e) {
    throw;
  }
}

bool TrackerUpdateCommand::execute() {
  if(btAnnounce->noMoreAnnounce()) {
    return true;
  }
  if(e->_requestGroupMan->countRequestGroup() == 0 ||
     !e->_requestGroupMan->downloadFinished()) {
    return prepareForRetry();
  }

  try {
    string trackerResponse = getTrackerResponse();

    btAnnounce->processAnnounceResponse(trackerResponse.c_str(),
					trackerResponse.size());
    while(!btRuntime->isHalt() && btRuntime->lessThanMinPeer()) {
      PeerHandle peer = peerStorage->getUnusedPeer();
      if(peer.isNull()) {
	break;
      }
      peer->cuid = CUIDCounterSingletonHolder::instance()->newID();
      PeerInitiateConnectionCommand* command =
	new PeerInitiateConnectionCommand(peer->cuid,
					  peer,
					  e,
					  btContext);
      e->commands.push_back(command);
      logger->debug("CUID#%d - Adding new command CUID#%d", cuid, peer->cuid);
    }
    btAnnounce->announceSuccess();
    btAnnounce->resetAnnounce();
    e->_requestGroupMan->removeStoppedGroup();
  } catch(RecoverableException* err) {
    logger->error(MSG_TRACKER_RESPONSE_PROCESSING_FAILED, err, cuid);
    e->_requestGroupMan->getRequestGroup(0)->getSegmentMan()->errors++;
    delete err;
  }
  return prepareForRetry();
}

