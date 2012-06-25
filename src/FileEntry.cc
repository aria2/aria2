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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include "Logger.h"
#include "LogFactory.h"
#include "wallclock.h"
#include "a2algo.h"
#include "uri.h"
#include "PeerStat.h"
#include "fmt.h"
#include "ServerStatMan.h"
#include "ServerStat.h"

namespace aria2 {

bool FileEntry::RequestFaster::operator()
  (const SharedHandle<Request>& lhs,
   const SharedHandle<Request>& rhs) const
{
  if(!lhs->getPeerStat()) {
    return false;
  }
  if(!rhs->getPeerStat()) {
    return true;
  }
  int lspd = lhs->getPeerStat()->getAvgDownloadSpeed();
  int rspd = rhs->getPeerStat()->getAvgDownloadSpeed();
  return lspd > rspd || (lspd == rspd && lhs.get() < rhs.get());
}

FileEntry::FileEntry
(const std::string& path,
 int64_t length,
 int64_t offset,
 const std::vector<std::string>& uris)
  : path_(path),
    uris_(uris.begin(), uris.end()),
    length_(length),
    offset_(offset),
    requested_(true),
    uniqueProtocol_(false),
    maxConnectionPerServer_(1),
    lastFasterReplace_(0)
{}

FileEntry::FileEntry()
 : length_(0),
   offset_(0),
   requested_(false),
   uniqueProtocol_(false),
   maxConnectionPerServer_(1)
{}

FileEntry::~FileEntry() {}

FileEntry& FileEntry::operator=(const FileEntry& entry)
{
  if(this != &entry) {
    path_ = entry.path_;
    length_ = entry.length_;
    offset_ = entry.offset_;
    requested_ = entry.requested_;
  }
  return *this;
}

bool FileEntry::operator<(const FileEntry& fileEntry) const
{
  return offset_ < fileEntry.offset_;
}

bool FileEntry::exists() const
{
  return File(getPath()).exists();
}

int64_t FileEntry::gtoloff(int64_t goff) const
{
  assert(offset_ <= goff);
  return goff-offset_;
}

void FileEntry::getUris(std::vector<std::string>& uris) const
{
  uris.insert(uris.end(), spentUris_.begin(), spentUris_.end());
  uris.insert(uris.end(), uris_.begin(), uris_.end());
}

namespace {
template<typename InputIterator, typename OutputIterator>
OutputIterator enumerateInFlightHosts
(InputIterator first, InputIterator last, OutputIterator out)
{
  for(; first != last; ++first) {
    uri::UriStruct us;
    if(uri::parse(us, (*first)->getUri())) {
      *out++ = us.host;
    }
  }
  return out;
}
} // namespace

SharedHandle<Request>
FileEntry::getRequest
(const SharedHandle<URISelector>& selector,
 bool uriReuse,
 const std::vector<std::pair<size_t, std::string> >& usedHosts,
 const std::string& referer,
 const std::string& method)
{
  SharedHandle<Request> req;
  if(requestPool_.empty()) {
    std::vector<std::string> inFlightHosts;
    enumerateInFlightHosts(inFlightRequests_.begin(), inFlightRequests_.end(),
                           std::back_inserter(inFlightHosts));
    for(int g = 0; g < 2; ++g) {
      std::vector<std::string> pending;
      std::vector<std::string> ignoreHost;
      while(1) {
        std::string uri = selector->select(this, usedHosts);
        if(uri.empty()) {
          break;
        }
        req.reset(new Request());
        if(req->setUri(uri)) {
          if(std::count(inFlightHosts.begin(),
                        inFlightHosts.end(),req->getHost())
             >= maxConnectionPerServer_) {
            pending.push_back(uri);
            ignoreHost.push_back(req->getHost());
            req.reset();
            continue;
          }
          req->setReferer(util::percentEncodeMini(referer));
          req->setMethod(method);
          spentUris_.push_back(uri);
          inFlightRequests_.insert(req);
          break;
        } else {
          req.reset();
        }
      }
      uris_.insert(uris_.begin(), pending.begin(), pending.end());
      if(g == 0 && uriReuse && !req && uris_.size() == pending.size()) {
        // Reuse URIs other than ones in pending
        reuseUri(ignoreHost);
      } else {
        break;
      }
    }
  } else {
    // Skip Request object if it is still
    // sleeping(Request::getWakeTime() < global::wallclock()).  If all
    // pooled objects are sleeping, return first one.  Caller should
    // inspect returned object's getWakeTime().
    RequestPool::iterator i = requestPool_.begin();
    RequestPool::iterator eoi = requestPool_.end();
    for(; i != eoi; ++i) {
      if((*i)->getWakeTime() <= global::wallclock()) {
        break;
      }
    }
    if(i == eoi) {
      i = requestPool_.begin();
    }
    req = *i;
    requestPool_.erase(i);
    inFlightRequests_.insert(req);
    A2_LOG_DEBUG(fmt("Picked up from pool: %s", req->getUri().c_str()));
  }
  return req;
}

SharedHandle<Request>
FileEntry::findFasterRequest(const SharedHandle<Request>& base)
{
  const int startupIdleTime = 10;
  if(requestPool_.empty() ||
     lastFasterReplace_.difference(global::wallclock()) < startupIdleTime) {
    return SharedHandle<Request>();
  }
  const SharedHandle<PeerStat>& fastest =
    (*requestPool_.begin())->getPeerStat();
  if(!fastest) {
    return SharedHandle<Request>();
  }
  const SharedHandle<PeerStat>& basestat = base->getPeerStat();
  // TODO hard coded value. See PREF_STARTUP_IDLE_TIME
  if(!basestat ||
     (basestat->getDownloadStartTime().
      difference(global::wallclock()) >= startupIdleTime &&
      fastest->getAvgDownloadSpeed()*0.8 > basestat->calculateDownloadSpeed())){
    // TODO we should consider that "fastest" is very slow.
    SharedHandle<Request> fastestRequest = *requestPool_.begin();
    requestPool_.erase(requestPool_.begin());
    inFlightRequests_.insert(fastestRequest);
    lastFasterReplace_ = global::wallclock();
    return fastestRequest;
  }
  return SharedHandle<Request>();
}

SharedHandle<Request>
FileEntry::findFasterRequest
(const SharedHandle<Request>& base,
 const std::vector<std::pair<size_t, std::string> >& usedHosts,
 const SharedHandle<ServerStatMan>& serverStatMan)
{
  const int startupIdleTime = 10;
  const int SPEED_THRESHOLD = 20*1024;
  if(lastFasterReplace_.difference(global::wallclock()) < startupIdleTime) {
    return SharedHandle<Request>();
  }
  std::vector<std::string> inFlightHosts;
  enumerateInFlightHosts(inFlightRequests_.begin(), inFlightRequests_.end(),
                         std::back_inserter(inFlightHosts));
  const SharedHandle<PeerStat>& basestat = base->getPeerStat();
  A2_LOG_DEBUG("Search faster server using ServerStat.");
  // Use first 10 good URIs to introduce some randomness.
  const size_t NUM_URI = 10;
  std::vector<std::pair<SharedHandle<ServerStat>, std::string> > fastCands;
  std::vector<std::string> normCands;
  for(std::deque<std::string>::const_iterator i = uris_.begin(),
        eoi = uris_.end(); i != eoi && fastCands.size() < NUM_URI; ++i) {
    uri::UriStruct us;
    if(!uri::parse(us, *i)) {
      continue;
    }
    if(std::count(inFlightHosts.begin(), inFlightHosts.end(),us.host)
       >= maxConnectionPerServer_) {
      A2_LOG_DEBUG(fmt("%s has already used %d times, not considered.",
                       (*i).c_str(),
                       maxConnectionPerServer_));
      continue;
    }
    if(findSecond(usedHosts.begin(), usedHosts.end(), us.host) !=
       usedHosts.end()) {
      A2_LOG_DEBUG(fmt("%s is in usedHosts, not considered", (*i).c_str()));
      continue;
    }
    SharedHandle<ServerStat> ss = serverStatMan->find(us.host, us.protocol);
    if(ss && ss->isOK()) {
      if((basestat &&
          ss->getDownloadSpeed() > basestat->calculateDownloadSpeed()*1.5) ||
         (!basestat && ss->getDownloadSpeed() > SPEED_THRESHOLD)) {
        fastCands.push_back(std::make_pair(ss, *i));
      }
    }
  }
  if(!fastCands.empty()) {
    std::sort(fastCands.begin(), fastCands.end(), ServerStatFaster());
    SharedHandle<Request> fastestRequest(new Request());
    const std::string& uri = fastCands.front().second;
    A2_LOG_DEBUG(fmt("Selected %s from fastCands", uri.c_str()));
    fastestRequest->setUri(uri);
    fastestRequest->setReferer(base->getReferer());
    uris_.erase(std::find(uris_.begin(), uris_.end(), uri));
    spentUris_.push_back(uri);
    inFlightRequests_.insert(fastestRequest);
    lastFasterReplace_ = global::wallclock();
    return fastestRequest;
  }
  A2_LOG_DEBUG("No faster server found.");
  return SharedHandle<Request>();
}

void FileEntry::storePool(const SharedHandle<Request>& request)
{
  const SharedHandle<PeerStat>& peerStat = request->getPeerStat();
  if(peerStat) {
    // We need to calculate average download speed here in order to
    // store Request in the right position in the pool.
    peerStat->calculateAvgDownloadSpeed();
  }
  requestPool_.insert(request);
}

void FileEntry::poolRequest(const SharedHandle<Request>& request)
{
  removeRequest(request);
  if(!request->removalRequested()) {
    storePool(request);
  }
}

bool FileEntry::removeRequest(const SharedHandle<Request>& request)
{
  return inFlightRequests_.erase(request) == 1;
}

void FileEntry::removeURIWhoseHostnameIs(const std::string& hostname)
{
  std::deque<std::string> newURIs;
  for(std::deque<std::string>::const_iterator itr = uris_.begin(),
        eoi = uris_.end(); itr != eoi; ++itr) {
    uri::UriStruct us;
    if(!uri::parse(us, *itr)) {
      continue;
    }
    if(us.host != hostname) {
      newURIs.push_back(*itr);
    }
  }
  A2_LOG_DEBUG(fmt("Removed %lu duplicate hostname URIs for path=%s",
                   static_cast<unsigned long>(uris_.size()-newURIs.size()),
                   getPath().c_str()));
  uris_.swap(newURIs);
}

void FileEntry::removeIdenticalURI(const std::string& uri)
{
  uris_.erase(std::remove(uris_.begin(), uris_.end(), uri), uris_.end());
}

void FileEntry::addURIResult(std::string uri, error_code::Value result)
{
  uriResults_.push_back(URIResult(uri, result));
}

namespace {
class FindURIResultByResult {
private:
  error_code::Value r_;
public:
  FindURIResultByResult(error_code::Value r):r_(r) {}

