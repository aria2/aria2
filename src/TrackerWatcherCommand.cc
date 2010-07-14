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
#include "RequestGroupMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "ServerStatMan.h"

namespace aria2 {

TrackerWatcherCommand::TrackerWatcherCommand
(cuid_t cuid, RequestGroup* requestGroup, DownloadEngine* e):
  Command(cuid),
  requestGroup_(requestGroup),
  e_(e)
{
  requestGroup_->increaseNumCommand();
}

TrackerWatcherCommand::~TrackerWatcherCommand()
{
  requestGroup_->decreaseNumCommand();
}

bool TrackerWatcherCommand::execute() {
  if(requestGroup_->isForceHaltRequested()) {
    if(trackerRequestGroup_.isNull()) {
      return true;
    } else if(trackerRequestGroup_->getNumCommand() == 0 ||
              trackerRequestGroup_->downloadFinished()) {
      return true;
    } else {
      trackerRequestGroup_->setForceHaltRequested(true);
      e_->addCommand(this);
      return false;
    }
  }
  if(btAnnounce_->noMoreAnnounce()) {
    if(getLogger()->debug()) {
      getLogger()->debug("no more announce");
    }
    return true;
  }
  if(trackerRequestGroup_.isNull()) {
    trackerRequestGroup_ = createAnnounce();
    if(!trackerRequestGroup_.isNull()) {
      try {
        std::vector<Command*>* commands = new std::vector<Command*>();
        auto_delete_container<std::vector<Command*> > commandsDel(commands);
        trackerRequestGroup_->createInitialCommand(*commands, e_);
        e_->addCommand(*commands);
        commands->clear();
        if(getLogger()->debug()) {
          getLogger()->debug("added tracker request command");
        }
      } catch(RecoverableException& ex) {
        getLogger()->error(EX_EXCEPTION_CAUGHT, ex);
      }
    }
  } else if(trackerRequestGroup_->downloadFinished()){
    try {
      std::string trackerResponse = getTrackerResponse(trackerRequestGroup_);

      processTrackerResponse(trackerResponse);
      btAnnounce_->announceSuccess();
      btAnnounce_->resetAnnounce();
    } catch(RecoverableException& ex) {
      getLogger()->error(EX_EXCEPTION_CAUGHT, ex);      
      btAnnounce_->announceFailure();
      if(btAnnounce_->isAllAnnounceFailed()) {
        btAnnounce_->resetAnnounce();
      }
    }
    trackerRequestGroup_.reset();
  } else if(trackerRequestGroup_->getNumCommand() == 0){
    // handle errors here
    btAnnounce_->announceFailure(); // inside it, trackers = 0.
    trackerRequestGroup_.reset();
    if(btAnnounce_->isAllAnnounceFailed()) {
      btAnnounce_->resetAnnounce();
    }
  }
  e_->addCommand(this);
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
  btAnnounce_->processAnnounceResponse
    (reinterpret_cast<const unsigned char*>(trackerResponse.c_str()),
     trackerResponse.size());
  while(!btRuntime_->isHalt() && btRuntime_->lessThanMinPeers()) {
    SharedHandle<Peer> peer = peerStorage_->getUnusedPeer();
    if(peer.isNull()) {
      break;
    }
    peer->usedBy(e_->newCUID());
    PeerInitiateConnectionCommand* command =
      new PeerInitiateConnectionCommand
      (peer->usedBy(), requestGroup_, peer, e_, btRuntime_);
    command->setPeerStorage(peerStorage_);
    command->setPieceStorage(pieceStorage_);
    e_->addCommand(command);
    if(getLogger()->debug()) {
      getLogger()->debug("CUID#%s - Adding new command CUID#%s",
                         util::itos(getCuid()).c_str(),
                         util::itos(peer->usedBy()).c_str());
    }
  }
}

SharedHandle<RequestGroup> TrackerWatcherCommand::createAnnounce() {
  SharedHandle<RequestGroup> rg;
  if(btAnnounce_->isAnnounceReady()) {
    rg = createRequestGroup(btAnnounce_->getAnnounceUrl());
    btAnnounce_->announceStart(); // inside it, trackers++.
  }
  return rg;
}

static bool backupTrackerIsAvailable
(const SharedHandle<DownloadContext>& context)
{
  SharedHandle<TorrentAttribute> torrentAttrs =
    bittorrent::getTorrentAttrs(context);
  if(torrentAttrs->announceList.size() >= 2) {
    return true;
  }
  if(torrentAttrs->announceList.empty()) {
    return false;
  }
  if(torrentAttrs->announceList[0].size() >= 2) {
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
  if(backupTrackerIsAvailable(requestGroup_->getDownloadContext())) {
    if(getLogger()->debug()) {
      getLogger()->debug("This is multi-tracker announce.");
    }
  } else {
    if(getLogger()->debug()) {
      getLogger()->debug("This is single-tracker announce.");
    }
  }
  rg->setNumConcurrentCommand(1);
  // If backup tracker is available, try 2 times for each tracker
  // and if they all fails, then try next one.
  rg->getOption()->put(PREF_MAX_TRIES, "2");
  // TODO When dry-run mode becomes available in BitTorrent, set
  // PREF_DRY_RUN=false too.
  rg->getOption()->put(PREF_USE_HEAD, V_FALSE);
  // Setting tracker timeouts
  rg->setTimeout(rg->getOption()->getAsInt(PREF_BT_TRACKER_TIMEOUT));
  rg->getOption()->put(PREF_CONNECT_TIMEOUT,
                       rg->getOption()->get(PREF_BT_TRACKER_CONNECT_TIMEOUT));
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
  util::removeMetalinkContentTypes(rg);
  if(getLogger()->info()) {
    getLogger()->info("Creating tracker request group GID#%s",
                      util::itos(rg->getGID()).c_str());
  }
  return rg;
}

void TrackerWatcherCommand::setBtRuntime
(const SharedHandle<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

void TrackerWatcherCommand::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void TrackerWatcherCommand::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void TrackerWatcherCommand::setBtAnnounce
(const SharedHandle<BtAnnounce>& btAnnounce)
{
  btAnnounce_ = btAnnounce;
}

const SharedHandle<Option>& TrackerWatcherCommand::getOption() const
{
  return requestGroup_->getOption();
}

} // namespace aria2
