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
#ifndef _D_BT_RUNTIME_H_
#define _D_BT_RUNTIME_H_

#include "common.h"
#include "BtConstants.h"

namespace aria2 {

class BtRuntime {
private:
  uint64_t uploadLengthAtStartup_;
  uint16_t port_;
  bool halt_;
  unsigned int connections_;
  bool ready_;
  // Maximum number of peers to hold connections at the same time.
  // 0 means unlimited.
  unsigned int maxPeers_;
  // Minimum number of peers. This value is used for getting more peers from
  // tracker. 0 means always the number of peers is under minimum.
  unsigned int minPeers_;

  static const unsigned int DEFAULT_MIN_PEERS = 40;

public:
  BtRuntime():
    uploadLengthAtStartup_(0),
    port_(0),
    halt_(false),
    connections_(0),
    ready_(false),
    maxPeers_(DEFAULT_MAX_PEERS),
    minPeers_(DEFAULT_MIN_PEERS)
  {}

  ~BtRuntime() {}

  uint64_t getUploadLengthAtStartup() const {
    return uploadLengthAtStartup_;
  }

  void setUploadLengthAtStartup(uint64_t length) {
    uploadLengthAtStartup_ = length;
  }

  void setListenPort(uint16_t port) {
    port_ = port;
  }

  uint16_t getListenPort() const { return port_; }

  bool isHalt() const { return halt_; }

  void setHalt(bool halt) {
    halt_ = halt;
  }

  unsigned int getConnections() const { return connections_; }

  void increaseConnections() { ++connections_; }

  void decreaseConnections() { --connections_; }

  bool lessThanMaxPeers() const
  {
    return maxPeers_ == 0 || connections_ < maxPeers_;
  }

  bool lessThanMinPeers() const
  {
    return minPeers_ == 0 || connections_ < minPeers_;
  }

  bool lessThanEqMinPeers() const
  {
    return minPeers_ == 0 || connections_ <= minPeers_;
  }

  bool ready() { return ready_; }

  void setReady(bool go) { ready_ = go; }

  void setMaxPeers(unsigned int maxPeers)
  {
    maxPeers_ = maxPeers;
    minPeers_ = static_cast<unsigned int>(maxPeers*0.8);
    if(minPeers_ == 0 && maxPeers != 0) {
      minPeers_ = maxPeers;
    }
  }

  unsigned int getMaxPeers() const
  {
    return maxPeers_;
  }

  static const unsigned int DEFAULT_MAX_PEERS = 55;
};

typedef SharedHandle<BtRuntime> BtRuntimeHandle;

} // namespace aria2

#endif // _D_BT_RUNTIME_H_
