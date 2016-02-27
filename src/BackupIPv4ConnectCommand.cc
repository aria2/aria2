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
#include "BackupIPv4ConnectCommand.h"
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "SocketCore.h"
#include "wallclock.h"
#include "RecoverableException.h"
#include "fmt.h"
#include "LogFactory.h"
#include "prefs.h"
#include "Option.h"

namespace aria2 {

BackupConnectInfo::BackupConnectInfo() : cancel(false) {}

BackupIPv4ConnectCommand::BackupIPv4ConnectCommand(
    cuid_t cuid, const std::string& ipaddr, uint16_t port,
    const std::shared_ptr<BackupConnectInfo>& info, Command* mainCommand,
    RequestGroup* requestGroup, DownloadEngine* e)
    : Command(cuid),
      ipaddr_(ipaddr),
      port_(port),
      info_(info),
      mainCommand_(mainCommand),
      requestGroup_(requestGroup),
      e_(e),
      startTime_(global::wallclock()),
      timeoutCheck_(global::wallclock()),
      timeout_(requestGroup_->getOption()->getAsInt(PREF_CONNECT_TIMEOUT))
{
  requestGroup_->increaseStreamCommand();
  requestGroup_->increaseNumCommand();
}

BackupIPv4ConnectCommand::~BackupIPv4ConnectCommand()
{
  requestGroup_->decreaseNumCommand();
  requestGroup_->decreaseStreamCommand();
  if (socket_) {
    e_->deleteSocketForWriteCheck(socket_, this);
  }
}

bool BackupIPv4ConnectCommand::execute()
{
  bool retval = false;
  if (requestGroup_->downloadFinished() || requestGroup_->isHaltRequested()) {
    retval = true;
  }
  else if (info_->cancel) {
    A2_LOG_INFO(
        fmt("CUID#%" PRId64 " - Backup connection canceled", getCuid()));
    retval = true;
  }
  else if (socket_) {
    if (writeEventEnabled()) {
      try {
        std::string error = socket_->getSocketError();
        if (error.empty()) {
          A2_LOG_INFO(fmt("CUID#%" PRId64 " - Backup connection to %s "
                          "established",
                          getCuid(), ipaddr_.c_str()));
          info_->ipaddr = ipaddr_;
          e_->deleteSocketForWriteCheck(socket_, this);
          info_->socket.swap(socket_);
          mainCommand_->setStatus(STATUS_ONESHOT_REALTIME);
          e_->setNoWait(true);
          retval = true;
        }
        else {
          A2_LOG_INFO(fmt("CUID#%" PRId64 " - Backup connection failed: %s",
                          getCuid(), error.c_str()));
          retval = true;
        }
      }
      catch (RecoverableException& e) {
        A2_LOG_INFO_EX(
            fmt("CUID#%" PRId64 " - Backup connection failed", getCuid()), e);
        retval = true;
      }
    }
  }
  else if (!socket_) {
    // TODO Although we check 300ms initial timeout as described in
    // RFC 6555, the interval will be much longer and around 1 second
    // due to the refresh interval mechanism in DownloadEngine.
    if (startTime_.difference(global::wallclock()) >=
        std::chrono::milliseconds(300)) {
      socket_ = std::make_shared<SocketCore>();
      try {
        socket_->establishConnection(ipaddr_, port_);
        e_->addSocketForWriteCheck(socket_, this);
        timeoutCheck_ = global::wallclock();
      }
      catch (RecoverableException& e) {
        A2_LOG_INFO_EX(
            fmt("CUID#%" PRId64 " - Backup connection failed", getCuid()), e);
        socket_.reset();
        retval = true;
      }
    }
  }
  else if (timeoutCheck_.difference(global::wallclock()) >= timeout_) {
    A2_LOG_INFO(
        fmt("CUID#%" PRId64 " - Backup connection command timeout", getCuid()));
    retval = true;
  }
  if (!retval) {
    e_->addCommand(std::unique_ptr<Command>(this));
  }
  return retval;
}

} // namespace aria2
