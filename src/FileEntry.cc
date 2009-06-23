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

#include "Util.h"
#include "URISelector.h"

namespace aria2 {

FileEntry::FileEntry(const std::string& path,
		     uint64_t length,
		     off_t offset,
		     const std::deque<std::string>& uris):
  path(path), _uris(uris), length(length), offset(offset),
  extracted(false), requested(true) {}

FileEntry::~FileEntry() {}

void FileEntry::setupDir()
{
  Util::mkdirs(File(path).getDirname());
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
  return uriSelector->select(_uris);
}

SharedHandle<Request>
FileEntry::getRequest(const SharedHandle<URISelector>& selector)
{
  SharedHandle<Request> req;
  if(_requestPool.empty()) {
    while(1) {
      std::string uri = selector->select(_uris);
      if(uri.empty()) {
	return req;
      }
      req.reset(new Request());
      if(req->setUrl(uri)) {
	_spentUris.push_back(uri);
	_inFlightRequests.push_back(req);
	return req;
      } else {
	req.reset();
      }
    }
  } else {
    req = _requestPool.back();
    _requestPool.pop_back();
    _inFlightRequests.push_back(req);
    return req;
  }
}

void FileEntry::poolRequest(const SharedHandle<Request>& request)
{
  removeRequest(request);
  _requestPool.push_back(request);
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

} // namespace aria2
