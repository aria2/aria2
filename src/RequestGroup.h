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
#ifndef _D_REQUEST_GROUP_H_
#define _D_REQUEST_GROUP_H_

#include "common.h"
#include "SegmentMan.h"
#include "LogFactory.h"
#include "Command.h"
#include "ChunkChecksum.h"
#include "Checksum.h"
#include "SegmentManFactory.h"
#include "DefaultSegmentManFactory.h"

class DownloadCommand;

class DownloadEngine;

class RequestGroup {
private:
  int64_t _hintTotalLength;
  string _hintFilename;
  string _ufilename;
  Strings _uris;
  Strings _spentUris;
  SegmentManHandle _segmentMan;
  SegmentManFactoryHandle _segmentManFactory;
  const Option* _option;
  const Logger* logger;
  ChunkChecksumHandle _chunkChecksum;
  ChecksumHandle _checksum;
  int32_t _numConcurrentCommand;

  void validateFilename(const string& expectedFilename,
			const string& actualFilename) const;

  void validateTotalLength(int64_t expectedTotalLength,
			   int64_t actualTotalLength) const;

public:

  int32_t numConnection;

  bool isTorrent;

  RequestGroup(const Strings& uris, const Option* option):
    _hintTotalLength(0),
    _uris(uris),
    _segmentMan(0),
    _segmentManFactory(new DefaultSegmentManFactory(option)),
    _option(option),
    logger(LogFactory::getInstance()),
    _chunkChecksum(0),
    _checksum(0),
    _numConcurrentCommand(0),
    numConnection(0),
    isTorrent(false) {}

  RequestGroup(const string& uri, const Option* option):
    _hintTotalLength(0),
    _segmentMan(0),
    _segmentManFactory(new DefaultSegmentManFactory(option)),
    _option(option),
    logger(LogFactory::getInstance()),
    _chunkChecksum(0),
    _numConcurrentCommand(0),
    numConnection(0),
    isTorrent(false)
  {
    _uris.push_back(uri);
  }

  /**
   * Reinitializes SegmentMan based on current property values and
   * returns new one.
   */
  SegmentManHandle initSegmentMan();

  SegmentManHandle getSegmentMan() const
  {
    return _segmentMan;
  }

  Commands createNextCommandWithAdj(DownloadEngine* e, int32_t numAdj);

  Commands createNextCommand(DownloadEngine* e, int32_t numCommand, const string& method = "GET");
  
  void addURI(const string& uri)
  {
    _uris.push_back(uri);
  }

  void setChunkChecksum(const ChunkChecksumHandle& chunkChecksum)
  {
    _chunkChecksum = chunkChecksum;
  }

  ChunkChecksumHandle getChunkChecksum() const
  {
    return _chunkChecksum;
  }

  void initBitfield();

  void openExistingFile();

  void markAllPiecesDone();

  void markExistingPiecesDone();

  void markPieceDone(int64_t length);

  void shouldCancelDownloadForSafety();

  void initAndOpenFile();
  
  bool needsFileAllocation() const;
  
  bool downloadFinished() const
  {
    if(_segmentMan.isNull()) {
      return false;
    } else {
      return _segmentMan->finished();
    }
  }

  void load()
  {
    _segmentMan->load();
  }

  void save()
  {
    _segmentMan->save();
  }

  void remove()
  {
    _segmentMan->remove();
  }

  void closeFile()
  {
    _segmentMan->diskWriter->closeFile();
  }

  bool fileExists() const;

  bool segmentFileExists() const;

  string getFilePath() const;

  int64_t getExistingFileLength() const;

  int64_t getTotalLength() const
  {
    return _segmentMan->totalSize;
  }

  int64_t getDownloadLength() const
  {
    return _segmentMan->getDownloadLength();
  }

  void loadAndOpenFile();

  void prepareForNextAction(int cuid, const RequestHandle& req, DownloadEngine* e, DownloadCommand* downloadCommand = 0);

  bool downloadFinishedByFileLength();

  void setChecksum(const ChecksumHandle& checksum)
  {
    _checksum = checksum;
  }

  ChecksumHandle getChecksum() const
  {
    return _checksum;
  }

  const string& getHintFilename() const
  {
    return _hintFilename;
  }

  void setHintFilename(const string& filename)
  {
    _hintFilename = filename;
  }

  int64_t getHintTotalLength() const
  {
    return _hintTotalLength;
  }

  void setHintTotalLength(int64_t totalLength)
  {
    _hintTotalLength = totalLength;
  }

  const Strings& getRemainingUris() const
  {
    return _uris;
  }

  const Strings& getSpentUris() const
  {
    return _spentUris;
  }

  Strings getUris() const
  {
    Strings temp(_spentUris.begin(), _spentUris.end());
    temp.insert(temp.end(), _uris.begin(), _uris.end());
    return temp;
  }

  /**
   * Compares expected filename with specified actualFilename.
   * The expected filename refers to SegmentMan::filename.
   */
  void validateFilename(const string& actualFilename) const;

  void validateTotalLength(int64_t actualTotalLength) const;

  /**
   * Compares expected filename with specified actualFilename.
   * The expected filename refers to RequestGroup::hintFilename.
   */
  void validateFilenameByHint(const string& actualFilename) const;

  void validateTotalLengthByHint(int64_t actualTotalLength) const;

  void setSegmentManFactory(const SegmentManFactoryHandle& segmentManFactory)
  {
    _segmentManFactory = segmentManFactory;
  }

  void setNumConcurrentCommand(int32_t num)
  {
    _numConcurrentCommand = num;
  }

  void setUserDefinedFilename(const string& filename);
};

typedef SharedHandle<RequestGroup> RequestGroupHandle;
typedef deque<RequestGroupHandle> RequestGroups;

#endif // _D_REQUEST_GROUP_H_
