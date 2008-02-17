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
#include "SharedHandle.h"
#include "HandleRegistry.h"
#include <string>
#include <map>

namespace aria2 {

class PeerStorage;
class PieceStorage;
class BtAnnounce;
class BtRuntime;
class BtProgressInfoFile;
class BtContext;
class PeerObject;

typedef HandleRegistry<std::string, PeerStorage> PeerStorageMap;
typedef HandleRegistry<std::string, PieceStorage> PieceStorageMap;
typedef HandleRegistry<std::string, BtAnnounce> BtAnnounceMap;
typedef HandleRegistry<std::string, BtRuntime> BtRuntimeMap;
typedef HandleRegistry<std::string, BtProgressInfoFile> BtProgressInfoFileMap;
typedef HandleRegistry<std::string, BtContext>  BtContextMap;

// for BtMessageFactory
typedef HandleRegistry<std::string, PeerObject> PeerObjectCluster;
typedef SharedHandle<PeerObjectCluster> PeerObjectClusterHandle;
typedef HandleRegistry<std::string, PeerObjectCluster> PeerObjectClusterRegistry;

class BtRegistry {
private:
  BtRegistry() {}

  static BtContextMap btContextMap;
  static PeerStorageMap peerStorageMap;
  static PieceStorageMap pieceStorageMap;
  static BtAnnounceMap btAnnounceMap;
  static BtRuntimeMap btRuntimeMap;
  static BtProgressInfoFileMap btProgressInfoFileMap;
  static PeerObjectClusterRegistry peerObjectClusterRegistry;
public:
  static SharedHandle<BtContext> getBtContext(const std::string& key);
  static void registerBtContext(const std::string& key,
				const SharedHandle<BtContext>& btContext);

  static SharedHandle<PeerStorage> getPeerStorage(const std::string& key);
  static void registerPeerStorage(const std::string& key,
				  const SharedHandle<PeerStorage>& peer);
				  
  static SharedHandle<PieceStorage> getPieceStorage(const std::string& key);
  static void registerPieceStorage(const std::string& key,
				   const SharedHandle<PieceStorage>& pieceStorage);

  static SharedHandle<BtRuntime> getBtRuntime(const std::string& key);
  static void registerBtRuntime(const std::string& key,
				const SharedHandle<BtRuntime>& btRuntime);

  static SharedHandle<BtAnnounce> getBtAnnounce(const std::string& key);
  static void registerBtAnnounce(const std::string& key,
				 const SharedHandle<BtAnnounce>& btAnnounce);

  static SharedHandle<BtProgressInfoFile> getBtProgressInfoFile(const std::string& key);
  static void registerBtProgressInfoFile(const std::string& key,
					 const SharedHandle<BtProgressInfoFile>& btProgressInfoFile);

  static std::deque<SharedHandle<BtContext> > getAllBtContext();

  // for PeerObject
  static PeerObjectClusterHandle
  getPeerObjectCluster(const std::string& key);

  static void
  registerPeerObjectCluster(const std::string& key,
			    const PeerObjectClusterHandle& cluster);

  static void
  unregisterPeerObjectCluster(const std::string& key);

  static void unregisterAll();

  static void unregister(const std::string& key);
};

#define PEER_STORAGE(btContext) \
BtRegistry::getPeerStorage(btContext->getInfoHashAsString())

#define PIECE_STORAGE(btContext) \
BtRegistry::getPieceStorage(btContext->getInfoHashAsString())

#define BT_ANNOUNCE(btContext) \
BtRegistry::getBtAnnounce(btContext->getInfoHashAsString())

#define BT_RUNTIME(btContext) \
BtRegistry::getBtRuntime(btContext->getInfoHashAsString())

#define BT_PROGRESS_INFO_FILE(btContext) \
BtRegistry::getBtProgressInfoFile(btContext->getInfoHashAsString())

#define PEER_OBJECT_CLUSTER(btContext) \
BtRegistry::getPeerObjectCluster(btContext->getInfoHashAsString())

#define PEER_OBJECT(btContext, peer) \
PEER_OBJECT_CLUSTER(btContext)->getHandle(peer->getID())

#define BT_MESSAGE_DISPATCHER(btContext, peer) \
PEER_OBJECT(btContext, peer)->btMessageDispatcher

#define BT_MESSAGE_RECEIVER(btContext, peer) \
PEER_OBJECT(btContext, peer)->btMessageReceiver

#define BT_MESSAGE_FACTORY(btContext, peer) \
PEER_OBJECT(btContext, peer)->btMessageFactory

#define BT_REQUEST_FACTORY(btContext, peer) \
PEER_OBJECT(btContext, peer)->btRequestFactory

#define PEER_CONNECTION(btContext, peer) \
PEER_OBJECT(btContext, peer)->peerConnection

#define EXTENSION_MESSAGE_FACTORY(btContext, peer) \
PEER_OBJECT(btContext, peer)->extensionMessageFactory

} // namespace aria2

#endif // _D_BT_REGISTRY_H_
