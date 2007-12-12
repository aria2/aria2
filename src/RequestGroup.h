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
#include "TransferStat.h"

class DownloadEngine;
class SegmentMan;
typedef SharedHandle<SegmentMan> SegmentManHandle;
class SegmentManFactory;
typedef SharedHandle<SegmentManFactory> SegmentManFactoryHandle;
class Command;
typedef deque<Command*> Commands;
class DownloadContext;
typedef SharedHandle<DownloadContext> DownloadContextHandle;
class PieceStorage;
typedef SharedHandle<PieceStorage> PieceStorageHandle;
class BtProgressInfoFile;
typedef SharedHandle<BtProgressInfoFile> BtProgressInfoFileHandle;
class Dependency;
typedef SharedHandle<Dependency> DependencyHandle;
class DlAbortEx;
class PreDownloadHandler;
typedef SharedHandle<PreDownloadHandler> PreDownloadHandlerHandle;
typedef deque<PreDownloadHandlerHandle> PreDownloadHandlers;
class PostDownloadHandler;
typedef SharedHandle<PostDownloadHandler> PostDownloadHandlerHandle;
typedef deque<PostDownloadHandlerHandle> PostDownloadHandlers;
class DiskWriterFactory;
typedef SharedHandle<DiskWriterFactory> DiskWriterFactoryHandle;
class Option;
class Logger;
class RequestGroup;
typedef SharedHandle<RequestGroup> RequestGroupHandle;
typedef deque<RequestGroupHandle> RequestGroups;
class CheckIntegrityEntry;
typedef SharedHandle<CheckIntegrityEntry> CheckIntegrityEntryHandle;
class DownloadResult;
typedef SharedHandle<DownloadResult> DownloadResultHandle;
class ServerHost;
typedef SharedHandle<ServerHost> ServerHostHandle;
typedef deque<ServerHostHandle> ServerHosts;

class RequestGroup {
private:
  static int32_t _gidCounter;

  int32_t _gid;

  Strings _uris;
  Strings _spentUris;

  int32_t _numConcurrentCommand;

  /**
   * This is the number of connections used in streaming protocol(http/ftp)
   */
  int32_t _numStreamConnection;

  int32_t _numCommand;

  SegmentManHandle _segmentMan;
  SegmentManFactoryHandle _segmentManFactory;

  DownloadContextHandle _downloadContext;

  PieceStorageHandle _pieceStorage;

  BtProgressInfoFileHandle _progressInfoFile;

  DiskWriterFactoryHandle _diskWriterFactory;

  DependencyHandle _dependency;

  ServerHosts _serverHosts;

  bool _fileAllocationEnabled;

  bool _preLocalFileCheckEnabled;

  bool _haltRequested;

  bool _forceHaltRequested;

  bool _singleHostMultiConnectionEnabled;

  PreDownloadHandlers _preDownloadHandlers;

  PostDownloadHandlers _postDownloadHandlers;

  const Option* _option;

  const Logger* _logger;

  void validateFilename(const string& expectedFilename,
			const string& actualFilename) const;

  void validateTotalLength(int64_t expectedTotalLength,
			   int64_t actualTotalLength) const;

  void initializePreDownloadHandler();

  void initializePostDownloadHandler();

  bool tryAutoFileRenaming();

public:
  RequestGroup(const Option* option, const Strings& uris);

  ~RequestGroup();
  /**
   * Reinitializes SegmentMan based on current property values and
   * returns new one.
   */
  SegmentManHandle initSegmentMan();

  SegmentManHandle getSegmentMan() const;

  Commands createInitialCommand(DownloadEngine* e);

  Commands createNextCommandWithAdj(DownloadEngine* e, int32_t numAdj);

  Commands createNextCommand(DownloadEngine* e, int32_t numCommand, const string& method = "GET");
  
  void addURI(const string& uri)
  {
    _uris.push_back(uri);
  }

  bool downloadFinished() const;

  bool allDownloadFinished() const;

  void closeFile();

  string getFilePath() const;

  string getDir() const;

  int64_t getTotalLength() const;

  int64_t getCompletedLength() const;

  const Strings& getRemainingUris() const
  {
    return _uris;
  }

  const Strings& getSpentUris() const
  {
    return _spentUris;
  }

