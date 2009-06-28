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
#ifndef _D_BT_REGISTRY_H_
#define _D_BT_REGISTRY_H_

#include "common.h"

#include <string>
#include <map>

#include "SharedHandle.h"

namespace aria2 {

class PeerStorage;
class PieceStorage;
class BtAnnounce;
class BtRuntime;
class BtProgressInfoFile;
class DownloadContext;

struct BtObject {
  SharedHandle<DownloadContext> _downloadContext;
  SharedHandle<PieceStorage> _pieceStorage;
  SharedHandle<PeerStorage> _peerStorage;
  SharedHandle<BtAnnounce> _btAnnounce;
  SharedHandle<BtRuntime> _btRuntime;
  SharedHandle<BtProgressInfoFile> _btProgressInfoFile;

  BtObject(const SharedHandle<DownloadContext>& downloadContext,
	   const SharedHandle<PieceStorage>& pieceStorage,
	   const SharedHandle<PeerStorage>& peerStorage,
	   const SharedHandle<BtAnnounce>& btAnnounce,
	   const SharedHandle<BtRuntime>& btRuntime,
	   const SharedHandle<BtProgressInfoFile>& btProgressInfoFile):
    _downloadContext(downloadContext),
    _pieceStorage(pieceStorage),
    _peerStorage(peerStorage),
    _btAnnounce(btAnnounce),
    _btRuntime(btRuntime),
    _btProgressInfoFile(btProgressInfoFile) {}

  BtObject() {}

  bool isNull() const
  {
    return _downloadContext.isNull() &&
      _pieceStorage.isNull() &&
      _peerStorage.isNull() &&
      _btAnnounce.isNull() &&
      _btRuntime.isNull() &&
      _btProgressInfoFile.isNull();
  }
};

class BtRegistry {
private:
  std::map<std::string, BtObject> _pool;
public:
  SharedHandle<DownloadContext>
  getDownloadContext(const std::string& infoHash) const;

  void put(const std::string& infoHash, const BtObject& obj);

  BtObject get(const std::string& infoHash) const;

  template<typename OutputIterator>
  OutputIterator getAllDownloadContext(OutputIterator dest)
  {
    for(std::map<std::string, BtObject>::const_iterator i = _pool.begin();
	i != _pool.end(); ++i) {
      *dest++ = (*i).second._downloadContext;
    }
    return dest;
  }

  void removeAll();

  bool remove(const std::string& infoHash);
};

} // namespace aria2

#endif // _D_BT_REGISTRY_H_
