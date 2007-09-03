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
#include "DownloadEngineFactory.h"
#include "prefs.h"
#include "DefaultDiskWriter.h"
#include "InitiateConnectionCommandFactory.h"
#include "Util.h"
#include "FileAllocator.h"
#include "FileAllocationMonitor.h"
#include "FillRequestGroupCommand.h"
#include "CUIDCounter.h"
#include "FileAllocationDispatcherCommand.h"
#include "FileAllocationMan.h"
#include "AutoSaveCommand.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "CheckIntegrityMan.h"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_BITTORRENT
# include "PeerListenCommand.h"
# include "TrackerWatcherCommand.h"
# include "TrackerUpdateCommand.h"
# include "TorrentAutoSaveCommand.h"
# include "SeedCheckCommand.h"
# include "PeerChokeCommand.h"
# include "HaveEraseCommand.h"
# include "ActivePeerConnectionCommand.h"
# include "UnionSeedCriteria.h"
# include "TimeSeedCriteria.h"
# include "ShareRatioSeedCriteria.h"
# include "DefaultPieceStorage.h"
# include "DefaultPeerStorage.h"
# include "DefaultBtAnnounce.h"
# include "DefaultBtProgressInfoFile.h"
#endif // ENABLE_BITTORRENT

ConsoleDownloadEngine*
DownloadEngineFactory::newConsoleEngine(const Option* op,
					const RequestGroups& requestGroups)
{
  // set PREF_OUT parameter to requestGroup in non-multi download mode.
  if(requestGroups.size() == 1) {
    requestGroups.front()->setUserDefinedFilename(op->get(PREF_OUT));
  }
  RequestGroups workingSet;
  RequestGroups reservedSet;
  if(op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS) < (int32_t)requestGroups.size()) {
    copy(requestGroups.begin(), requestGroups.begin()+op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS), back_inserter(workingSet));
    copy(requestGroups.begin()+op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS),
	 requestGroups.end(), back_inserter(reservedSet));
  } else {
    workingSet = requestGroups;
  }

  ConsoleDownloadEngine* e = new ConsoleDownloadEngine();
  e->option = op;
  RequestGroupManHandle requestGroupMan = new RequestGroupMan(workingSet,
							      op->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS));
  requestGroupMan->addReservedGroup(reservedSet);
  e->_requestGroupMan = requestGroupMan;
  e->_fileAllocationMan = new FileAllocationMan();
#ifdef ENABLE_MESSAGE_DIGEST
  e->_checkIntegrityMan = new CheckIntegrityMan();
#endif // ENABLE_MESSAGE_DIGEST
  e->commands.push_back(new FillRequestGroupCommand(CUIDCounterSingletonHolder::instance()->newID(), e, 1));
  e->commands.push_back(new FileAllocationDispatcherCommand(CUIDCounterSingletonHolder::instance()->newID(), e));
  e->commands.push_back(new AutoSaveCommand(CUIDCounterSingletonHolder::instance()->newID(), e, op->getAsInt(PREF_AUTO_SAVE_INTERVAL)));
  return e;
}

ConsoleDownloadEngine*
DownloadEngineFactory::newConsoleEngine(const Option* op,
					const Requests& requests,
					const Requests& reserved)
{
  ConsoleDownloadEngine* e = new ConsoleDownloadEngine();
  e->option = op;
//   e->segmentMan = new SegmentMan();
//   e->segmentMan->diskWriter = DefaultDiskWriter::createNewDiskWriter(op);
//   e->segmentMan->dir = op->get(PREF_DIR);
//   e->segmentMan->ufilename = op->get(PREF_OUT);
//   e->segmentMan->option = op;
//   e->segmentMan->reserved = reserved;
  
//   int cuidCounter = 1;
//   for(Requests::const_iterator itr = requests.begin();
//       itr != requests.end();
//       itr++, cuidCounter++) {
//     e->commands.push_back(InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuidCounter, *itr, e));
//   }
  return e;
}

