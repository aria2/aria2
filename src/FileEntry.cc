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

bool FileEntry::RequestFaster::operator()(
    const std::shared_ptr<Request>& lhs,
    const std::shared_ptr<Request>& rhs) const
{
  if (!lhs->getPeerStat()) {
    return false;
  }
  if (!rhs->getPeerStat()) {
    return true;
  }
  int lspd = lhs->getPeerStat()->getAvgDownloadSpeed();
  int rspd = rhs->getPeerStat()->getAvgDownloadSpeed();
  return lspd > rspd || (lspd == rspd && lhs.get() < rhs.get());
}

FileEntry::FileEntry(std::string path, int64_t length, int64_t offset,
                     const std::vector<std::string>& uris)
    : length_(length),
      offset_(offset),
      uris_(uris.begin(), uris.end()),
      path_(std::move(path)),
      lastFasterReplace_(Timer::zero()),
      maxConnectionPerServer_(1),
      requested_(true),
      uniqueProtocol_(false)
{
}

FileEntry::FileEntry()
    : length_(0),
      offset_(0),
      maxConnectionPerServer_(1),
      requested_(false),
      uniqueProtocol_(false)
{
}

FileEntry::~FileEntry() = default;

bool FileEntry::operator<(const FileEntry& fileEntry) const
{
  return offset_ < fileEntry.offset_;
}

bool FileEntry::exists() const { return File(getPath()).exists(); }

int64_t FileEntry::gtoloff(int64_t goff) const
{
  assert(offset_ <= goff);
  return goff - offset_;
}

std::vector<std::string> FileEntry::getUris() const
{
  std::vector<std::string> uris(std::begin(spentUris_), std::end(spentUris_));
  uris.insert(std::end(uris), std::begin(uris_), std::end(uris_));
  return uris;
}

namespace {
template <typename InputIterator, typename OutputIterator>
OutputIterator enumerateInFlightHosts(InputIterator first, InputIterator last,
                                      OutputIterator out)
{
  for (; first != last; ++first) {
    uri_split_result us;
    if (uri_split(&us, (*first)->getUri().c_str()) == 0) {
      *out++ = uri::getFieldString(us, USR_HOST, (*first)->getUri().c_str());
    }
  }
  return out;
}
} // namespace

std::shared_ptr<Request> FileEntry::getRequestWithInFlightHosts(
    URISelector* selector, bool uriReuse,
    const std::vector<std::pair<size_t, std::string>>& usedHosts,
    const std::string& referer, const std::string& method,
    const std::vector<std::string>& inFlightHosts)
{
  std::shared_ptr<Request> req;

  for (int g = 0; g < 2; ++g) {
    std::vector<std::string> pending;
    std::vector<std::string> ignoreHost;
    while (1) {
      std::string uri = selector->select(this, usedHosts);
      if (uri.empty()) {
        break;
      }
      req = std::make_shared<Request>();
      if (req->setUri(uri)) {
        if (std::count(std::begin(inFlightHosts), std::end(inFlightHosts),
                       req->getHost()) >= maxConnectionPerServer_) {
          pending.push_back(uri);
          ignoreHost.push_back(req->getHost());
          req.reset();
          continue;
        }
        if (referer == "*") {
          // Assuming uri has already been percent-encoded.
          req->setReferer(uri);
        }
        else {
          req->setReferer(util::percentEncodeMini(referer));
        }
        req->setMethod(method);
        spentUris_.push_back(uri);
        inFlightRequests_.insert(req);
        break;
      }
      else {
        req.reset();
      }
    }
    uris_.insert(std::begin(uris_), std::begin(pending), std::end(pending));
    if (g == 0 && uriReuse && !req && uris_.size() == pending.size()) {
      // Reuse URIs other than ones in pending
      reuseUri(ignoreHost);
      continue;
    }

    break;
  }
  return req;
}

