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

#ifndef D_LPD_MESSAGE_DISPATCHER_H
#define D_LPD_MESSAGE_DISPATCHER_H

#include "common.h"

#include <string>
#include <memory>

#include "TimerA2.h"

namespace aria2 {

class SocketCore;

class LpdMessageDispatcher {
private:
  std::shared_ptr<SocketCore> socket_;
  std::string infoHash_;
  uint16_t port_;
  std::string multicastAddress_;
  uint16_t multicastPort_;
  Timer timer_;
  std::chrono::seconds interval_;
  std::string request_;

public:
  LpdMessageDispatcher(const std::string& infoHash, uint16_t port,
                       const std::string& multicastAddr, uint16_t multicastPort,
                       std::chrono::seconds interval = 5_min);

  ~LpdMessageDispatcher();

  // No throw
  bool init(const std::string& localAddr, unsigned char ttl,
            unsigned char loop);

  // Returns true if timer_ reached announce interval, which is by
  // default 5mins.
  bool isAnnounceReady() const;

  // Sends LPD message. If message is sent returns true. Otherwise
  // returns false.
  bool sendMessage();

  // Reset timer_ to the current time.
  void resetAnnounceTimer();

  const std::string& getInfoHash() const { return infoHash_; }

  uint16_t getPort() const { return port_; }
};

namespace bittorrent {

std::string createLpdRequest(const std::string& multicastAddress,
                             uint16_t multicastPort,
                             const std::string& infoHash, uint16_t port);

} // namespace bittorrent

} // namespace aria2

#endif // D_LPD_MESSAGE_DISPATCHER_H
