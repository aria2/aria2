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
  _filename(filename),
  _ufilename(ufilename),
  _knowsTotalLength(true)
{
  _fileEntries.push_back
    (SharedHandle<FileEntry>(new FileEntry(filename, totalLength, 0)));
  updateFileEntry();
}

void SingleFileDownloadContext::updateFileEntry()
{
  const SharedHandle<FileEntry>& fileEntry = _fileEntries.front();
  if(!_ufilename.empty()) {
    fileEntry->setPath(_ufilename);
  } else if(!_filename.empty()) {
    fileEntry->setPath(_filename);
  } else {
    fileEntry->setPath("");
  }
}

const std::deque<std::string>&
SingleFileDownloadContext::getPieceHashes() const
{
  return _pieceHashes;
}

uint64_t SingleFileDownloadContext::getTotalLength() const
{
  return _fileEntries.front()->getLength();
}

bool SingleFileDownloadContext::knowsTotalLength() const
{
  return _knowsTotalLength;
}

size_t SingleFileDownloadContext::getNumPieces() const
{
  return (_fileEntries.front()->getLength()+_pieceLength-1)/_pieceLength;
}

std::string SingleFileDownloadContext::getActualBasePath() const
{
  return _fileEntries.front()->getPath();
}

void SingleFileDownloadContext::setTotalLength(uint64_t totalLength)
{
  _fileEntries.front()->setLength(totalLength);
}

void SingleFileDownloadContext::markTotalLengthIsUnknown()
{
  _knowsTotalLength = false;
}

} // namespace aria2
