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
#include "DHTGetPeersCommand.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTask.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "DHTPeerLookupTask.h"
#include "DHTNode.h"
#include "DHTNodeLookupEntry.h"

DHTGetPeersCommand::DHTGetPeersCommand(int32_t cuid,
				       RequestGroup* requestGroup,
				       DownloadEngine* e,
				       const BtContextHandle& ctx):
  Command(cuid),
  BtContextAwareCommand(ctx),
  RequestGroupAware(requestGroup),
  _e(e),
  _taskQueue(0),
  _taskFactory(0),
  _task(0),
  _numRetry(0),
  _lastGetPeerTime(0)
{}

DHTGetPeersCommand::~DHTGetPeersCommand() {}

bool DHTGetPeersCommand::execute()
{
  if(btRuntime->isHalt()) {
    return true;
  }
  if(_task.isNull() &&
     (_numRetry > 0 && _lastGetPeerTime.elapsed(RETRY_INTERVAL) ||
      _lastGetPeerTime.elapsed(GET_PEER_INTERVAL))) {
    logger->debug("Issuing PeerLookup for infoHash=%s",
		  btContext->getInfoHashAsString().c_str());
    _task = _taskFactory->createPeerLookupTask(btContext);
    _taskQueue->addPeriodicTask2(_task);
  } else if(!_task.isNull() && _task->finished()) {
    _lastGetPeerTime.reset();
    if(_numRetry < MAX_RETRIES && btRuntime->lessThanEqMinPeer()) {
      ++_numRetry;
    } else {
      _numRetry = 0;
    }
    _task = 0;
  }

  _e->commands.push_back(this);
  return false;
}

void DHTGetPeersCommand::setTaskQueue(const DHTTaskQueueHandle& taskQueue)
{
  _taskQueue = taskQueue;
}

void DHTGetPeersCommand::setTaskFactory(const DHTTaskFactoryHandle& taskFactory)
{
  _taskFactory = taskFactory;
}
