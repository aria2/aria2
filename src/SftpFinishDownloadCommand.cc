/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2015 Tatsuhiro Tsujikawa
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
#include "SftpFinishDownloadCommand.h"

#include "Request.h"
#include "DownloadEngine.h"
#include "prefs.h"
#include "Option.h"
#include "message.h"
#include "fmt.h"
#include "DlAbortEx.h"
#include "SocketCore.h"
#include "RequestGroup.h"
#include "Logger.h"
#include "LogFactory.h"
#include "wallclock.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"

namespace aria2 {

SftpFinishDownloadCommand::SftpFinishDownloadCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    DownloadEngine* e, const std::shared_ptr<SocketCore>& socket)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e, socket)
{
  disableReadCheckSocket();
  setWriteCheckSocket(getSocket());
}

SftpFinishDownloadCommand::~SftpFinishDownloadCommand() {}

// overrides AbstractCommand::execute().
// AbstractCommand::_segments is empty.
bool SftpFinishDownloadCommand::execute()
{
  if (getRequestGroup()->isHaltRequested()) {
    return true;
  }
  try {
    if (readEventEnabled() || writeEventEnabled() || hupEventEnabled()) {
      getCheckPoint() = global::wallclock();

      if (!getSocket()->sshSFTPClose()) {
        setWriteCheckSocketIf(getSocket(), getSocket()->wantWrite());
        setReadCheckSocketIf(getSocket(), getSocket()->wantRead());
        addCommandSelf();
        return false;
      }

      auto authConfig =
          getDownloadEngine()->getAuthConfigFactory()->createAuthConfig(
              getRequest(), getRequestGroup()->getOption().get());

      getDownloadEngine()->poolSocket(getRequest(), authConfig->getUser(),
                                      createProxyRequest(), getSocket(), "");
    }
    else if (getCheckPoint().difference(global::wallclock()) >= getTimeout()) {
      A2_LOG_INFO(fmt("CUID#%" PRId64
                      " - Timeout before receiving transfer complete.",
                      getCuid()));
    }
    else {
      addCommandSelf();
      return false;
    }
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX(fmt("CUID#%" PRId64
                       " - Exception was thrown, but download was"
                       " finished, so we can ignore the exception.",
                       getCuid()),
                   e);
  }
  if (getRequestGroup()->downloadFinished()) {
    return true;
  }
  else {
    return prepareForRetry(0);
  }
}

// This function never be called.
bool SftpFinishDownloadCommand::executeInternal() { return true; }

} // namespace aria2
