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
#include "LogFactory.h"
#include "RecoverableException.h"
#include "SocketCore.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

LpdDispatchMessageCommand::LpdDispatchMessageCommand(
    cuid_t cuid, const std::shared_ptr<LpdMessageDispatcher>& dispatcher,
    DownloadEngine* e)
    : Command(cuid), dispatcher_(dispatcher), e_(e), tryCount_(0)
{
}

LpdDispatchMessageCommand::~LpdDispatchMessageCommand() = default;

bool LpdDispatchMessageCommand::execute()
{
  if (btRuntime_->isHalt()) {
    return true;
  }
  if (dispatcher_->isAnnounceReady()) {
    try {
      A2_LOG_INFO(fmt("Dispatching LPD message for infohash=%s",
                      util::toHex(dispatcher_->getInfoHash()).c_str()));
      if (dispatcher_->sendMessage()) {
        A2_LOG_INFO("Sending LPD message is complete.");
        dispatcher_->resetAnnounceTimer();
        tryCount_ = 0;
      }
      else {
        ++tryCount_;
        if (tryCount_ >= 5) {
          A2_LOG_INFO(
              fmt("Sending LPD message %u times but all failed.", tryCount_));
          dispatcher_->resetAnnounceTimer();
          tryCount_ = 0;
        }
        else {
          A2_LOG_INFO("Could not send LPD message, retry shortly.");
        }
      }
    }
    catch (RecoverableException& e) {
      A2_LOG_INFO_EX("Failed to send LPD message.", e);
      dispatcher_->resetAnnounceTimer();
      tryCount_ = 0;
    }
  }
  e_->addCommand(std::unique_ptr<Command>(this));
  return false;
}

} // namespace aria2
