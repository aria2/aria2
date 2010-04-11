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
#ifndef _D_LPD_MESSAGE_DISPATCHER_H_
#define _D_LPD_MESSAGE_DISPATCHER_H_

#include "common.h"

#include <string>

#include "SharedHandle.h"
#include "TimerA2.h"

namespace aria2 {

class SocketCore;
class Logger;

class LpdMessageDispatcher {
private:
  SharedHandle<SocketCore> _socket;
  std::string _infoHash;
  uint16_t _port;
  std::string _multicastAddress;
  uint16_t _multicastPort;
  Timer _timer;
  time_t _interval;
  std::string _request;
  Logger* _logger;
public:
  LpdMessageDispatcher
  (const std::string& infoHash, uint16_t port,
   const std::string& multicastAddr, uint16_t multicastPort,
   time_t interval = 5*60);

  // No throw
  bool init(const std::string& localAddr, unsigned char ttl,unsigned char loop);

  // Returns true if _timer reached announce interval, which is by
  // default 5mins.
  bool isAnnounceReady() const;

  // Sends LPD message. If message is sent returns true. Otherwise
  // returns false.
  bool sendMessage();

  // Reset _timer to the current time.
  void resetAnnounceTimer();

  const std::string& getInfoHash() const
  {
    return _infoHash;
  }

  uint16_t getPort() const
  {
    return _port;
  }
};

namespace bittorrent {

std::string createLpdRequest
(const std::string& multicastAddress, uint16_t multicastPort,
 const std::string& infoHash, uint16_t port);

} // namespace bittorrent

} // namespace aria2

#endif // _D_LPD_MESSAGE_DISPATCHER_H_
