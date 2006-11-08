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
#include "UrlRequestInfo.h"
#include "TorrentRequestInfo.h"
#include "MetalinkRequestInfo.h"
#include "prefs.h"
#include "DownloadEngineFactory.h"

extern void setSignalHander(int signal, void (*handler)(int), int flags);
extern volatile sig_atomic_t haltRequested;

void UrlRequestInfo::adjustRequestSize(Requests& requests,
				       Requests& reserved,
				       int maxConnections) const
{
  if(maxConnections > 0 && (int)requests.size() > maxConnections) {
    copy(requests.begin()+maxConnections, requests.end(),
	 back_inserter(reserved));
    //insert_iterator<Requests>(reserved, reserved.end()));
    requests.erase(requests.begin()+maxConnections, requests.end());
  }
}

RequestInfo* UrlRequestInfo::createNextRequestInfo() const
{
#ifdef ENABLE_BITTORRENT
  if(op->getAsBool(PREF_FOLLOW_TORRENT) &&
     Util::endsWith(fileInfo.filename, ".torrent")) {
    return new TorrentRequestInfo(fileInfo.filename, op);
  } else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
    if(op->getAsBool(PREF_FOLLOW_METALINK) &&
       Util::endsWith(fileInfo.filename, ".metalink")) {
      return new MetalinkRequestInfo(fileInfo.filename, op);
    } else
#endif // ENABLE_METALINK
      {
	return 0;
      }
}

void handler(int signal) {
  haltRequested = true;
}

class CreateRequest {
private:
  Requests* requestsPtr;
  string referer;
  int split;
public:
  CreateRequest(Requests* requestsPtr,
		const string& referer,
		int split)
    :requestsPtr(requestsPtr),
     referer(referer),
     split(split) {}

  void operator()(const string& url) {
    for(int s = 1; s <= split; s++) {
      RequestHandle req;
      req->setReferer(referer);
      if(req->setUrl(url)) {
	requestsPtr->push_back(req);
      } else {
	fprintf(stderr, _("Unrecognized URL or unsupported protocol: %s\n"),
		req->getUrl().c_str());
      }
    }
  }
};

void UrlRequestInfo::printUrls(const Strings& urls) const {
  for(Strings::const_iterator itr = urls.begin(); itr != urls.end(); itr++) {
    logger->notice("Adding URL: %s", itr->c_str());
  }
}

RequestInfos UrlRequestInfo::execute() {
  Requests requests;
  Requests reserved;
  printUrls(urls);
  for_each(urls.begin(), urls.end(),
	   CreateRequest(&requests,
			 op->get(PREF_REFERER),
			 op->getAsInt(PREF_SPLIT)));
  
  adjustRequestSize(requests, reserved, maxConnections);
  
  SharedHandle<ConsoleDownloadEngine> e(DownloadEngineFactory::newConsoleEngine(op, requests, reserved));
  
  setSignalHander(SIGINT, handler, 0);
  setSignalHander(SIGTERM, handler, 0);
  
  RequestInfo* next = 0;
  try {
    e->run();
    
    if(e->segmentMan->finished()) {
      printDownloadCompeleteMessage(e->segmentMan->getFilePath());
      fileInfo.filename = e->segmentMan->getFilePath();
      fileInfo.length = e->segmentMan->totalSize;
      fileInfo.checksum = checksum;
      
      next = createNextRequestInfo();
    } else {
      e->segmentMan->save();
      e->segmentMan->diskWriter->closeFile();
      printDownloadAbortMessage();
    }
  } catch(Exception *ex) {
    logger->error("Exception caught", ex);
    delete ex;
    fail = true;
  }
  RequestInfos nextReqInfos;
  if(next) {
    nextReqInfos.push_front(next);
  }
  setSignalHander(SIGINT, SIG_DFL, 0);
  setSignalHander(SIGTERM, SIG_DFL, 0);
  
  return nextReqInfos;
}
