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

BtContextMap BtRegistry::btContextMap;
PeerStorageMap BtRegistry::peerStorageMap;
PieceStorageMap BtRegistry::pieceStorageMap;
BtAnnounceMap BtRegistry::btAnnounceMap;
BtRuntimeMap BtRegistry::btRuntimeMap;
BtProgressInfoFileMap BtRegistry::btProgressInfoFileMap;
PeerObjectClusterRegistry BtRegistry::peerObjectClusterRegistry;

PeerStorageHandle BtRegistry::getPeerStorage(const string& key)
{
  return peerStorageMap.getHandle(key);
}

void BtRegistry::registerPeerStorage(const string& key,
				     const PeerStorageHandle& peerStorage)
{
  peerStorageMap.registerHandle(key, peerStorage);
}
				  
PieceStorageHandle
BtRegistry::getPieceStorage(const string& key)
{
  return pieceStorageMap.getHandle(key);
}

void
BtRegistry::registerPieceStorage(const string& key,
				 const PieceStorageHandle& pieceStorage)
{
  pieceStorageMap.registerHandle(key, pieceStorage);
}

BtRuntimeHandle BtRegistry::getBtRuntime(const string& key)
{
  return btRuntimeMap.getHandle(key);
}

void
BtRegistry::registerBtRuntime(const string& key,
			      const BtRuntimeHandle& btRuntime)
{
  btRuntimeMap.registerHandle(key, btRuntime);
}

BtAnnounceHandle BtRegistry::getBtAnnounce(const string& key)
{
  return btAnnounceMap.getHandle(key);
}

void
BtRegistry::registerBtAnnounce(const string& key,
			       const BtAnnounceHandle& btAnnounce)
{
  btAnnounceMap.registerHandle(key, btAnnounce);
}

BtProgressInfoFileHandle BtRegistry::getBtProgressInfoFile(const string& key)
{
  return btProgressInfoFileMap.getHandle(key);
}

void
BtRegistry::registerBtProgressInfoFile(const string& key,
				       const BtProgressInfoFileHandle& btProgressInfoFile)
{
  btProgressInfoFileMap.registerHandle(key, btProgressInfoFile);
}

BtContextHandle
BtRegistry::getBtContext(const string& key)
{
  return btContextMap.getHandle(key);
}

void
BtRegistry::registerBtContext(const string& key,
			      const BtContextHandle& btContext)
{
  btContextMap.registerHandle(key, btContext);
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

void BtRegistry::unregisterAll() {
  btContextMap.clear();
  peerStorageMap.clear();
  pieceStorageMap.clear();
  btAnnounceMap.clear();
  btRuntimeMap.clear();
  btProgressInfoFileMap.clear();
  peerObjectClusterRegistry.clear();
}

void BtRegistry::unregister(const string& key)
{
  btContextMap.unregisterHandle(key);
  peerStorageMap.unregisterHandle(key);
  pieceStorageMap.unregisterHandle(key);
  btAnnounceMap.unregisterHandle(key);
  btRuntimeMap.unregisterHandle(key);
  btProgressInfoFileMap.unregisterHandle(key);
  peerObjectClusterRegistry.unregisterHandle(key);
}
