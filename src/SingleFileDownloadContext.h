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

class SingleFileDownloadContext:public DownloadContext
{
private:
  int32_t _pieceLength;
  /**
   * Actual file path is _dir + _filename.
   * If _ufilename is not zero-length string, then _dir + _ufilename.
   */
  FileEntryHandle _fileEntry;
  /**
   * _filename and _ufilename may contains directory path name.
   * So usr/local/aria2c is acceptable here.
   */
  string _filename;
  string _ufilename;

  Strings _pieceHashes;
  string _pieceHashAlgo;

  void updateFileEntry()
  {
    if(_ufilename != "") {
      _fileEntry->setPath(_ufilename);
    } else if(_filename != "") {
      _fileEntry->setPath(_filename);
    } else {
      _fileEntry->setPath("index.html");
    }
  }
public:
  SingleFileDownloadContext(int32_t pieceLength,
			    int64_t totalLength,
			    const string& filename,
			    const string& ufilename = ""):
    _pieceLength(pieceLength),
    _fileEntry(new FileEntry(filename, totalLength, 0)),
    _filename(filename),
    _ufilename(ufilename)
  {
    updateFileEntry();
  }

  virtual ~SingleFileDownloadContext() {}

  virtual string getPieceHash(int32_t index) const
  {
    if(index < 0 || _pieceHashes.size() <= (size_t)index) {
      return "";
    }
    return _pieceHashes[index];
  }
  
  virtual const Strings& getPieceHashes() const
  {
    return _pieceHashes;
  }

  virtual int64_t getTotalLength() const
  {
    return _fileEntry->getLength();
  }

  virtual FILE_MODE getFileMode() const
  {
    return SINGLE;
  }

  virtual FileEntries getFileEntries() const
  {
    FileEntries fs;
    fs.push_back(_fileEntry);
    return fs;
  }

  virtual string getName() const
  {
    return _filename;
  }
  
  virtual int32_t getPieceLength() const
  {
    return _pieceLength;
  }

  virtual int32_t getNumPieces() const
  {
    return (_fileEntry->getLength()+_pieceLength-1)/_pieceLength;
  }

  virtual string getActualBasePath() const
  {
    return _dir+"/"+_fileEntry->getPath();
  }

  virtual string getPieceHashAlgo() const
  {
    return _pieceHashAlgo;
  }

  void setPieceHashes(const Strings& pieceHashes)
  {
    _pieceHashes = pieceHashes;
  }
  
  void setFilename(const string& filename)
  {
    _filename = filename;
    updateFileEntry();
  }

  void setUFilename(const string& ufilename)
  {
    _ufilename = ufilename;
    updateFileEntry();
  }

  void setTotalLength(int64_t totalLength)
  {
    _fileEntry->setLength(totalLength);
  }

  void setPieceHashAlgo(const string& algo)
  {
    _pieceHashAlgo = algo;
  }
};

typedef SharedHandle<SingleFileDownloadContext> SingleFileDownloadContextHandle;

#endif // _D_SINGLE_FILE_DOWNLOAD_CONTEXT_H_
