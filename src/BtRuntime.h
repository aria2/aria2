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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#ifndef _D_BT_RUNTIME_H_
#define _D_BT_RUNTIME_H_

#include "common.h"
#include "BtConstants.h"

namespace aria2 {

class BtRuntime {
private:
  uint64_t uploadLengthAtStartup;
  uint16_t port;
  bool halt;
  unsigned int connections;
  bool _ready;

  static const unsigned int MIN_PEERS = 40;

public:
  BtRuntime():
    uploadLengthAtStartup(0),
    port(0),
    halt(false),
    connections(0),
    _ready(false)
  {}

  ~BtRuntime() {}

  uint64_t getUploadLengthAtStartup() const {
    return uploadLengthAtStartup;
  }

  void setUploadLengthAtStartup(uint64_t length) {
    this->uploadLengthAtStartup = length;
  }

  void setListenPort(uint16_t port) {
    this->port = port;
  }

  uint16_t getListenPort() const { return port; }

  bool isHalt() const { return halt; }

  void setHalt(bool halt) {
    this->halt = halt;
  }

  unsigned int getConnections() const { return connections; }

  void increaseConnections() { connections++; }

  void decreaseConnections() { connections--; }

  bool lessThanMaxPeers() const { return connections < MAX_PEERS; }

  bool lessThanMinPeers() const { return connections < MIN_PEERS; }

  bool lessThanEqMinPeers() const { return connections <= MIN_PEERS; }

  bool ready() { return _ready; }

  void setReady(bool go) { _ready = go; }

  static const unsigned int MAX_PEERS = 55;
};

typedef SharedHandle<BtRuntime> BtRuntimeHandle;

} // namespace aria2

#endif // _D_BT_RUNTIME_H_
