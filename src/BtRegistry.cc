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
#include "BtContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtAnnounce.h"
#include "BtRuntime.h"
#include "BtProgressInfoFile.h"
#include "PeerObject.h"
#include "BtMessageFactory.h"
#include "BtRequestFactory.h"
#include "BtMessageDispatcher.h"
#include "BtMessageReceiver.h"
#include "PeerConnection.h"
#include "ExtensionMessageFactory.h"

namespace aria2 {

BtContextMap BtRegistry::btContextMap;
PeerStorageMap BtRegistry::peerStorageMap;
PieceStorageMap BtRegistry::pieceStorageMap;
BtAnnounceMap BtRegistry::btAnnounceMap;
BtRuntimeMap BtRegistry::btRuntimeMap;
BtProgressInfoFileMap BtRegistry::btProgressInfoFileMap;
PeerObjectClusterRegistry BtRegistry::peerObjectClusterRegistry;

PeerStorageHandle BtRegistry::getPeerStorage(const std::string& key)
{
  return peerStorageMap.getHandle(key);
}

void BtRegistry::registerPeerStorage(const std::string& key,
				     const PeerStorageHandle& peerStorage)
{
  peerStorageMap.registerHandle(key, peerStorage);
}
				  
PieceStorageHandle
BtRegistry::getPieceStorage(const std::string& key)
{
  return pieceStorageMap.getHandle(key);
}

void
BtRegistry::registerPieceStorage(const std::string& key,
				 const PieceStorageHandle& pieceStorage)
{
  pieceStorageMap.registerHandle(key, pieceStorage);
}

BtRuntimeHandle BtRegistry::getBtRuntime(const std::string& key)
{
  return btRuntimeMap.getHandle(key);
}

void
BtRegistry::registerBtRuntime(const std::string& key,
			      const BtRuntimeHandle& btRuntime)
{
  btRuntimeMap.registerHandle(key, btRuntime);
}

BtAnnounceHandle BtRegistry::getBtAnnounce(const std::string& key)
{
  return btAnnounceMap.getHandle(key);
}

void
BtRegistry::registerBtAnnounce(const std::string& key,
			       const BtAnnounceHandle& btAnnounce)
{
  btAnnounceMap.registerHandle(key, btAnnounce);
}

BtProgressInfoFileHandle BtRegistry::getBtProgressInfoFile(const std::string& key)
{
  return btProgressInfoFileMap.getHandle(key);
}

void
BtRegistry::registerBtProgressInfoFile(const std::string& key,
				       const BtProgressInfoFileHandle& btProgressInfoFile)
{
  btProgressInfoFileMap.registerHandle(key, btProgressInfoFile);
}

BtContextHandle
BtRegistry::getBtContext(const std::string& key)
{
  return btContextMap.getHandle(key);
}

void
BtRegistry::registerBtContext(const std::string& key,
			      const BtContextHandle& btContext)
{
  btContextMap.registerHandle(key, btContext);
}

std::deque<SharedHandle<BtContext> > BtRegistry::getAllBtContext()
{
  return btContextMap.getAll();
}

PeerObjectClusterHandle
BtRegistry::getPeerObjectCluster(const std::string& key)
{
  return peerObjectClusterRegistry.getHandle(key);
}

void
BtRegistry::registerPeerObjectCluster(const std::string& key,
				      const PeerObjectClusterHandle& cluster)
{
  peerObjectClusterRegistry.registerHandle(key, cluster);
}

void
BtRegistry::unregisterPeerObjectCluster(const std::string& key)
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

void BtRegistry::unregister(const std::string& key)
{
  btContextMap.unregisterHandle(key);
  peerStorageMap.unregisterHandle(key);
  pieceStorageMap.unregisterHandle(key);
  btAnnounceMap.unregisterHandle(key);
  btRuntimeMap.unregisterHandle(key);
  btProgressInfoFileMap.unregisterHandle(key);
  peerObjectClusterRegistry.unregisterHandle(key);
}

} // namespace aria2
