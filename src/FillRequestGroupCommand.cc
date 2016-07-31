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
#include "FillRequestGroupCommand.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "RecoverableException.h"
#include "message.h"
#include "Logger.h"
#include "LogFactory.h"
#include "DownloadContext.h"
#include "fmt.h"
#include "wallclock.h"

namespace aria2 {

FillRequestGroupCommand::FillRequestGroupCommand(cuid_t cuid, DownloadEngine* e)
    : Command(cuid), e_(e)
{
  setStatusRealtime();
}

FillRequestGroupCommand::~FillRequestGroupCommand() = default;

bool FillRequestGroupCommand::execute()
{
  if (e_->isHaltRequested()) {
    return true;
  }
  auto& rgman = e_->getRequestGroupMan();
  if (rgman->queueCheckRequested()) {
    while (rgman->queueCheckRequested()) {
      try {
        // During adding RequestGroup,
        // RequestGroupMan::requestQueueCheck() might be called, so
        // first clear it here.
        rgman->clearQueueCheck();
        rgman->fillRequestGroupFromReserver(e_);
      }
      catch (RecoverableException& ex) {
        A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, ex);
        // Re-request queue check to fulfill the requests of all
        // downloads, some might come after this exception.
        rgman->requestQueueCheck();
      }
    }
    if (rgman->downloadFinished()) {
      return true;
    }
  }
  e_->addRoutineCommand(std::unique_ptr<Command>(this));

  // let's make sure we come back here every second or so
  // if we use the optimize-concurrent-download option
  if (rgman->getOptimizeConcurrentDownloads()) {
    const auto& now = global::wallclock();
    if (std::chrono::duration_cast<std::chrono::seconds>(
            lastExecTime.difference(now)) >= 1_s) {
      lastExecTime = now;
      rgman->requestQueueCheck();
    }
  }

  return false;
}

} // namespace aria2
