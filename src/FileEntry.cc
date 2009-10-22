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
#include "FileEntry.h"

#include <cassert>
#include <algorithm>

#include "util.h"
#include "URISelector.h"
#include "LogFactory.h"

namespace aria2 {

FileEntry::FileEntry(const std::string& path,
		     uint64_t length,
		     off_t offset,
		     const std::deque<std::string>& uris):
  path(path), _uris(uris), length(length), offset(offset),
  extracted(false), requested(true),
  _singleHostMultiConnection(true),
  _logger(LogFactory::getInstance()) {}

FileEntry::FileEntry():
  length(0), offset(0), extracted(false), requested(false),
  _singleHostMultiConnection(true),
  _logger(LogFactory::getInstance()) {}

FileEntry::~FileEntry() {}

void FileEntry::setupDir()
{
  util::mkdirs(File(path).getDirname());
}

FileEntry& FileEntry::operator=(const FileEntry& entry)
{
  if(this != &entry) {
    path = entry.path;
    length = entry.length;
    offset = entry.offset;
    extracted = entry.extracted;
    requested = entry.requested;
  }
  return *this;
}

bool FileEntry::operator<(const FileEntry& fileEntry) const
{
  return offset < fileEntry.offset;
}

bool FileEntry::exists() const
{
  return File(getPath()).exists();
}

off_t FileEntry::gtoloff(off_t goff) const
{
  assert(offset <= goff);
  return goff-offset;
}

void FileEntry::getUris(std::deque<std::string>& uris) const
{
  uris.insert(uris.end(), _spentUris.begin(), _spentUris.end());
  uris.insert(uris.end(), _uris.begin(), _uris.end());
}

std::string FileEntry::selectUri(const SharedHandle<URISelector>& uriSelector)
{
  return uriSelector->select(this);
}

template<typename InputIterator>
static bool inFlightHost(InputIterator first, InputIterator last,
			 const std::string& hostname)
{
  // TODO redirection should be considered here. We need to parse
  // original URI to get hostname.
  for(; first != last; ++first) {
    if((*first)->getHost() == hostname) {
      return true;
    }
  }
  return false;
}

SharedHandle<Request>
FileEntry::getRequest
(const SharedHandle<URISelector>& selector,
 const std::string& referer,
 const std::string& method)
{
  SharedHandle<Request> req;
  if(_requestPool.empty()) {
    std::deque<std::string> pending;
    while(1) {
      std::string uri = selector->select(this);
      if(uri.empty()) {
	return req;
      }
      req.reset(new Request());
      if(req->setUrl(uri)) {
	if(!_singleHostMultiConnection) {
	  if(inFlightHost(_inFlightRequests.begin(), _inFlightRequests.end(),
			  req->getHost())) {
	    pending.push_back(uri);
	    req.reset();
	    continue;
	  }
	}
	req->setReferer(referer);
	req->setMethod(method);
	_spentUris.push_back(uri);
	_inFlightRequests.push_back(req);
	break;
      } else {
	req.reset();
      }
    }
    _uris.insert(_uris.begin(), pending.begin(), pending.end());
  } else {
    req = _requestPool.front();
    _requestPool.pop_front();
    _inFlightRequests.push_back(req);
  }
  return req;
}

SharedHandle<Request>
FileEntry::findFasterRequest(const SharedHandle<Request>& base)
{
  if(_requestPool.empty()) {
    return SharedHandle<Request>();
  }
  const SharedHandle<PeerStat>& fastest = _requestPool.front()->getPeerStat();
  if(fastest.isNull()) {
    return SharedHandle<Request>();
  }
  const SharedHandle<PeerStat>& basestat = base->getPeerStat();
  // TODO hard coded value. See PREF_STARTUP_IDLE_TIME
  const int startupIdleTime = 10;
  if(basestat.isNull() ||
     (basestat->getDownloadStartTime().elapsed(startupIdleTime) &&
      fastest->getAvgDownloadSpeed()*0.8 > basestat->calculateDownloadSpeed())){
    // TODO we should consider that "fastest" is very slow.
    SharedHandle<Request> fastestRequest = _requestPool.front();
    _requestPool.pop_front();
    return fastestRequest;
  }
  return SharedHandle<Request>();
}

class RequestFaster {
public:
  bool operator()(const SharedHandle<Request>& lhs,
		  const SharedHandle<Request>& rhs) const
  {
    if(lhs->getPeerStat().isNull()) {
      return false;
    }
    if(rhs->getPeerStat().isNull()) {
      return true;
    }
    return
      lhs->getPeerStat()->getAvgDownloadSpeed() > rhs->getPeerStat()->getAvgDownloadSpeed();
  }
};

void FileEntry::storePool(const SharedHandle<Request>& request)
{
  const SharedHandle<PeerStat>& peerStat = request->getPeerStat();
  if(!peerStat.isNull()) {
    // We need to calculate average download speed here in order to
    // store Request in the right position in the pool.
    peerStat->calculateAvgDownloadSpeed();
  }
  std::deque<SharedHandle<Request> >::iterator i =
    std::lower_bound(_requestPool.begin(), _requestPool.end(), request,
		     RequestFaster());
  _requestPool.insert(i, request);
}

void FileEntry::poolRequest(const SharedHandle<Request>& request)
{
  removeRequest(request);
  storePool(request);
}

bool FileEntry::removeRequest(const SharedHandle<Request>& request)
{
  for(std::deque<SharedHandle<Request> >::iterator i =
	_inFlightRequests.begin(); i != _inFlightRequests.end(); ++i) {
    if((*i).get() == request.get()) {
      _inFlightRequests.erase(i);
      return true;
    }
  }
  return false;
}

void FileEntry::removeURIWhoseHostnameIs(const std::string& hostname)
{
  std::deque<std::string> newURIs;
  Request req;
  for(std::deque<std::string>::const_iterator itr = _uris.begin(); itr != _uris.end(); ++itr) {
    if(((*itr).find(hostname) == std::string::npos) ||
       (req.setUrl(*itr) && (req.getHost() != hostname))) {
      newURIs.push_back(*itr);
    }
  }
  _logger->debug("Removed %d duplicate hostname URIs for path=%s",
		 _uris.size()-newURIs.size(), getPath().c_str());
  _uris = newURIs;
}

void FileEntry::removeIdenticalURI(const std::string& uri)
{
  _uris.erase(std::remove(_uris.begin(), _uris.end(), uri), _uris.end());
}

void FileEntry::addURIResult(std::string uri, downloadresultcode::RESULT result)
{
  _uriResults.push_back(URIResult(uri, result));
}

class FindURIResultByResult {
private:
  downloadresultcode::RESULT _r;
public:
  FindURIResultByResult(downloadresultcode::RESULT r):_r(r) {}

