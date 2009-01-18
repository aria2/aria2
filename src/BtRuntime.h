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
  // Maximum number of peers to hold connections at the same time.
  // 0 means unlimited.
  unsigned int _maxPeers;
  // Minimum number of peers. This value is used for getting more peers from
  // tracker. 0 means always the number of peers is under minimum.
  unsigned int _minPeers;

  static const unsigned int DEFAULT_MIN_PEERS = 40;

public:
  BtRuntime():
    uploadLengthAtStartup(0),
    port(0),
    halt(false),
    connections(0),
    _ready(false),
    _maxPeers(DEFAULT_MAX_PEERS),
    _minPeers(DEFAULT_MIN_PEERS)
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

  bool lessThanMaxPeers() const
  {
    return _maxPeers == 0 || connections < _maxPeers;
  }

  bool lessThanMinPeers() const
  {
    return _minPeers == 0 || connections < _minPeers;
  }

  bool lessThanEqMinPeers() const
  {
    return _minPeers == 0 || connections <= _minPeers;
  }

  bool ready() { return _ready; }

  void setReady(bool go) { _ready = go; }

  void setMaxPeers(unsigned int maxPeers)
  {
    _maxPeers = maxPeers;
    _minPeers = maxPeers*0.8;
    if(_minPeers == 0 && maxPeers != 0) {
      _minPeers = maxPeers;
    }
  }

  unsigned int getMaxPeers() const
  {
    return _maxPeers;
  }

  static const unsigned int DEFAULT_MAX_PEERS = 55;
};

typedef SharedHandle<BtRuntime> BtRuntimeHandle;

} // namespace aria2

#endif // _D_BT_RUNTIME_H_
