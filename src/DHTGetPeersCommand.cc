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
#include "DHTGetPeersCommand.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTask.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "DHTNode.h"
#include "DHTNodeLookupEntry.h"
#include "BtRuntime.h"
#include "PeerStorage.h"
#include "Peer.h"
#include "Logger.h"
#include "LogFactory.h"
#include "bittorrent_helper.h"
#include "DownloadContext.h"
#include "wallclock.h"
#include "fmt.h"
#include "BtRegistry.h"

namespace aria2 {

namespace {

const time_t GET_PEER_INTERVAL = (15*60);
// Interval when the size of peer list is low.
const time_t GET_PEER_INTERVAL_LOW = (5*60);
// Interval when the peer list is empty.
const time_t GET_PEER_INTERVAL_ZERO = 60;
// Interval for retry.
const time_t GET_PEER_INTERVAL_RETRY = 5;
// Maximum retries. Try more than 5 to drop bad node.
const int MAX_RETRIES = 10;

} // namespace

DHTGetPeersCommand::DHTGetPeersCommand
(cuid_t cuid,
 RequestGroup* requestGroup,
 DownloadEngine* e)
  : Command(cuid),
    requestGroup_(requestGroup),
    e_(e),
    numRetry_(0),
    lastGetPeerTime_(0)
{
  requestGroup_->increaseNumCommand();
}

DHTGetPeersCommand::~DHTGetPeersCommand()
{
  requestGroup_->decreaseNumCommand();
}

bool DHTGetPeersCommand::execute()
{
  if(btRuntime_->isHalt()) {
    return true;
  }
  time_t elapsed = lastGetPeerTime_.difference(global::wallclock());
  if(!task_ &&
     (elapsed >= GET_PEER_INTERVAL ||
      (((btRuntime_->lessThanMinPeers() &&
         ((numRetry_ && elapsed >= GET_PEER_INTERVAL_RETRY) ||
          elapsed >= GET_PEER_INTERVAL_LOW)) ||
        (btRuntime_->getConnections() == 0 &&
         elapsed >= GET_PEER_INTERVAL_ZERO))
       && !requestGroup_->downloadFinished()))) {
    A2_LOG_DEBUG(fmt("Issuing PeerLookup for infoHash=%s",
                     bittorrent::getInfoHashString
                     (requestGroup_->getDownloadContext()).c_str()));
    task_ = taskFactory_->createPeerLookupTask
      (requestGroup_->getDownloadContext(),
       e_->getBtRegistry()->getTcpPort(),
       peerStorage_);
    taskQueue_->addPeriodicTask2(task_);
  } else if(task_ && task_->finished()) {
    A2_LOG_DEBUG("task finished detected");
    lastGetPeerTime_ = global::wallclock();
    if(numRetry_ < MAX_RETRIES &&
       (btRuntime_->getMaxPeers() == 0 ||
        btRuntime_->getMaxPeers()) > peerStorage_->countPeer()) {
      ++numRetry_;
      A2_LOG_DEBUG(fmt("Too few peers. peers=%lu, max_peers=%d."
                       " Try again(%d)",
                       static_cast<unsigned long>(peerStorage_->countPeer()),
                       btRuntime_->getMaxPeers(),
                       numRetry_));
    } else {
      numRetry_ = 0;
    }
    task_.reset();
  }

  e_->addCommand(this);
  return false;
}

void DHTGetPeersCommand::setTaskQueue(const SharedHandle<DHTTaskQueue>& taskQueue)
{
  taskQueue_ = taskQueue;
}

void DHTGetPeersCommand::setTaskFactory(const SharedHandle<DHTTaskFactory>& taskFactory)
{
  taskFactory_ = taskFactory;
}

void DHTGetPeersCommand::setBtRuntime(const SharedHandle<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

void DHTGetPeersCommand::setPeerStorage(const SharedHandle<PeerStorage>& ps)
{
  peerStorage_ = ps;
}

} // namespace aria2
