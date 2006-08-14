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
#include "UrlRequestInfo.h"
#include "TorrentRequestInfo.h"
#include "MetalinkRequestInfo.h"
#include "prefs.h"
#include "DownloadEngineFactory.h"

extern RequestInfo* requestInfo;
extern void setSignalHander(int signal, void (*handler)(int), int flags);

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
  printf(_("\nstopping application...\n"));
  fflush(stdout);
  requestInfo->getDownloadEngine()->segmentMan->save();
  requestInfo->getDownloadEngine()->segmentMan->diskWriter->closeFile();
  delete requestInfo->getDownloadEngine();
  printf(_("done\n"));
  exit(EXIT_SUCCESS);
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
      Request* req = new Request();
      req->setReferer(referer);
      if(req->setUrl(url)) {
	requestsPtr->push_back(req);
      } else {
	fprintf(stderr, _("Unrecognized URL or unsupported protocol: %s\n"),
		req->getUrl().c_str());
	delete req;
      }
    }
  }
};

RequestInfo* UrlRequestInfo::execute() {
  Requests requests;
  Requests reserved;
  for_each(urls.begin(), urls.end(),
	   CreateRequest(&requests,
			 op->get(PREF_REFERER),
			 op->getAsInt(PREF_SPLIT)));
  
  adjustRequestSize(requests, reserved, maxConnections);
  
  e = DownloadEngineFactory::newConsoleEngine(op, requests, reserved);
  
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
  } catch(Exception *e) {
    logger->error("Exception caught", e);
    delete e;
    fail = true;
  }
  for_each(requests.begin(), requests.end(), Deleter());
  for_each(reserved.begin(), reserved.end(), Deleter());
  
  setSignalHander(SIGINT, SIG_DFL, 0);
  setSignalHander(SIGTERM, SIG_DFL, 0);
  
  delete e;
  e = 0;
  
  return next;
}
