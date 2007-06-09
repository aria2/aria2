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
#include "RecoverableException.h"
#include "FatalException.h"
#include "message.h"
#include "RequestFactory.h"
#include "GlowFileAllocator.h"
#include "File.h"
#include "DefaultDiskWriter.h"
#include "DlAbortEx.h"
#include "DNSCache.h"

std::ostream& operator<<(std::ostream& o, const HeadResult& hr) {
  o << "filename = " << hr.filename << ", " << "totalLength = " << hr.totalLength;
  return o;
}

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
/*
class CreateRequest {
private:
  Requests* requestsPtr;
  string referer;
  int split;
  string method;
public:
  CreateRequest(Requests* requestsPtr,
		const string& referer,
		int split,
		const string& method = Request::METHOD_GET)
    :requestsPtr(requestsPtr),
     referer(referer),
     split(split),
     method(method)
  {}

  void operator()(const string& url) {
    for(int s = 1; s <= split; s++) {
      RequestHandle req = RequestFactorySingletonHolder::instance()->createRequest();
      req->setReferer(referer);
      req->setMethod(method);
      if(req->setUrl(url)) {
	requestsPtr->push_back(req);
      } else {
	fprintf(stderr, _("Unrecognized URL or unsupported protocol: %s\n"),
		req->getUrl().c_str());
      }
    }
  }
};
*/
void UrlRequestInfo::printUrls(const Strings& urls) const {
  for(Strings::const_iterator itr = urls.begin(); itr != urls.end(); itr++) {
    logger->info("Adding URL: %s", itr->c_str());
  }
}
/*
HeadResultHandle UrlRequestInfo::getHeadResult() {
  Requests requests;
  for_each(urls.begin(), urls.end(),
	   CreateRequest(&requests,
			 op->get(PREF_REFERER),
			 1,
			 Request::METHOD_HEAD));
  if(requests.size() == 0) {
    return 0;
  }
  Requests reserved(requests.begin()+1, requests.end());
  requests.erase(requests.begin()+1, requests.end());

  SharedHandle<ConsoleDownloadEngine> e(DownloadEngineFactory::newConsoleEngine(op, requests, reserved));

  HeadResultHandle hr = 0;
  
  e->run();
  hr = new HeadResult();
  hr->filename = e->segmentMan->filename;
  hr->totalLength = e->segmentMan->totalSize;

  return hr;
}
*/

RequestInfos UrlRequestInfo::execute() {
  {
    DNSCacheHandle dnsCache = new SimpleDNSCache();
    DNSCacheSingletonHolder::instance(dnsCache);
  }

  RequestInfo* next = 0;

  try {
    RequestGroups requestGroups;

    Strings urls;
    urls.push_back("http://localhost/~tujikawa/linux-2.6.6.tar.bz2");    
    RequestGroupHandle rg1 = new RequestGroup(urls, op);

    Strings urls2;
    urls2.push_back("http://localhost/~tujikawa/linux-2.6.19.1.tar.bz2");
    RequestGroupHandle rg2 = new RequestGroup(urls2, op);

    requestGroups.push_back(rg1);
    requestGroups.push_back(rg2);
    
    SharedHandle<ConsoleDownloadEngine> e(DownloadEngineFactory::newConsoleEngine(op, requestGroups));


    Strings reservedUrls1;
    reservedUrls1.push_back("http://localhost/~tujikawa/linux-2.6.1.tar.bz2");

    RequestGroupHandle rrg1 = new RequestGroup(reservedUrls1, op);

    e->_requestGroupMan->addReservedGroup(rrg1);
    
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
    
    if(e->_requestGroupMan->downloadFinished()) {
      next = createNextRequestInfo();
    } else {
      e->_requestGroupMan->save();
      e->_requestGroupMan->closeFile();
      printDownloadAbortMessage();
    }
  } catch(RecoverableException *ex) {
    logger->error("Exception caught", ex);
    delete ex;
    fail = true;
  }
  RequestInfos nextReqInfos;
  if(next) {
    nextReqInfos.push_front(next);
  }
  Util::setGlobalSignalHandler(SIGINT, SIG_DFL, 0);
  Util::setGlobalSignalHandler(SIGTERM, SIG_DFL, 0);
  
  return nextReqInfos;
}