std::shared_ptr<Request> FileEntry::getRequest(
    URISelector* selector, bool uriReuse,
    const std::vector<std::pair<size_t, std::string>>& usedHosts,
    const std::string& referer, const std::string& method)
{
  std::shared_ptr<Request> req;
  if (requestPool_.empty()) {
    std::vector<std::string> inFlightHosts;
    enumerateInFlightHosts(std::begin(inFlightRequests_),
                           std::end(inFlightRequests_),
                           std::back_inserter(inFlightHosts));
    return getRequestWithInFlightHosts(selector, uriReuse, usedHosts, referer,
                                       method, inFlightHosts);
  }

  // Skip Request object if it is still
  // sleeping(Request::getWakeTime() < global::wallclock()).  If all
  // pooled objects are sleeping, we may return first one.  Caller
  // should inspect returned object's getWakeTime().
  auto i = std::begin(requestPool_);
  for (; i != std::end(requestPool_); ++i) {
    if ((*i)->getWakeTime() <= global::wallclock()) {
      break;
    }
  }
  if (i == std::end(requestPool_)) {
    // all requests are sleeping; try to another URI
    std::vector<std::string> inFlightHosts;
    enumerateInFlightHosts(std::begin(inFlightRequests_),
                           std::end(inFlightRequests_),
                           std::back_inserter(inFlightHosts));
    enumerateInFlightHosts(std::begin(requestPool_), std::end(requestPool_),
                           std::back_inserter(inFlightHosts));

    req = getRequestWithInFlightHosts(selector, uriReuse, usedHosts, referer,
                                      method, inFlightHosts);
    if (!req || req->getUri() == (*std::begin(requestPool_))->getUri()) {
      i = std::begin(requestPool_);
    }
  }

  if (i != std::end(requestPool_)) {
    req = *i;
    requestPool_.erase(i);
    A2_LOG_DEBUG(fmt("Picked up from pool: %s", req->getUri().c_str()));
  }

  inFlightRequests_.insert(req);

  return req;
}

namespace {
constexpr auto startupIdleTime = 10_s;
} // namespace

std::shared_ptr<Request>
FileEntry::findFasterRequest(const std::shared_ptr<Request>& base)
{
  if (requestPool_.empty() ||
      lastFasterReplace_.difference(global::wallclock()) < startupIdleTime) {
    return nullptr;
  }
  const std::shared_ptr<PeerStat>& fastest =
      (*requestPool_.begin())->getPeerStat();
  if (!fastest) {
    return nullptr;
  }
  const std::shared_ptr<PeerStat>& basestat = base->getPeerStat();
  // TODO hard coded value. See PREF_STARTUP_IDLE_TIME
  if (!basestat || (basestat->getDownloadStartTime().difference(
                        global::wallclock()) >= startupIdleTime &&
                    fastest->getAvgDownloadSpeed() * 0.8 >
                        basestat->calculateDownloadSpeed())) {
    // TODO we should consider that "fastest" is very slow.
    std::shared_ptr<Request> fastestRequest = *requestPool_.begin();
    requestPool_.erase(requestPool_.begin());
    inFlightRequests_.insert(fastestRequest);
    lastFasterReplace_ = global::wallclock();
    return fastestRequest;
  }
  return nullptr;
}

std::shared_ptr<Request> FileEntry::findFasterRequest(
    const std::shared_ptr<Request>& base,
    const std::vector<std::pair<size_t, std::string>>& usedHosts,
    const std::shared_ptr<ServerStatMan>& serverStatMan)
{
  constexpr int SPEED_THRESHOLD = 20_k;
  if (lastFasterReplace_.difference(global::wallclock()) < startupIdleTime) {
    return nullptr;
  }
  std::vector<std::string> inFlightHosts;
  enumerateInFlightHosts(inFlightRequests_.begin(), inFlightRequests_.end(),
                         std::back_inserter(inFlightHosts));
  const std::shared_ptr<PeerStat>& basestat = base->getPeerStat();
  A2_LOG_DEBUG("Search faster server using ServerStat.");
  // Use first 10 good URIs to introduce some randomness.
  const size_t NUM_URI = 10;
  std::vector<std::pair<std::shared_ptr<ServerStat>, std::string>> fastCands;
  std::vector<std::string> normCands;
  for (std::deque<std::string>::const_iterator i = uris_.begin(),
                                               eoi = uris_.end();
       i != eoi && fastCands.size() < NUM_URI; ++i) {
    uri_split_result us;
    if (uri_split(&us, (*i).c_str()) == -1) {
      continue;
    }
    std::string host = uri::getFieldString(us, USR_HOST, (*i).c_str());
    std::string protocol = uri::getFieldString(us, USR_SCHEME, (*i).c_str());
    if (std::count(inFlightHosts.begin(), inFlightHosts.end(), host) >=
        maxConnectionPerServer_) {
      A2_LOG_DEBUG(fmt("%s has already used %d times, not considered.",
                       (*i).c_str(), maxConnectionPerServer_));
      continue;
    }
    if (findSecond(usedHosts.begin(), usedHosts.end(), host) !=
        usedHosts.end()) {
      A2_LOG_DEBUG(fmt("%s is in usedHosts, not considered", (*i).c_str()));
      continue;
    }
    std::shared_ptr<ServerStat> ss = serverStatMan->find(host, protocol);
    if (ss && ss->isOK()) {
      if ((basestat &&
           ss->getDownloadSpeed() > basestat->calculateDownloadSpeed() * 1.5) ||
          (!basestat && ss->getDownloadSpeed() > SPEED_THRESHOLD)) {
        fastCands.push_back(std::make_pair(ss, *i));
      }
    }
  }
  if (!fastCands.empty()) {
    std::sort(fastCands.begin(), fastCands.end(), ServerStatFaster());
    auto fastestRequest = std::make_shared<Request>();
    const std::string& uri = fastCands.front().second;
    A2_LOG_DEBUG(fmt("Selected %s from fastCands", uri.c_str()));
    // Candidate URIs where already parsed when populating fastCands.
    (void)fastestRequest->setUri(uri);
    fastestRequest->setReferer(base->getReferer());
    uris_.erase(std::find(uris_.begin(), uris_.end(), uri));
    spentUris_.push_back(uri);
    inFlightRequests_.insert(fastestRequest);
    lastFasterReplace_ = global::wallclock();
    return fastestRequest;
  }
  A2_LOG_DEBUG("No faster server found.");
  return nullptr;
}