#ifdef ENABLE_BITTORRENT
TorrentConsoleDownloadEngine*
DownloadEngineFactory::newTorrentConsoleEngine(const BtContextHandle& btContext,
					       const Option* op,
					       const Strings& targetFiles)
{
  TorrentConsoleDownloadEngine* te = new TorrentConsoleDownloadEngine();
  te->option = op;
  RequestGroupManHandle requestGroupMan = new RequestGroupMan();
  te->_requestGroupMan = requestGroupMan;
  //  ByteArrayDiskWriter* byteArrayDiskWriter = new ByteArrayDiskWriter();
//   te->segmentMan = new SegmentMan();
//   te->segmentMan->diskWriter = byteArrayDiskWriter;
//   te->segmentMan->option = op;
  BtRuntimeHandle btRuntime(new BtRuntime());
  BtRegistry::registerBtRuntime(btContext->getInfoHashAsString(), btRuntime);

  PieceStorageHandle pieceStorage(new DefaultPieceStorage(btContext, op));
  BtRegistry::registerPieceStorage(btContext->getInfoHashAsString(), pieceStorage);

  PeerStorageHandle peerStorage(new DefaultPeerStorage(btContext, op));
  BtRegistry::registerPeerStorage(btContext->getInfoHashAsString(), peerStorage);

  BtAnnounceHandle btAnnounce(new DefaultBtAnnounce(btContext, op));
  BtRegistry::registerBtAnnounce(btContext->getInfoHashAsString(), btAnnounce);
  btAnnounce->shuffleAnnounce();

  BtProgressInfoFileHandle btProgressInfoFile(new DefaultBtProgressInfoFile(btContext, op));
  BtRegistry::registerBtProgressInfoFile(btContext->getInfoHashAsString(),
					 btProgressInfoFile);

  BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					new PeerObjectCluster());

  /*
  DefaultBtMessageFactoryAdaptorHandle factoryAdaptor =
    new DefaultBtMessageFactoryAdaptor();
  BtRegistry::registerBtMessageFactoryAdaptor(btContext->getInfoHashAsString(),
					      factoryAdaptor);

  BtMessageFactoryClusterHandle factoryCluster = new BtMessageFactoryCluster();
  BtRegistry::registerBtMessageFactoryCluster(btContext->getInfoHashAsString(),
					      factoryCluster);

  BtMessageDispatcherClusterHandle dispatcherCluster =
    new BtMessageDispatcherCluster();
  BtRegistry::registerBtMessageDispatcherCluster(btContext->getInfoHashAsString(),
						 dispatcherCluster);
  */
  te->setBtContext(btContext);
  // initialize file storage
  pieceStorage->initStorage();

  Integers selectIndexes;
  Util::unfoldRange(op->get(PREF_SELECT_FILE), selectIndexes);
  if(selectIndexes.size()) {
    pieceStorage->setFileFilter(selectIndexes);
  } else {
    pieceStorage->setFileFilter(targetFiles);
  }

  PeerListenCommand* listenCommand =
    new PeerListenCommand(CUIDCounterSingletonHolder::instance()->newID(),
			  te, btContext);
  int32_t port;
  int32_t listenPort = op->getAsInt(PREF_LISTEN_PORT);
  if(listenPort == -1) {
    port = listenCommand->bindPort(6881, 6999);
  } else {
    port = listenCommand->bindPort(listenPort, listenPort);
  }
  if(port == -1) {
    printf(_("Errors occurred while binding port.\n"));
    exit(EXIT_FAILURE);
  }
  btRuntime->setListenPort(port);
  te->commands.push_back(listenCommand);
  
  te->commands.push_back(new TrackerWatcherCommand(CUIDCounterSingletonHolder::instance()->newID(),
						   te,
						   btContext));
  te->commands.push_back(new TrackerUpdateCommand(CUIDCounterSingletonHolder::instance()->newID(),
						  te,
						  btContext));
  te->commands.push_back(new TorrentAutoSaveCommand(CUIDCounterSingletonHolder::instance()->newID(),
						    te,
						    btContext,
						    op->getAsInt(PREF_AUTO_SAVE_INTERVAL)));
  te->commands.push_back(new PeerChokeCommand(CUIDCounterSingletonHolder::instance()->newID(),
					      te,
					      btContext,
					      10));
  te->commands.push_back(new HaveEraseCommand(CUIDCounterSingletonHolder::instance()->newID(),
					      te,
					      btContext,
					      10));
  te->commands.push_back(new ActivePeerConnectionCommand(CUIDCounterSingletonHolder::instance()->newID(),
							 te,
							 btContext,
							 30));

  SharedHandle<UnionSeedCriteria> unionCri = new UnionSeedCriteria();
  if(op->defined(PREF_SEED_TIME)) {
    unionCri->addSeedCriteria(new TimeSeedCriteria(op->getAsInt(PREF_SEED_TIME)*60));
  }
  if(op->defined(PREF_SEED_RATIO)) {
    unionCri->addSeedCriteria(new ShareRatioSeedCriteria(op->getAsDouble(PREF_SEED_RATIO), btContext));
  }
  if(unionCri->getSeedCriterion().size() > 0) {
    te->commands.push_back(new SeedCheckCommand(CUIDCounterSingletonHolder::instance()->newID(),
						te,
						btContext,
						unionCri));
  }
  return te;
}
#endif // ENABLE_BITTORRENT
