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
#ifndef D_FILE_ENTRY_H
#define D_FILE_ENTRY_H

#include "common.h"

#include <string>
#include <deque>
#include <vector>
#include <ostream>
#include <set>

#include "SharedHandle.h"
#include "File.h"
#include "Request.h"
#include "URIResult.h"
#include "error_code.h"
#include "A2STR.h"
#include "TimerA2.h"
#include "util.h"
#include "a2functional.h"

namespace aria2 {

class URISelector;
class ServerStatMan;

class FileEntry {
public:
  typedef std::set<SharedHandle<Request>, RefLess<Request> >
  InFlightRequestSet;
private:
  std::string path_;
  std::deque<std::string> uris_;
  std::deque<std::string> spentUris_;
  int64_t length_;
  int64_t offset_;
  bool requested_;

  class RequestFaster {
  public:
    bool operator()(const SharedHandle<Request>& lhs,
                    const SharedHandle<Request>& rhs) const;
  };

  typedef std::set<SharedHandle<Request>, RequestFaster> RequestPool;
  RequestPool requestPool_;
  InFlightRequestSet inFlightRequests_;
  std::string contentType_;
  // URIResult is stored in the ascending order of the time when its result is
  // available.
  std::deque<URIResult> uriResults_;
  bool uniqueProtocol_;
  int maxConnectionPerServer_;
  std::string originalName_;
  Timer lastFasterReplace_;

  void storePool(const SharedHandle<Request>& request);
public:
  FileEntry();

  FileEntry(const std::string& path, int64_t length, int64_t offset,
            const std::vector<std::string>& uris = std::vector<std::string>());

  ~FileEntry();

  FileEntry& operator=(const FileEntry& entry);

  std::string getBasename() const;

  std::string getDirname() const;

  const std::string& getPath() const { return path_; }

  void setPath(const std::string& path);

  int64_t getLength() const { return length_; }

  void setLength(int64_t length) { length_ = length; }

  int64_t getOffset() const { return offset_; }

  void setOffset(int64_t offset) { offset_ = offset; }

  int64_t getLastOffset() { return offset_+length_; }

  bool isRequested() const { return requested_; }

  void setRequested(bool flag) { requested_ = flag; }

  const std::deque<std::string>& getRemainingUris() const
  {
    return uris_;
  }

  std::deque<std::string>& getRemainingUris()
  {
    return uris_;
  }

  const std::deque<std::string>& getSpentUris() const
  {
    return spentUris_;
  }

  size_t setUris(const std::vector<std::string>& uris);

  template<typename InputIterator>
  size_t addUris(InputIterator first, InputIterator last)
  {
    size_t count = 0;
    for(; first != last; ++first) {
      if(addUri(*first)) {
        ++count;
      }
    }
    return count;
  }

  bool addUri(const std::string& uri);

  bool insertUri(const std::string& uri, size_t pos);

  // Inserts uris_ and spentUris_ into uris.
  void getUris(std::vector<std::string>& uris) const;

  void setContentType(const std::string& contentType);

  const std::string& getContentType() const { return contentType_; }

  // If pooled Request object is available, one of them is removed
  // from the pool and returned.  If pool is empty, then select URI
  // using selectUri(selector) and construct Request object using it
  // and return the Request object.  If referer is given, it is set to
  // newly created Request. If Request object is retrieved from the
  // pool, referer is ignored.  If method is given, it is set to newly
  // created Request. If Request object is retrieved from the pool,
  // method is ignored. If uriReuse is true and selector does not
  // returns Request object either because uris_ is empty or all URI
  // are not be usable because maxConnectionPerServer_ limit, then
  // reuse used URIs and do selection again.
  SharedHandle<Request> getRequest
  (const SharedHandle<URISelector>& selector,
   bool uriReuse,
   const std::vector<std::pair<size_t, std::string> >& usedHosts,
   const std::string& referer = A2STR::NIL,
   const std::string& method = Request::METHOD_GET);

  // Finds pooled Request object which is faster than passed one,
  // comparing their PeerStat objects. If such Request is found, it is
  // removed from the pool and returned.
  SharedHandle<Request> findFasterRequest(const SharedHandle<Request>& base);

