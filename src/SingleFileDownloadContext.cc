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
#include "SingleFileDownloadContext.h"
#include "FileEntry.h"

namespace aria2 {

const std::string SingleFileDownloadContext::DEFAULT_FILENAME("index.html");

SingleFileDownloadContext::SingleFileDownloadContext(size_t pieceLength,
						     uint64_t totalLength,
						     const std::string& filename,
						     const std::string& ufilename):
  _pieceLength(pieceLength),
  _fileEntry(new FileEntry(filename, totalLength, 0)),
  _filename(filename),
  _ufilename(ufilename),
  _knowsTotalLength(true)
{
  updateFileEntry();
}

void SingleFileDownloadContext::updateFileEntry()
{
  if(!_ufilename.empty()) {
    _fileEntry->setPath(_ufilename);
  } else if(!_filename.empty()) {
    _fileEntry->setPath(_filename);
  } else {
    _fileEntry->setPath("");
  }
}

const std::deque<std::string>&
SingleFileDownloadContext::getPieceHashes() const
{
  return _pieceHashes;
}

uint64_t SingleFileDownloadContext::getTotalLength() const
{
  return _fileEntry->getLength();
}

bool SingleFileDownloadContext::knowsTotalLength() const
{
  return _knowsTotalLength;
}

FileEntries
SingleFileDownloadContext::getFileEntries() const
{
  FileEntries fs;
  fs.push_back(_fileEntry);
  return fs;
}

size_t SingleFileDownloadContext::getNumPieces() const
{
  return (_fileEntry->getLength()+_pieceLength-1)/_pieceLength;
}

std::string SingleFileDownloadContext::getActualBasePath() const
{
  return _fileEntry->getPath();
}

void SingleFileDownloadContext::setTotalLength(uint64_t totalLength)
{
  _fileEntry->setLength(totalLength);
}

void SingleFileDownloadContext::markTotalLengthIsUnknown()
{
  _knowsTotalLength = false;
}

const std::string& SingleFileDownloadContext::getName() const
{
  return _fileEntry->getPath();
}

} // namespace aria2
