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
#ifndef D_DHT_REGISTRY_H
#define D_DHT_REGISTRY_H

#include "common.h"

#include <memory>

namespace aria2 {

class DHTNode;
class DHTRoutingTable;
class DHTTaskQueue;
class DHTTaskFactory;
class DHTPeerAnnounceStorage;
class DHTTokenTracker;
class DHTMessageDispatcher;
class DHTMessageReceiver;
class DHTMessageFactory;

class DHTRegistry {
private:
  struct Data {
    bool initialized;

    std::shared_ptr<DHTNode> localNode;

    std::unique_ptr<DHTRoutingTable> routingTable;

    std::unique_ptr<DHTTaskQueue> taskQueue;

    std::unique_ptr<DHTTaskFactory> taskFactory;

    std::unique_ptr<DHTPeerAnnounceStorage> peerAnnounceStorage;

    std::unique_ptr<DHTTokenTracker> tokenTracker;

    std::unique_ptr<DHTMessageDispatcher> messageDispatcher;

    std::unique_ptr<DHTMessageReceiver> messageReceiver;

    std::unique_ptr<DHTMessageFactory> messageFactory;

    Data() : initialized(false) {}
  };

  static Data data_;
  static Data data6_;

  static void clear(Data& data);

  DHTRegistry();

public:
  static const Data& getData() { return data_; }

  static Data& getMutableData() { return data_; }

  static void clearData();

  static bool isInitialized() { return data_.initialized; }

  static void setInitialized(bool f) { data_.initialized = f; }

  static const Data& getData6() { return data6_; }

  static Data& getMutableData6() { return data6_; }

  static void clearData6();

  static bool isInitialized6() { return data6_.initialized; }

  static void setInitialized6(bool f) { data6_.initialized = f; }
};

} // namespace aria2

#endif // D_DHT_REGISTRY_H
