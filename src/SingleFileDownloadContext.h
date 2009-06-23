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
#ifndef _D_SINGLE_FILE_DOWNLOAD_CONTEXT_H_
#define _D_SINGLE_FILE_DOWNLOAD_CONTEXT_H_

#include "DownloadContext.h"

#include <string>
#include <deque>

#include "A2STR.h"

namespace aria2 {

class SingleFileDownloadContext:public DownloadContext
{
private:
  size_t _pieceLength;
  /**
   * _filename and _ufilename may contains directory path name.  So
   * /usr/local/aria2c is acceptable here.  These should be complete
   * file path.
   */
  std::string _filename;
  std::string _ufilename;

  std::string _contentType;

  std::deque<std::string> _pieceHashes;
  std::string _pieceHashAlgo;

  std::string _checksum;
  std::string _checksumHashAlgo;

  bool _knowsTotalLength;

  void updateFileEntry();
public:
  SingleFileDownloadContext(size_t pieceLength,
			    uint64_t totalLength,
			    const std::string& filename,
			    const std::string& ufilename = A2STR::NIL);

  virtual ~SingleFileDownloadContext() {}

  virtual const std::string& getPieceHash(size_t index) const
  {
    if(index < _pieceHashes.size()) {
      return _pieceHashes[index];
    } else {
      return A2STR::NIL;
    }
  }
  
  virtual const std::deque<std::string>& getPieceHashes() const;

  virtual uint64_t getTotalLength() const;

  virtual bool knowsTotalLength() const;


  virtual FILE_MODE getFileMode() const
  {
    if(_fileEntries.size() == 1) {
      return SINGLE;
    } else {
      return MULTI;
    }
  }

  virtual size_t getPieceLength() const
  {
    return _pieceLength;
  }

  virtual size_t getNumPieces() const;

  virtual std::string getActualBasePath() const;

  virtual const std::string& getPieceHashAlgo() const
  {
    return _pieceHashAlgo;
  }

  const std::string& getChecksumHashAlgo() const
  {
    return _checksumHashAlgo;
  }

  const std::string& getChecksum() const
  {
    return _checksum;
  }

  void setPieceHashes(const std::deque<std::string>& pieceHashes)
  {
    _pieceHashes = pieceHashes;
  }
  
  void setChecksumHashAlgo(const std::string& algo)
  {
    _checksumHashAlgo = algo;
  }

  void setChecksum(const std::string& checksum)
  {
    _checksum = checksum;
  }

  void setFilename(const std::string& filename)
  {
    _filename = filename;
    updateFileEntry();
  }

  void setUFilename(const std::string& ufilename)
  {
    _ufilename = ufilename;
    updateFileEntry();
  }

  void setTotalLength(uint64_t totalLength);

  void markTotalLengthIsUnknown();

  void setPieceHashAlgo(const std::string& algo)
  {
    _pieceHashAlgo = algo;
  }

  void setContentType(const std::string& contentType)
  {
    _contentType = contentType;
  }

  const std::string& getContentType()
  {
    return _contentType;
  }

  static const std::string DEFAULT_FILENAME;
};

typedef SharedHandle<SingleFileDownloadContext> SingleFileDownloadContextHandle;

} // namespace aria2

#endif // _D_SINGLE_FILE_DOWNLOAD_CONTEXT_H_
