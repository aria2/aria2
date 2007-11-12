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
#include "DownloadEngine.h"
#include "Util.h"
#include "prefs.h"
#include "message.h"
#include "SingleFileDownloadContext.h"
#include "ByteArrayDiskWriterFactory.h"
#include "RecoverableException.h"
#include "CUIDCounter.h"
#include "PeerInitiateConnectionCommand.h"
#include "DiskAdaptor.h"
#include "RequestGroup.h"
#include "Option.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"

TrackerWatcherCommand::TrackerWatcherCommand(int32_t cuid,
					     RequestGroup* requestGroup,
					     DownloadEngine* e,
					     const BtContextHandle& btContext):
  Command(cuid),
  BtContextAwareCommand(btContext),
  RequestGroupAware(requestGroup),
  e(e),
  _trackerRequestGroup(0)
{
}

TrackerWatcherCommand::~TrackerWatcherCommand() {}


bool TrackerWatcherCommand::execute() {
  if(btAnnounce->noMoreAnnounce()) {
    logger->debug("no more announce");
    return true;
  }
  if(_trackerRequestGroup.isNull()) {
    _trackerRequestGroup = createAnnounce();
    if(!_trackerRequestGroup.isNull()) {
      e->addCommand(_trackerRequestGroup->createInitialCommand(e));
      logger->debug("added tracker request command");
    }
  } else if(_trackerRequestGroup->downloadFinished()){
    try {
      string trackerResponse = getTrackerResponse(_trackerRequestGroup);

      processTrackerResponse(trackerResponse);
      btAnnounce->announceSuccess();
      btAnnounce->resetAnnounce();
    } catch(RecoverableException* ex) {
      logger->error(EX_EXCEPTION_CAUGHT, ex);      
      delete ex;
      btAnnounce->announceFailure();
      if(btAnnounce->isAllAnnounceFailed()) {
	btAnnounce->resetAnnounce();
      }
    }
    _trackerRequestGroup = 0;
  } else if(_trackerRequestGroup->getNumCommand() == 0){
    // handle errors here
    btAnnounce->announceFailure(); // inside it, trackers = 0.
    _trackerRequestGroup = 0;
    if(btAnnounce->isAllAnnounceFailed()) {
      btAnnounce->resetAnnounce();
    }
  }
  e->commands.push_back(this);
  return false;
}

string TrackerWatcherCommand::getTrackerResponse(const RequestGroupHandle& requestGroup)
{
  stringstream strm;
  char data[2048];
  requestGroup->getPieceStorage()->getDiskAdaptor()->openFile();
  while(1) {
    int32_t dataLength = requestGroup->getPieceStorage()->getDiskAdaptor()->readData((unsigned char*)data, sizeof(data), strm.tellp());
    strm.write(data, dataLength);
    if(dataLength == 0) {
      break;
    }
  }
  return strm.str();
}

// TODO we have to deal with the exception thrown By BtAnnounce
void TrackerWatcherCommand::processTrackerResponse(const string& trackerResponse)
{
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
					_requestGroup,
					peer,
					e,
					btContext);
    e->commands.push_back(command);
    logger->debug("CUID#%d - Adding new command CUID#%d", cuid, peer->cuid);
  }
}

RequestGroupHandle TrackerWatcherCommand::createAnnounce() {
  RequestGroupHandle rg = 0;
  if(btAnnounce->isAnnounceReady()) {
    rg = createRequestGroup(btAnnounce->getAnnounceUrl());
    btAnnounce->announceStart(); // inside it, trackers++.
  }
  return rg;
}

RequestGroupHandle
TrackerWatcherCommand::createRequestGroup(const string& uri)
{
  Strings uris;
  uris.push_back(uri);
  RequestGroupHandle rg = new RequestGroup(e->option, uris);

  SingleFileDownloadContextHandle dctx =
    new SingleFileDownloadContext(e->option->getAsInt(PREF_SEGMENT_SIZE),
				  0,
				  "",
				  "[tracker.announce]");
  dctx->setDir("");
  rg->setDownloadContext(dctx);
  rg->setDiskWriterFactory(new ByteArrayDiskWriterFactory());
  rg->setFileAllocationEnabled(false);
  rg->setPreLocalFileCheckEnabled(false);
  logger->info("Creating tracker request group GID#%d", rg->getGID());
  return rg;
}
