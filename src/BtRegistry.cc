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
#include "NullHandle.h"

namespace aria2 {

BtRegistry::BtRegistry()
  : tcpPort_(0)
{}

BtRegistry::~BtRegistry() {}

const SharedHandle<DownloadContext>&
BtRegistry::getDownloadContext(a2_gid_t gid) const
{
  const SharedHandle<BtObject>& res = get(gid);
  if(res) {
    return res->downloadContext;
  } else {
    return getNull<DownloadContext>();
  }
}

const SharedHandle<DownloadContext>&
BtRegistry::getDownloadContext(const std::string& infoHash) const
{
  for(std::map<a2_gid_t, SharedHandle<BtObject> >::const_iterator i =
        pool_.begin(), eoi = pool_.end(); i != eoi; ++i) {
    if(bittorrent::getTorrentAttrs((*i).second->downloadContext)->infoHash ==
       infoHash) {
      return (*i).second->downloadContext;
    }
  }      
  return getNull<DownloadContext>();
}

void BtRegistry::put(a2_gid_t gid, const SharedHandle<BtObject>& obj)
{
  pool_[gid] = obj;
}

const SharedHandle<BtObject>& BtRegistry::get(a2_gid_t gid) const
{
  std::map<a2_gid_t, SharedHandle<BtObject> >::const_iterator i =
    pool_.find(gid);
  if(i == pool_.end()) {
    return getNull<BtObject>();
  } else {
    return (*i).second;
  }
}

bool BtRegistry::remove(a2_gid_t gid)
{
  return pool_.erase(gid);
}

void BtRegistry::removeAll() {
  pool_.clear();
}

void BtRegistry::setLpdMessageReceiver
(const SharedHandle<LpdMessageReceiver>& receiver)
{
  lpdMessageReceiver_ = receiver;
}

BtObject::BtObject
(const SharedHandle<DownloadContext>& downloadContext,
 const SharedHandle<PieceStorage>& pieceStorage,
 const SharedHandle<PeerStorage>& peerStorage,
 const SharedHandle<BtAnnounce>& btAnnounce,
 const SharedHandle<BtRuntime>& btRuntime,
 const SharedHandle<BtProgressInfoFile>& btProgressInfoFile)
  : downloadContext(downloadContext),
    pieceStorage(pieceStorage),
    peerStorage(peerStorage),
    btAnnounce(btAnnounce),
    btRuntime(btRuntime),
    btProgressInfoFile(btProgressInfoFile)
{}

BtObject::BtObject() {}

BtObject::BtObject(const BtObject& c)
  : downloadContext(c.downloadContext),
    pieceStorage(c.pieceStorage),
    peerStorage(c.peerStorage),
    btAnnounce(c.btAnnounce),
    btRuntime(c.btRuntime),
    btProgressInfoFile(c.btProgressInfoFile)
{}

BtObject::~BtObject() {}

BtObject& BtObject::operator=(const BtObject& c)
{
  if(this != &c) {
    downloadContext = c.downloadContext;
    pieceStorage = c.pieceStorage;
    peerStorage = c.peerStorage;
    btAnnounce = c.btAnnounce;
    btRuntime = c.btRuntime;
    btProgressInfoFile = c.btProgressInfoFile;
  }
  return *this;
}

} // namespace aria2
