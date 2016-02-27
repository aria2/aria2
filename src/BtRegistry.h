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
#ifndef D_BT_REGISTRY_H
#define D_BT_REGISTRY_H

#include "common.h"

#include <map>
#include <memory>

#include "RequestGroup.h"

namespace aria2 {

class PeerStorage;
class PieceStorage;
class BtAnnounce;
class BtRuntime;
class BtProgressInfoFile;
class DownloadContext;
class LpdMessageReceiver;
class UDPTrackerClient;

struct BtObject {
  std::shared_ptr<DownloadContext> downloadContext;
  std::shared_ptr<PieceStorage> pieceStorage;
  std::shared_ptr<PeerStorage> peerStorage;
  std::shared_ptr<BtAnnounce> btAnnounce;
  std::shared_ptr<BtRuntime> btRuntime;
  std::shared_ptr<BtProgressInfoFile> btProgressInfoFile;

  BtObject(const std::shared_ptr<DownloadContext>& downloadContext,
           const std::shared_ptr<PieceStorage>& pieceStorage,
           const std::shared_ptr<PeerStorage>& peerStorage,
           const std::shared_ptr<BtAnnounce>& btAnnounce,
           const std::shared_ptr<BtRuntime>& btRuntime,
           const std::shared_ptr<BtProgressInfoFile>& btProgressInfoFile);

  BtObject();
};

class BtRegistry {
private:
  std::map<a2_gid_t, std::unique_ptr<BtObject>> pool_;
  uint16_t tcpPort_;
  // This is UDP port for DHT and UDP tracker. But currently UDP
  // tracker is not supported in IPv6.
  uint16_t udpPort_;
  std::shared_ptr<LpdMessageReceiver> lpdMessageReceiver_;
  std::shared_ptr<UDPTrackerClient> udpTrackerClient_;

public:
  BtRegistry();

  const std::shared_ptr<DownloadContext>&
  getDownloadContext(a2_gid_t gid) const;

  const std::shared_ptr<DownloadContext>&
  getDownloadContext(const std::string& infoHash) const;

  void put(a2_gid_t gid, std::unique_ptr<BtObject> obj);

  BtObject* get(a2_gid_t gid) const;

  template <typename OutputIterator>
  OutputIterator getAllDownloadContext(OutputIterator dest)
  {
    for (auto& kv : pool_) {
      *dest++ = kv.second->downloadContext;
    }
    return dest;
  }

  void removeAll();

  bool remove(a2_gid_t gid);

  void setTcpPort(uint16_t port) { tcpPort_ = port; }
  uint16_t getTcpPort() const { return tcpPort_; }

  void setUdpPort(uint16_t port) { udpPort_ = port; }
  uint16_t getUdpPort() const { return udpPort_; }

  void
  setLpdMessageReceiver(const std::shared_ptr<LpdMessageReceiver>& receiver);
  const std::shared_ptr<LpdMessageReceiver>& getLpdMessageReceiver() const
  {
    return lpdMessageReceiver_;
  }

  void setUDPTrackerClient(const std::shared_ptr<UDPTrackerClient>& tracker);
  const std::shared_ptr<UDPTrackerClient>& getUDPTrackerClient() const
  {
    return udpTrackerClient_;
  }
};

} // namespace aria2

#endif // D_BT_REGISTRY_H
