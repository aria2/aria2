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
#include "DownloadEngine.h"
#include "RecoverableException.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageReceiver.h"
#include "DHTTaskQueue.h"
#include "DHTMessage.h"
#include "Socket.h"
#include "message.h"
#include "RequestGroupMan.h"
#include "Logger.h"
#include "DHTMessageCallback.h"
#include "DHTNode.h"
#include "FileEntry.h"

namespace aria2 {

DHTInteractionCommand::DHTInteractionCommand(int32_t cuid, DownloadEngine* e):
  Command(cuid),
  _e(e) {}

DHTInteractionCommand::~DHTInteractionCommand()
{
  disableReadCheckSocket(_readCheckSocket);
}

void DHTInteractionCommand::setReadCheckSocket(const SocketHandle& socket)
{
  _readCheckSocket = socket;
  _e->addSocketForReadCheck(socket, this);
}

void DHTInteractionCommand::disableReadCheckSocket(const SocketHandle& socket)
{
  _e->deleteSocketForReadCheck(socket, this);
}

bool DHTInteractionCommand::execute()
{
  if(_e->_requestGroupMan->downloadFinished() || _e->isHaltRequested()) {
    return true;
  }

  _taskQueue->executeTask();

  for(size_t i = 0; i < 20; ++i) {
    SharedHandle<DHTMessage> m = _receiver->receiveMessage();
    if(m.isNull()) {
      break;
    }
  }
  _receiver->handleTimeout();
  try {
    _dispatcher->sendMessages();
  } catch(RecoverableException& e) {
    logger->error(EX_EXCEPTION_CAUGHT, e);
  }
  _e->commands.push_back(this);
  return false;
}

void DHTInteractionCommand::setMessageDispatcher(const SharedHandle<DHTMessageDispatcher>& dispatcher)
{
  _dispatcher = dispatcher;
}

void DHTInteractionCommand::setMessageReceiver(const SharedHandle<DHTMessageReceiver>& receiver)
{
  _receiver = receiver;
}

void DHTInteractionCommand::setTaskQueue(const SharedHandle<DHTTaskQueue>& taskQueue)
{
  _taskQueue = taskQueue;
}

} // namespace aria2
