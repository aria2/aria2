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
#ifndef D_BT_RUNTIME_H
#define D_BT_RUNTIME_H

#include "common.h"

namespace aria2 {

class BtRuntime {
private:
  int64_t uploadLengthAtStartup_;
  bool halt_;
  int connections_;
  bool ready_;
  // Maximum number of peers to hold connections at the same time.
  // 0 means unlimited.
  int maxPeers_;
  // Minimum number of peers. This value is used for getting more peers from
  // tracker. 0 means always the number of peers is under minimum.
  int minPeers_;

public:
  BtRuntime();

  ~BtRuntime();

  int64_t getUploadLengthAtStartup() const { return uploadLengthAtStartup_; }

  void setUploadLengthAtStartup(int64_t length)
  {
    uploadLengthAtStartup_ = length;
  }

  bool isHalt() const { return halt_; }

  void setHalt(bool halt) { halt_ = halt; }

  int getConnections() const { return connections_; }

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

  void setMaxPeers(int maxPeers);

  int getMaxPeers() const { return maxPeers_; }

  static const int DEFAULT_MAX_PEERS = 55;
  static const int DEFAULT_MIN_PEERS = 40;
};

} // namespace aria2

#endif // D_BT_RUNTIME_H
