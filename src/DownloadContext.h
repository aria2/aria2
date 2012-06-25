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

#include "SharedHandle.h"
#include "TimerA2.h"
#include "A2STR.h"
#include "ValueBase.h"
#include "SegList.h"

namespace aria2 {

class RequestGroup;
class Signature;
class FileEntry;
struct ContextAttribute;

class DownloadContext
{
private:
  std::vector<SharedHandle<FileEntry> > fileEntries_;

  std::vector<std::string> pieceHashes_;

  int32_t pieceLength_;

  std::string pieceHashType_;

  std::string digest_;

  std::string hashType_;

  bool checksumVerified_;

  std::string basePath_;

  bool knowsTotalLength_;

  RequestGroup* ownerRequestGroup_;
  
  std::map<std::string, SharedHandle<ContextAttribute> > attrs_;

  Timer downloadStartTime_;

  Timer downloadStopTime_;

  SharedHandle<Signature> signature_;
  // This member variable is required to avoid to parse Metalink/HTTP
  // Link header fields multiple times.
  bool metalinkServerContacted_;
public:
  DownloadContext();

  // Convenient constructor that creates single file download.  path
  // should be escaped with util::escapePath(...).
  DownloadContext(int32_t pieceLength,
                  int64_t totalLength,
                  const std::string& path = A2STR::NIL);

  ~DownloadContext();

  const std::string& getPieceHash(size_t index) const;
  
  const std::vector<std::string>& getPieceHashes() const
  {
    return pieceHashes_;
  }

  template<typename InputIterator>
  void setPieceHashes
  (const std::string& hashType,
   InputIterator first, InputIterator last)
  {
    pieceHashType_ = hashType;
    pieceHashes_.assign(first, last);
  }

  int64_t getTotalLength() const;

  bool knowsTotalLength() const { return knowsTotalLength_; }

  void markTotalLengthIsUnknown() { knowsTotalLength_ = false; }

  void markTotalLengthIsKnown() { knowsTotalLength_ = true; }

  const std::vector<SharedHandle<FileEntry> >& getFileEntries() const
  {
    return fileEntries_;
  }

  const SharedHandle<FileEntry>& getFirstFileEntry() const
  {
    assert(!fileEntries_.empty());
    return fileEntries_[0];
  }

  // This function returns first FileEntry whose isRequested() returns
  // true.  If there is no such FileEntry, returns
  // SharedHandle<FileEntry>().
  SharedHandle<FileEntry> getFirstRequestedFileEntry() const;

  size_t countRequestedFileEntry() const;

  template<typename InputIterator>
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

  const SharedHandle<Signature>& getSignature() const { return signature_; }

  void setSignature(const SharedHandle<Signature>& signature);

  RequestGroup* getOwnerRequestGroup() { return ownerRequestGroup_; }

  void setOwnerRequestGroup(RequestGroup* owner)
  {
    ownerRequestGroup_ = owner;
  }

  // sgl must be normalized before the call.
  void setFileFilter(SegList<int>& sgl);

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

  void setChecksumVerified(bool f)
  {
    checksumVerified_ = f;
  }

  void setAttribute
  (const std::string& key, const SharedHandle<ContextAttribute>& value);

  const SharedHandle<ContextAttribute>& getAttribute(const std::string& key);

  bool hasAttribute(const std::string& key) const;

  void resetDownloadStartTime();

  void resetDownloadStopTime();

  const Timer& getDownloadStopTime() const
  {
    return downloadStopTime_;
  }

  int64_t calculateSessionTime() const;
  
  // Returns FileEntry at given offset. SharedHandle<FileEntry>() is
  // returned if no such FileEntry is found.
  SharedHandle<FileEntry> findFileEntryByOffset(int64_t offset) const;

  void releaseRuntimeResource();

  void setMetalinkServerContacted(bool f)
  {
    metalinkServerContacted_ = f;
  }
  bool getMetalinkServerContacted() const
  {
    return metalinkServerContacted_;
  }
};

} // namespace aria2

#endif // D_DOWNLOAD_CONTEXT_H
