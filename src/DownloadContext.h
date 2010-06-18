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
#ifndef _D_DOWNLOAD_CONTEXT_H_
#define _D_DOWNLOAD_CONTEXT_H_

#include "common.h"

#include <cassert>
#include <string>
#include <vector>

#include "SharedHandle.h"
#include "Signature.h"
#include "TimerA2.h"
#include "A2STR.h"
#include "ValueBase.h"
#include "IntSequence.h"
#include "FileEntry.h"
#include "TorrentAttribute.h"

namespace aria2 {

class RequestGroup;

class DownloadContext
{
private:
  std::vector<SharedHandle<FileEntry> > _fileEntries;

  std::string _dir;

  std::vector<std::string> _pieceHashes;

  size_t _pieceLength;

  std::string _pieceHashAlgo;

  std::string _checksum;

  std::string _checksumHashAlgo;

  std::string _basePath;

  bool _knowsTotalLength;

  RequestGroup* _ownerRequestGroup;
  
  std::map<std::string, SharedHandle<ContextAttribute> > _attrs;

  Timer _downloadStartTime;

  Timer _downloadStopTime;

  SharedHandle<Signature> _signature;
public:
  DownloadContext();

  // Convenient constructor that creates single file download.  path
  // should be escaped with util::escapePath(...).
  DownloadContext(size_t pieceLength,
                  uint64_t totalLength,
                  const std::string& path = A2STR::NIL);

  const std::string& getPieceHash(size_t index) const
  {
    if(index < _pieceHashes.size()) {
      return _pieceHashes[index];
    } else {
      return A2STR::NIL;
    }
  }
  
  const std::vector<std::string>& getPieceHashes() const
  {
    return _pieceHashes;
  }

  template<typename InputIterator>
  void setPieceHashes(InputIterator first, InputIterator last)
  {
    _pieceHashes.assign(first, last);
  }

  uint64_t getTotalLength() const
  {
    if(_fileEntries.empty()) {
      return 0;
    } else {
      return _fileEntries.back()->getLastOffset();
    }
  }

  bool knowsTotalLength() const { return _knowsTotalLength; }

  void markTotalLengthIsUnknown() { _knowsTotalLength = false; }

  void markTotalLengthIsKnown() { _knowsTotalLength = true; }

  const std::vector<SharedHandle<FileEntry> >& getFileEntries() const
  {
    return _fileEntries;
  }

  const SharedHandle<FileEntry>& getFirstFileEntry() const
  {
    assert(!_fileEntries.empty());
    return _fileEntries[0];
  }

  template<typename InputIterator>
  void setFileEntries(InputIterator first, InputIterator last)
  {
    _fileEntries.assign(first, last);
  }

  size_t getPieceLength() const { return _pieceLength; }

  void setPieceLength(size_t length) { _pieceLength = length; }

  size_t getNumPieces() const
  {
    if(_pieceLength == 0) {
      return 0;
    } else {
      assert(!_fileEntries.empty());
      return (_fileEntries.back()->getLastOffset()+_pieceLength-1)/_pieceLength;
    }
  }

  const std::string& getPieceHashAlgo() const { return _pieceHashAlgo; }

  void setPieceHashAlgo(const std::string& algo)
  {
    _pieceHashAlgo = algo;
  }

  const std::string& getChecksum() const { return _checksum; }

  void setChecksum(const std::string& checksum)
  {
    _checksum = checksum;
  }

  const std::string& getChecksumHashAlgo() const { return _checksumHashAlgo; }

  void setChecksumHashAlgo(const std::string& algo)
  {
    _checksumHashAlgo = algo;
  }

  // The representative path name for this context. It is used as a
  // part of .aria2 control file. If _basePath is set, returns
  // _basePath. Otherwise, the first FileEntry's getFilePath() is
  // returned.
  const std::string& getBasePath() const
  {
    if(_basePath.empty()) {
      assert(!_fileEntries.empty());
      return getFirstFileEntry()->getPath();
    } else {
      return _basePath;
    }
  }

  void setBasePath(const std::string& basePath) { _basePath = basePath; }

  const std::string& getDir() const { return _dir; }

  void setDir(const std::string& dir) { _dir = dir; }

  const SharedHandle<Signature>& getSignature() const { return _signature; }

  void setSignature(const SharedHandle<Signature>& signature)
  {
    _signature = signature;
  }

  RequestGroup* getOwnerRequestGroup() { return _ownerRequestGroup; }

  void setOwnerRequestGroup(RequestGroup* owner)
  {
    _ownerRequestGroup = owner;
  }

  void setFileFilter(IntSequence seq);

  // Sets file path for specified index. index starts from 1. The
  // index is the same used in setFileFilter().  Please note that path
  // is not the actual file path. The actual file path is
  // getDir()+"/"+path. path is not escaped by util::escapePath() in
  // this function.
  void setFilePathWithIndex(size_t index, const std::string& path);

  void setAttribute
  (const std::string& key, const SharedHandle<ContextAttribute>& value);

  const SharedHandle<ContextAttribute>& getAttribute(const std::string& key);

  bool hasAttribute(const std::string& key) const;

  void resetDownloadStartTime();

  void resetDownloadStopTime();

  const Timer& getDownloadStopTime() const
  {
    return _downloadStopTime;
  }

  int64_t calculateSessionTime() const;
  
  // Returns FileEntry at given offset. SharedHandle<FileEntry>() is
  // returned if no such FileEntry is found.
  SharedHandle<FileEntry> findFileEntryByOffset(off_t offset) const;

  void releaseRuntimeResource();
};

} // namespace aria2

#endif // _D_DOWNLOAD_CONTEXT_H_