  bool operator()(const URIResult& uriResult) const
  {
    return uriResult.getResult() == r_;
  }
};
} // namespace

void FileEntry::extractURIResult
(std::deque<URIResult>& res, error_code::Value r)
{
  std::deque<URIResult>::iterator i =
    std::stable_partition(uriResults_.begin(), uriResults_.end(),
                          FindURIResultByResult(r));
  std::copy(uriResults_.begin(), i, std::back_inserter(res));
  uriResults_.erase(uriResults_.begin(), i);
}

void FileEntry::reuseUri(const std::vector<std::string>& ignore)
{
  if(A2_LOG_DEBUG_ENABLED) {
    for(std::vector<std::string>::const_iterator i = ignore.begin(),
          eoi = ignore.end(); i != eoi; ++i) {
      A2_LOG_DEBUG(fmt("ignore host=%s", (*i).c_str()));
    }
  }
  std::deque<std::string> uris = spentUris_;
  std::sort(uris.begin(), uris.end());
  uris.erase(std::unique(uris.begin(), uris.end()), uris.end());

  std::vector<std::string> errorUris(uriResults_.size());
  std::transform(uriResults_.begin(), uriResults_.end(),
                 errorUris.begin(), std::mem_fun_ref(&URIResult::getURI));
  std::sort(errorUris.begin(), errorUris.end());
  errorUris.erase(std::unique(errorUris.begin(), errorUris.end()),
                  errorUris.end());
  if(A2_LOG_DEBUG_ENABLED) {
    for(std::vector<std::string>::const_iterator i = errorUris.begin(),
          eoi = errorUris.end(); i != eoi; ++i) {
      A2_LOG_DEBUG(fmt("error URI=%s", (*i).c_str()));
    }
  }
  std::vector<std::string> reusableURIs;
  std::set_difference(uris.begin(), uris.end(),
                      errorUris.begin(), errorUris.end(),
                      std::back_inserter(reusableURIs));
  std::vector<std::string>::iterator insertionPoint = reusableURIs.begin();
  for(std::vector<std::string>::iterator i = reusableURIs.begin(),
        eoi = reusableURIs.end(); i != eoi; ++i) {
    uri::UriStruct us;
    if(uri::parse(us, *i) &&
       std::find(ignore.begin(), ignore.end(), us.host) == ignore.end()) {
      if(i != insertionPoint) {
        *insertionPoint = *i;
      }
      ++insertionPoint;
    }
  }
  reusableURIs.erase(insertionPoint, reusableURIs.end());
  size_t ininum = reusableURIs.size();
  if(A2_LOG_DEBUG_ENABLED) {
    A2_LOG_DEBUG(fmt("Found %u reusable URIs",
                     static_cast<unsigned int>(ininum)));
    for(std::vector<std::string>::const_iterator i = reusableURIs.begin(),
          eoi = reusableURIs.end(); i != eoi; ++i) {
      A2_LOG_DEBUG(fmt("URI=%s", (*i).c_str()));
    }
  }
  uris_.insert(uris_.end(), reusableURIs.begin(), reusableURIs.end());
}

void FileEntry::releaseRuntimeResource()
{
  requestPool_.clear();
  inFlightRequests_.clear();
}

namespace {
template<typename InputIterator>
void putBackUri
(std::deque<std::string>& uris,
 InputIterator first,
 InputIterator last)
{
  for(; first != last; ++first) {
    uris.push_front((*first)->getUri());
  }
}
} // namespace

void FileEntry::putBackRequest()
{
  putBackUri(uris_, requestPool_.begin(), requestPool_.end());
  putBackUri(uris_, inFlightRequests_.begin(), inFlightRequests_.end());
}

namespace {
template<typename InputIterator, typename T>
InputIterator findRequestByUri
(InputIterator first, InputIterator last, const T& uri)
{
  for(; first != last; ++first) {
    if(!(*first)->removalRequested() && (*first)->getUri() == uri) {
      return first;
    }
  }
  return last;
}
} // namespace

bool FileEntry::removeUri(const std::string& uri)
{
  std::deque<std::string>::iterator itr =
    std::find(spentUris_.begin(), spentUris_.end(), uri);
  if(itr == spentUris_.end()) {
    itr = std::find(uris_.begin(), uris_.end(), uri);
    if(itr == uris_.end()) {
      return false;
    } else {
      uris_.erase(itr);
      return true;
    }
  } else {
    spentUris_.erase(itr);
    SharedHandle<Request> req;
    InFlightRequestSet::iterator riter =
      findRequestByUri(inFlightRequests_.begin(), inFlightRequests_.end(), uri);
    if(riter == inFlightRequests_.end()) {
      RequestPool::iterator riter = findRequestByUri(requestPool_.begin(),
                                                     requestPool_.end(), uri);
      if(riter == requestPool_.end()) {
        return true;
      } else {
        req = *riter;
        requestPool_.erase(riter);
      }
    } else {
      req = *riter;
    }
    req->requestRemoval();
    return true;
  }
}

std::string FileEntry::getBasename() const
{
  return File(path_).getBasename();
}

std::string FileEntry::getDirname() const
{
  return File(path_).getDirname();
}

size_t FileEntry::setUris(const std::vector<std::string>& uris)
{
  uris_.clear();
  return addUris(uris.begin(), uris.end());
}

bool FileEntry::addUri(const std::string& uri)
{
  uri::UriStruct us;
  std::string peUri = util::percentEncodeMini(uri);
  if(uri::parse(us, peUri)) {
    uris_.push_back(peUri);
    return true;
  } else {
    return false;
  }
}

bool FileEntry::insertUri(const std::string& uri, size_t pos)
{
  uri::UriStruct us;
  std::string peUri = util::percentEncodeMini(uri);
  if(uri::parse(us, peUri)) {
    pos = std::min(pos, uris_.size());
    uris_.insert(uris_.begin()+pos, peUri);
    return true;
  } else {
    return false;
  }
}

void FileEntry::setPath(const std::string& path)
{
  path_ = path;
}

void FileEntry::setContentType(const std::string& contentType)
{
  contentType_ = contentType;
}

size_t FileEntry::countInFlightRequest() const
{
  return inFlightRequests_.size();
}

size_t FileEntry::countPooledRequest() const
{
  return requestPool_.size();
}

void FileEntry::setOriginalName(const std::string& originalName)
{
  originalName_ = originalName;
}

bool FileEntry::emptyRequestUri() const
{
  return uris_.empty() && inFlightRequests_.empty() && requestPool_.empty();
}

void writeFilePath
(std::ostream& o,
 const SharedHandle<FileEntry>& entry,
 bool memory)
{
  if(entry->getPath().empty()) {
    std::vector<std::string> uris;
    entry->getUris(uris);
    if(uris.empty()) {
      o << "n/a";
    } else {
      o << uris.front();
    }
  } else {
    if(memory) {
      o << "[MEMORY]" << File(entry->getPath()).getBasename();
    } else {
      o << entry->getPath();
    }
  }
}

} // namespace aria2
