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
#include "InitiateConnectionCommand.h"
#include "Request.h"
#include "DownloadEngine.h"
#include "Option.h"
#include "Logger.h"
#include "message.h"
#include "prefs.h"
#include "NameResolver.h"
#include "SocketCore.h"
#include "FileEntry.h"
#include "RequestGroup.h"
#include "DownloadContext.h"
#include "Segment.h"
#include "a2functional.h"
#include "InitiateConnectionCommandFactory.h"
#include "util.h"
#include "RequestGroupMan.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "RecoverableException.h"

namespace aria2 {

InitiateConnectionCommand::InitiateConnectionCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 DownloadEngine* e):
  AbstractCommand(cuid, req, fileEntry, requestGroup, e)
{
  setTimeout(getOption()->getAsInt(PREF_DNS_TIMEOUT));
  // give a chance to be executed in the next loop in DownloadEngine
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

InitiateConnectionCommand::~InitiateConnectionCommand() {}

bool InitiateConnectionCommand::executeInternal() {
  std::string hostname;
  uint16_t port;
  SharedHandle<Request> proxyRequest = createProxyRequest();
  if(!proxyRequest) {
    hostname = getRequest()->getHost();
    port = getRequest()->getPort();
  } else {
    hostname = proxyRequest->getHost();
    port = proxyRequest->getPort();
  }
  std::vector<std::string> addrs;
  std::string ipaddr = resolveHostname(addrs, hostname, port);
  if(ipaddr.empty()) {
    getDownloadEngine()->addCommand(this);
    return false;
  }
  try {
    Command* command = createNextCommand(hostname, ipaddr, port,
                                         addrs, proxyRequest);
    getDownloadEngine()->addCommand(command);
    return true;
  } catch(RecoverableException& ex) {
    // Catch exception and retry another address.
    // See also AbstractCommand::checkIfConnectionEstablished

    // TODO ipaddr might not be used if pooled sockt was found.
    getDownloadEngine()->markBadIPAddress(hostname, ipaddr, port);
    if(!getDownloadEngine()->findCachedIPAddress(hostname, port).empty()) {
      if(getLogger()->info()) {
        getLogger()->info(EX_EXCEPTION_CAUGHT, ex);
        getLogger()->info(MSG_CONNECT_FAILED_AND_RETRY,
                          util::itos(getCuid()).c_str(), ipaddr.c_str(), port);
      }
      Command* command =
        InitiateConnectionCommandFactory::createInitiateConnectionCommand
        (getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
         getDownloadEngine());
      getDownloadEngine()->setNoWait(true);
      getDownloadEngine()->addCommand(command);
      return true;
    }
    getDownloadEngine()->removeCachedIPAddress(hostname, port);
    throw;
  }
}

void InitiateConnectionCommand::setConnectedAddrInfo
(const SharedHandle<Request>& req,
 const std::string& hostname,
 const SharedHandle<SocketCore>& socket)
{
  std::pair<std::string, uint16_t> peerAddr;
  socket->getPeerInfo(peerAddr);
  req->setConnectedAddrInfo(hostname, peerAddr.first, peerAddr.second);
}

} // namespace aria2
