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

#include "SharedHandle.h"
#include "DHTConstants.h"
#include "TimerA2.h"

namespace aria2 {

class DHTNode;
class DHTMessage;
class DHTMessageCallback;

class DHTMessageTrackerEntry {
private:
  SharedHandle<DHTNode> targetNode_;

  std::string transactionID_;

  std::string messageType_;
  
  SharedHandle<DHTMessageCallback> callback_;

  Timer dispatchedTime_;

  time_t timeout_;
public:
  DHTMessageTrackerEntry(const SharedHandle<DHTMessage>& sentMessage,
                         time_t timeout,
                         const SharedHandle<DHTMessageCallback>& callback =
                         SharedHandle<DHTMessageCallback>());

  ~DHTMessageTrackerEntry();

  bool isTimeout() const;

  void extendTimeout();

  bool match(const std::string& transactionID, const std::string& ipaddr, uint16_t port) const;

  const SharedHandle<DHTNode>& getTargetNode() const
  {
    return targetNode_;
  }

  const std::string& getMessageType() const
  {
    return messageType_;
  }

  const SharedHandle<DHTMessageCallback>& getCallback() const
  {
    return callback_;
  }  

  int64_t getElapsedMillis() const;
};

} // namespace aria2

#endif // D_DHT_MESSAGE_TRACKER_ENTRY_H
