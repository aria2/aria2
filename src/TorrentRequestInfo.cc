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
#include "TorrentRequestInfo.h"
#include "DownloadEngineFactory.h"
#include "prefs.h"
#include "Util.h"
#include "BtRegistry.h"
#include "DefaultBtContext.h"
#include "FatalException.h"
#include "message.h"
#include "RecoverableException.h"
#include "DNSCache.h"
#include <signal.h>

extern volatile sig_atomic_t btHaltRequested;

void torrentHandler(int signal) {
  btHaltRequested = 1;
}

RequestInfos TorrentRequestInfo::execute() {
  {
    DNSCacheHandle dnsCache = new NullDNSCache();
    DNSCacheSingletonHolder::instance(dnsCache);
  }

  DefaultBtContextHandle btContext = new DefaultBtContext();
  btContext->load(torrentFile);
  if(op->defined(PREF_PEER_ID_PREFIX)) {
    btContext->setPeerIdPrefix(op->get(PREF_PEER_ID_PREFIX));
  }

  if(op->get(PREF_SHOW_FILES) == V_TRUE) {
    Util::toStream(cout, btContext->getFileEntries());
    return RequestInfos();
  }
  // set max_tries to 1. AnnounceList handles retries.
  op->put(PREF_MAX_TRIES, "1");
  SharedHandle<TorrentDownloadEngine>
    e(DownloadEngineFactory::newTorrentConsoleEngine(btContext,
						     op,
						     targetFiles));

  if(BT_PROGRESS_INFO_FILE(btContext)->exists()) {
    // load .aria2 file if it exists.
    BT_PROGRESS_INFO_FILE(btContext)->load();
    PIECE_STORAGE(btContext)->getDiskAdaptor()->openExistingFile();
#ifdef ENABLE_MESSAGE_DIGEST
    if(op->get(PREF_CHECK_INTEGRITY) == V_TRUE) {
      PIECE_STORAGE(btContext)->checkIntegrity();
    }
#endif // ENABLE_MESSAGE_DIGEST
  } else {
    if(PIECE_STORAGE(btContext)->getDiskAdaptor()->fileExists()) {
      if(op->get(PREF_ALLOW_OVERWRITE) != V_TRUE) {
	logger->notice(MSG_FILE_ALREADY_EXISTS,
		       PIECE_STORAGE(btContext)->getDiskAdaptor()->getFilePath().c_str(),
		       BT_PROGRESS_INFO_FILE(btContext)->getFilename().c_str());
	throw new FatalException(EX_DOWNLOAD_ABORTED);
      } else {
	PIECE_STORAGE(btContext)->getDiskAdaptor()->openExistingFile();
#ifdef ENABLE_MESSAGE_DIGEST
	if(op->get(PREF_CHECK_INTEGRITY) == V_TRUE) {
	  PIECE_STORAGE(btContext)->markAllPiecesDone();
	  PIECE_STORAGE(btContext)->checkIntegrity();
	}
#endif // ENABLE_MESSAGE_DIGEST
      }
    } else {
      PIECE_STORAGE(btContext)->getDiskAdaptor()->initAndOpenFile();
    }
  }

  Util::setGlobalSignalHandler(SIGINT, torrentHandler, SA_RESETHAND);
  Util::setGlobalSignalHandler(SIGTERM, torrentHandler, SA_RESETHAND);
    
  try {
    e->run();
    if(PIECE_STORAGE(btContext)->downloadFinished()) {
      printDownloadCompeleteMessage();
    }
  } catch(RecoverableException* ex) {
    logger->error(EX_EXCEPTION_CAUGHT, ex);
    fail = true;
    delete ex;
  }
  // TODO we want just 1 torrent download to clear
  BtRegistry::clear();

  Util::setGlobalSignalHandler(SIGINT, SIG_DFL, 0);
  Util::setGlobalSignalHandler(SIGTERM, SIG_DFL, 0);
  
  return RequestInfos();
}
