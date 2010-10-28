/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "LpdDispatchMessageCommand.h"
#include "LpdMessageDispatcher.h"
#include "DownloadEngine.h"
#include "BtRuntime.h"
#include "Logger.h"
#include "RecoverableException.h"
#include "SocketCore.h"
#include "util.h"
#include "RequestGroupMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "ServerStatMan.h"
#include "FileEntry.h"

namespace aria2 {

LpdDispatchMessageCommand::LpdDispatchMessageCommand
(cuid_t cuid,
 const SharedHandle<LpdMessageDispatcher>& dispatcher,
 DownloadEngine* e):
  Command(cuid),
  dispatcher_(dispatcher),
  e_(e),
  tryCount_(0) {}

bool LpdDispatchMessageCommand::execute()
{
  if(btRuntime_->isHalt()) {
    return true;
  }
  if(dispatcher_->isAnnounceReady()) {
    try {
      getLogger()->info("Dispatching LPD message for infohash=%s",
                   util::toHex(dispatcher_->getInfoHash()).c_str());
      if(dispatcher_->sendMessage()) {
        getLogger()->info("Sending LPD message is complete.");
        dispatcher_->resetAnnounceTimer();
        tryCount_ = 0;
      } else {
        ++tryCount_;
        if(tryCount_ >= 5) {
          getLogger()->info("Sending LPD message %u times but all failed.",
                            tryCount_);
          dispatcher_->resetAnnounceTimer();
          tryCount_ = 0;
        } else {
          getLogger()->info("Could not send LPD message, retry shortly.");
        }
      }
    } catch(RecoverableException& e) {
      getLogger()->info("Failed to send LPD message.", e);
      dispatcher_->resetAnnounceTimer();
      tryCount_ = 0;
    }
  }
  e_->addCommand(this);
  return false;
}

} // namespace aria2
