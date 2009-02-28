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

#include <string>
#include <deque>

#include "SharedHandle.h"
#include "TransferStat.h"
#include "TimeA2.h"
#include "Request.h"
#include "DownloadResult.h"
#include "URIResult.h"

namespace aria2 {

class DownloadEngine;
class SegmentMan;
class SegmentManFactory;
class Command;
class DownloadCommand;
class DownloadContext;
class PieceStorage;
class BtProgressInfoFile;
class Dependency;
class PreDownloadHandler;
class PostDownloadHandler;
class DiskWriterFactory;
class Option;
class Logger;
class RequestGroup;
class CheckIntegrityEntry;
class DownloadResult;
class ServerHost;
class URISelector;
#ifdef ENABLE_BITTORRENT
class BtRuntime;
class PeerStorage;
#endif // ENABLE_BITTORRENT

class RequestGroup {
private:
  static int32_t _gidCounter;

  int32_t _gid;

  std::deque<std::string> _uris;
  std::deque<std::string> _spentUris;

  unsigned int _numConcurrentCommand;

  /**
   * This is the number of connections used in streaming protocol(http/ftp)
   */
  unsigned int _numStreamConnection;

  unsigned int _numCommand;

  SharedHandle<SegmentMan> _segmentMan;
  SharedHandle<SegmentManFactory> _segmentManFactory;

  SharedHandle<DownloadContext> _downloadContext;

  SharedHandle<PieceStorage> _pieceStorage;

  SharedHandle<BtProgressInfoFile> _progressInfoFile;

  SharedHandle<DiskWriterFactory> _diskWriterFactory;

  SharedHandle<Dependency> _dependency;

  std::deque<SharedHandle<ServerHost> > _serverHosts;

  bool _fileAllocationEnabled;

  bool _preLocalFileCheckEnabled;

  bool _haltRequested;

  bool _forceHaltRequested;

  // URIResult is stored in the ascending order of the time when its result is
  // available.
  std::deque<URIResult> _uriResults;

  bool _singleHostMultiConnectionEnabled;

  std::deque<SharedHandle<PreDownloadHandler> > _preDownloadHandlers;

  std::deque<SharedHandle<PostDownloadHandler> > _postDownloadHandlers;

  std::deque<std::string> _acceptFeatures;

  std::deque<std::string> _acceptTypes;

  SharedHandle<URISelector> _uriSelector;

  Time _lastModifiedTime;

  unsigned int _fileNotFoundCount;

  // Timeout used for HTTP/FTP downloads.
  time_t _timeout;

  // How many times HTTP/FTP download should retry.
  unsigned int _maxTries;

#ifdef ENABLE_BITTORRENT
  WeakHandle<BtRuntime> _btRuntime;

  WeakHandle<PeerStorage> _peerStorage;
#endif // ENABLE_BITTORRENT

  // This flag just indicates that the downloaded file is not saved disk but
  // just sits in memory.
  bool _inMemoryDownload;

  unsigned int _maxDownloadSpeedLimit;

  unsigned int _maxUploadSpeedLimit;

  const Option* _option;

  Logger* _logger;

  void validateFilename(const std::string& expectedFilename,
			const std::string& actualFilename) const;

  void validateTotalLength(uint64_t expectedTotalLength,
			   uint64_t actualTotalLength) const;

  void initializePreDownloadHandler();

  void initializePostDownloadHandler();

  bool tryAutoFileRenaming();

  // Returns the result code of this RequestGroup.
  // If the download finished, then returns DownloadResult::FINISHED.
  // If the download didn't finish and error result is available in _uriResults,
  // then last result code is returned.
  // Otherwise returns DownloadResult::UNKNOWN_ERROR.
  DownloadResult::RESULT downloadResult() const;
public:
  RequestGroup(const Option* option, const std::deque<std::string>& uris);

  ~RequestGroup();
  /**
   * Reinitializes SegmentMan based on current property values and
   * returns new one.
   */
  SharedHandle<SegmentMan> initSegmentMan();

  SharedHandle<SegmentMan> getSegmentMan() const;

  // Returns first bootstrap commands to initiate a download.
  // If this is HTTP/FTP download and file size is unknown, only 1 command
  // (usually, HttpInitiateConnection or FtpInitiateConnection) will be created
  // with its Request object having Requet::METHOD_HEAD in its method.
  // This behavior can be changed by providing 3rd argument.
  // The method has effect only for using HTTP request including FTP via HTTP
  // proxy.
  void createInitialCommand(std::deque<Command*>& commands,
			    DownloadEngine* e,
			    const std::string& method = Request::METHOD_HEAD);

  void createNextCommandWithAdj(std::deque<Command*>& commands,
				DownloadEngine* e, int numAdj);

  void createNextCommand(std::deque<Command*>& commands,
			 DownloadEngine* e, unsigned int numCommand,
			 const std::string& method = Request::METHOD_GET);
  
  void addURI(const std::string& uri)
  {
    _uris.push_back(uri);
  }

  bool downloadFinished() const;

  bool allDownloadFinished() const;

  void closeFile();

  std::string getFilePath() const;

  uint64_t getTotalLength() const;

  uint64_t getCompletedLength() const;

  const std::deque<std::string>& getRemainingUris() const
  {
    return _uris;
  }

