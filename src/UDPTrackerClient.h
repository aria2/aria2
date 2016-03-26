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
#ifndef D_UDP_TRACKER_CLIENT_H
#define D_UDP_TRACKER_CLIENT_H

#include "common.h"

#include <string>
#include <deque>
#include <map>
#include <memory>

#include "TimerA2.h"

namespace aria2 {

#define UDPT_INITIAL_CONNECTION_ID 0x41727101980LL

struct UDPTrackerRequest;

enum UDPTrackerConnectionState { UDPT_CST_CONNECTING, UDPT_CST_CONNECTED };

struct UDPTrackerConnection {
  int state;
  int64_t connectionId;
  Timer lastUpdated;
  UDPTrackerConnection()
      : state(UDPT_CST_CONNECTING),
        connectionId(UDPT_INITIAL_CONNECTION_ID),
        lastUpdated(Timer::zero())
  {
  }
  UDPTrackerConnection(int state, int64_t connectionId,
                       const Timer& lastUpdated)
      : state(state), connectionId(connectionId), lastUpdated(lastUpdated)
  {
  }
};

class UDPTrackerClient {
public:
  UDPTrackerClient();
  ~UDPTrackerClient();

  int receiveReply(std::shared_ptr<UDPTrackerRequest>& req,
                   const unsigned char* data, size_t length,
                   const std::string& remoteAddr, uint16_t remotePort,
                   const Timer& now);

  // Creates data frame for the next pending request. This function
  // always processes first entry of pendingRequests_.  If the data is
  // sent successfully, call requestSent(). Otherwise call
  // requestFail().
  ssize_t createRequest(unsigned char* data, size_t length,
                        std::string& remoteAddr, uint16_t& remotePort,
                        const Timer& now);

  // Tells this object that first entry of pendingRequests_ is
  // successfully sent.
  void requestSent(const Timer& now);
  // Tells this object that first entry of pendingRequests_ is not
  // successfully sent. The |error| should indicate error situation.
  void requestFail(int error);

  void addRequest(const std::shared_ptr<UDPTrackerRequest>& req);

  // Handles timeout for inflight requests.
  void handleTimeout(const Timer& now);

  const std::deque<std::shared_ptr<UDPTrackerRequest>>&
  getPendingRequests() const
  {
    return pendingRequests_;
  }
  const std::deque<std::shared_ptr<UDPTrackerRequest>>&
  getConnectRequests() const
  {
    return connectRequests_;
  }
  const std::deque<std::shared_ptr<UDPTrackerRequest>>&
  getInflightRequests() const
  {
    return inflightRequests_;
  }

  bool noRequest() const
  {
    return pendingRequests_.empty() && connectRequests_.empty() &&
           getInflightRequests().empty();
  }

  // Makes all contained requests fail.
  void failAll();

  int getNumWatchers() const { return numWatchers_; }

  void increaseWatchers();
  void decreaseWatchers();

  // Actually private function, but made public, to be used by unnamed
  // function.
  void failConnect(const std::string& remoteAddr, uint16_t remotePort,
                   int error);

private:
  std::shared_ptr<UDPTrackerRequest>
  findInflightRequest(const std::string& remoteAddr, uint16_t remotePort,
                      uint32_t transactionId, bool remove);

  UDPTrackerConnection* getConnectionId(const std::string& remoteAddr,
                                        uint16_t remotePort, const Timer& now);

  std::map<std::pair<std::string, uint16_t>, UDPTrackerConnection>
      connectionIdCache_;
  std::deque<std::shared_ptr<UDPTrackerRequest>> inflightRequests_;
  std::deque<std::shared_ptr<UDPTrackerRequest>> pendingRequests_;
  std::deque<std::shared_ptr<UDPTrackerRequest>> connectRequests_;
  int numWatchers_;
};

ssize_t createUDPTrackerConnect(unsigned char* data, size_t length,
                                std::string& remoteAddr, uint16_t& remotePort,
                                const std::shared_ptr<UDPTrackerRequest>& req);

ssize_t createUDPTrackerAnnounce(unsigned char* data, size_t length,
                                 std::string& remoteAddr, uint16_t& remotePort,
                                 const std::shared_ptr<UDPTrackerRequest>& req);

const char* getUDPTrackerActionStr(int action);

const char* getUDPTrackerEventStr(int event);

} // namespace aria2

#endif // D_UDP_TRACKER_CLIENT_H