  // Finds faster server using ServerStatMan.
  SharedHandle<Request>
  findFasterRequest
  (const SharedHandle<Request>& base,
   const std::vector<std::pair<size_t, std::string> >& usedHosts,
   const SharedHandle<ServerStatMan>& serverStatMan);

  void poolRequest(const SharedHandle<Request>& request);

  bool removeRequest(const SharedHandle<Request>& request);

  size_t countInFlightRequest() const;

  size_t countPooledRequest() const;

  const InFlightRequestSet& getInFlightRequests() const
  {
    return inFlightRequests_;
  }

  bool operator<(const FileEntry& fileEntry) const;

  bool exists() const;

  // Translate global offset goff to file local offset.
  int64_t gtoloff(int64_t goff) const;

  void removeURIWhoseHostnameIs(const std::string& hostname);

  void removeIdenticalURI(const std::string& uri);

  void addURIResult(std::string uri, error_code::Value result);

  const std::deque<URIResult>& getURIResults() const
  {
    return uriResults_;
  }

  // Extracts URIResult whose _result is r and stores them into res.
  // The extracted URIResults are removed from uriResults_.
  void extractURIResult
  (std::deque<URIResult>& res, error_code::Value r);

  void setMaxConnectionPerServer(int n)
  {
    maxConnectionPerServer_ = n;
  }

  int getMaxConnectionPerServer() const
  {
    return maxConnectionPerServer_;
  }

  // Reuse URIs which have not emitted error so far and whose host
  // component is not included in ignore. The reusable URIs are
  // appended to uris_ maxConnectionPerServer_ times.
  void reuseUri(const std::vector<std::string>& ignore);

  void releaseRuntimeResource();

  // Push URIs in pooled or in-flight requests to the front of uris_.
  void putBackRequest();

  void setOriginalName(const std::string& originalName);

  const std::string& getOriginalName() const
  {
    return originalName_;
  }

  bool removeUri(const std::string& uri);

  bool emptyRequestUri() const;

  void setUniqueProtocol(bool f)
  {
    uniqueProtocol_ = f;
  }

  bool isUniqueProtocol() const
  {
    return uniqueProtocol_;
  }
};

// Returns the first FileEntry which isRequested() method returns
// true.  If no such FileEntry exists, then returns
// SharedHandle<FileEntry>().
template<typename InputIterator>
SharedHandle<FileEntry> getFirstRequestedFileEntry
(InputIterator first, InputIterator last)
{
  for(; first != last; ++first) {
    if((*first)->isRequested()) {
      return *first;
    }
  }
  return SharedHandle<FileEntry>();
}

// Counts the number of files selected in the given iterator range
// [first, last).
template<typename InputIterator>
size_t countRequestedFileEntry(InputIterator first, InputIterator last)
{
  size_t count = 0;
  for(; first != last; ++first) {
    if((*first)->isRequested()) {
      ++count;
    }
  }
  return count;
}

// Returns true if at least one requested FileEntry has URIs.
template<typename InputIterator>
bool isUriSuppliedForRequsetFileEntry(InputIterator first, InputIterator last)
{
  for(; first != last; ++first) {
    if((*first)->isRequested() && !(*first)->getRemainingUris().empty()) {
      return true;
    }
  }
  return false;
}

// Writes filename to given o.  If memory is true, the output is
// "[MEMORY]" plus the basename of the filename.  If there is no
// FileEntry, writes "n/a" to o.
void writeFilePath
(std::ostream& o,
 const SharedHandle<FileEntry>& entry,
 bool memory);

// Writes first filename to given o.  If memory is true, the output is
// "[MEMORY]" plus the basename of the first filename.  If there is no
// FileEntry, writes "n/a" to o.  If more than 1 FileEntry are in the
// iterator range [first, last), "(Nmore)" is written at the end where
// N is the number of files in iterator range [first, last) minus 1.
template<typename InputIterator>
void writeFilePath
(InputIterator first, InputIterator last, std::ostream& o, bool memory)
{
  SharedHandle<FileEntry> e = getFirstRequestedFileEntry(first, last);
  if(!e) {
    o << "n/a";
  } else {
    writeFilePath(o, e, memory);
    if(!e->getPath().empty()) {
      size_t count = countRequestedFileEntry(first, last);
      if(count > 1) {
        o << " (" << count-1 << "more)";
      }
    }
  }
}

} // namespace aria2

#endif // D_FILE_ENTRY_H
