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
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "DefaultSegmentManFactory.h"
#include "NullProgressInfoFile.h"
#include "SegmentManFactory.h"
#include "Dependency.h"
#include "prefs.h"
#include "RequestFactory.h"
#include "InitiateConnectionCommandFactory.h"
#include "CUIDCounter.h"
#include "File.h"
#include "message.h"
#include "Util.h"
#include "BtRegistry.h"
#include "LogFactory.h"
#include "DiskAdaptor.h"
#include "DiskWriterFactory.h"
#include "RecoverableException.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "CheckIntegrityCommand.h"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_BITTORRENT
# include "BtCheckIntegrityEntry.h"
# include "DefaultPieceStorage.h"
# include "DefaultBtProgressInfoFile.h"
# include "DefaultPeerStorage.h"
# include "DefaultBtAnnounce.h"
# include "BtSetup.h"
# include "BtFileAllocationEntry.h"
# include "BtPostDownloadHandler.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
# include "MetalinkPostDownloadHandler.h"
#endif // ENABLE_METALINK
#include <cerrno>

int32_t RequestGroup::_gidCounter = 0;

RequestGroup::RequestGroup(const Option* option,
			   const Strings& uris):
  _gid(++_gidCounter),
  _hintTotalLength(0),
  _uris(uris),
  _numConcurrentCommand(0),
  _numStreamConnection(0),
  _numCommand(0),
  _segmentMan(0),
  _segmentManFactory(new DefaultSegmentManFactory(option)),
  _downloadContext(0),
  _pieceStorage(0),
  _progressInfoFile(new NullProgressInfoFile()),
  _diskWriterFactory(0),
  _dependency(0),
  _preLocalFileCheckEnabled(true),
  _haltRequested(false),
  _option(option),
  _logger(LogFactory::getInstance())
{
  if(_option->get(PREF_FILE_ALLOCATION) == V_PREALLOC) {
    _fileAllocationEnabled = true;
  } else {
    _fileAllocationEnabled = false;
  }
  initializePostDownloadHandler();
}

RequestGroup::~RequestGroup() {}

SegmentManHandle RequestGroup::initSegmentMan()
{
  _segmentMan = _segmentManFactory->createNewInstance(_downloadContext,
						      _pieceStorage);
  return _segmentMan;
}

bool RequestGroup::downloadFinished() const
{
  if(_pieceStorage.isNull()) {
    return false;
  } else {
    return _pieceStorage->downloadFinished();
  }
}

void RequestGroup::closeFile()
{
  if(!_pieceStorage.isNull()) {
    _pieceStorage->getDiskAdaptor()->closeFile();
  }
}

