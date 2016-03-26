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
#include "BtRegistry.h"
#include "DlAbortEx.h"
#include "DownloadContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtAnnounce.h"
#include "BtRuntime.h"
#include "BtProgressInfoFile.h"
#include "bittorrent_helper.h"
#include "LpdMessageReceiver.h"
#include "UDPTrackerClient.h"
#include "NullHandle.h"

namespace aria2 {

BtRegistry::BtRegistry() : tcpPort_{0}, udpPort_{0} {}

const std::shared_ptr<DownloadContext>&
BtRegistry::getDownloadContext(a2_gid_t gid) const
{
  auto res = get(gid);
  if (res) {
    return res->downloadContext;
  }
  else {
    return getNull<DownloadContext>();
  }
}

const std::shared_ptr<DownloadContext>&
BtRegistry::getDownloadContext(const std::string& infoHash) const
{
  for (auto& kv : pool_) {
    if (bittorrent::getTorrentAttrs(kv.second->downloadContext)->infoHash ==
        infoHash) {
      return kv.second->downloadContext;
    }
  }
  return getNull<DownloadContext>();
}

void BtRegistry::put(a2_gid_t gid, std::unique_ptr<BtObject> obj)
{
  pool_[gid] = std::move(obj);
}

BtObject* BtRegistry::get(a2_gid_t gid) const
{
  auto i = pool_.find(gid);
  if (i == std::end(pool_)) {
    return nullptr;
  }
  else {
    return (*i).second.get();
  }
}

bool BtRegistry::remove(a2_gid_t gid) { return pool_.erase(gid); }

void BtRegistry::removeAll() { pool_.clear(); }

void BtRegistry::setLpdMessageReceiver(
    const std::shared_ptr<LpdMessageReceiver>& receiver)
{
  lpdMessageReceiver_ = receiver;
}

void BtRegistry::setUDPTrackerClient(
    const std::shared_ptr<UDPTrackerClient>& tracker)
{
  udpTrackerClient_ = tracker;
}

BtObject::BtObject(
    const std::shared_ptr<DownloadContext>& downloadContext,
    const std::shared_ptr<PieceStorage>& pieceStorage,
    const std::shared_ptr<PeerStorage>& peerStorage,
    const std::shared_ptr<BtAnnounce>& btAnnounce,
    const std::shared_ptr<BtRuntime>& btRuntime,
    const std::shared_ptr<BtProgressInfoFile>& btProgressInfoFile)
    : downloadContext{downloadContext},
      pieceStorage{pieceStorage},
      peerStorage{peerStorage},
      btAnnounce{btAnnounce},
      btRuntime{btRuntime},
      btProgressInfoFile{btProgressInfoFile}
{
}

BtObject::BtObject() {}

} // namespace aria2