  const std::deque<std::string>& getSpentUris() const
  {
    return _spentUris;
  }

  void getURIs(std::deque<std::string>& uris) const;

  /**
   * Compares expected filename with specified actualFilename.
   * The expected filename refers to FileEntry::getBasename() of the first
   * element of DownloadContext::getFileEntries()
   */
  void validateFilename(const std::string& actualFilename) const;

  void validateTotalLength(uint64_t actualTotalLength) const;

  void setSegmentManFactory(const SharedHandle<SegmentManFactory>& segmentManFactory);

  void setNumConcurrentCommand(unsigned int num)
  {
    _numConcurrentCommand = num;
  }

  unsigned int getNumConcurrentCommand() const;

  int32_t getGID() const
  {
    return _gid;
  }

  TransferStat calculateStat();

  SharedHandle<DownloadContext> getDownloadContext() const;

  void setDownloadContext(const SharedHandle<DownloadContext>& downloadContext);

  SharedHandle<PieceStorage> getPieceStorage() const;

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  SharedHandle<BtProgressInfoFile> getProgressInfoFile() const;

  void setProgressInfoFile(const SharedHandle<BtProgressInfoFile>& progressInfoFile);

  void increaseStreamConnection();

  void decreaseStreamConnection();

  unsigned int getNumConnection() const;

  void increaseNumCommand();

  void decreaseNumCommand();

  unsigned int getNumCommand() const
  {
    return _numCommand;
  }

  // TODO is it better to move the following 2 methods to SingleFileDownloadContext?
  void setDiskWriterFactory(const SharedHandle<DiskWriterFactory>& diskWriterFactory);

  SharedHandle<DiskWriterFactory> getDiskWriterFactory() const;

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

  void addURIResult(std::string uri, DownloadResult::RESULT result);

  const std::deque<URIResult>& getURIResults() const;

  // Extracts URIResult whose _result is r and stores them into res.
  // The extracted URIResults are removed from _uriResults.
  void extractURIResult(std::deque<URIResult>& res, DownloadResult::RESULT r);

  void dependsOn(const SharedHandle<Dependency>& dep);

  bool isDependencyResolved();

  void releaseRuntimeResource(DownloadEngine* e);

  void postDownloadProcessing(std::deque<SharedHandle<RequestGroup> >& groups);

  void addPostDownloadHandler(const SharedHandle<PostDownloadHandler>& handler);

  void clearPostDowloadHandler();

  void preDownloadProcessing();

  void addPreDownloadHandler(const SharedHandle<PreDownloadHandler>& handler);

  void clearPreDowloadHandler();

  void processCheckIntegrityEntry(std::deque<Command*>& commands,
				  const SharedHandle<CheckIntegrityEntry>& entry,
				  DownloadEngine* e);

  void initPieceStorage();

  bool downloadFinishedByFileLength();

  void loadAndOpenFile(const SharedHandle<BtProgressInfoFile>& progressInfoFile);

  void shouldCancelDownloadForSafety();

  SharedHandle<DownloadResult> createDownloadResult() const;

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
  void registerServerHost(const SharedHandle<ServerHost>& serverHost);

  /**
   * Returns ServerHost whose cuid is given cuid. If it is not found, returns
   * 0.
   */
  SharedHandle<ServerHost> searchServerHost(int32_t cuid) const;

  SharedHandle<ServerHost> searchServerHost(const std::string& hostname) const;

  void removeServerHost(int32_t cuid);
  
  void removeURIWhoseHostnameIs(const std::string& hostname);

  void removeIdenticalURI(const std::string& uri);

  void reportDownloadFinished();

  const std::deque<std::string>& getAcceptFeatures() const;

  void addAcceptFeatureHeader(const std::string& feature);

  void removeAcceptFeatureHeader(const std::string& feature);

  const std::deque<std::string>& getAcceptTypes() const;

  void addAcceptType(const std::string& type);

  void removeAcceptType(const std::string& type);

  static const std::string ACCEPT_METALINK;

  void setURISelector(const SharedHandle<URISelector>& uriSelector);

  void applyLastModifiedTimeToLocalFiles();

  void updateLastModifiedTime(const Time& time);

  void increaseAndValidateFileNotFoundCount();

  // Just set inMemoryDownload flag true.
  void markInMemoryDownload();

  // Returns inMemoryDownload flag.
  bool inMemoryDownload() const;

  void tuneDownloadCommand(DownloadCommand* command);

  void setTimeout(time_t timeout);

  time_t getTimeout() const;

  void setMaxTries(unsigned int maxTries);

  unsigned int getMaxTries() const;

  // Returns true if current download speed exceeds
  // _maxDownloadSpeedLimit.  Always returns false if
  // _maxDownloadSpeedLimit == 0.  Otherwise returns false.
  bool doesDownloadSpeedExceed();

  // Returns true if current upload speed exceeds
  // _maxUploadSpeedLimit. Always returns false if
  // _maxUploadSpeedLimit == 0. Otherwise returns false.
  bool doesUploadSpeedExceed();
};

typedef SharedHandle<RequestGroup> RequestGroupHandle;
typedef WeakHandle<RequestGroup> RequestGroupWeakHandle;
typedef std::deque<RequestGroupHandle> RequestGroups;

} // namespace aria2

#endif // _D_REQUEST_GROUP_H_