Commands RequestGroup::createInitialCommand(DownloadEngine* e)
{
  // If this includes torrent download, then this method returns
  // the command that torrent download requires, such as
  // TrackerWatcherCommand and so on.

  // It is better to avoid to using BtTorrent specific classes here.
#ifdef ENABLE_BITTORRENT
  {
    BtContextHandle btContext = _downloadContext;
    if(!btContext.isNull()) {
      if(btContext->getFileEntries().size() > 1) {
	// this is really multi file torrent.
	// clear http/ftp uris because the current implementation does not
	// allow integrating multi-file torrent and http/ftp.
	_logger->debug("Clearing http/ftp URIs because the current implementation does not allow integrating multi-file torrent and http/ftp.");
	_uris.clear();
      }

      _pieceStorage = new DefaultPieceStorage(btContext, _option);
      _pieceStorage->initStorage();
      initSegmentMan();

      BtProgressInfoFileHandle progressInfoFile =
	new DefaultBtProgressInfoFile(_downloadContext,
				      _pieceStorage,
				      _option);
      
      BtRegistry::registerBtContext(btContext->getInfoHashAsString(), btContext);
      BtRegistry::registerPieceStorage(btContext->getInfoHashAsString(),
				       _pieceStorage);
      BtRegistry::registerBtProgressInfoFile(btContext->getInfoHashAsString(),
					     progressInfoFile);

  
      BtRuntimeHandle btRuntime = new BtRuntime();
      btRuntime->setListenPort(_option->getAsInt(PREF_LISTEN_PORT));
      BtRegistry::registerBtRuntime(btContext->getInfoHashAsString(), btRuntime);

      PeerStorageHandle peerStorage = new DefaultPeerStorage(btContext, _option);
      BtRegistry::registerPeerStorage(btContext->getInfoHashAsString(), peerStorage);

      BtAnnounceHandle btAnnounce = new DefaultBtAnnounce(btContext, _option);
      BtRegistry::registerBtAnnounce(btContext->getInfoHashAsString(), btAnnounce);
      btAnnounce->shuffleAnnounce();
      
      BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					    new PeerObjectCluster());

      // Call Load, Save and file allocation command here
      if(progressInfoFile->exists()) {
	// load .aria2 file if it exists.
	progressInfoFile->load();
	_pieceStorage->getDiskAdaptor()->openFile();
      } else {
	if(_pieceStorage->getDiskAdaptor()->fileExists()) {
	  if(_option->get(PREF_ALLOW_OVERWRITE) != V_TRUE) {
	    _logger->error(MSG_FILE_ALREADY_EXISTS,
			   getFilePath().c_str(),
			   progressInfoFile->getFilename().c_str());
	    // TODO we need this->haltRequested = true?
	    return Commands();
	  } else {
	    _pieceStorage->getDiskAdaptor()->openFile();
	  }
	} else {
	  _pieceStorage->getDiskAdaptor()->openFile();
	}
      }
      _progressInfoFile = progressInfoFile;
      Commands commands;
      CheckIntegrityEntryHandle entry =	new BtCheckIntegrityEntry(this);
#ifdef ENABLE_MESSAGE_DIGEST
      if(File(getFilePath()).size() > 0 &&
	 e->option->get(PREF_CHECK_INTEGRITY) == V_TRUE &&
	 entry->isValidationReady()) {
	entry->initValidator();
	CheckIntegrityCommand* command =
	  new CheckIntegrityCommand(CUIDCounterSingletonHolder::instance()->newID(), this, e, entry);
	commands.push_back(command);
      } else
#endif // ENABLE_MESSAGE_DIGEST
	{
	  commands = entry->prepareForNextAction(e);
	}
      return commands;
    }
  }
#endif // ENABLE_BITTORRENT
  return createNextCommand(e, 1);
}

Commands RequestGroup::createNextCommandWithAdj(DownloadEngine* e, int32_t numAdj)
{
  int32_t numCommand = _numConcurrentCommand == 0 ? _uris.size() : _numConcurrentCommand+numAdj;
  return createNextCommand(e, numCommand, "GET");
}

Commands RequestGroup::createNextCommand(DownloadEngine* e, int32_t numCommand, const string& method)
{
  Commands commands;
  for(;!_uris.empty() && numCommand--; _uris.pop_front()) {
    string uri = _uris.front();
    _spentUris.push_back(uri);
    RequestHandle req = RequestFactorySingletonHolder::instance()->createRequest();
    req->setReferer(_option->get(PREF_REFERER));
    req->setMethod(method);
    if(req->setUrl(uri)) {
      commands.push_back(InitiateConnectionCommandFactory::createInitiateConnectionCommand(CUIDCounterSingletonHolder::instance()->newID(), req, this, e));
    } else {
      _logger->error(MSG_UNRECOGNIZED_URI, req->getUrl().c_str());
    }
  }
  return commands;
}

string RequestGroup::getFilePath() const
{
  assert(!_downloadContext.isNull());
  if(_downloadContext.isNull()) {
    return "";
  } else {
    return _downloadContext->getActualBasePath();
  }
}

