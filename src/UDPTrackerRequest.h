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
#ifndef D_UDP_TRACKER_REQUEST_H
#define D_UDP_TRACKER_REQUEST_H

#include "common.h"

#include <string>
#include <vector>
#include <memory>

#include "TimerA2.h"

namespace aria2 {

enum UDPTrackerAction {
  UDPT_ACT_CONNECT = 0,
  UDPT_ACT_ANNOUNCE = 1,
  UDPT_ACT_SCRAPE = 2,
  UDPT_ACT_ERROR = 3
};

enum UDPTrackerError {
  UDPT_ERR_SUCCESS,
  UDPT_ERR_TRACKER,
  UDPT_ERR_TIMEOUT,
  UDPT_ERR_NETWORK,
  UDPT_ERR_SHUTDOWN
};

enum UDPTrackerState { UDPT_STA_PENDING, UDPT_STA_COMPLETE };

enum UDPTrackerEvent {
  UDPT_EVT_NONE = 0,
  UDPT_EVT_COMPLETED = 1,
  UDPT_EVT_STARTED = 2,
  UDPT_EVT_STOPPED = 3
};

struct UDPTrackerReply {
  int32_t action;
  uint32_t transactionId;
  int32_t interval;
  int32_t leechers;
  int32_t seeders;
  std::vector<std::pair<std::string, uint16_t>> peers;
  UDPTrackerReply();
};

struct UDPTrackerRequest {
  std::string remoteAddr;
  uint16_t remotePort;
  uint64_t connectionId;
  int32_t action;
  uint32_t transactionId;
  std::string infohash;
  std::string peerId;
  int64_t downloaded;
  int64_t left;
  int64_t uploaded;
  int32_t event;
  uint32_t ip;
  uint32_t key;
  int32_t numWant;
  uint16_t port;
  uint16_t extensions;
  int state;
  int error;
  Timer dispatched;
  int failCount;
  std::shared_ptr<UDPTrackerReply> reply;
  void* user_data;
  UDPTrackerRequest();
};

} // namespace aria2

#endif // D_UDP_TRACKER_REQUEST_H
