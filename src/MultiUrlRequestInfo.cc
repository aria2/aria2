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
#include "MultiUrlRequestInfo.h"
#include "prefs.h"
#include "DownloadEngineFactory.h"
#include "RecoverableException.h"
#include "message.h"
#include "DNSCache.h"
#include "TorrentRequestInfo.h"
#include "MetalinkRequestInfo.h"
#include "Util.h"
#include <signal.h>

extern volatile sig_atomic_t haltRequested;

RequestInfoHandle MultiUrlRequestInfo::createNextRequestInfo(const string& filename) const
{
#ifdef ENABLE_BITTORRENT
  if(op->getAsBool(PREF_FOLLOW_TORRENT) &&
     Util::endsWith(filename, ".torrent")) {
    return new TorrentRequestInfo(filename, op);
  } else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
    if(op->getAsBool(PREF_FOLLOW_METALINK) &&
       Util::endsWith(filename, ".metalink")) {
      return new MetalinkRequestInfo(filename, op);
    } else
#endif // ENABLE_METALINK
      {
	return 0;
      }
}

static void handler(int signal) {
  haltRequested = true;
}

RequestInfos MultiUrlRequestInfo::execute() {
  {
    DNSCacheHandle dnsCache = new SimpleDNSCache();
    DNSCacheSingletonHolder::instance(dnsCache);
  }

  RequestInfos nextReqInfos;
  try {
    SharedHandle<ConsoleDownloadEngine> e(DownloadEngineFactory::newConsoleEngine(op, _requestGroups));

    e->fillCommand();

    // The number of simultaneous download is specified by PREF_MAX_CONCURRENT_DOWNLOADS.
    // The remaining urls are queued into FillRequestGroupCommand.
    // It observes the number of simultaneous downloads and if it is under
    // the limit, it adds RequestGroup object from its queue to DownloadEngine.
    // This is done every 1 second. At the same time, it removes finished/error
    // RequestGroup from DownloadEngine.

    Util::setGlobalSignalHandler(SIGINT, handler, 0);
    Util::setGlobalSignalHandler(SIGTERM, handler, 0);
    
    e->run();
    
    for(RequestGroups::iterator itr = _requestGroups.begin();
	itr != _requestGroups.end(); ++itr) {
      if((*itr)->downloadFinished()) {
	RequestInfoHandle reqInfo = createNextRequestInfo((*itr)->getFilePath());
	if(!reqInfo.isNull()) {
	  nextReqInfos.push_back(reqInfo);
	}
      }
    }
    RequestGroupMan rgman(_requestGroups);
    // TODO print summary of the download result
    rgman.showDownloadResults(cout);
    cout << flush;
    // TODO Do we have to print a message when some of the downloads are failed?
    if(!rgman.downloadFinished()) {
      printDownloadAbortMessage();
    }
  } catch(RecoverableException *ex) {
    logger->error(EX_EXCEPTION_CAUGHT, ex);
    delete ex;
    fail = true;
  }
  Util::setGlobalSignalHandler(SIGINT, SIG_DFL, 0);
  Util::setGlobalSignalHandler(SIGTERM, SIG_DFL, 0);
  
  return nextReqInfos;
}
