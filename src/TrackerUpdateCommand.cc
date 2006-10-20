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

TrackerUpdateCommand::TrackerUpdateCommand(int cuid, TorrentDownloadEngine* e):Command(cuid), e(e) {
  logger = LogFactory::getInstance();
}

TrackerUpdateCommand::~TrackerUpdateCommand() {}

bool TrackerUpdateCommand::prepareForRetry() {
  e->commands.push_back(this);
  return false;
}

char* TrackerUpdateCommand::getTrackerResponse(size_t& trackerResponseLength) {
  int maxBufLength = 2048;
  char* buf = new char[maxBufLength];
  int bufLength = 0;
  char data[2048];
  try {
    while(1) {
      int dataLength = e->segmentMan->diskWriter->readData(data, sizeof(data), bufLength);
      if(bufLength+dataLength >= maxBufLength) {
	maxBufLength = Util::expandBuffer(&buf, bufLength, bufLength+dataLength);
      }
      memcpy(buf+bufLength, data, dataLength);
      bufLength += dataLength;
      if(dataLength != sizeof(data)) {
	break;
      }
    }
    trackerResponseLength = bufLength;
    return buf;
  } catch(Exception* e) {
    delete [] buf;
    throw;
  }
}

bool TrackerUpdateCommand::execute() {
  if(e->segmentMan->errors > 0 && e->torrentMan->isHalt()) {
    return true;
  }
  if(!e->segmentMan->finished()) {
    return prepareForRetry();
  }
  char* trackerResponse = NULL;
  size_t trackerResponseLength = 0;

  try {
    trackerResponse = getTrackerResponse(trackerResponseLength);

    e->torrentMan->processAnnounceResponse(trackerResponse,
					   trackerResponseLength);
    while(e->torrentMan->needMorePeerConnection()) {
      PeerHandle peer = e->torrentMan->getPeer();
      int newCuid =  e->torrentMan->getNewCuid();
      peer->cuid = newCuid;
      PeerInitiateConnectionCommand* command =
	new PeerInitiateConnectionCommand(newCuid, peer, e);
      e->commands.push_back(command);
      logger->debug("CUID#%d - Adding new command CUID#%d", cuid, newCuid);
    }
    e->torrentMan->announceSuccess();
    e->torrentMan->resetAnnounce();
    e->segmentMan->init();
  } catch(Exception* err) {
    logger->error("CUID#%d - Error occurred while processing tracker response.", cuid, err);
    e->segmentMan->errors++;
    delete err;
  }
  if(trackerResponse) {
    delete [] trackerResponse;
  }
  if(e->torrentMan->isHalt()) {
    return true;
  } else {
    return prepareForRetry();
  }
}

