/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "HttpListenCommand.h"
#include "DownloadEngine.h"
#include "RecoverableException.h"
#include "message.h"
#include "Logger.h"
#include "SocketCore.h"
#include "HttpServerCommand.h"
#include "CUIDCounter.h"
#include "RequestGroupMan.h"
#include "FileEntry.h"
#include "prefs.h"
#include "Option.h"
#include "util.h"

namespace aria2 {

HttpListenCommand::HttpListenCommand(cuid_t cuid, DownloadEngine* e):
  Command(cuid),_e(e) {}

HttpListenCommand::~HttpListenCommand()
{
  if(!_serverSocket.isNull()) {
    _e->deleteSocketForReadCheck(_serverSocket, this);
  }
}

bool HttpListenCommand::execute()
{
  if(_e->_requestGroupMan->downloadFinished() || _e->isHaltRequested()) {
    return true;
  }
  try {
    if(_serverSocket->isReadable(0)) {
      SharedHandle<SocketCore> socket(_serverSocket->acceptConnection());
      socket->setNonBlockingMode();

      std::pair<std::string, uint16_t> peerInfo;
      socket->getPeerInfo(peerInfo);

      logger->info("XML-RPC: Accepted the connection from %s:%u.",
                   peerInfo.first.c_str(), peerInfo.second);

      HttpServerCommand* c =
        new HttpServerCommand(_e->newCUID(), _e, socket);
      _e->setNoWait(true);
      _e->commands.push_back(c);
    }
  } catch(RecoverableException& e) {
    if(logger->debug()) {
      logger->debug(MSG_ACCEPT_FAILURE, _e, util::itos(cuid).c_str());
    }
  }
  _e->commands.push_back(this);
  return false;
}

bool HttpListenCommand::bindPort(uint16_t port)
{
  if(!_serverSocket.isNull()) {
    _e->deleteSocketForReadCheck(_serverSocket, this);
  }
  _serverSocket.reset(new SocketCore());
  if(logger->info()) {
    logger->info("CUID#%s - Setting up HttpListenCommand",
                 util::itos(cuid).c_str());
  }
  try {
    int flags = 0;
    if(_e->option->getAsBool(PREF_XML_RPC_LISTEN_ALL)) {
      flags = AI_PASSIVE;
    }
    _serverSocket->bind(port, flags);
    _serverSocket->beginListen();
    _serverSocket->setNonBlockingMode();
    if(logger->info()) {
      logger->info(MSG_LISTENING_PORT, util::itos(cuid).c_str(), port);
    }
    _e->addSocketForReadCheck(_serverSocket, this);
    return true;
  } catch(RecoverableException& e) {
    logger->error(MSG_BIND_FAILURE, e, util::itos(cuid).c_str(), port);
    if(!_serverSocket.isNull()) {
      _e->deleteSocketForReadCheck(_serverSocket, this);
    }
    _serverSocket->closeConnection();
  }
  return false;
}

} // namespace aria2