int64_t RequestGroup::getTotalLength() const
{
  if(_pieceStorage.isNull()) {
    return 0;
  } else {
    if(_pieceStorage->isSelectiveDownloadingMode()) {
      return _pieceStorage->getFilteredTotalLength();
    } else {
      return _pieceStorage->getTotalLength();
    }
  }
}

int64_t RequestGroup::getCompletedLength() const
{
  if(_pieceStorage.isNull()) {
    return 0;
  } else {
    if(_pieceStorage->isSelectiveDownloadingMode()) {
      return _pieceStorage->getFilteredCompletedLength();
    } else {
      return _pieceStorage->getCompletedLength();
    }
  }
}

void RequestGroup::validateFilename(const string& expectedFilename,
				    const string& actualFilename) const
{
  if(expectedFilename.empty()) {
    return;
  }
  if(expectedFilename != actualFilename) {
    throw new DlAbortEx(EX_FILENAME_MISMATCH,
			expectedFilename.c_str(),
			actualFilename.c_str());
  }
}

void RequestGroup::validateTotalLength(int64_t expectedTotalLength,
				       int64_t actualTotalLength) const
{
  if(expectedTotalLength <= 0) {
    return;
  }
  if(expectedTotalLength != actualTotalLength) {
    throw new DlAbortEx(EX_SIZE_MISMATCH,
			Util::llitos(expectedTotalLength, true).c_str(),
			Util::llitos(actualTotalLength, true).c_str());
  }
}

void RequestGroup::validateFilename(const string& actualFilename) const
{
  validateFilename(_downloadContext->getFileEntries().front()->getBasename(), actualFilename);
}

void RequestGroup::validateTotalLength(int64_t actualTotalLength) const
{
  validateTotalLength(getTotalLength(), actualTotalLength);
}

void RequestGroup::validateFilenameByHint(const string& actualFilename) const
{
  validateFilename(_hintFilename, actualFilename);
}

void RequestGroup::validateTotalLengthByHint(int64_t actualTotalLength) const
{
  validateTotalLength(_hintTotalLength, actualTotalLength);
}

void RequestGroup::increaseStreamConnection()
{
  ++_numStreamConnection;
}

void RequestGroup::decreaseStreamConnection()
{
  --_numStreamConnection;
}

int32_t RequestGroup::getNumConnection() const
{
  int32_t numConnection = _numStreamConnection;
#ifdef ENABLE_BITTORRENT
  {
    BtContextHandle btContext = _downloadContext;
    if(!btContext.isNull()) {
      BtRuntimeHandle btRuntime = BT_RUNTIME(btContext);
      if(!btRuntime.isNull()) {
	numConnection += btRuntime->getConnections();
      }
    }
  }
#endif // ENABLE_BITTORRENT
  return numConnection;
}

void RequestGroup::increaseNumCommand()
{
  ++_numCommand;
}

void RequestGroup::decreaseNumCommand()
{
  --_numCommand;
}


TransferStat RequestGroup::calculateStat()
{
  TransferStat stat;
#ifdef ENABLE_BITTORRENT
  {
    BtContextHandle btContext = _downloadContext;
    if(!btContext.isNull()) {
      PeerStorageHandle peerStorage = PEER_STORAGE(btContext);
      if(!peerStorage.isNull()) {
	stat = peerStorage->calculateStat();
      }
    }
  }
#endif // ENABLE_BITTORRENT
  if(!_segmentMan.isNull()) {
    stat.setDownloadSpeed(stat.getDownloadSpeed()+_segmentMan->calculateDownloadSpeed());
  }
  return stat;
}

void RequestGroup::setHaltRequested(bool f)
{
  _haltRequested = f;
#ifdef ENABLE_BITTORRENT
  {
    BtContextHandle btContext = _downloadContext;
    if(!btContext.isNull()) {
      BtRuntimeHandle btRuntime = BT_RUNTIME(btContext);
      if(!btRuntime.isNull()) {
	btRuntime->setHalt(f);
      }
    }
  }
#endif // ENABLE_BITTORRENT
}

