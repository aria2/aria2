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
#include "BtRegistry.h"
#include "DlAbortEx.h"

PeerStorageMap BtRegistry::peerStorageMap;
PieceStorageMap BtRegistry::pieceStorageMap;
BtAnnounceMap BtRegistry::btAnnounceMap;
BtRuntimeMap BtRegistry::btRuntimeMap;
BtProgressInfoFileMap BtRegistry::btProgressInfoFileMap;
PeerObjectClusterRegistry BtRegistry::peerObjectClusterRegistry;

PeerStorageHandle BtRegistry::getPeerStorage(const string& key) {
  PeerStorageMap::iterator itr = peerStorageMap.find(key);
  if(itr == peerStorageMap.end()) {
    return PeerStorageHandle(0);
  } else {
    return itr->second;
  }
}

bool BtRegistry::registerPeerStorage(const string& key,
				     const PeerStorageHandle& peerStorage) {
  PeerStorageMap::value_type p(key, peerStorage);
  pair<PeerStorageMap::iterator, bool> retval = peerStorageMap.insert(p);
  return retval.second;
}
				  
PieceStorageHandle
BtRegistry::getPieceStorage(const string& key) {
  PieceStorageMap::iterator itr = pieceStorageMap.find(key);
  if(itr == pieceStorageMap.end()) {
    return PieceStorageHandle(0);
  } else {
    return itr->second;
  }
}

bool
BtRegistry::registerPieceStorage(const string& key,
				 const PieceStorageHandle& pieceStorage) {
  pieceStorageMap.erase(key);
  PieceStorageMap::value_type p(key, pieceStorage);
  pair<PieceStorageMap::iterator, bool> retval = pieceStorageMap.insert(p);
  return retval.second;
}

BtRuntimeHandle BtRegistry::getBtRuntime(const string& key) {
  BtRuntimeMap::iterator itr = btRuntimeMap.find(key);
  if(itr == btRuntimeMap.end()) {
    return BtRuntimeHandle(0);
  } else {
    return itr->second;
  }
}

bool
BtRegistry::registerBtRuntime(const string& key,
			      const BtRuntimeHandle& btRuntime) {
  BtRuntimeMap::value_type p(key, btRuntime);
  pair<BtRuntimeMap::iterator, bool> retval =
    btRuntimeMap.insert(p);
  return retval.second;
}

BtAnnounceHandle BtRegistry::getBtAnnounce(const string& key) {
  BtAnnounceMap::iterator itr = btAnnounceMap.find(key);
  if(itr == btAnnounceMap.end()) {
    return BtAnnounceHandle(0);
  } else {
    return itr->second;
  }
}

bool
BtRegistry::registerBtAnnounce(const string& key,
			       const BtAnnounceHandle& btAnnounce) {
  BtAnnounceMap::value_type p(key, btAnnounce);
  pair<BtAnnounceMap::iterator, bool> retval =
    btAnnounceMap.insert(p);
  return retval.second;
}

BtProgressInfoFileHandle BtRegistry::getBtProgressInfoFile(const string& key) {
  BtProgressInfoFileMap::iterator itr = btProgressInfoFileMap.find(key);
  if(itr == btProgressInfoFileMap.end()) {
    return BtProgressInfoFileHandle(0);
  } else {
    return itr->second;
  }
}

bool
BtRegistry::registerBtProgressInfoFile(const string& key,
				       const BtProgressInfoFileHandle& btProgressInfoFile) {
  BtProgressInfoFileMap::value_type p(key, btProgressInfoFile);
  pair<BtProgressInfoFileMap::iterator, bool> retval =
    btProgressInfoFileMap.insert(p);
  return retval.second;
}

PeerObjectClusterHandle
BtRegistry::getPeerObjectCluster(const string& key)
{
  return peerObjectClusterRegistry.getHandle(key);
}

void
BtRegistry::registerPeerObjectCluster(const string& key,
				      const PeerObjectClusterHandle& cluster)
{
  peerObjectClusterRegistry.registerHandle(key, cluster);
}

void
BtRegistry::unregisterPeerObjectCluster(const string& key)
{
  peerObjectClusterRegistry.unregisterHandle(key);
}

void BtRegistry::clear() {
  peerStorageMap.clear();
  pieceStorageMap.clear();
  btAnnounceMap.clear();
  btRuntimeMap.clear();
  btProgressInfoFileMap.clear();
  peerObjectClusterRegistry.clear();
}
