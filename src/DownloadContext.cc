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
  SharedHandle<FileEntry> fileEntry
    (new FileEntry(util::escapePath(path), totalLength, 0));
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

void DownloadContext::ensureAttrs()
{
  if(_attrs.isNone()) {
    _attrs = BDE::dict();
  }
}

void DownloadContext::setAttribute(const std::string& key, const BDE& value)
{
  ensureAttrs();
  _attrs[key] = value;
}

BDE& DownloadContext::getAttribute(const std::string& key)
{
  ensureAttrs();
  if(_attrs.containsKey(key)) {
    return _attrs[key];
  } else {
    throw DL_ABORT_EX(StringFormat("No attribute named %s", key.c_str()).str());
  }
}

bool DownloadContext::hasAttribute(const std::string& key) const
{
  if(_attrs.isNone()) {
    return false;
  } else {
    return _attrs.containsKey(key);
  }
}

void DownloadContext::releaseRuntimeResource()
{
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        _fileEntries.begin(), eoi = _fileEntries.end(); i != eoi; ++i) {
    (*i)->releaseRuntimeResource();
  }
}

} // namespace aria2
