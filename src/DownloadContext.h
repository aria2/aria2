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
#ifndef D_DOWNLOAD_CONTEXT_H
#define D_DOWNLOAD_CONTEXT_H

#include "common.h"

#include <cassert>
#include <string>
#include <vector>
#include <memory>

#include "TimerA2.h"
#include "A2STR.h"
#include "ValueBase.h"
#include "SegList.h"
#include "ContextAttribute.h"
#include "NetStat.h"

namespace aria2 {

class RequestGroup;
class Signature;
class FileEntry;

class DownloadContext {
private:
  std::unique_ptr<Signature> signature_;

  RequestGroup* ownerRequestGroup_;

  std::vector<std::shared_ptr<ContextAttribute>> attrs_;

  std::vector<std::shared_ptr<FileEntry>> fileEntries_;

  std::vector<std::string> pieceHashes_;

  NetStat netStat_;

  Timer downloadStopTime_;

  std::string pieceHashType_;

  std::string digest_;

  std::string hashType_;

  std::string basePath_;

  int32_t pieceLength_;

  bool checksumVerified_;

  bool knowsTotalLength_;

  // This member variable is required to avoid to use parse Metalink
  // (including both Metalink XML and Metalink/HTTP) twice.
  bool acceptMetalink_;

public:
  DownloadContext();

  // Convenient constructor that creates single file download.  path
  // should be escaped with util::escapePath(...).
  DownloadContext(int32_t pieceLength, int64_t totalLength,
                  std::string path = A2STR::NIL);

  ~DownloadContext();

  const std::string& getPieceHash(size_t index) const;

  const std::vector<std::string>& getPieceHashes() const
  {
    return pieceHashes_;
  }

  template <typename InputIterator>
  void setPieceHashes(const std::string& hashType, InputIterator first,
                      InputIterator last)
  {
    pieceHashType_ = hashType;
    pieceHashes_.assign(first, last);
  }

  int64_t getTotalLength() const;

  bool knowsTotalLength() const { return knowsTotalLength_; }

  void markTotalLengthIsUnknown() { knowsTotalLength_ = false; }

  void markTotalLengthIsKnown() { knowsTotalLength_ = true; }

  const std::vector<std::shared_ptr<FileEntry>>& getFileEntries() const
  {
    return fileEntries_;
  }

  const std::shared_ptr<FileEntry>& getFirstFileEntry() const
  {
    assert(!fileEntries_.empty());
    return fileEntries_[0];
  }

  // This function returns first FileEntry whose isRequested() returns
  // true.  If there is no such FileEntry, returns
  // std::shared_ptr<FileEntry>().
  std::shared_ptr<FileEntry> getFirstRequestedFileEntry() const;

  size_t countRequestedFileEntry() const;

  template <typename InputIterator>
  void setFileEntries(InputIterator first, InputIterator last)
  {
    fileEntries_.assign(first, last);
  }

  int32_t getPieceLength() const { return pieceLength_; }

  void setPieceLength(int32_t length) { pieceLength_ = length; }

  size_t getNumPieces() const;

  const std::string& getPieceHashType() const { return pieceHashType_; }

  const std::string& getDigest() const { return digest_; }

  const std::string& getHashType() const { return hashType_; }

  void setDigest(const std::string& hashType, const std::string& digest);

  // The representative path name for this context. It is used as a
  // part of .aria2 control file. If basePath_ is set, returns
  // basePath_. Otherwise, the first FileEntry's getFilePath() is
  // returned.
  const std::string& getBasePath() const;

  void setBasePath(const std::string& basePath);

  const std::unique_ptr<Signature>& getSignature() const { return signature_; }

  void setSignature(std::unique_ptr<Signature> signature);

  RequestGroup* getOwnerRequestGroup() { return ownerRequestGroup_; }

  void setOwnerRequestGroup(RequestGroup* owner) { ownerRequestGroup_ = owner; }

  // sgl must be normalized before the call.
  void setFileFilter(SegList<int> sgl);

  // Sets file path for specified index. index starts from 1. The
  // index is the same used in setFileFilter(). path is not escaped by
  // util::escapePath() in this function.
  void setFilePathWithIndex(size_t index, const std::string& path);

  // Returns true if hash check(whole file hash, not piece hash) is
  // need to be done
  bool isChecksumVerificationNeeded() const;

  // Returns true if whole hash(not piece hash) is available.
  bool isChecksumVerificationAvailable() const;

  // Returns true if piece hash(not whole file hash) is available.
  bool isPieceHashVerificationAvailable() const;

  void setChecksumVerified(bool f) { checksumVerified_ = f; }

  void setAttribute(ContextAttributeType key,
                    std::shared_ptr<ContextAttribute> value);

  const std::shared_ptr<ContextAttribute>&
  getAttribute(ContextAttributeType key);

  bool hasAttribute(ContextAttributeType key) const;

  const std::vector<std::shared_ptr<ContextAttribute>>& getAttributes() const;

  void resetDownloadStartTime();

  void resetDownloadStopTime();

  const Timer& getDownloadStopTime() const { return downloadStopTime_; }

  Timer::Clock::duration calculateSessionTime() const;

  // Returns FileEntry at given offset. std::shared_ptr<FileEntry>() is
  // returned if no such FileEntry is found.
  std::shared_ptr<FileEntry> findFileEntryByOffset(int64_t offset) const;

  void releaseRuntimeResource();

  void setAcceptMetalink(bool f) { acceptMetalink_ = f; }
  bool getAcceptMetalink() const { return acceptMetalink_; }

  NetStat& getNetStat() { return netStat_; }

  // This method also updates global download length held by
  // RequestGroupMan via getOwnerRequestGroup().
  void updateDownload(size_t bytes);

  // This method also updates global upload length held by
  // RequestGroupMan via getOwnerRequestGroup().
  void updateUploadLength(size_t bytes);
  void updateUploadSpeed(size_t bytes);
};

} // namespace aria2

#endif // D_DOWNLOAD_CONTEXT_H
