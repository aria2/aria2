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

#ifndef D_LPD_MESSAGE_RECEIVER_H
#define D_LPD_MESSAGE_RECEIVER_H

#include "common.h"

#include <string>
#include <memory>

namespace aria2 {

class SocketCore;
struct LpdMessage;

class LpdMessageReceiver {
private:
  std::shared_ptr<SocketCore> socket_;
  std::string multicastAddress_;
  uint16_t multicastPort_;
  std::string localAddress_;

public:
  // Currently only IPv4 multicastAddresses are supported.
  LpdMessageReceiver(const std::string& multicastAddress,
                     uint16_t multicastPort);

  ~LpdMessageReceiver();

  // No throw.
  bool init(const std::string& localAddr);

  // Receives LPD message and returns LpdMessage which contains
  // sender(peer) and infohash. If no data is available on socket,
  // returns std::shared_ptr<LpdMessage>().  If received data is bad,
  // they are just skipped.
  std::unique_ptr<LpdMessage> receiveMessage();

  const std::shared_ptr<SocketCore>& getSocket() const { return socket_; }

  const std::string& getLocalAddress() const { return localAddress_; }
};

} // namespace aria2

#endif // D_LPD_MESSAGE_RECEIVER_H
