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
#include "RequestGroupMan.h"

namespace aria2 {

DownloadContext::DownloadContext()
    : ownerRequestGroup_(nullptr),
      attrs_(MAX_CTX_ATTR),
      downloadStopTime_(Timer::zero()),
      pieceLength_(0),
      checksumVerified_(false),
      knowsTotalLength_(true),
#ifdef ENABLE_METALINK
      acceptMetalink_(true)
#else  // !ENABLE_METALINK
      acceptMetalink_(false)
#endif // !ENABLE_METALINK
{
}

DownloadContext::DownloadContext(int32_t pieceLength, int64_t totalLength,
                                 std::string path)
    : ownerRequestGroup_(nullptr),
      attrs_(MAX_CTX_ATTR),
      downloadStopTime_(Timer::zero()),
      pieceLength_(pieceLength),
      checksumVerified_(false),
      knowsTotalLength_(true),
#ifdef ENABLE_METALINK
      acceptMetalink_(true)
#else  // !ENABLE_METALINK
      acceptMetalink_(false)
#endif // !ENABLE_METALINK
{
  fileEntries_.push_back(
      std::make_shared<FileEntry>(std::move(path), totalLength, 0));
}

DownloadContext::~DownloadContext() = default;

void DownloadContext::resetDownloadStartTime()
{
  downloadStopTime_ = Timer::zero();
  netStat_.downloadStart();
}

void DownloadContext::resetDownloadStopTime()
{
  downloadStopTime_ = global::wallclock();
  netStat_.downloadStop();
}

Timer::Clock::duration DownloadContext::calculateSessionTime() const
{
  const Timer& startTime = netStat_.getDownloadStartTime();
  return startTime.difference(downloadStopTime_);
}

std::shared_ptr<FileEntry>
DownloadContext::findFileEntryByOffset(int64_t offset) const
{
  if (fileEntries_.empty() ||
      (offset > 0 && fileEntries_.back()->getLastOffset() <= offset)) {
    return nullptr;
  }

  auto obj = std::make_shared<FileEntry>();
  obj->setOffset(offset);
  auto i = std::upper_bound(fileEntries_.begin(), fileEntries_.end(), obj,
                            DerefLess<std::shared_ptr<FileEntry>>());
  if (i != fileEntries_.end() && (*i)->getOffset() == offset) {
    return *i;
  }
  else {
    return *(--i);
  }
}

void DownloadContext::setFilePathWithIndex(size_t index,
                                           const std::string& path)
{
  if (0 < index && index <= fileEntries_.size()) {
    // We don't escape path because path may come from users.
    fileEntries_[index - 1]->setPath(path);
  }
  else {
    throw DL_ABORT_EX(
        fmt("No such file with index=%u", static_cast<unsigned int>(index)));
  }
}

void DownloadContext::setFileFilter(SegList<int> sgl)
{
  using namespace std::placeholders;

  if (!sgl.hasNext() || fileEntries_.size() == 1) {
    std::for_each(fileEntries_.begin(), fileEntries_.end(),
                  std::bind(&FileEntry::setRequested, _1, true));
    return;
  }
  assert(sgl.peek() >= 1);
  size_t len = fileEntries_.size();
  size_t i = 0;
  for (; i < len && sgl.hasNext(); ++i) {
    size_t idx = sgl.peek() - 1;
    if (i == idx) {
      fileEntries_[i]->setRequested(true);
      sgl.next();
    }
    else if (i < idx) {
      fileEntries_[i]->setRequested(false);
    }
  }
  for (; i < len; ++i) {
    fileEntries_[i]->setRequested(false);
  }
}

void DownloadContext::setAttribute(ContextAttributeType key,
                                   std::shared_ptr<ContextAttribute> value)
{
  assert(key < MAX_CTX_ATTR);
  attrs_[key] = std::move(value);
}

const std::shared_ptr<ContextAttribute>&
DownloadContext::getAttribute(ContextAttributeType key)
{
  assert(key < MAX_CTX_ATTR);
  const auto& attr = attrs_[key];
  if (!attr) {
    throw DL_ABORT_EX(
        fmt("No attribute named %s", strContextAttributeType(key)));
  }

  return attr;
}

bool DownloadContext::hasAttribute(ContextAttributeType key) const
{
  assert(key < MAX_CTX_ATTR);
  return attrs_[key].get();
}

const std::vector<std::shared_ptr<ContextAttribute>>&
DownloadContext::getAttributes() const
{
  return attrs_;
}

void DownloadContext::releaseRuntimeResource()
{
  for (std::vector<std::shared_ptr<FileEntry>>::const_iterator
           i = fileEntries_.begin(),
           eoi = fileEntries_.end();
       i != eoi; ++i) {
    (*i)->putBackRequest();
    (*i)->releaseRuntimeResource();
  }
}

size_t DownloadContext::getNumPieces() const
{
  if (pieceLength_ == 0) {
    return 0;
  }
  else {
    assert(!fileEntries_.empty());
    return (fileEntries_.back()->getLastOffset() + pieceLength_ - 1) /
           pieceLength_;
  }
}

int64_t DownloadContext::getTotalLength() const
{
  if (fileEntries_.empty()) {
    return 0;
  }
  else {
    return fileEntries_.back()->getLastOffset();
  }
}

const std::string& DownloadContext::getBasePath() const
{
  if (basePath_.empty()) {
    assert(!fileEntries_.empty());
    return getFirstFileEntry()->getPath();
  }
  else {
    return basePath_;
  }
}

std::shared_ptr<FileEntry> DownloadContext::getFirstRequestedFileEntry() const
{
  for (auto& e : fileEntries_) {
    if (e->isRequested()) {
      return e;
    }
  }
  return nullptr;
}

size_t DownloadContext::countRequestedFileEntry() const
{
  size_t numFiles = 0;
  for (const auto& e : fileEntries_) {
    if (e->isRequested()) {
      ++numFiles;
    }
  }
  return numFiles;
}

bool DownloadContext::isChecksumVerificationNeeded() const
{
  return pieceHashType_.empty() && !digest_.empty() && !hashType_.empty() &&
         !checksumVerified_;
}

bool DownloadContext::isChecksumVerificationAvailable() const
{
  return !digest_.empty() && !hashType_.empty();
}

bool DownloadContext::isPieceHashVerificationAvailable() const
{
  return !pieceHashType_.empty() && pieceHashes_.size() > 0 &&
         pieceHashes_.size() == getNumPieces();
}

const std::string& DownloadContext::getPieceHash(size_t index) const
{
  if (index < pieceHashes_.size()) {
    return pieceHashes_[index];
  }
  else {
    return A2STR::NIL;
  }
}

void DownloadContext::setDigest(const std::string& hashType,
                                const std::string& digest)
{
  hashType_ = hashType;
  digest_ = digest;
}

void DownloadContext::setBasePath(const std::string& basePath)
{
  basePath_ = basePath;
}

void DownloadContext::setSignature(std::unique_ptr<Signature> signature)
{
  signature_ = std::move(signature);
}

void DownloadContext::updateDownload(size_t bytes)
{
  netStat_.updateDownload(bytes);
  RequestGroupMan* rgman = ownerRequestGroup_->getRequestGroupMan();
  if (rgman) {
    rgman->getNetStat().updateDownload(bytes);
  }
}

void DownloadContext::updateUploadSpeed(size_t bytes)
{
  netStat_.updateUploadSpeed(bytes);
  auto rgman = ownerRequestGroup_->getRequestGroupMan();
  if (rgman) {
    rgman->getNetStat().updateUploadSpeed(bytes);
  }
}

void DownloadContext::updateUploadLength(size_t bytes)
{
  netStat_.updateUploadLength(bytes);
  auto rgman = ownerRequestGroup_->getRequestGroupMan();
  if (rgman) {
    rgman->getNetStat().updateUploadLength(bytes);
  }
}

} // namespace aria2