  Strings getUris() const;

  /**
   * Compares expected filename with specified actualFilename.
   * The expected filename refers to FileEntry::getBasename() of the first
   * element of DownloadContext::getFileEntries()
   */
  void validateFilename(const string& actualFilename) const;

  void validateTotalLength(int64_t actualTotalLength) const;

  void setSegmentManFactory(const SegmentManFactoryHandle& segmentManFactory);

  void setNumConcurrentCommand(int32_t num)
  {
    _numConcurrentCommand = num;
  }

  int32_t getGID() const
  {
    return _gid;
  }

  TransferStat calculateStat();

  DownloadContextHandle getDownloadContext() const;

  void setDownloadContext(const DownloadContextHandle& downloadContext);

  PieceStorageHandle getPieceStorage() const;

  void setPieceStorage(const PieceStorageHandle& pieceStorage);

  BtProgressInfoFileHandle getProgressInfoFile() const;

  void setProgressInfoFile(const BtProgressInfoFileHandle& progressInfoFile);

  void increaseStreamConnection();

  void decreaseStreamConnection();

  int32_t getNumConnection() const;

  void increaseNumCommand();

  void decreaseNumCommand();

  int32_t getNumCommand() const
  {
    return _numCommand;
  }

  // TODO is it better to move the following 2 methods to SingleFileDownloadContext?
  void setDiskWriterFactory(const DiskWriterFactoryHandle& diskWriterFactory);

  DiskWriterFactoryHandle getDiskWriterFactory() const;

  void setFileAllocationEnabled(bool f)
  {
    _fileAllocationEnabled = f;
  }

  bool isFileAllocationEnabled() const
  {
    return _fileAllocationEnabled;
  }

  bool needsFileAllocation() const;

  /**
   * Setting _preLocalFileCheckEnabled to false, then skip the check to see
   * if a file is already exists and control file exists etc.
   * Always open file with DiskAdaptor::initAndOpenFile()
   */
  void setPreLocalFileCheckEnabled(bool f)
  {
    _preLocalFileCheckEnabled = f;
  }

  bool isPreLocalFileCheckEnabled() const
  {
    return _preLocalFileCheckEnabled;
  }

  void setHaltRequested(bool f);

  void setForceHaltRequested(bool f);

  bool isHaltRequested() const
  {
    return _haltRequested;
  }

  bool isForceHaltRequested() const
  {
    return _forceHaltRequested;
  }

  void dependsOn(const DependencyHandle& dep);

  bool isDependencyResolved();

  void releaseRuntimeResource();

  RequestGroups postDownloadProcessing();

  void addPostDownloadHandler(const PostDownloadHandlerHandle& handler);

  void clearPostDowloadHandler();

  void preDownloadProcessing();

  void addPreDownloadHandler(const PreDownloadHandlerHandle& handler);

  void clearPreDowloadHandler();

  Commands processCheckIntegrityEntry(const CheckIntegrityEntryHandle& entry, DownloadEngine* e);

  void initPieceStorage();

  bool downloadFinishedByFileLength();

  void loadAndOpenFile(const BtProgressInfoFileHandle& progressInfoFile);

  void shouldCancelDownloadForSafety();

  DownloadResultHandle createDownloadResult() const;

  const Option* getOption() const
  {
    return _option;
  }

  bool isSingleHostMultiConnectionEnabled() const
  {
    return _singleHostMultiConnectionEnabled;
  }

  void setSingleHostMultiConnectionEnabled(bool f)
  {
    _singleHostMultiConnectionEnabled = f;
  }

  /**
   * Registers given ServerHost.
   */
  void registerServerHost(const ServerHostHandle& serverHost);

  /**
   * Returns ServerHost whose cuid is given cuid. If it is not found, returns
   * 0.
   */
  ServerHostHandle searchServerHost(int32_t cuid) const;

  ServerHostHandle searchServerHost(const string& hostname) const;

  void removeServerHost(int32_t cuid);
  
  void removeURIWhoseHostnameIs(const string& hostname);
};

typedef SharedHandle<RequestGroup> RequestGroupHandle;
typedef WeakHandle<RequestGroup> RequestGroupWeakHandle;
typedef deque<RequestGroupHandle> RequestGroups;

#endif // _D_REQUEST_GROUP_H_