void RequestGroup::releaseRuntimeResource()
{
#ifdef ENABLE_BITTORRENT
  BtContextHandle btContext = _downloadContext;
  if(!btContext.isNull()) {
    BtRegistry::unregister(btContext->getInfoHashAsString());
  }
#endif // ENABLE_BITTORRENT
  if(!_pieceStorage.isNull()) {
    _pieceStorage->removeAdvertisedPiece(0);
  }
}

RequestGroups RequestGroup::postDownloadProcessing()
{
  _logger->debug("Finding PostDownloadHandler for path %s.", getFilePath().c_str());
  try {
    for(PostDownloadHandlers::const_iterator itr = _postDownloadHandlers.begin();
	itr != _postDownloadHandlers.end(); ++itr) {
      if((*itr)->canHandle(getFilePath())) {
	return (*itr)->getNextRequestGroups(getFilePath());
      }
    }
  } catch(RecoverableException* ex) {
    _logger->error(EX_EXCEPTION_CAUGHT, ex);
    delete ex;
    return RequestGroups();
  }
  _logger->debug("No PostDownloadHandler found.");
  return RequestGroups();
}

void RequestGroup::initializePostDownloadHandler()
{
  _postDownloadHandlers.push_back(new BtPostDownloadHandler(_option));
  _postDownloadHandlers.push_back(new MetalinkPostDownloadHandler(_option));
}

Strings RequestGroup::getUris() const
{
  Strings temp(_spentUris.begin(), _spentUris.end());
  temp.insert(temp.end(), _uris.begin(), _uris.end());
  return temp;
}

bool RequestGroup::isDependencyResolved()
{
  if(_dependency.isNull()) {
    return true;
  }
  return _dependency->resolve();
}

void RequestGroup::setSegmentManFactory(const SegmentManFactoryHandle& segmentManFactory)
{
  _segmentManFactory = segmentManFactory;
}

void RequestGroup::dependsOn(const DependencyHandle& dep)
{
  _dependency = dep;
}

void RequestGroup::setDiskWriterFactory(const DiskWriterFactoryHandle& diskWriterFactory)
{
  _diskWriterFactory = diskWriterFactory;
}

DiskWriterFactoryHandle RequestGroup::getDiskWriterFactory() const
{
  return _diskWriterFactory;
}

void RequestGroup::addPostDownloadHandler(const PostDownloadHandlerHandle& handler)
{
  _postDownloadHandlers.push_back(handler);
}

void RequestGroup::clearPostDowloadHandler()
{
  _postDownloadHandlers.clear();
}

SegmentManHandle RequestGroup::getSegmentMan() const
{
  return _segmentMan;
}

DownloadContextHandle RequestGroup::getDownloadContext() const
{
  return _downloadContext;
}

void RequestGroup::setDownloadContext(const DownloadContextHandle& downloadContext)
{
  _downloadContext = downloadContext;
}

PieceStorageHandle RequestGroup::getPieceStorage() const
{
  return _pieceStorage;
}

void RequestGroup::setPieceStorage(const PieceStorageHandle& pieceStorage)
{
  _pieceStorage = pieceStorage;
}

BtProgressInfoFileHandle RequestGroup::getProgressInfoFile() const
{
  return _progressInfoFile;
}

void RequestGroup::setProgressInfoFile(const BtProgressInfoFileHandle& progressInfoFile)
{
  _progressInfoFile = progressInfoFile;
}

bool RequestGroup::needsFileAllocation() const
{
  return isFileAllocationEnabled() &&
    _option->getAsLLInt(PREF_NO_FILE_ALLOCATION_LIMIT) <= getTotalLength() &&
    !_pieceStorage->getDiskAdaptor()->fileAllocationIterator()->finished();
}
