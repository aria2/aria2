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
#include "LogFactory.h"
#include "A2STR.h"
#include "SocketCore.h"
#include "Request.h"
#include "AnnounceTier.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "a2functional.h"
#include "util.h"
#include "fmt.h"
#include "UDPTrackerRequest.h"
#include "UDPTrackerClient.h"
#include "BtRegistry.h"
#include "NameResolveCommand.h"

namespace aria2 {

HTTPAnnRequest::HTTPAnnRequest(std::unique_ptr<RequestGroup> rg)
    : rg_{std::move(rg)}
{
}

HTTPAnnRequest::~HTTPAnnRequest() = default;

bool HTTPAnnRequest::stopped() const { return rg_->getNumCommand() == 0; }

bool HTTPAnnRequest::success() const { return rg_->downloadFinished(); }

void HTTPAnnRequest::stop(DownloadEngine* e)
{
  rg_->setForceHaltRequested(true);
}

bool HTTPAnnRequest::issue(DownloadEngine* e)
{
  try {
    std::vector<std::unique_ptr<Command>> commands;
    rg_->createInitialCommand(commands, e);
    e->addCommand(std::move(commands));
    e->setNoWait(true);
    A2_LOG_DEBUG("added tracker request command");
    return true;
  }
  catch (RecoverableException& ex) {
    A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, ex);
    return false;
  }
}

bool HTTPAnnRequest::processResponse(
    const std::shared_ptr<BtAnnounce>& btAnnounce)
{
  try {
    std::stringstream strm;
    unsigned char data[2048];
    rg_->getPieceStorage()->getDiskAdaptor()->openFile();
    while (1) {
      ssize_t dataLength = rg_->getPieceStorage()->getDiskAdaptor()->readData(
          data, sizeof(data), strm.tellp());
      if (dataLength == 0) {
        break;
      }
      strm.write(reinterpret_cast<const char*>(data), dataLength);
    }
    std::string res = strm.str();
    btAnnounce->processAnnounceResponse(
        reinterpret_cast<const unsigned char*>(res.c_str()), res.size());
    return true;
  }
  catch (RecoverableException& e) {
    const auto& dctx = rg_->getDownloadContext();
    const auto& fe = dctx->getFirstFileEntry();
    auto uris = fe->getUris();

    A2_LOG_ERROR_EX(fmt("GID#%s - Tracker request %s failed",
                        GroupId::toHex(rg_->getGID()).c_str(), uris[0].c_str()),
                    e);
    return false;
  }
}

UDPAnnRequest::UDPAnnRequest(const std::shared_ptr<UDPTrackerRequest>& req)
    : req_(req)
{
}

UDPAnnRequest::~UDPAnnRequest() = default;

bool UDPAnnRequest::stopped() const
{
  return !req_ || req_->state == UDPT_STA_COMPLETE;
}

bool UDPAnnRequest::success() const
{
  return req_ && req_->state == UDPT_STA_COMPLETE &&
         req_->error == UDPT_ERR_SUCCESS;
}

void UDPAnnRequest::stop(DownloadEngine* e)
{
  if (req_) {
    req_->user_data = nullptr;
    req_.reset();
  }
}

bool UDPAnnRequest::issue(DownloadEngine* e)
{
  if (req_) {
    e->addCommand(make_unique<NameResolveCommand>(e->newCUID(), e, req_));
    e->setNoWait(true);
    return true;
  }
  else {
    return false;
  }
}

bool UDPAnnRequest::processResponse(
    const std::shared_ptr<BtAnnounce>& btAnnounce)
{
  if (req_) {
    btAnnounce->processUDPTrackerResponse(req_);
    return true;
  }
  else {
    return false;
  }
}

TrackerWatcherCommand::TrackerWatcherCommand(cuid_t cuid,
                                             RequestGroup* requestGroup,
                                             DownloadEngine* e)
    : Command(cuid),
      requestGroup_(requestGroup),
      e_(e),
      udpTrackerClient_(e_->getBtRegistry()->getUDPTrackerClient())
{
  requestGroup_->increaseNumCommand();
  if (udpTrackerClient_) {
    udpTrackerClient_->increaseWatchers();
  }
}

