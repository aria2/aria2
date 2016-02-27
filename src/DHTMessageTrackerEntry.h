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
#ifndef D_DHT_MESSAGE_TRACKER_ENTRY_H
#define D_DHT_MESSAGE_TRACKER_ENTRY_H

#include "common.h"

#include <string>
#include <memory>

#include "DHTConstants.h"
#include "TimerA2.h"

namespace aria2 {

class DHTNode;
class DHTMessage;
class DHTMessageCallback;

class DHTMessageTrackerEntry {
private:
  std::shared_ptr<DHTNode> targetNode_;

  std::string transactionID_;

  std::string messageType_;

  std::unique_ptr<DHTMessageCallback> callback_;

  Timer dispatchedTime_;

  std::chrono::seconds timeout_;

public:
  DHTMessageTrackerEntry(std::shared_ptr<DHTNode> targetNode,
                         std::string transactionID, std::string messageType,
                         std::chrono::seconds timeout,
                         std::unique_ptr<DHTMessageCallback> callback =
                             std::unique_ptr<DHTMessageCallback>{});

  bool isTimeout() const;

  void extendTimeout();

  bool match(const std::string& transactionID, const std::string& ipaddr,
             uint16_t port) const;

  const std::shared_ptr<DHTNode>& getTargetNode() const;
  const std::string& getMessageType() const;
  const std::unique_ptr<DHTMessageCallback>& getCallback() const;
  std::unique_ptr<DHTMessageCallback> popCallback();
  Timer::Clock::duration getElapsed() const;
};

} // namespace aria2

#endif // D_DHT_MESSAGE_TRACKER_ENTRY_H
