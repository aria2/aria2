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
#include "DHTMessageTrackerEntry.h"
#include "DHTNode.h"
#include "DHTMessage.h"
#include "DHTMessageCallback.h"
#include "DHTConstants.h"
#include "util.h"
#include "wallclock.h"

namespace aria2 {

DHTMessageTrackerEntry::DHTMessageTrackerEntry(
    std::shared_ptr<DHTNode> targetNode, std::string transactionID,
    std::string messageType, std::chrono::seconds timeout,
    std::unique_ptr<DHTMessageCallback> callback)
    : targetNode_{std::move(targetNode)},
      transactionID_{std::move(transactionID)},
      messageType_{std::move(messageType)},
      callback_{std::move(callback)},
      dispatchedTime_{global::wallclock()},
      timeout_{std::move(timeout)}
{
}

bool DHTMessageTrackerEntry::isTimeout() const
{
  return dispatchedTime_.difference(global::wallclock()) >= timeout_;
}

void DHTMessageTrackerEntry::extendTimeout() {}

bool DHTMessageTrackerEntry::match(const std::string& transactionID,
                                   const std::string& ipaddr,
                                   uint16_t port) const
{
  if (transactionID_ != transactionID || targetNode_->getPort() != port) {
    return false;
  }
  if (targetNode_->getIPAddress() == ipaddr) {
    return true;
  }
  if (util::endsWith(targetNode_->getIPAddress(), ipaddr)) {
    return targetNode_->getIPAddress() == "::ffff:" + ipaddr;
  }
  else if (util::endsWith(ipaddr, targetNode_->getIPAddress())) {
    return ipaddr == "::ffff:" + targetNode_->getIPAddress();
  }
  return false;
}

Timer::Clock::duration DHTMessageTrackerEntry::getElapsed() const
{
  return dispatchedTime_.difference(global::wallclock());
}

const std::shared_ptr<DHTNode>& DHTMessageTrackerEntry::getTargetNode() const
{
  return targetNode_;
}

const std::string& DHTMessageTrackerEntry::getMessageType() const
{
  return messageType_;
}

const std::unique_ptr<DHTMessageCallback>&
DHTMessageTrackerEntry::getCallback() const
{
  return callback_;
}

std::unique_ptr<DHTMessageCallback> DHTMessageTrackerEntry::popCallback()
{
  return std::move(callback_);
}

} // namespace aria2
