/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "DownloadEngineFactory.h"
#include "prefs.h"
#include "DefaultDiskWriter.h"
#include "InitiateConnectionCommandFactory.h"
#include "ByteArrayDiskWriter.h"
#include "Util.h"
#ifdef ENABLE_BITTORRENT
# include "PeerListenCommand.h"
# include "TrackerWatcherCommand.h"
# include "TrackerUpdateCommand.h"
# include "TorrentAutoSaveCommand.h"
# include "SeedCheckCommand.h"
# include "PeerChokeCommand.h"
# include "HaveEraseCommand.h"
# include "UnionSeedCriteria.h"
# include "TimeSeedCriteria.h"
# include "ShareRatioSeedCriteria.h"
#endif // ENABLE_BITTORRENT

ConsoleDownloadEngine*
DownloadEngineFactory::newConsoleEngine(const Option* op,
					const Requests& requests,
					const Requests& reserved)
{
  ConsoleDownloadEngine* e = new ConsoleDownloadEngine();
  e->option = op;
  e->segmentMan = new SegmentMan();
  e->segmentMan->diskWriter = new DefaultDiskWriter();
  e->segmentMan->dir = op->get(PREF_DIR);
  e->segmentMan->ufilename = op->get(PREF_OUT);
  e->segmentMan->option = op;
  e->segmentMan->reserved = reserved;
  
  int cuidCounter = 1;
  for(Requests::const_iterator itr = requests.begin();
      itr != requests.end();
      itr++, cuidCounter++) {
    e->commands.push_back(InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuidCounter, *itr, e));
  }
  return e;
}

#ifdef ENABLE_BITTORRENT
TorrentConsoleDownloadEngine*
DownloadEngineFactory::newTorrentConsoleEngine(const Option* op,
					       const string& torrentFile,
					       const Strings& targetFiles)
{
  Request* req = new Request();
  req->isTorrent = true;
  req->setTrackerEvent(Request::STARTED);
  TorrentConsoleDownloadEngine* te = new TorrentConsoleDownloadEngine();
  te->option = op;
  ByteArrayDiskWriter* byteArrayDiskWriter = new ByteArrayDiskWriter();
  te->segmentMan = new SegmentMan();
  te->segmentMan->diskWriter = byteArrayDiskWriter;
  te->segmentMan->option = op;
  te->torrentMan = new TorrentMan();
  te->torrentMan->setStoreDir(op->get(PREF_DIR));
  te->torrentMan->option = op;
  te->torrentMan->req = req;
  Integers selectIndexes;
  Util::unfoldRange(op->get(PREF_SELECT_FILE), selectIndexes);
  if(selectIndexes.size()) {
    te->torrentMan->setup(torrentFile, selectIndexes);
  } else {
    te->torrentMan->setup(torrentFile, targetFiles);
  }

  PeerListenCommand* listenCommand =
    new PeerListenCommand(te->torrentMan->getNewCuid(), te);
  int port;
  int listenPort = op->getAsInt(PREF_LISTEN_PORT);
  if(listenPort == -1) {
    port = listenCommand->bindPort(6881, 6999);
  } else {
    port = listenCommand->bindPort(listenPort, listenPort);
  }
  if(port == -1) {
    printf(_("Errors occurred while binding port.\n"));
    exit(EXIT_FAILURE);
  }
  te->torrentMan->setPort(port);
  te->commands.push_back(listenCommand);
  
  te->commands.push_back(new TrackerWatcherCommand(te->torrentMan->getNewCuid(),
						   te,
						   te->torrentMan->minInterval));
  te->commands.push_back(new TrackerUpdateCommand(te->torrentMan->getNewCuid(),
						  te));
  te->commands.push_back(new TorrentAutoSaveCommand(te->torrentMan->getNewCuid(),
						    te,
						    op->getAsInt(PREF_AUTO_SAVE_INTERVAL)));
  te->commands.push_back(new PeerChokeCommand(te->torrentMan->getNewCuid(),
					      te, 10));
  te->commands.push_back(new HaveEraseCommand(te->torrentMan->getNewCuid(),
					      te, 10));

  SharedHandle<UnionSeedCriteria> unionCri = new UnionSeedCriteria();
  if(op->defined(PREF_SEED_TIME)) {
    unionCri->addSeedCriteria(new TimeSeedCriteria(op->getAsInt(PREF_SEED_TIME)*60));
  }
  if(op->defined(PREF_SEED_RATIO)) {
    unionCri->addSeedCriteria(new ShareRatioSeedCriteria(op->getAsDouble(PREF_SEED_RATIO), te->torrentMan));
  }
  if(unionCri->getSeedCriterion().size() > 0) {
    te->commands.push_back(new SeedCheckCommand(te->torrentMan->getNewCuid(),
						te,
						unionCri));
  }
  return te;
}
#endif // ENABLE_BITTORRENT
