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
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "A2STR.h"

namespace aria2 {

HttpListenCommand::HttpListenCommand
(cuid_t cuid, DownloadEngine* e, int family):
  Command(cuid), e_(e), family_(family) {}

HttpListenCommand::~HttpListenCommand()
{
  if(!serverSocket_.isNull()) {
    e_->deleteSocketForReadCheck(serverSocket_, this);
  }
}

bool HttpListenCommand::execute()
{
  if(e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
    return true;
  }
  try {
    if(serverSocket_->isReadable(0)) {
      SharedHandle<SocketCore> socket(serverSocket_->acceptConnection());
      socket->setNonBlockingMode();

      std::pair<std::string, uint16_t> peerInfo;
      socket->getPeerInfo(peerInfo);

      getLogger()->info("XML-RPC: Accepted the connection from %s:%u.",
                        peerInfo.first.c_str(), peerInfo.second);

      HttpServerCommand* c =
        new HttpServerCommand(e_->newCUID(), e_, socket);
      e_->setNoWait(true);
      e_->addCommand(c);
    }
  } catch(RecoverableException& e) {
    if(getLogger()->debug()) {
      getLogger()->debug(MSG_ACCEPT_FAILURE, e, util::itos(getCuid()).c_str());
    }
  }
  e_->addCommand(this);
  return false;
}

bool HttpListenCommand::bindPort(uint16_t port)
{
  if(!serverSocket_.isNull()) {
    e_->deleteSocketForReadCheck(serverSocket_, this);
  }
  serverSocket_.reset(new SocketCore());
  if(getLogger()->info()) {
    getLogger()->info("CUID#%s - Setting up HttpListenCommand for IPv%d",
                      util::itos(getCuid()).c_str(),
                      family_ == AF_INET?4:6);
  }
  try {
    int flags = 0;
    if(e_->getOption()->getAsBool(PREF_XML_RPC_LISTEN_ALL)) {
      flags = AI_PASSIVE;
    }
    serverSocket_->bind(A2STR::NIL, port, family_, flags);
    serverSocket_->beginListen();
    serverSocket_->setNonBlockingMode();
    if(getLogger()->info()) {
      getLogger()->info(MSG_LISTENING_PORT,
                        util::itos(getCuid()).c_str(), port);
    }
    e_->addSocketForReadCheck(serverSocket_, this);
    return true;
  } catch(RecoverableException& e) {
    getLogger()->error("Failed to setup XML-RPC server for IPv%d",
                       family_ == AF_INET?4:6);
    getLogger()->error(MSG_BIND_FAILURE, e,
                       util::itos(getCuid()).c_str(), port);
    if(!serverSocket_.isNull()) {
      e_->deleteSocketForReadCheck(serverSocket_, this);
    }
    serverSocket_->closeConnection();
  }
  return false;
}

} // namespace aria2