void FileEntry::storePool(const std::shared_ptr<Request>& request)
{
  const std::shared_ptr<PeerStat>& peerStat = request->getPeerStat();
  if (peerStat) {
    // We need to calculate average download speed here in order to
    // store Request in the right position in the pool.
    peerStat->calculateAvgDownloadSpeed();
  }
  requestPool_.insert(request);
}

void FileEntry::poolRequest(const std::shared_ptr<Request>& request)
{
  removeRequest(request);
  if (!request->removalRequested()) {
    storePool(request);
  }
}

bool FileEntry::removeRequest(const std::shared_ptr<Request>& request)
{
  return inFlightRequests_.erase(request) == 1;
}

void FileEntry::removeURIWhoseHostnameIs(const std::string& hostname)
{
  std::deque<std::string> newURIs;
  for (std::deque<std::string>::const_iterator itr = uris_.begin(),
                                               eoi = uris_.end();
       itr != eoi; ++itr) {
    uri_split_result us;
    if (uri_split(&us, (*itr).c_str()) == -1) {
      continue;
    }
    if (us.fields[USR_HOST].len != hostname.size() ||
        memcmp((*itr).c_str() + us.fields[USR_HOST].off, hostname.c_str(),
               hostname.size()) != 0) {
      newURIs.push_back(*itr);
    }
  }
  A2_LOG_DEBUG(fmt("Removed %lu duplicate hostname URIs for path=%s",
                   static_cast<unsigned long>(uris_.size() - newURIs.size()),
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
  FindURIResultByResult(error_code::Value r) : r_(r) {}

  bool operator()(const URIResult& uriResult) const
  {
    return uriResult.getResult() == r_;
  }
};
} // namespace

void FileEntry::extractURIResult(std::deque<URIResult>& res,
                                 error_code::Value r)
{
  auto i = std::stable_partition(uriResults_.begin(), uriResults_.end(),
                                 FindURIResultByResult(r));
  std::copy(uriResults_.begin(), i, std::back_inserter(res));
  uriResults_.erase(uriResults_.begin(), i);
}

void FileEntry::reuseUri(const std::vector<std::string>& ignore)
{
  if (A2_LOG_DEBUG_ENABLED) {
    for (const auto& i : ignore) {
      A2_LOG_DEBUG(fmt("ignore host=%s", i.c_str()));
    }
  }
  std::deque<std::string> uris = spentUris_;
  std::sort(uris.begin(), uris.end());
  uris.erase(std::unique(uris.begin(), uris.end()), uris.end());

  std::vector<std::string> errorUris(uriResults_.size());
  std::transform(uriResults_.begin(), uriResults_.end(), errorUris.begin(),
                 std::mem_fn(&URIResult::getURI));
  std::sort(errorUris.begin(), errorUris.end());
  errorUris.erase(std::unique(errorUris.begin(), errorUris.end()),
                  errorUris.end());
  if (A2_LOG_DEBUG_ENABLED) {
    for (std::vector<std::string>::const_iterator i = errorUris.begin(),
                                                  eoi = errorUris.end();
         i != eoi; ++i) {
      A2_LOG_DEBUG(fmt("error URI=%s", (*i).c_str()));
    }
  }
  std::vector<std::string> reusableURIs;
  std::set_difference(uris.begin(), uris.end(), errorUris.begin(),
                      errorUris.end(), std::back_inserter(reusableURIs));
  auto insertionPoint = reusableURIs.begin();
  for (auto i = reusableURIs.begin(), eoi = reusableURIs.end(); i != eoi; ++i) {
    uri_split_result us;
    if (uri_split(&us, (*i).c_str()) == 0 &&
        std::find(ignore.begin(), ignore.end(),
                  uri::getFieldString(us, USR_HOST, (*i).c_str())) ==
            ignore.end()) {
      if (i != insertionPoint) {
        *insertionPoint = *i;
      }
      ++insertionPoint;
    }
  }
  reusableURIs.erase(insertionPoint, reusableURIs.end());
  size_t ininum = reusableURIs.size();
  if (A2_LOG_DEBUG_ENABLED) {
    A2_LOG_DEBUG(
        fmt("Found %u reusable URIs", static_cast<unsigned int>(ininum)));
    for (std::vector<std::string>::const_iterator i = reusableURIs.begin(),
                                                  eoi = reusableURIs.end();
         i != eoi; ++i) {
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
template <typename InputIterator>
void putBackUri(std::deque<std::string>& uris, InputIterator first,
                InputIterator last)
{
  for (; first != last; ++first) {
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
template <typename InputIterator, typename T>
InputIterator findRequestByUri(InputIterator first, InputIterator last,
                               const T& uri)
{
  for (; first != last; ++first) {
    if (!(*first)->removalRequested() && (*first)->getUri() == uri) {
      return first;
    }
  }
  return last;
}
} // namespace

bool FileEntry::removeUri(const std::string& uri)
{
  auto itr = std::find(spentUris_.begin(), spentUris_.end(), uri);
  if (itr == spentUris_.end()) {
    itr = std::find(uris_.begin(), uris_.end(), uri);
    if (itr == uris_.end()) {
      return false;
    }
    uris_.erase(itr);
    return true;
  }
  spentUris_.erase(itr);
  std::shared_ptr<Request> req;
  auto riter =
      findRequestByUri(inFlightRequests_.begin(), inFlightRequests_.end(), uri);
  if (riter == inFlightRequests_.end()) {
    auto riter =
        findRequestByUri(requestPool_.begin(), requestPool_.end(), uri);
    if (riter == requestPool_.end()) {
      return true;
    }
    req = *riter;
    requestPool_.erase(riter);
  }
  else {
    req = *riter;
  }
  req->requestRemoval();
  return true;
}

std::string FileEntry::getBasename() const { return File(path_).getBasename(); }

std::string FileEntry::getDirname() const { return File(path_).getDirname(); }

size_t FileEntry::setUris(const std::vector<std::string>& uris)
{
  uris_.clear();
  return addUris(uris.begin(), uris.end());
}

bool FileEntry::addUri(const std::string& uri)
{
  std::string peUri = util::percentEncodeMini(uri);
  if (uri_split(nullptr, peUri.c_str()) == 0) {
    uris_.push_back(peUri);
    return true;
  }
  else {
    return false;
  }
}

bool FileEntry::insertUri(const std::string& uri, size_t pos)
{
  std::string peUri = util::percentEncodeMini(uri);
  if (uri_split(nullptr, peUri.c_str()) != 0) {
    return false;
  }
  pos = std::min(pos, uris_.size());
  uris_.insert(uris_.begin() + pos, peUri);
  return true;
}

void FileEntry::setPath(std::string path) { path_ = std::move(path); }

void FileEntry::setContentType(std::string contentType)
{
  contentType_ = std::move(contentType);
}

size_t FileEntry::countInFlightRequest() const
{
  return inFlightRequests_.size();
}

size_t FileEntry::countPooledRequest() const { return requestPool_.size(); }

void FileEntry::setOriginalName(std::string originalName)
{
  originalName_ = std::move(originalName);
}

void FileEntry::setSuffixPath(std::string suffixPath)
{
  suffixPath_ = std::move(suffixPath);
}

bool FileEntry::emptyRequestUri() const
{
  return uris_.empty() && inFlightRequests_.empty() && requestPool_.empty();
}

void writeFilePath(std::ostream& o, const std::shared_ptr<FileEntry>& entry,
                   bool memory)
{
  if (entry->getPath().empty()) {
    auto uris = entry->getUris();
    if (uris.empty()) {
      o << "n/a";
    }
    else {
      o << uris.front();
    }
    return;
  }

  if (memory) {
    o << "[MEMORY]" << File(entry->getPath()).getBasename();
  }
  else {
    o << entry->getPath();
  }
}

} // namespace aria2
