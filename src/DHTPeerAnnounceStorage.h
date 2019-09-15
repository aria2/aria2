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
#ifndef D_DHT_PEER_ANNOUNCE_STORAGE_H
#define D_DHT_PEER_ANNOUNCE_STORAGE_H

#include "common.h"

#include <set>
#include <vector>
#include <string>
#include <memory>

namespace aria2 {

class Peer;
class DHTPeerAnnounceEntry;
class DHTTaskQueue;
class DHTTaskFactory;

class DHTPeerAnnounceStorage {
private:
  class InfoHashLess {
  public:
    bool operator()(const std::shared_ptr<DHTPeerAnnounceEntry>& lhs,
                    const std::shared_ptr<DHTPeerAnnounceEntry>& rhs) const;
  };
  typedef std::set<std::shared_ptr<DHTPeerAnnounceEntry>, InfoHashLess>
      DHTPeerAnnounceEntrySet;
  DHTPeerAnnounceEntrySet entries_;

  std::shared_ptr<DHTPeerAnnounceEntry>
  getPeerAnnounceEntry(const unsigned char* infoHash);

  DHTTaskQueue* taskQueue_;

  DHTTaskFactory* taskFactory_;

public:
  DHTPeerAnnounceStorage();

  void addPeerAnnounce(const unsigned char* infoHash, const std::string& ipaddr,
                       uint16_t port);

  bool contains(const unsigned char* infoHash) const;

  void getPeers(std::vector<std::shared_ptr<Peer>>& peers,
                const unsigned char* infoHash);

  // drop peer announce entry which is not updated in the past
  // DHT_PEER_ANNOUNCE_PURGE_INTERVAL seconds.
  void handleTimeout();

  // announce peer in every DHT_PEER_ANNOUNCE_PURGE_INTERVAL.
  // The torrents which are announced in the past
  // DHT_PEER_ANNOUNCE_PURGE_INTERVAL
  // are excluded from announce.
  void announcePeer();

  void setTaskQueue(DHTTaskQueue* taskQueue);

  void setTaskFactory(DHTTaskFactory* taskFactory);
};

} // namespace aria2

#endif // D_DHT_PEER_ANNOUNCE_STORAGE_H