TrackerWatcherCommand::~TrackerWatcherCommand()
{
  requestGroup_->decreaseNumCommand();
  if (udpTrackerClient_) {
    udpTrackerClient_->decreaseWatchers();
  }
}

bool TrackerWatcherCommand::execute()
{
  if (requestGroup_->isForceHaltRequested()) {
    if (!trackerRequest_) {
      return true;
    }
    else if (trackerRequest_->stopped() || trackerRequest_->success()) {
      return true;
    }
    else {
      trackerRequest_->stop(e_);
      e_->setRefreshInterval(std::chrono::milliseconds(0));
      e_->addCommand(std::unique_ptr<Command>(this));
      return false;
    }
  }
  if (btAnnounce_->noMoreAnnounce()) {
    A2_LOG_DEBUG("no more announce");
    return true;
  }
  if (!trackerRequest_) {
    trackerRequest_ = createAnnounce(e_);
    if (trackerRequest_) {
      trackerRequest_->issue(e_);
      A2_LOG_DEBUG("tracker request created");
    }
  }
  else if (trackerRequest_->stopped()) {
    // We really want to make sure that tracker request has finished
    // by checking getNumCommand() == 0. Because we reset
    // trackerRequestGroup_, if it is still used in other Command, we
    // will get Segmentation fault.
    if (trackerRequest_->success()) {
      if (trackerRequest_->processResponse(btAnnounce_)) {
        btAnnounce_->announceSuccess();
        btAnnounce_->resetAnnounce();
        addConnection();
      }
      else {
        btAnnounce_->announceFailure();
        if (btAnnounce_->isAllAnnounceFailed()) {
          btAnnounce_->resetAnnounce();
        }
      }
      trackerRequest_.reset();
    }
    else {
      // handle errors here
      btAnnounce_->announceFailure(); // inside it, trackers = 0.
      trackerRequest_.reset();
      if (btAnnounce_->isAllAnnounceFailed()) {
        btAnnounce_->resetAnnounce();
      }
    }
  }

  if (!trackerRequest_ && btAnnounce_->noMoreAnnounce()) {
    A2_LOG_DEBUG("no more announce");
    return true;
  }

  e_->addCommand(std::unique_ptr<Command>(this));
  return false;
}

void TrackerWatcherCommand::addConnection()
{
  while (!btRuntime_->isHalt() && btRuntime_->lessThanMinPeers()) {
    if (!peerStorage_->isPeerAvailable()) {
      break;
    }
    cuid_t ncuid = e_->newCUID();
    std::shared_ptr<Peer> peer = peerStorage_->checkoutPeer(ncuid);
    // sanity check
    if (!peer) {
      break;
    }
    auto command = make_unique<PeerInitiateConnectionCommand>(
        ncuid, requestGroup_, peer, e_, btRuntime_);
    command->setPeerStorage(peerStorage_);
    command->setPieceStorage(pieceStorage_);
    e_->addCommand(std::move(command));
    A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Adding new command CUID#%" PRId64 "",
                     getCuid(), peer->usedBy()));
  }
}

std::unique_ptr<AnnRequest>
TrackerWatcherCommand::createAnnounce(DownloadEngine* e)
{
  while (!btAnnounce_->isAllAnnounceFailed() &&
         btAnnounce_->isAnnounceReady()) {
    std::string uri = btAnnounce_->getAnnounceUrl();
    uri_split_result res;
    memset(&res, 0, sizeof(res));
    if (uri_split(&res, uri.c_str()) == 0) {
      // Without UDP tracker support, send it to normal tracker flow
      // and make it fail.
      std::unique_ptr<AnnRequest> treq;
      if (udpTrackerClient_ &&
          uri::getFieldString(res, USR_SCHEME, uri.c_str()) == "udp") {
        uint16_t localPort;
        localPort = e->getBtRegistry()->getTcpPort();
        treq =
            createUDPAnnRequest(uri::getFieldString(res, USR_HOST, uri.c_str()),
                                res.port, localPort);
      }
      else {
        treq = createHTTPAnnRequest(btAnnounce_->getAnnounceUrl());
      }
      btAnnounce_->announceStart(); // inside it, trackers++.
      return treq;
    }
    else {
      btAnnounce_->announceFailure();
    }
  }
  if (btAnnounce_->isAllAnnounceFailed()) {
    btAnnounce_->resetAnnounce();
  }
  return nullptr;
}