  bool operator()(const URIResult& uriResult) const
  {
    return uriResult.getResult() == _r;
  }
};

void FileEntry::extractURIResult
(std::deque<URIResult>& res, downloadresultcode::RESULT r)
{
  std::deque<URIResult>::iterator i =
    std::stable_partition(_uriResults.begin(), _uriResults.end(),
			  FindURIResultByResult(r));
  std::copy(_uriResults.begin(), i, std::back_inserter(res));
  _uriResults.erase(_uriResults.begin(), i);
}

void FileEntry::reuseUri(size_t num)
{
  std::deque<std::string> uris = _spentUris;
  std::sort(uris.begin(), uris.end());
  uris.erase(std::unique(uris.begin(), uris.end()), uris.end());

  std::deque<std::string> errorUris(_uriResults.size());
  std::transform(_uriResults.begin(), _uriResults.end(),
		 errorUris.begin(), std::mem_fun_ref(&URIResult::getURI));
  std::sort(errorUris.begin(), errorUris.end());
  errorUris.erase(std::unique(errorUris.begin(), errorUris.end()),
		  errorUris.end());
     
  std::deque<std::string> reusableURIs;
  std::set_difference(uris.begin(), uris.end(),
		      errorUris.begin(), errorUris.end(),
		      std::back_inserter(reusableURIs));
  size_t ininum = reusableURIs.size();
  _logger->debug("Found %u reusable URIs",
		 static_cast<unsigned int>(ininum));
  // Reuse at least num URIs here to avoid to
  // run this process repeatedly.
  if(ininum > 0 && ininum < num) {
    _logger->debug("fewer than num=%u",
		   num);
    for(size_t i = 0; i < num/ininum; ++i) {
      _uris.insert(_uris.end(), reusableURIs.begin(), reusableURIs.end());
    }
    _uris.insert(_uris.end(), reusableURIs.begin(),
		 reusableURIs.begin()+(num%ininum));
    _logger->debug("Duplication complete: now %u URIs for reuse",
		   static_cast<unsigned int>(_uris.size()));
  }
}

void FileEntry::releaseRuntimeResource()
{
  _requestPool.clear();
  _inFlightRequests.clear();
  _uriResults.clear();
}

} // namespace aria2
