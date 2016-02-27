/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "ConnectCommand.h"
#include "BackupIPv4ConnectCommand.h"
#include "ControlChain.h"
#include "Option.h"
#include "message.h"
#include "SocketCore.h"
#include "fmt.h"
#include "LogFactory.h"
#include "DownloadEngine.h"
#include "Request.h"
#include "prefs.h"
#include "SocketRecvBuffer.h"

namespace aria2 {

ConnectCommand::ConnectCommand(cuid_t cuid, const std::shared_ptr<Request>& req,
                               const std::shared_ptr<Request>& proxyRequest,
                               const std::shared_ptr<FileEntry>& fileEntry,
                               RequestGroup* requestGroup, DownloadEngine* e,
                               const std::shared_ptr<SocketCore>& s)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e, s),
      proxyRequest_(proxyRequest)
{
  setTimeout(std::chrono::seconds(getOption()->getAsInt(PREF_CONNECT_TIMEOUT)));
  disableReadCheckSocket();
  setWriteCheckSocket(getSocket());
}

ConnectCommand::~ConnectCommand()
{
  if (backupConnectionInfo_) {
    backupConnectionInfo_->cancel = true;
  }
}

void ConnectCommand::setControlChain(
    const std::shared_ptr<ControlChain<ConnectCommand*>>& chain)
{
  chain_ = chain;
}

void ConnectCommand::setBackupConnectInfo(
    const std::shared_ptr<BackupConnectInfo>& info)
{
  backupConnectionInfo_ = info;
}

const std::shared_ptr<Request>& ConnectCommand::getProxyRequest() const
{
  return proxyRequest_;
}

bool ConnectCommand::executeInternal()
{
  if (backupConnectionInfo_ && !backupConnectionInfo_->ipaddr.empty()) {
    A2_LOG_INFO(fmt("CUID#%" PRId64 " - Use backup connection address %s",
                    getCuid(), backupConnectionInfo_->ipaddr.c_str()));
    getDownloadEngine()->markBadIPAddress(getRequest()->getConnectedHostname(),
                                          getRequest()->getConnectedAddr(),
                                          getRequest()->getConnectedPort());

    getRequest()->setConnectedAddrInfo(getRequest()->getConnectedHostname(),
                                       backupConnectionInfo_->ipaddr,
                                       getRequest()->getConnectedPort());
    swapSocket(backupConnectionInfo_->socket);
    backupConnectionInfo_.reset();
  }
  if (!checkIfConnectionEstablished(
          getSocket(), getRequest()->getConnectedHostname(),
          getRequest()->getConnectedAddr(), getRequest()->getConnectedPort())) {
    return true;
  }
  if (backupConnectionInfo_) {
    backupConnectionInfo_->cancel = true;
    backupConnectionInfo_.reset();
  }
  chain_->run(this, getDownloadEngine());
  return true;
}

bool ConnectCommand::noCheck() const
{
  return backupConnectionInfo_ && !backupConnectionInfo_->ipaddr.empty();
}

} // namespace aria2
