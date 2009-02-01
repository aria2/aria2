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
#include "TrackerWatcherCommand.h"

#include <sstream>

#include "DownloadEngine.h"
#include "BtContext.h"
#include "BtAnnounce.h"
#include "BtRuntime.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "Peer.h"
#include "prefs.h"
#include "message.h"
#include "SingleFileDownloadContext.h"
#include "ByteArrayDiskWriterFactory.h"
#include "RecoverableException.h"
#include "PeerInitiateConnectionCommand.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include "RequestGroup.h"
#include "Option.h"
#include "DlAbortEx.h"
#include "Logger.h"
#include "A2STR.h"
#include "SocketCore.h"
#include "Request.h"
#include "AnnounceTier.h"

namespace aria2 {

TrackerWatcherCommand::TrackerWatcherCommand(int32_t cuid,
					     RequestGroup* requestGroup,
					     DownloadEngine* e,
					     const BtContextHandle& btContext):
  Command(cuid),
  RequestGroupAware(requestGroup),
  e(e),
  _btContext(btContext) {}

TrackerWatcherCommand::~TrackerWatcherCommand() {}


bool TrackerWatcherCommand::execute() {
  if(_requestGroup->isForceHaltRequested()) {
    if(_trackerRequestGroup.isNull()) {
      return true;
    } else if(_trackerRequestGroup->getNumCommand() == 0 ||
	      _trackerRequestGroup->downloadFinished()) {
      return true;
    } else {
      _trackerRequestGroup->setForceHaltRequested(true);
      return false;
    }
  }
  if(_btAnnounce->noMoreAnnounce()) {
    logger->debug("no more announce");
    return true;
  }
  if(_trackerRequestGroup.isNull()) {
    _trackerRequestGroup = createAnnounce();
    if(!_trackerRequestGroup.isNull()) {
      std::deque<Command*> commands;
      _trackerRequestGroup->createInitialCommand(commands, e,
						 Request::METHOD_GET);
      e->addCommand(commands);
      logger->debug("added tracker request command");
    }
  } else if(_trackerRequestGroup->downloadFinished()){
    try {
      std::string trackerResponse = getTrackerResponse(_trackerRequestGroup);

      processTrackerResponse(trackerResponse);
      _btAnnounce->announceSuccess();
      _btAnnounce->resetAnnounce();
    } catch(RecoverableException& ex) {
      logger->error(EX_EXCEPTION_CAUGHT, ex);      
      _btAnnounce->announceFailure();
      if(_btAnnounce->isAllAnnounceFailed()) {
	_btAnnounce->resetAnnounce();
      }
    }
    _trackerRequestGroup.reset();
  } else if(_trackerRequestGroup->getNumCommand() == 0){
    // handle errors here
    _btAnnounce->announceFailure(); // inside it, trackers = 0.
    _trackerRequestGroup.reset();
    if(_btAnnounce->isAllAnnounceFailed()) {
      _btAnnounce->resetAnnounce();
    }
  }
  e->commands.push_back(this);
  return false;
}

std::string TrackerWatcherCommand::getTrackerResponse
(const RequestGroupHandle& requestGroup)
{
  std::stringstream strm;
  unsigned char data[2048];
  requestGroup->getPieceStorage()->getDiskAdaptor()->openFile();
  while(1) {
    ssize_t dataLength = requestGroup->getPieceStorage()->
      getDiskAdaptor()->readData(data, sizeof(data), strm.tellp());
    if(dataLength == 0) {
      break;
    }
    strm.write(reinterpret_cast<const char*>(data), dataLength);
  }
  return strm.str();
}

// TODO we have to deal with the exception thrown By BtAnnounce
void TrackerWatcherCommand::processTrackerResponse
(const std::string& trackerResponse)
{
  _btAnnounce->processAnnounceResponse
    (reinterpret_cast<const unsigned char*>(trackerResponse.c_str()),
				      trackerResponse.size());
  while(!_btRuntime->isHalt() && _btRuntime->lessThanMinPeers()) {
    PeerHandle peer = _peerStorage->getUnusedPeer();
    if(peer.isNull()) {
      break;
    }
    peer->usedBy(e->newCUID());
    PeerInitiateConnectionCommand* command =
      new PeerInitiateConnectionCommand(peer->usedBy(),
					_requestGroup,
					peer,
					e,
					_btContext,
					_btRuntime);
    command->setPeerStorage(_peerStorage);
    command->setPieceStorage(_pieceStorage);
    e->commands.push_back(command);
    logger->debug("CUID#%d - Adding new command CUID#%d", cuid, peer->usedBy());
  }
}

RequestGroupHandle TrackerWatcherCommand::createAnnounce() {
  RequestGroupHandle rg;
  if(_btAnnounce->isAnnounceReady()) {
    rg = createRequestGroup(_btAnnounce->getAnnounceUrl());
    _btAnnounce->announceStart(); // inside it, trackers++.
  }
  return rg;
}

static bool backupTrackerIsAvailable
(const std::deque<SharedHandle<AnnounceTier> >& announceTiers)
{
  if(announceTiers.size() >= 2) {
    return true;
  }
  if(announceTiers.empty()) {
    return false;
  }
  if(announceTiers[0]->urls.size() >= 2) {
    return true;
  } else {
    return false;
  }
}

RequestGroupHandle
TrackerWatcherCommand::createRequestGroup(const std::string& uri)
{
  std::deque<std::string> uris;
  uris.push_back(uri);
  RequestGroupHandle rg(new RequestGroup(e->option, uris));
  // If backup tracker is available, only try 2 times for each tracker
  // and if they all fails, then try next one.
  if(backupTrackerIsAvailable(_btContext->getAnnounceTiers())) {
    logger->debug("This is multi-tracker announce.");
    rg->setMaxTries(2);
  } else {
    logger->debug("This is single-tracker announce.");
    rg->setMaxTries(5);
  }

  static const std::string TRACKER_ANNOUNCE_FILE("[tracker.announce]");
  SingleFileDownloadContextHandle dctx
    (new SingleFileDownloadContext(e->option->getAsInt(PREF_SEGMENT_SIZE),
				   0,
				   A2STR::NIL,
				   TRACKER_ANNOUNCE_FILE));
  dctx->setDir(A2STR::NIL);
  rg->setDownloadContext(dctx);
  SharedHandle<DiskWriterFactory> dwf(new ByteArrayDiskWriterFactory());
  rg->setDiskWriterFactory(dwf);
  rg->setFileAllocationEnabled(false);
  rg->setPreLocalFileCheckEnabled(false);
  logger->info("Creating tracker request group GID#%d", rg->getGID());
  return rg;
}

void TrackerWatcherCommand::setBtRuntime
(const SharedHandle<BtRuntime>& btRuntime)
{
  _btRuntime = btRuntime;
}

void TrackerWatcherCommand::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  _peerStorage = peerStorage;
}

void TrackerWatcherCommand::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  _pieceStorage = pieceStorage;
}

void TrackerWatcherCommand::setBtAnnounce
(const SharedHandle<BtAnnounce>& btAnnounce)
{
  _btAnnounce = btAnnounce;
}

} // namespace aria2
