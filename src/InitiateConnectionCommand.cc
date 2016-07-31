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
#include "LogFactory.h"
#include "message.h"
#include "prefs.h"
#include "NameResolver.h"
#include "SocketCore.h"
#include "FileEntry.h"
#include "RequestGroup.h"
#include "Segment.h"
#include "a2functional.h"
#include "InitiateConnectionCommandFactory.h"
#include "util.h"
#include "RecoverableException.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"
#include "BackupIPv4ConnectCommand.h"
#include "ConnectCommand.h"

namespace aria2 {

InitiateConnectionCommand::InitiateConnectionCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    DownloadEngine* e)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e)
{
  setTimeout(std::chrono::seconds(getOption()->getAsInt(PREF_DNS_TIMEOUT)));
  // give a chance to be executed in the next loop in DownloadEngine
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

InitiateConnectionCommand::~InitiateConnectionCommand() = default;

bool InitiateConnectionCommand::executeInternal()
{
  std::string hostname;
  uint16_t port;
  std::shared_ptr<Request> proxyRequest = createProxyRequest();
  if (!proxyRequest) {
    hostname = getRequest()->getHost();
    port = getRequest()->getPort();
  }
  else {
    hostname = proxyRequest->getHost();
    port = proxyRequest->getPort();
  }
  std::vector<std::string> addrs;
  std::string ipaddr = resolveHostname(addrs, hostname, port);
  if (ipaddr.empty()) {
    addCommandSelf();
    return false;
  }
  try {
    auto c = createNextCommand(hostname, ipaddr, port, addrs, proxyRequest);
    c->setStatus(Command::STATUS_ONESHOT_REALTIME);
    getDownloadEngine()->setNoWait(true);
    getDownloadEngine()->addCommand(std::move(c));
    return true;
  }
  catch (RecoverableException& ex) {
    // Catch exception and retry another address.
    // See also AbstractCommand::checkIfConnectionEstablished

    // TODO ipaddr might not be used if pooled socket was found.
    getDownloadEngine()->markBadIPAddress(hostname, ipaddr, port);
    if (!getDownloadEngine()->findCachedIPAddress(hostname, port).empty()) {
      A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, ex);
      A2_LOG_INFO(
          fmt(MSG_CONNECT_FAILED_AND_RETRY, getCuid(), ipaddr.c_str(), port));
      auto command =
          InitiateConnectionCommandFactory::createInitiateConnectionCommand(
              getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
              getDownloadEngine());
      getDownloadEngine()->setNoWait(true);
      getDownloadEngine()->addCommand(std::move(command));
      return true;
    }
    getDownloadEngine()->removeCachedIPAddress(hostname, port);
    throw;
  }
}

void InitiateConnectionCommand::setConnectedAddrInfo(
    const std::shared_ptr<Request>& req, const std::string& hostname,
    const std::shared_ptr<SocketCore>& socket)
{
  auto endpoint = socket->getPeerInfo();
  req->setConnectedAddrInfo(hostname, endpoint.addr, endpoint.port);
}

std::shared_ptr<BackupConnectInfo>
InitiateConnectionCommand::createBackupIPv4ConnectCommand(
    const std::string& hostname, const std::string& ipaddr, uint16_t port,
    Command* mainCommand)
{
  // Prepare IPv4 backup connection attempt in "Happy Eyeballs"
  // fashion.
  std::shared_ptr<BackupConnectInfo> info;
  char buf[sizeof(in6_addr)];
  if (inetPton(AF_INET6, ipaddr.c_str(), &buf) == -1) {
    return info;
  }
  A2_LOG_INFO("Searching IPv4 address for backup connection attempt");
  std::vector<std::string> addrs;
  getDownloadEngine()->findAllCachedIPAddresses(std::back_inserter(addrs),
                                                hostname, port);
  for (std::vector<std::string>::const_iterator i = addrs.begin(),
                                                eoi = addrs.end();
       i != eoi; ++i) {
    if (inetPton(AF_INET, (*i).c_str(), &buf) == 0) {
      info = std::make_shared<BackupConnectInfo>();
      auto command = make_unique<BackupIPv4ConnectCommand>(
          getDownloadEngine()->newCUID(), *i, port, info, mainCommand,
          getRequestGroup(), getDownloadEngine());
      A2_LOG_INFO(fmt("Issue backup connection command CUID#%" PRId64
                      ", addr=%s",
                      command->getCuid(), (*i).c_str()));
      getDownloadEngine()->addCommand(std::move(command));
      return info;
    }
  }
  return info;
}

void InitiateConnectionCommand::setupBackupConnection(
    const std::string& hostname, const std::string& addr, uint16_t port,
    ConnectCommand* c)
{
  std::shared_ptr<BackupConnectInfo> backupConnectInfo =
      createBackupIPv4ConnectCommand(hostname, addr, port, c);
  if (backupConnectInfo) {
    c->setBackupConnectInfo(backupConnectInfo);
  }
}

} // namespace aria2
