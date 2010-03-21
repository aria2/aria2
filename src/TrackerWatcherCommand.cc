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
#include "TrackerWatcherCommand.h"

#include <sstream>

#include "DownloadEngine.h"
#include "BtAnnounce.h"
#include "BtRuntime.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "Peer.h"
#include "prefs.h"
#include "message.h"
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
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "a2functional.h"
#include "util.h"

namespace aria2 {

TrackerWatcherCommand::TrackerWatcherCommand
(cuid_t cuid, RequestGroup* requestGroup, DownloadEngine* e):
  Command(cuid),
  _requestGroup(requestGroup),
  e(e)
{
  _requestGroup->increaseNumCommand();
}

TrackerWatcherCommand::~TrackerWatcherCommand()
{
  _requestGroup->decreaseNumCommand();
}

bool TrackerWatcherCommand::execute() {
  if(_requestGroup->isForceHaltRequested()) {
    if(_trackerRequestGroup.isNull()) {
      return true;
    } else if(_trackerRequestGroup->getNumCommand() == 0 ||
              _trackerRequestGroup->downloadFinished()) {
      return true;
    } else {
      _trackerRequestGroup->setForceHaltRequested(true);
      e->commands.push_back(this);
      return false;
    }
  }
  if(_btAnnounce->noMoreAnnounce()) {
    if(logger->debug()) {
      logger->debug("no more announce");
    }
    return true;
  }
  if(_trackerRequestGroup.isNull()) {
    _trackerRequestGroup = createAnnounce();
    if(!_trackerRequestGroup.isNull()) {
      std::vector<Command*> commands;
      try {
        _trackerRequestGroup->createInitialCommand(commands, e);
      } catch(RecoverableException& ex) {
        logger->error(EX_EXCEPTION_CAUGHT, ex);
        std::for_each(commands.begin(), commands.end(), Deleter());
        commands.clear();
      }
      e->addCommand(commands);
      if(logger->debug()) {
        logger->debug("added tracker request command");
      }
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
(const SharedHandle<RequestGroup>& requestGroup)
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
    SharedHandle<Peer> peer = _peerStorage->getUnusedPeer();
    if(peer.isNull()) {
      break;
    }
    peer->usedBy(e->newCUID());
    PeerInitiateConnectionCommand* command =
      new PeerInitiateConnectionCommand
      (peer->usedBy(), _requestGroup, peer, e, _btRuntime);
    command->setPeerStorage(_peerStorage);
    command->setPieceStorage(_pieceStorage);
    e->commands.push_back(command);
    if(logger->debug()) {
      logger->debug("CUID#%s - Adding new command CUID#%s",
                    util::itos(cuid).c_str(),
                    util::itos(peer->usedBy()).c_str());
    }
  }
}

SharedHandle<RequestGroup> TrackerWatcherCommand::createAnnounce() {
  SharedHandle<RequestGroup> rg;
  if(_btAnnounce->isAnnounceReady()) {
    rg = createRequestGroup(_btAnnounce->getAnnounceUrl());
    _btAnnounce->announceStart(); // inside it, trackers++.
  }
  return rg;
}

static bool backupTrackerIsAvailable
(const SharedHandle<DownloadContext>& context)
{
  const BDE& announceList =
    context->getAttribute(bittorrent::BITTORRENT)[bittorrent::ANNOUNCE_LIST];
  if(announceList.size() >= 2) {
    return true;
  }
  if(announceList.empty()) {
    return false;
  }
  if(announceList[0].size() >= 2) {
    return true;
  } else {
    return false;
  }
}

SharedHandle<RequestGroup>
TrackerWatcherCommand::createRequestGroup(const std::string& uri)
{
  std::vector<std::string> uris;
  uris.push_back(uri);
  SharedHandle<RequestGroup> rg(new RequestGroup(getOption()));
  // If backup tracker is available, only try 2 times for each tracker
  // and if they all fails, then try next one.
  if(backupTrackerIsAvailable(_requestGroup->getDownloadContext())) {
    if(logger->debug()) {
      logger->debug("This is multi-tracker announce.");
    }
  } else {
    if(logger->debug()) {
      logger->debug("This is single-tracker announce.");
    }
  }
  rg->getOption()->put(PREF_MAX_TRIES, "2");
  // TODO When dry-run mode becomes available in BitTorrent, set
  // PREF_DRY_RUN=false too.
  rg->getOption()->put(PREF_USE_HEAD, V_FALSE);

  static const std::string TRACKER_ANNOUNCE_FILE("[tracker.announce]");
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(getOption()->getAsInt(PREF_SEGMENT_SIZE),
                         0,
                         TRACKER_ANNOUNCE_FILE));
  dctx->setDir(A2STR::NIL);
  dctx->getFileEntries().front()->setUris(uris);
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

const SharedHandle<Option>& TrackerWatcherCommand::getOption() const
{
  return _requestGroup->getOption();
}

} // namespace aria2
