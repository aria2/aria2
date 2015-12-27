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
#ifndef D_DHT_NODE_H
#define D_DHT_NODE_H

#include "common.h"

#include <string>

#include "DHTConstants.h"
#include "TimerA2.h"

namespace aria2 {

class DHTNode {
private:
  unsigned char id_[DHT_ID_LENGTH];

  std::string ipaddr_;

  uint16_t port_;

  std::chrono::milliseconds rtt_;

  int condition_;

  Timer lastContact_;

public:
  DHTNode();

  /**
   * id must be 20 bytes length
   */
  DHTNode(const unsigned char* id);

  ~DHTNode();

  void generateID();

  const unsigned char* getID() const { return id_; }

  void updateRTT(std::chrono::milliseconds t) { rtt_ = std::move(t); }

  const std::string& getIPAddress() const { return ipaddr_; }

  void setIPAddress(const std::string& ipaddr);

  void setID(const unsigned char* id);

  uint16_t getPort() const { return port_; }

  void setPort(uint16_t port) { port_ = port; }

  bool isGood() const;

  bool isBad() const;

  bool isQuestionable() const;

  void updateLastContact();

  void markGood();

  void markBad();

  void timeout();

  bool operator==(const DHTNode& node) const;

  bool operator!=(const DHTNode& node) const;

  bool operator<(const DHTNode& node) const;

  std::string toString() const;
};

} // namespace aria2

#endif // D_DHT_NODE_H
