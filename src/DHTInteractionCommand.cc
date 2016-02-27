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
#include "DHTInteractionCommand.h"

#include <array>

#include "DownloadEngine.h"
#include "RecoverableException.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageReceiver.h"
#include "DHTTaskQueue.h"
#include "DHTMessage.h"
#include "SocketCore.h"
#include "message.h"
#include "RequestGroupMan.h"
#include "Logger.h"
#include "LogFactory.h"
#include "DHTMessageCallback.h"
#include "DHTNode.h"
#include "DHTConnection.h"
#include "UDPTrackerClient.h"
#include "UDPTrackerRequest.h"
#include "fmt.h"
#include "wallclock.h"
#include "TrackerWatcherCommand.h"

namespace aria2 {

// TODO This name of this command is misleading, because now it also
// handles UDP trackers as well as DHT.
DHTInteractionCommand::DHTInteractionCommand(cuid_t cuid, DownloadEngine* e)
    : Command{cuid},
      e_{e},
      dispatcher_{nullptr},
      receiver_{nullptr},
      taskQueue_{nullptr}
{
  setStatusRealtime();
}

DHTInteractionCommand::~DHTInteractionCommand()
{
  disableReadCheckSocket(readCheckSocket_);
}

void DHTInteractionCommand::setReadCheckSocket(
    const std::shared_ptr<SocketCore>& socket)
{
  readCheckSocket_ = socket;
  if (socket) {
    e_->addSocketForReadCheck(socket, this);
  }
}

void DHTInteractionCommand::disableReadCheckSocket(
    const std::shared_ptr<SocketCore>& socket)
{
  if (socket) {
    e_->deleteSocketForReadCheck(socket, this);
  }
}

bool DHTInteractionCommand::execute()
{
  // We need to keep this command alive while TrackerWatcherCommand
  // needs this.
  if (e_->getRequestGroupMan()->downloadFinished() ||
      (e_->isHaltRequested() && udpTrackerClient_->getNumWatchers() == 0)) {
    A2_LOG_DEBUG("DHTInteractionCommand exiting");
    return true;
  }
  else if (e_->isForceHaltRequested()) {
    udpTrackerClient_->failAll();
    A2_LOG_DEBUG("DHTInteractionCommand exiting");
    return true;
  }

  taskQueue_->executeTask();

  std::string remoteAddr;
  uint16_t remotePort;
  std::array<unsigned char, 64_k> data;
  try {
    while (1) {
      ssize_t length = connection_->receiveMessage(data.data(), data.size(),
                                                   remoteAddr, remotePort);
      if (length <= 0) {
        break;
      }
      if (data[0] == 'd') {
        // udp tracker response does not start with 'd', so assume
        // this message belongs to DHT. nothrow.
        receiver_->receiveMessage(remoteAddr, remotePort, data.data(), length);
      }
      else {
        // this may be udp tracker response. nothrow.
        std::shared_ptr<UDPTrackerRequest> req;
        if (udpTrackerClient_->receiveReply(req, data.data(), length,
                                            remoteAddr, remotePort,
                                            global::wallclock()) == 0) {
          if (req->action == UDPT_ACT_ANNOUNCE) {
            auto c = static_cast<TrackerWatcherCommand*>(req->user_data);
            if (c) {
              c->setStatus(Command::STATUS_ONESHOT_REALTIME);
              e_->setNoWait(true);
            }
          }
        }
      }
    }
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX("Exception thrown while receiving UDP message.", e);
  }
  receiver_->handleTimeout();
  udpTrackerClient_->handleTimeout(global::wallclock());
  dispatcher_->sendMessages();
  while (!udpTrackerClient_->getPendingRequests().empty()) {
    // no throw
    ssize_t length = udpTrackerClient_->createRequest(
        data.data(), data.size(), remoteAddr, remotePort, global::wallclock());
    if (length == -1) {
      break;
    }
    try {
      // throw
      connection_->sendMessage(data.data(), length, remoteAddr, remotePort);
      udpTrackerClient_->requestSent(global::wallclock());
    }
    catch (RecoverableException& e) {
      A2_LOG_INFO_EX("Exception thrown while sending UDP tracker request.", e);
      udpTrackerClient_->requestFail(UDPT_ERR_NETWORK);
    }
  }
  e_->addRoutineCommand(std::unique_ptr<Command>(this));
  return false;
}

void DHTInteractionCommand::setMessageDispatcher(
    DHTMessageDispatcher* dispatcher)
{
  dispatcher_ = dispatcher;
}

void DHTInteractionCommand::setMessageReceiver(DHTMessageReceiver* receiver)
{
  receiver_ = receiver;
}

void DHTInteractionCommand::setTaskQueue(DHTTaskQueue* taskQueue)
{
  taskQueue_ = taskQueue;
}

void DHTInteractionCommand::setConnection(
    std::unique_ptr<DHTConnection> connection)
{
  connection_ = std::move(connection);
}

void DHTInteractionCommand::setUDPTrackerClient(
    const std::shared_ptr<UDPTrackerClient>& udpTrackerClient)
{
  udpTrackerClient_ = udpTrackerClient;
}

} // namespace aria2