std::unique_ptr<AnnRequest>
TrackerWatcherCommand::createUDPAnnRequest(const std::string& host,
                                           uint16_t port, uint16_t localPort)
{
  auto req = btAnnounce_->createUDPTrackerRequest(host, port, localPort);
  req->user_data = this;

  return make_unique<UDPAnnRequest>(std::move(req));
}

namespace {
bool backupTrackerIsAvailable(const std::shared_ptr<DownloadContext>& context)
{
  auto torrentAttrs = bittorrent::getTorrentAttrs(context);
  if (torrentAttrs->announceList.size() >= 2) {
    return true;
  }
  if (torrentAttrs->announceList.empty()) {
    return false;
  }
  if (torrentAttrs->announceList[0].size() >= 2) {
    return true;
  }
  else {
    return false;
  }
}
} // namespace

std::unique_ptr<AnnRequest>
TrackerWatcherCommand::createHTTPAnnRequest(const std::string& uri)
{
  std::vector<std::string> uris;
  uris.push_back(uri);
  auto option = util::copy(getOption());
  auto rg = make_unique<RequestGroup>(GroupId::create(), option);
  if (backupTrackerIsAvailable(requestGroup_->getDownloadContext())) {
    A2_LOG_DEBUG("This is multi-tracker announce.");
  }
  else {
    A2_LOG_DEBUG("This is single-tracker announce.");
  }
  rg->setNumConcurrentCommand(1);
  // If backup tracker is available, try 2 times for each tracker
  // and if they all fails, then try next one.
  option->put(PREF_MAX_TRIES, "2");
  // TODO When dry-run mode becomes available in BitTorrent, set
  // PREF_DRY_RUN=false too.
  option->put(PREF_USE_HEAD, A2_V_FALSE);
  // Setting tracker timeouts
  rg->setTimeout(
      std::chrono::seconds(option->getAsInt(PREF_BT_TRACKER_TIMEOUT)));
  option->put(PREF_CONNECT_TIMEOUT,
              option->get(PREF_BT_TRACKER_CONNECT_TIMEOUT));
  option->put(PREF_REUSE_URI, A2_V_FALSE);
  option->put(PREF_SELECT_LEAST_USED_HOST, A2_V_FALSE);
  auto dctx = std::make_shared<DownloadContext>(
      option->getAsInt(PREF_PIECE_LENGTH), 0, "[tracker.announce]");
  dctx->getFileEntries().front()->setUris(uris);
  rg->setDownloadContext(dctx);
  auto dwf = std::make_shared<ByteArrayDiskWriterFactory>();
  rg->setDiskWriterFactory(dwf);
  rg->setFileAllocationEnabled(false);
  rg->setPreLocalFileCheckEnabled(false);
  // Clearing pre- and post handler is not needed because the
  // RequestGroup is not handled by RequestGroupMan.
  rg->clearPreDownloadHandler();
  rg->clearPostDownloadHandler();
  dctx->setAcceptMetalink(false);
  A2_LOG_INFO(fmt("Creating tracker request group GID#%s",
                  GroupId::toHex(rg->getGID()).c_str()));
  return make_unique<HTTPAnnRequest>(std::move(rg));
}

void TrackerWatcherCommand::setBtRuntime(
    const std::shared_ptr<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

void TrackerWatcherCommand::setPeerStorage(
    const std::shared_ptr<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void TrackerWatcherCommand::setPieceStorage(
    const std::shared_ptr<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void TrackerWatcherCommand::setBtAnnounce(
    const std::shared_ptr<BtAnnounce>& btAnnounce)
{
  btAnnounce_ = btAnnounce;
}

const std::shared_ptr<Option>& TrackerWatcherCommand::getOption() const
{
  return requestGroup_->getOption();
}

} // namespace aria2
