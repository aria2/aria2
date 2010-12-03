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
#include "fmt.h"
#include "util.h"
#include "wallclock.h"
#include "DlAbortEx.h"
#include "a2functional.h"
#include "Signature.h"
#include "ContextAttribute.h"

namespace aria2 {

DownloadContext::DownloadContext():
  pieceLength_(0),
  checksumVerified_(false),
  knowsTotalLength_(true),
  ownerRequestGroup_(0),
  downloadStartTime_(0),
  downloadStopTime_(downloadStartTime_) {}

DownloadContext::DownloadContext(size_t pieceLength,
                                 uint64_t totalLength,
                                 const std::string& path):
  pieceLength_(pieceLength),
  checksumVerified_(false),
  knowsTotalLength_(true),
  ownerRequestGroup_(0),
  downloadStartTime_(0),
  downloadStopTime_(0)
{
  SharedHandle<FileEntry> fileEntry(new FileEntry(path, totalLength, 0));
  fileEntries_.push_back(fileEntry);
}

DownloadContext::~DownloadContext() {}

void DownloadContext::resetDownloadStartTime()
{
  downloadStartTime_ = global::wallclock;
  downloadStopTime_.reset(0);
}

void DownloadContext::resetDownloadStopTime()
{
  downloadStopTime_ = global::wallclock;
}

int64_t DownloadContext::calculateSessionTime() const
{
  if(downloadStopTime_ > downloadStartTime_) {
    return
      downloadStartTime_.differenceInMillis(downloadStopTime_);
  } else {
    return 0;
  }
}

SharedHandle<FileEntry>
DownloadContext::findFileEntryByOffset(off_t offset) const
{
  if(fileEntries_.empty() ||
     (offset > 0 &&
      fileEntries_.back()->getOffset()+fileEntries_.back()->getLength() <=
      static_cast<uint64_t>(offset))){
    return SharedHandle<FileEntry>();
  }

  SharedHandle<FileEntry> obj(new FileEntry());
  obj->setOffset(offset);
  std::vector<SharedHandle<FileEntry> >::const_iterator i =
    std::upper_bound(fileEntries_.begin(), fileEntries_.end(), obj,
                     DerefLess<SharedHandle<FileEntry> >());
  if(i != fileEntries_.end() && (*i)->getOffset() == offset) {
    return *i;
  } else {
    return *(--i);
  }
}

void DownloadContext::setFilePathWithIndex
(size_t index, const std::string& path)
{
  if(0 < index && index <= fileEntries_.size()) {
    // We don't escape path because path may come from users.
    fileEntries_[index-1]->setPath(path);
  } else {
    throw DL_ABORT_EX(fmt("No such file with index=%u",
                          static_cast<unsigned int>(index)));
  }
}

void DownloadContext::setFileFilter(IntSequence seq)
{
  std::vector<int32_t> fileIndexes = seq.flush();
  std::sort(fileIndexes.begin(), fileIndexes.end());
  fileIndexes.erase(std::unique(fileIndexes.begin(), fileIndexes.end()),
                    fileIndexes.end());

  bool selectAll = fileIndexes.empty() || fileEntries_.size() == 1;
    
  int32_t index = 1;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        fileEntries_.begin(), eoi = fileEntries_.end();
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
            bool> r = attrs_.insert(p);
  if(!r.second) {
    (*r.first).second = value;
  }
}

const SharedHandle<ContextAttribute>& DownloadContext::getAttribute
(const std::string& key)
{
  std::map<std::string, SharedHandle<ContextAttribute> >::const_iterator itr =
    attrs_.find(key);
  if(itr == attrs_.end()) {
    throw DL_ABORT_EX(fmt("No attribute named %s", key.c_str()));
  } else {
    return (*itr).second;
  }
}

bool DownloadContext::hasAttribute(const std::string& key) const
{
  return attrs_.count(key) == 1;
}

void DownloadContext::releaseRuntimeResource()
{
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        fileEntries_.begin(), eoi = fileEntries_.end(); i != eoi; ++i) {
    (*i)->releaseRuntimeResource();
  }
}

size_t DownloadContext::getNumPieces() const
{
  if(pieceLength_ == 0) {
    return 0;
  } else {
    assert(!fileEntries_.empty());
    return (fileEntries_.back()->getLastOffset()+pieceLength_-1)/pieceLength_;
  }
}

uint64_t DownloadContext::getTotalLength() const
{
  if(fileEntries_.empty()) {
    return 0;
  } else {
    return fileEntries_.back()->getLastOffset();
  }
}

const std::string& DownloadContext::getBasePath() const
{
  if(basePath_.empty()) {
    assert(!fileEntries_.empty());
    return getFirstFileEntry()->getPath();
  } else {
    return basePath_;
  }
}

bool DownloadContext::isChecksumVerificationNeeded() const
{
  return pieceHashAlgo_.empty() &&
    !checksum_.empty() && !checksumHashAlgo_.empty() && !checksumVerified_;
}

bool DownloadContext::isChecksumVerificationAvailable() const
{
  return !checksum_.empty() && !checksumHashAlgo_.empty();
}

bool DownloadContext::isPieceHashVerificationAvailable() const
{
  return !pieceHashAlgo_.empty() &&
    pieceHashes_.size() > 0 && pieceHashes_.size() == getNumPieces();
}

const std::string& DownloadContext::getPieceHash(size_t index) const
{
  if(index < pieceHashes_.size()) {
    return pieceHashes_[index];
  } else {
    return A2STR::NIL;
  }
}

void DownloadContext::setPieceHashAlgo(const std::string& algo)
{
  pieceHashAlgo_ = algo;
}

void DownloadContext::setChecksum(const std::string& checksum)
{
  checksum_ = checksum;
}

void DownloadContext::setChecksumHashAlgo(const std::string& algo)
{
  checksumHashAlgo_ = algo;
}

void DownloadContext::setBasePath(const std::string& basePath)
{
  basePath_ = basePath;
}

void DownloadContext::setSignature(const SharedHandle<Signature>& signature)
{
  signature_ = signature;
}

} // namespace aria2
