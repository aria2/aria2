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
#include "LogFactory.h"
#include "wallclock.h"

namespace aria2 {

FileEntry::FileEntry(const std::string& path,
                     uint64_t length,
                     off_t offset,
                     const std::vector<std::string>& uris):
  path_(path), uris_(uris.begin(), uris.end()), length_(length),
  offset_(offset),
  requested_(true),
  singleHostMultiConnection_(true),
  lastFasterReplace_(0),
  logger_(LogFactory::getInstance()) {}

FileEntry::FileEntry():
  length_(0), offset_(0), requested_(false),
  singleHostMultiConnection_(true),
  logger_(LogFactory::getInstance()) {}

FileEntry::~FileEntry() {}

void FileEntry::setupDir()
{
  util::mkdirs(File(path_).getDirname());
}

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

off_t FileEntry::gtoloff(off_t goff) const
{
  assert(offset_ <= goff);
  return goff-offset_;
}

void FileEntry::getUris(std::vector<std::string>& uris) const
{
  uris.insert(uris.end(), spentUris_.begin(), spentUris_.end());
  uris.insert(uris.end(), uris_.begin(), uris_.end());
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
  if(requestPool_.empty()) {
    std::vector<std::string> pending;
    while(1) {
      std::string uri = selector->select(this);
      if(uri.empty()) {
        return req;
      }
      req.reset(new Request());
      if(req->setUri(uri)) {
        if(!singleHostMultiConnection_) {
          if(inFlightHost(inFlightRequests_.begin(), inFlightRequests_.end(),
                          req->getHost())) {
            pending.push_back(uri);
            req.reset();
            continue;
          }
        }
        req->setReferer(referer);
        req->setMethod(method);
        spentUris_.push_back(uri);
        inFlightRequests_.push_back(req);
        break;
      } else {
        req.reset();
      }
    }
    uris_.insert(uris_.begin(), pending.begin(), pending.end());
  } else {
    req = requestPool_.front();
    requestPool_.pop_front();
    inFlightRequests_.push_back(req);
  }
  return req;
}

SharedHandle<Request>
FileEntry::findFasterRequest(const SharedHandle<Request>& base)
{
  const int startupIdleTime = 10;
  if(requestPool_.empty() ||
     lastFasterReplace_.difference(global::wallclock) < startupIdleTime) {
    return SharedHandle<Request>();
  }
  const SharedHandle<PeerStat>& fastest = requestPool_.front()->getPeerStat();
  if(fastest.isNull()) {
    return SharedHandle<Request>();
  }
  const SharedHandle<PeerStat>& basestat = base->getPeerStat();
  // TODO hard coded value. See PREF_STARTUP_IDLE_TIME
  if(basestat.isNull() ||
     (basestat->getDownloadStartTime().
      difference(global::wallclock) >= startupIdleTime &&
      fastest->getAvgDownloadSpeed()*0.8 > basestat->calculateDownloadSpeed())){
    // TODO we should consider that "fastest" is very slow.
    SharedHandle<Request> fastestRequest = requestPool_.front();
    requestPool_.pop_front();
    inFlightRequests_.push_back(fastestRequest);
    lastFasterReplace_.reset();
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
    std::lower_bound(requestPool_.begin(), requestPool_.end(), request,
                     RequestFaster());
  requestPool_.insert(i, request);
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
  for(std::deque<SharedHandle<Request> >::iterator i =
        inFlightRequests_.begin(), eoi = inFlightRequests_.end();
      i != eoi; ++i) {
    if((*i).get() == request.get()) {
      inFlightRequests_.erase(i);
      return true;
    }
  }
  return false;
}

void FileEntry::removeURIWhoseHostnameIs(const std::string& hostname)
{
  std::deque<std::string> newURIs;
  Request req;
  for(std::deque<std::string>::const_iterator itr = uris_.begin(),
        eoi = uris_.end(); itr != eoi; ++itr) {
    if(((*itr).find(hostname) == std::string::npos) ||
       (req.setUri(*itr) && (req.getHost() != hostname))) {
      newURIs.push_back(*itr);
    }
  }
  if(logger_->debug()) {
    logger_->debug("Removed %d duplicate hostname URIs for path=%s",
                   uris_.size()-newURIs.size(), getPath().c_str());
  }
  uris_ = newURIs;
}

void FileEntry::removeIdenticalURI(const std::string& uri)
{
  uris_.erase(std::remove(uris_.begin(), uris_.end(), uri), uris_.end());
}

void FileEntry::addURIResult(std::string uri, downloadresultcode::RESULT result)
{
  uriResults_.push_back(URIResult(uri, result));
}

class FindURIResultByResult {
private:
  downloadresultcode::RESULT r_;
public:
  FindURIResultByResult(downloadresultcode::RESULT r):r_(r) {}

  bool operator()(const URIResult& uriResult) const
  {
    return uriResult.getResult() == r_;
  }
};

void FileEntry::extractURIResult
(std::deque<URIResult>& res, downloadresultcode::RESULT r)
{
  std::deque<URIResult>::iterator i =
    std::stable_partition(uriResults_.begin(), uriResults_.end(),
                          FindURIResultByResult(r));
  std::copy(uriResults_.begin(), i, std::back_inserter(res));
  uriResults_.erase(uriResults_.begin(), i);
}

void FileEntry::reuseUri(size_t num)
{
  std::deque<std::string> uris = spentUris_;
  std::sort(uris.begin(), uris.end());
  uris.erase(std::unique(uris.begin(), uris.end()), uris.end());

  std::vector<std::string> errorUris(uriResults_.size());
  std::transform(uriResults_.begin(), uriResults_.end(),
                 errorUris.begin(), std::mem_fun_ref(&URIResult::getURI));
  std::sort(errorUris.begin(), errorUris.end());
  errorUris.erase(std::unique(errorUris.begin(), errorUris.end()),
                  errorUris.end());
     
  std::vector<std::string> reusableURIs;
  std::set_difference(uris.begin(), uris.end(),
                      errorUris.begin(), errorUris.end(),
                      std::back_inserter(reusableURIs));
  size_t ininum = reusableURIs.size();
  if(logger_->debug()) {
    logger_->debug("Found %u reusable URIs", static_cast<unsigned int>(ininum));
    for(std::vector<std::string>::const_iterator i = reusableURIs.begin(),
          eoi = reusableURIs.end(); i != eoi; ++i) {
      logger_->debug("URI=%s", (*i).c_str());
    }
  }
  // Reuse at least num URIs here to avoid to
  // run this process repeatedly.
  if(ininum > 0) {
    for(size_t i = 0; i < num/ininum; ++i) {
      uris_.insert(uris_.end(), reusableURIs.begin(), reusableURIs.end());
    }
    uris_.insert(uris_.end(), reusableURIs.begin(),
                 reusableURIs.begin()+(num%ininum));
    if(logger_->debug()) {
      logger_->debug("Duplication complete: now %u URIs for reuse",
                     static_cast<unsigned int>(uris_.size()));
    }
  }
}

void FileEntry::releaseRuntimeResource()
{
  requestPool_.clear();
  inFlightRequests_.clear();
}

template<typename InputIterator, typename T>
static InputIterator findRequestByUri
(InputIterator first, InputIterator last, const T& uri)
{
  for(; first != last; ++first) {
    if(!(*first)->removalRequested() && (*first)->getUri() == uri) {
      return first;
    }
  }
  return last;
}

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
    std::deque<SharedHandle<Request> >::iterator riter =
      findRequestByUri(inFlightRequests_.begin(), inFlightRequests_.end(), uri);
    if(riter == inFlightRequests_.end()) {
      riter = findRequestByUri(requestPool_.begin(), requestPool_.end(), uri);
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
  if(Request().setUri(uri)) {
    uris_.push_back(uri);
    return true;
  } else {
    return false;
  }
}

bool FileEntry::insertUri(const std::string& uri, size_t pos)
{
  if(Request().setUri(uri)) {
    pos = std::min(pos, uris_.size());
    uris_.insert(uris_.begin()+pos, uri);
    return true;
  } else {
    return false;
  }
}

} // namespace aria2
