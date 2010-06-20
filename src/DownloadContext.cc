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
#include "DownloadContext.h"

#include <algorithm>

#include "FileEntry.h"
#include "StringFormat.h"
#include "util.h"
#include "wallclock.h"
#include "DlAbortEx.h"

namespace aria2 {

DownloadContext::DownloadContext():
  _dir(A2STR::DOT_C),
  _pieceLength(0),
  _knowsTotalLength(true),
  _ownerRequestGroup(0),
  _downloadStartTime(0),
  _downloadStopTime(_downloadStartTime) {}

DownloadContext::DownloadContext(size_t pieceLength,
                                 uint64_t totalLength,
                                 const std::string& path):
  _dir(A2STR::DOT_C),
  _pieceLength(pieceLength),
  _knowsTotalLength(true),
  _ownerRequestGroup(0),
  _downloadStartTime(0),
  _downloadStopTime(0)
{
  SharedHandle<FileEntry> fileEntry(new FileEntry(path, totalLength, 0));
  _fileEntries.push_back(fileEntry);
}

void DownloadContext::resetDownloadStartTime()
{
  _downloadStartTime = global::wallclock;
  _downloadStopTime.reset(0);
}

void DownloadContext::resetDownloadStopTime()
{
  _downloadStopTime = global::wallclock;
}

int64_t DownloadContext::calculateSessionTime() const
{
  if(_downloadStopTime > _downloadStartTime) {
    return
      _downloadStartTime.differenceInMillis(_downloadStopTime);
  } else {
    return 0;
  }
}

SharedHandle<FileEntry>
DownloadContext::findFileEntryByOffset(off_t offset) const
{
  if(_fileEntries.empty() ||
     (offset > 0 &&
      _fileEntries.back()->getOffset()+_fileEntries.back()->getLength() <=
      static_cast<uint64_t>(offset))){
    return SharedHandle<FileEntry>();
  }

  SharedHandle<FileEntry> obj(new FileEntry());
  obj->setOffset(offset);
  std::vector<SharedHandle<FileEntry> >::const_iterator i =
    std::upper_bound(_fileEntries.begin(), _fileEntries.end(), obj);
  if(i != _fileEntries.end() && (*i)->getOffset() == offset) {
    return *i;
  } else {
    return *(--i);
  }
}

void DownloadContext::setFilePathWithIndex
(size_t index, const std::string& path)
{
  if(0 < index && index <= _fileEntries.size()) {
    // We don't escape path because path may come from users.
    _fileEntries[index-1]->setPath(path);
  } else {
    throw DL_ABORT_EX(StringFormat("No such file with index=%u",
                                   static_cast<unsigned int>(index)).str());
  }
}

void DownloadContext::setFileFilter(IntSequence seq)
{
  std::vector<int32_t> fileIndexes = seq.flush();
  std::sort(fileIndexes.begin(), fileIndexes.end());
  fileIndexes.erase(std::unique(fileIndexes.begin(), fileIndexes.end()),
                    fileIndexes.end());

  bool selectAll = fileIndexes.empty() || _fileEntries.size() == 1;
    
  int32_t index = 1;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        _fileEntries.begin(), eoi = _fileEntries.end();
      i != eoi; ++i, ++index) {
    (*i)->setRequested
      (selectAll ||
       std::binary_search(fileIndexes.begin(), fileIndexes.end(), index));
  }
}

void DownloadContext::setAttribute
(const std::string& key, const SharedHandle<ContextAttribute>& value)
{
  std::map<std::string, SharedHandle<ContextAttribute> >::value_type p =
    std::make_pair(key, value);
  std::pair<std::map<std::string, SharedHandle<ContextAttribute> >::iterator,
            bool> r = _attrs.insert(p);
  if(!r.second) {
    (*r.first).second = value;
  }
}

const SharedHandle<ContextAttribute>& DownloadContext::getAttribute
(const std::string& key)
{
  std::map<std::string, SharedHandle<ContextAttribute> >::const_iterator itr =
    _attrs.find(key);
  if(itr == _attrs.end()) {
    throw DL_ABORT_EX(StringFormat("No attribute named %s", key.c_str()).str());
  } else {
    return (*itr).second;
  }
}

bool DownloadContext::hasAttribute(const std::string& key) const
{
  return _attrs.count(key) == 1;
}

void DownloadContext::releaseRuntimeResource()
{
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        _fileEntries.begin(), eoi = _fileEntries.end(); i != eoi; ++i) {
    (*i)->releaseRuntimeResource();
  }
}

size_t DownloadContext::getNumPieces() const
{
  if(_pieceLength == 0) {
    return 0;
  } else {
    assert(!_fileEntries.empty());
    return (_fileEntries.back()->getLastOffset()+_pieceLength-1)/_pieceLength;
  }
}

uint64_t DownloadContext::getTotalLength() const
{
  if(_fileEntries.empty()) {
    return 0;
  } else {
    return _fileEntries.back()->getLastOffset();
  }
}

const std::string& DownloadContext::getBasePath() const
{
  if(_basePath.empty()) {
    assert(!_fileEntries.empty());
    return getFirstFileEntry()->getPath();
  } else {
    return _basePath;
  }
}

} // namespace aria2
