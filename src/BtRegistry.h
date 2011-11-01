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

#include "SharedHandle.h"
#include "RequestGroup.h"

namespace aria2 {

class PeerStorage;
class PieceStorage;
class BtAnnounce;
class BtRuntime;
class BtProgressInfoFile;
class DownloadContext;
class LpdMessageReceiver;

struct BtObject {
  SharedHandle<DownloadContext> downloadContext;
  SharedHandle<PieceStorage> pieceStorage;
  SharedHandle<PeerStorage> peerStorage;
  SharedHandle<BtAnnounce> btAnnounce;
  SharedHandle<BtRuntime> btRuntime;
  SharedHandle<BtProgressInfoFile> btProgressInfoFile;

  BtObject(const SharedHandle<DownloadContext>& downloadContext,
           const SharedHandle<PieceStorage>& pieceStorage,
           const SharedHandle<PeerStorage>& peerStorage,
           const SharedHandle<BtAnnounce>& btAnnounce,
           const SharedHandle<BtRuntime>& btRuntime,
           const SharedHandle<BtProgressInfoFile>& btProgressInfoFile);

  BtObject();

  BtObject(const BtObject& c);

  ~BtObject();

  BtObject& operator=(const BtObject& c);
};

class BtRegistry {
private:
  std::map<a2_gid_t, SharedHandle<BtObject> > pool_;
  uint16_t tcpPort_;
  SharedHandle<LpdMessageReceiver> lpdMessageReceiver_;
public:
  BtRegistry();
  ~BtRegistry();

  const SharedHandle<DownloadContext>&
  getDownloadContext(a2_gid_t gid) const;

  const SharedHandle<DownloadContext>&
  getDownloadContext(const std::string& infoHash) const;

  void put(a2_gid_t gid, const SharedHandle<BtObject>& obj);

  const SharedHandle<BtObject>& get(a2_gid_t gid) const;

  template<typename OutputIterator>
  OutputIterator getAllDownloadContext(OutputIterator dest)
  {
    for(std::map<a2_gid_t, SharedHandle<BtObject> >::const_iterator i =
          pool_.begin(), eoi = pool_.end(); i != eoi; ++i) {
      *dest++ = (*i).second->downloadContext;
    }
    return dest;
  }

  void removeAll();

  bool remove(a2_gid_t gid);

  void setTcpPort(uint16_t port)
  {
    tcpPort_ = port;
  }
  uint16_t getTcpPort() const
  {
    return tcpPort_;
  }

  void setLpdMessageReceiver(const SharedHandle<LpdMessageReceiver>& receiver);
  const SharedHandle<LpdMessageReceiver>& getLpdMessageReceiver() const
  {
    return lpdMessageReceiver_;
  }
};

} // namespace aria2

#endif // D_BT_REGISTRY_H
