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

#include <cassert>
#include <algorithm>

#include "PostDownloadHandler.h"
#include "DownloadEngine.h"
#include "DefaultSegmentManFactory.h"
#include "SegmentMan.h"
#include "NullProgressInfoFile.h"
#include "Dependency.h"
#include "prefs.h"
#include "CreateRequestCommand.h"
#include "File.h"
#include "message.h"
#include "util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DiskAdaptor.h"
#include "DiskWriterFactory.h"
#include "RecoverableException.h"
#include "StreamCheckIntegrityEntry.h"
#include "CheckIntegrityCommand.h"
#include "UnknownLengthPieceStorage.h"
#include "DownloadContext.h"
#include "DlAbortEx.h"
#include "DownloadFailureException.h"
#include "RequestGroupMan.h"
#include "DefaultBtProgressInfoFile.h"
#include "DefaultPieceStorage.h"
#include "DownloadHandlerFactory.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "DownloadHandlerConstants.h"
#include "Option.h"
#include "FileEntry.h"
#include "Request.h"
#include "FileAllocationIterator.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "URISelector.h"
#include "InOrderURISelector.h"
#include "PieceSelector.h"
#include "a2functional.h"
#include "SocketCore.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "CheckIntegrityCommand.h"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
# include "BtRegistry.h"
# include "BtCheckIntegrityEntry.h"
# include "DefaultPeerStorage.h"
# include "DefaultBtAnnounce.h"
# include "BtRuntime.h"
# include "BtSetup.h"
# include "BtFileAllocationEntry.h"
# include "BtPostDownloadHandler.h"
# include "DHTSetup.h"
# include "DHTRegistry.h"
# include "BtMessageFactory.h"
# include "BtRequestFactory.h"
# include "BtMessageDispatcher.h"
# include "BtMessageReceiver.h"
# include "PeerConnection.h"
# include "ExtensionMessageFactory.h"
# include "DHTPeerAnnounceStorage.h"
# include "DHTEntryPointNameResolveCommand.h"
# include "LongestSequencePieceSelector.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
# include "MetalinkPostDownloadHandler.h"
#endif // ENABLE_METALINK

namespace aria2 {

int32_t RequestGroup::_gidCounter = 0;

const std::string RequestGroup::ACCEPT_METALINK = "application/metalink+xml";

RequestGroup::RequestGroup(const SharedHandle<Option>& option):
  _gid(++_gidCounter),
  _option(new Option(*option.get())),
  _numConcurrentCommand(option->getAsInt(PREF_SPLIT)),
  _numStreamConnection(0),
  _numCommand(0),
  _segmentManFactory(new DefaultSegmentManFactory(_option.get())),
  _saveControlFile(true),
  _progressInfoFile(new NullProgressInfoFile()),
  _preLocalFileCheckEnabled(true),
  _haltRequested(false),
  _forceHaltRequested(false),
  _haltReason(RequestGroup::NONE),
  _uriSelector(new InOrderURISelector()),
  _lastModifiedTime(Time::null()),
  _fileNotFoundCount(0),
  _timeout(option->getAsInt(PREF_TIMEOUT)),
  _inMemoryDownload(false),
  _maxDownloadSpeedLimit(option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT)),
  _maxUploadSpeedLimit(option->getAsInt(PREF_MAX_UPLOAD_LIMIT)),
  _logger(LogFactory::getInstance())
{
  _fileAllocationEnabled = _option->get(PREF_FILE_ALLOCATION) != V_NONE;
  // Add types to be sent as a Accept header value here.
  // It would be good to put this value in Option so that user can tweak
  // and add this list.
  // ACCEPT_METALINK is used for `transparent metalink'.
  addAcceptType(ACCEPT_METALINK);
  if(!_option->getAsBool(PREF_DRY_RUN)) {
    initializePreDownloadHandler();
    initializePostDownloadHandler();
  }
}

RequestGroup::~RequestGroup() {}

const SegmentManHandle& RequestGroup::initSegmentMan()
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

bool RequestGroup::allDownloadFinished() const
{
  if(_pieceStorage.isNull()) {
    return false;
  } else {
    return _pieceStorage->allDownloadFinished();
  }
}

downloadresultcode::RESULT RequestGroup::downloadResult() const
{
  if (downloadFinished())
    return downloadresultcode::FINISHED;
  else {
    if (_lastUriResult.isNull()) {
      if(_haltReason == RequestGroup::USER_REQUEST) {
	return downloadresultcode::IN_PROGRESS;
      } else {
	return downloadresultcode::UNKNOWN_ERROR;
      }
    } else {
      return _lastUriResult->getResult();
    }
  }    
}

void RequestGroup::closeFile()
{
  if(!_pieceStorage.isNull()) {
    _pieceStorage->getDiskAdaptor()->closeFile();
  }
}

void RequestGroup::createInitialCommand(std::deque<Command*>& commands,
					DownloadEngine* e)
{
#ifdef ENABLE_BITTORRENT
  {
    if(_downloadContext->hasAttribute(bittorrent::BITTORRENT)) {
      const BDE& torrentAttrs =
	_downloadContext->getAttribute(bittorrent::BITTORRENT);
      if(_option->getAsBool(PREF_DRY_RUN)) {
	throw DOWNLOAD_FAILURE_EXCEPTION
	  ("Cancel BitTorrent download in dry-run context.");
      }
      SharedHandle<BtRegistry> btRegistry = e->getBtRegistry();
      if(!btRegistry->getDownloadContext
	 (torrentAttrs[bittorrent::INFO_HASH].s()).isNull()) {
	throw DOWNLOAD_FAILURE_EXCEPTION
	  (StringFormat
	   ("InfoHash %s is already registered.",
	    util::toHex(torrentAttrs[bittorrent::INFO_HASH].s()).c_str()).str());
      }

      if(e->_requestGroupMan->isSameFileBeingDownloaded(this)) {
	throw DOWNLOAD_FAILURE_EXCEPTION
	  (StringFormat(EX_DUPLICATE_FILE_DOWNLOAD,
			_downloadContext->getBasePath().c_str()).str());
      }
      initPieceStorage();
      if(_downloadContext->getFileEntries().size() > 1) {
	_pieceStorage->setupFileFilter();
      }
      
      SharedHandle<DefaultBtProgressInfoFile>
	progressInfoFile(new DefaultBtProgressInfoFile(_downloadContext,
						       _pieceStorage,
						       _option.get()));
        
      BtRuntimeHandle btRuntime(new BtRuntime());
      btRuntime->setMaxPeers(_option->getAsInt(PREF_BT_MAX_PEERS));
      _btRuntime = btRuntime;
      progressInfoFile->setBtRuntime(btRuntime);

      SharedHandle<DefaultPeerStorage> peerStorage
	(new DefaultPeerStorage(_option.get()));
      peerStorage->setBtRuntime(btRuntime);
      peerStorage->setPieceStorage(_pieceStorage);
      _peerStorage = peerStorage;
      progressInfoFile->setPeerStorage(peerStorage);

      SharedHandle<DefaultBtAnnounce> btAnnounce
	(new DefaultBtAnnounce(_downloadContext, _option.get()));
      btAnnounce->setBtRuntime(btRuntime);
      btAnnounce->setPieceStorage(_pieceStorage);
      btAnnounce->setPeerStorage(peerStorage);
      btAnnounce->setUserDefinedInterval
	(_option->getAsInt(PREF_BT_TRACKER_INTERVAL));
      btAnnounce->shuffleAnnounce();
      
      btRegistry->put(torrentAttrs[bittorrent::INFO_HASH].s(),
		      BtObject(_downloadContext,
			       _pieceStorage,
			       peerStorage,
			       btAnnounce,
			       btRuntime,
			       progressInfoFile));

      // Remove the control file if download file doesn't exist
      if(progressInfoFile->exists() && !_pieceStorage->getDiskAdaptor()->fileExists()) {
	progressInfoFile->removeFile();
	_logger->notice(MSG_REMOVED_DEFUNCT_CONTROL_FILE,
			progressInfoFile->getFilename().c_str(),
			_downloadContext->getBasePath().c_str());
      }
      {
	uint64_t actualFileSize = _pieceStorage->getDiskAdaptor()->size();
	if(actualFileSize == _downloadContext->getTotalLength()) {
	  // First, make DiskAdaptor read-only mode to allow the
	  // program to seed file in read-only media.
	  _pieceStorage->getDiskAdaptor()->enableReadOnly();
	} else {
	  // Open file in writable mode to allow the program
	  // truncate the file to _downloadContext->getTotalLength()
	  _logger->debug("File size not match. File is opened in writable mode."
			 " Expected:%s Actual:%s",
			 util::uitos(_downloadContext->getTotalLength()).c_str(),
			 util::uitos(actualFileSize).c_str());
	}
      }
      // Call Load, Save and file allocation command here
      if(progressInfoFile->exists()) {
	// load .aria2 file if it exists.
	progressInfoFile->load();
	_pieceStorage->getDiskAdaptor()->openFile();
      } else {
	if(_pieceStorage->getDiskAdaptor()->fileExists()) {
	  if(!_option->getAsBool(PREF_CHECK_INTEGRITY) &&
	     !_option->getAsBool(PREF_ALLOW_OVERWRITE) &&
	     !_option->getAsBool(PREF_BT_SEED_UNVERIFIED)) {
	    // TODO we need this->haltRequested = true?
	    throw DOWNLOAD_FAILURE_EXCEPTION
	      (StringFormat
	       (MSG_FILE_ALREADY_EXISTS,
		_downloadContext->getBasePath().c_str()).str());
	  } else {
	    _pieceStorage->getDiskAdaptor()->openFile();
	  }
	  if(_option->getAsBool(PREF_BT_SEED_UNVERIFIED)) {
	    _pieceStorage->markAllPiecesDone();
	  }
	} else {
	  _pieceStorage->getDiskAdaptor()->openFile();
	}
      }
      _progressInfoFile = progressInfoFile;

      if(torrentAttrs[bittorrent::PRIVATE].i() == 0 &&
	 _option->getAsBool(PREF_ENABLE_DHT)) {
	std::deque<Command*> commands;
	DHTSetup().setup(commands, e, _option.get());
	e->addCommand(commands);
	if(!torrentAttrs[bittorrent::NODES].empty() && DHTSetup::initialized()) {
	  std::deque<std::pair<std::string, uint16_t> > entryPoints;
	  const BDE& nodes = torrentAttrs[bittorrent::NODES];
	  for(BDE::List::const_iterator i = nodes.listBegin();
	      i != nodes.listEnd(); ++i) {
	    std::pair<std::string, uint16_t> addr
	      ((*i)[bittorrent::HOSTNAME].s(), (*i)[bittorrent::PORT].i());
	    entryPoints.push_back(addr);
	  }
	  DHTEntryPointNameResolveCommand* command =
	    new DHTEntryPointNameResolveCommand(e->newCUID(), e, entryPoints);
	  command->setTaskQueue(DHTRegistry::_taskQueue);
	  command->setTaskFactory(DHTRegistry::_taskFactory);
	  command->setRoutingTable(DHTRegistry::_routingTable);
	  command->setLocalNode(DHTRegistry::_localNode);
	  e->commands.push_back(command);
	}
      }
      CheckIntegrityEntryHandle entry(new BtCheckIntegrityEntry(this));
      // --bt-seed-unverified=true is given and download has completed, skip
      // validation for piece hashes.
      if(_option->getAsBool(PREF_BT_SEED_UNVERIFIED) &&
	 _pieceStorage->downloadFinished()) {
	entry->onDownloadFinished(commands, e);
      } else {
	processCheckIntegrityEntry(commands, entry, e);
      }
      return;
    }
  }
#endif // ENABLE_BITTORRENT
  // TODO Currently, BitTorrent+WEB-Seeding is only way to download
  // multiple files in one RequestGroup. In this context, we don't
  // have BitTorrent, so add assertion here. This situation will be
  // changed if Metalink spec is formalized to support multi-file
  // torrent.
  assert(_downloadContext->getFileEntries().size() == 1);
  // TODO I assume here when totallength is set to DownloadContext and it is
  // not 0, then filepath is also set DownloadContext correctly....
  if(_option->getAsBool(PREF_DRY_RUN) ||
     _downloadContext->getTotalLength() == 0) {
    createNextCommand(commands, e, 1);
  }else {
    if(e->_requestGroupMan->isSameFileBeingDownloaded(this)) {
      throw DOWNLOAD_FAILURE_EXCEPTION
	(StringFormat(EX_DUPLICATE_FILE_DOWNLOAD,
		      _downloadContext->getBasePath().c_str()).str());
    }
    adjustFilename
      (SharedHandle<BtProgressInfoFile>(new DefaultBtProgressInfoFile
					(_downloadContext,
					 SharedHandle<PieceStorage>(),
					 _option.get())));
    initPieceStorage();
    BtProgressInfoFileHandle infoFile
      (new DefaultBtProgressInfoFile(_downloadContext, _pieceStorage,
				     _option.get()));
    if(!infoFile->exists() && downloadFinishedByFileLength()) {
      _pieceStorage->markAllPiecesDone();
      _logger->notice(MSG_DOWNLOAD_ALREADY_COMPLETED,
		      _gid, _downloadContext->getBasePath().c_str());
    } else {
      loadAndOpenFile(infoFile);
      SharedHandle<CheckIntegrityEntry> checkIntegrityEntry
	(new StreamCheckIntegrityEntry(this));
      processCheckIntegrityEntry(commands, checkIntegrityEntry, e);
    }
  }
}

void RequestGroup::processCheckIntegrityEntry(std::deque<Command*>& commands,
					      const CheckIntegrityEntryHandle& entry,
					      DownloadEngine* e)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_option->getAsBool(PREF_CHECK_INTEGRITY) &&
     entry->isValidationReady()) {
    entry->initValidator();
    entry->cutTrailingGarbage();
    // Don't save control file(.aria2 file) when user presses
    // control-c key while aria2 is checking hashes. If control file
    // doesn't exist when aria2 launched, the completed length in
    // saved control file will be 0 byte and this confuses user.
    // enableSaveControlFile() will be called after hash checking is
    // done. See CheckIntegrityCommand.
    disableSaveControlFile();
    e->_checkIntegrityMan->pushEntry(entry);
  } else
#endif // ENABLE_MESSAGE_DIGEST
    {
      entry->onDownloadIncomplete(commands, e);
    }
}

void RequestGroup::initPieceStorage()
{
  if(_downloadContext->knowsTotalLength()) {
#ifdef ENABLE_BITTORRENT
    SharedHandle<DefaultPieceStorage> ps
      (new DefaultPieceStorage(_downloadContext, _option.get()));
    // Use LongestSequencePieceSelector when HTTP/FTP/BitTorrent integrated
    // downloads. Currently multi-file integrated download is not supported.
    if(_downloadContext->hasAttribute(bittorrent::BITTORRENT) &&
       isUriSuppliedForRequsetFileEntry
       (_downloadContext->getFileEntries().begin(),
	_downloadContext->getFileEntries().end())) {
      _logger->debug("Using LongestSequencePieceSelector");
      ps->setPieceSelector
	(SharedHandle<PieceSelector>(new LongestSequencePieceSelector()));
    }
#else // !ENABLE_BITTORRENT
    SharedHandle<DefaultPieceStorage> ps
      (new DefaultPieceStorage(_downloadContext, _option.get()));
#endif // !ENABLE_BITTORRENT
    if(!_diskWriterFactory.isNull()) {
      ps->setDiskWriterFactory(_diskWriterFactory);
    }
    _pieceStorage = ps;
  } else {
    UnknownLengthPieceStorageHandle ps
      (new UnknownLengthPieceStorage(_downloadContext, _option.get()));
    if(!_diskWriterFactory.isNull()) {
      ps->setDiskWriterFactory(_diskWriterFactory);
    }
    _pieceStorage = ps;
  }
  _pieceStorage->initStorage();
  initSegmentMan();
}

bool RequestGroup::downloadFinishedByFileLength()
{
  // assuming that a control file doesn't exist.
  if(!isPreLocalFileCheckEnabled() ||
     _option->getAsBool(PREF_ALLOW_OVERWRITE) ||
     (_option->getAsBool(PREF_CHECK_INTEGRITY) &&
      !_downloadContext->getPieceHashes().empty())) {
    return false;
  }
  if(!_downloadContext->knowsTotalLength()) {
    return false;
  }
  File outfile(getFirstFilePath());
  if(outfile.exists() && _downloadContext->getTotalLength() == outfile.size()) {
    return true;
  } else {
    return false;
  }
}

void RequestGroup::adjustFilename
(const SharedHandle<BtProgressInfoFile>& infoFile)
{
  if(!isPreLocalFileCheckEnabled()) {
    // OK, no need to care about filename.
  } else if(infoFile->exists()) {
    // Use current filename
  } else if(downloadFinishedByFileLength()) {
    // File was downloaded already, no need to change file name.
  } else {
    File outfile(getFirstFilePath());    
    if(outfile.exists() && _option->getAsBool(PREF_CONTINUE) &&
       outfile.size() <= _downloadContext->getTotalLength()) {
      // File exists but user decided to resume it.
    } else {
#ifdef ENABLE_MESSAGE_DIGEST
      if(outfile.exists() && _option->getAsBool(PREF_CHECK_INTEGRITY)) {
	// check-integrity existing file
      } else {
#endif // ENABLE_MESSAGE_DIGEST
	shouldCancelDownloadForSafety();
#ifdef ENABLE_MESSAGE_DIGEST
      }
#endif // ENABLE_MESSAGE_DIGEST
    }
  }
}

void RequestGroup::loadAndOpenFile(const BtProgressInfoFileHandle& progressInfoFile)
{
  try {
    if(!isPreLocalFileCheckEnabled()) {
      _pieceStorage->getDiskAdaptor()->initAndOpenFile();
      return;
    }
    // Remove the control file if download file doesn't exist
    if(progressInfoFile->exists() && !_pieceStorage->getDiskAdaptor()->fileExists()) {
      progressInfoFile->removeFile();
      _logger->notice(MSG_REMOVED_DEFUNCT_CONTROL_FILE,
		      progressInfoFile->getFilename().c_str(),
		      _downloadContext->getBasePath().c_str());
    }

    if(progressInfoFile->exists()) {
      progressInfoFile->load();
      _pieceStorage->getDiskAdaptor()->openExistingFile();
    } else {
      File outfile(getFirstFilePath());    
      if(outfile.exists() && _option->getAsBool(PREF_CONTINUE) &&
	 outfile.size() <= getTotalLength()) {
	_pieceStorage->getDiskAdaptor()->openExistingFile();
	_pieceStorage->markPiecesDone(outfile.size());
      } else {
#ifdef ENABLE_MESSAGE_DIGEST
	if(outfile.exists() && _option->getAsBool(PREF_CHECK_INTEGRITY)) {
	  _pieceStorage->getDiskAdaptor()->openExistingFile();
	} else {
#endif // ENABLE_MESSAGE_DIGEST
	  _pieceStorage->getDiskAdaptor()->initAndOpenFile();
#ifdef ENABLE_MESSAGE_DIGEST
	}
#endif // ENABLE_MESSAGE_DIGEST
      }
    }
    setProgressInfoFile(progressInfoFile);
  } catch(RecoverableException& e) {
    throw DOWNLOAD_FAILURE_EXCEPTION2
      (StringFormat(EX_DOWNLOAD_ABORTED).str(), e);
  }
}

// assuming that a control file does not exist
void RequestGroup::shouldCancelDownloadForSafety()
{
  if(_option->getAsBool(PREF_ALLOW_OVERWRITE)) {
    return;
  }
  File outfile(getFirstFilePath());
  if(outfile.exists()) {
    if(_option->getAsBool(PREF_AUTO_FILE_RENAMING)) {
      if(tryAutoFileRenaming()) {
	_logger->notice(MSG_FILE_RENAMED, getFirstFilePath().c_str());
      } else {
	throw DOWNLOAD_FAILURE_EXCEPTION
	  (StringFormat("File renaming failed: %s",
			getFirstFilePath().c_str()).str());
      }
    } else {
      throw DOWNLOAD_FAILURE_EXCEPTION
	(StringFormat(MSG_FILE_ALREADY_EXISTS,
		      getFirstFilePath().c_str()).str());
    }
  }
}

bool RequestGroup::tryAutoFileRenaming()
{
  std::string filepath = getFirstFilePath();
  if(filepath.empty()) {
    return false;
  }
  for(unsigned int i = 1; i < 10000; ++i) {
    File newfile(strconcat(filepath, ".", util::uitos(i)));
    File ctrlfile(newfile.getPath()+DefaultBtProgressInfoFile::getSuffix());
    if(!newfile.exists() || (newfile.exists() && ctrlfile.exists())) {
      _downloadContext->getFirstFileEntry()->setPath(newfile.getPath());
      return true;
    }
  }
  return false;
}

void RequestGroup::createNextCommandWithAdj(std::deque<Command*>& commands,
					    DownloadEngine* e, int numAdj)
{
  int numCommand;
  if(getTotalLength() == 0) {
    numCommand = 1+numAdj;
  } else {
    numCommand = std::min(_downloadContext->getNumPieces(),
			  _numConcurrentCommand);
    numCommand += numAdj;
  }
  if(numCommand > 0) {
    createNextCommand(commands, e, numCommand);
  }
}

void RequestGroup::createNextCommand(std::deque<Command*>& commands,
				     DownloadEngine* e,
				     unsigned int numCommand)
{
  for(; numCommand--; ) {
    Command* command = new CreateRequestCommand(e->newCUID(), this, e);
    commands.push_back(command);
  }
  if(!commands.empty()) {
    e->setNoWait(true);
  }
}

std::string RequestGroup::getFirstFilePath() const
{
  assert(!_downloadContext.isNull());
  if(inMemoryDownload()) {
    static const std::string DIR_MEMORY("[MEMORY]");
    return DIR_MEMORY+File(_downloadContext->getFirstFileEntry()->getPath()).getBasename();
  } else {
    return _downloadContext->getFirstFileEntry()->getPath();
  }
}

uint64_t RequestGroup::getTotalLength() const
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

uint64_t RequestGroup::getCompletedLength() const
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

void RequestGroup::validateFilename(const std::string& expectedFilename,
				    const std::string& actualFilename) const
{
  if(expectedFilename.empty()) {
    return;
  }
  if(expectedFilename != actualFilename) {
    throw DL_ABORT_EX(StringFormat(EX_FILENAME_MISMATCH,
				 expectedFilename.c_str(),
				 actualFilename.c_str()).str());
  }
}

void RequestGroup::validateTotalLength(uint64_t expectedTotalLength,
				       uint64_t actualTotalLength) const
{
  if(expectedTotalLength <= 0) {
    return;
  }
  if(expectedTotalLength != actualTotalLength) {
    throw DL_ABORT_EX
      (StringFormat(EX_SIZE_MISMATCH,
		    util::itos(expectedTotalLength, true).c_str(),
		    util::itos(actualTotalLength, true).c_str()).str());
  }
}

void RequestGroup::validateFilename(const std::string& actualFilename) const
{
  validateFilename(_downloadContext->getFileEntries().front()->getBasename(), actualFilename);
}

void RequestGroup::validateTotalLength(uint64_t actualTotalLength) const
{
  validateTotalLength(getTotalLength(), actualTotalLength);
}

void RequestGroup::increaseStreamConnection()
{
  ++_numStreamConnection;
}

void RequestGroup::decreaseStreamConnection()
{
  --_numStreamConnection;
}

unsigned int RequestGroup::getNumConnection() const
{
  unsigned int numConnection = _numStreamConnection;
#ifdef ENABLE_BITTORRENT
  if(!_btRuntime.isNull()) {
    numConnection += _btRuntime->getConnections();
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
  if(!_peerStorage.isNull()) {
    stat = _peerStorage->calculateStat();
  }
#endif // ENABLE_BITTORRENT
  if(!_segmentMan.isNull()) {
    stat.setDownloadSpeed(stat.getDownloadSpeed()+_segmentMan->calculateDownloadSpeed());
  }
  return stat;
}

void RequestGroup::setHaltRequested(bool f, HaltReason haltReason)
{
  _haltRequested = f;
  if(_haltRequested) {
    _haltReason = haltReason;
  }
#ifdef ENABLE_BITTORRENT
  if(!_btRuntime.isNull()) {
    _btRuntime->setHalt(f);
  }
#endif // ENABLE_BITTORRENT
}

void RequestGroup::setForceHaltRequested(bool f, HaltReason haltReason)
{
  setHaltRequested(f, haltReason);
  _forceHaltRequested = f;
}

void RequestGroup::releaseRuntimeResource(DownloadEngine* e)
{
#ifdef ENABLE_BITTORRENT
  if(_downloadContext->hasAttribute(bittorrent::BITTORRENT)) {
    SharedHandle<BtRegistry> btRegistry = e->getBtRegistry();
    const BDE& torrentAttrs =
      _downloadContext->getAttribute(bittorrent::BITTORRENT);
    const std::string& infoHash = torrentAttrs[bittorrent::INFO_HASH].s();
    SharedHandle<DownloadContext> contextInReg =
      btRegistry->getDownloadContext(infoHash);
    // Make sure that the registered DownloadContext's GID is equal to
    // _gid.  Even if createInitialCommand() throws exception without
    // registering this DownloadContext, after that, this method is
    // called. In this case, just finding DownloadContext using
    // infoHash may detect another download's DownloadContext and
    // deleting it from BtRegistry causes Segmentation Fault.
    if(!contextInReg.isNull() &&
       contextInReg->getOwnerRequestGroup()->getGID() == _gid) {
      btRegistry->remove(infoHash);
    }
  }
#endif // ENABLE_BITTORRENT
  if(!_pieceStorage.isNull()) {
    _pieceStorage->removeAdvertisedPiece(0);
  }
  _downloadContext->releaseRuntimeResource();
}

void RequestGroup::preDownloadProcessing()
{
  _logger->debug("Finding PreDownloadHandler for path %s.",
		 getFirstFilePath().c_str());
  try {
    for(PreDownloadHandlers::const_iterator itr = _preDownloadHandlers.begin();
	itr != _preDownloadHandlers.end(); ++itr) {
      if((*itr)->canHandle(this)) {
	(*itr)->execute(this);
	return;
      }
    }
  } catch(RecoverableException& ex) {
    _logger->error(EX_EXCEPTION_CAUGHT, ex);
    return;
  }
  _logger->debug("No PreDownloadHandler found.");
  return;
}

void RequestGroup::postDownloadProcessing
(std::deque<SharedHandle<RequestGroup> >& groups)
{
  _logger->debug("Finding PostDownloadHandler for path %s.",
		 getFirstFilePath().c_str());
  try {
    for(PostDownloadHandlers::const_iterator itr = _postDownloadHandlers.begin();
	itr != _postDownloadHandlers.end(); ++itr) {
      if((*itr)->canHandle(this)) {
	(*itr)->getNextRequestGroups(groups, this);
	return;
      }
    }
  } catch(RecoverableException& ex) {
    _logger->error(EX_EXCEPTION_CAUGHT, ex);
  }
  _logger->debug("No PostDownloadHandler found.");
}

void RequestGroup::initializePreDownloadHandler()
{
#ifdef ENABLE_BITTORRENT
  if(_option->get(PREF_FOLLOW_TORRENT) == V_MEM) {
    _preDownloadHandlers.push_back(DownloadHandlerFactory::getBtPreDownloadHandler());
  }
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  if(_option->get(PREF_FOLLOW_METALINK) == V_MEM) {
    _preDownloadHandlers.push_back(DownloadHandlerFactory::getMetalinkPreDownloadHandler());
  }
#endif // ENABLE_METALINK
}

void RequestGroup::initializePostDownloadHandler()
{
#ifdef ENABLE_BITTORRENT
  if(_option->getAsBool(PREF_FOLLOW_TORRENT) ||
     _option->get(PREF_FOLLOW_TORRENT) == V_MEM) {
    _postDownloadHandlers.push_back(DownloadHandlerFactory::getBtPostDownloadHandler());
  }
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  if(_option->getAsBool(PREF_FOLLOW_METALINK) ||
     _option->get(PREF_FOLLOW_METALINK) == V_MEM) {
    _postDownloadHandlers.push_back(DownloadHandlerFactory::getMetalinkPostDownloadHandler());
  }
#endif // ENABLE_METALINK
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

void RequestGroup::addPostDownloadHandler(const PostDownloadHandlerHandle& handler)
{
  _postDownloadHandlers.push_back(handler);
}

void RequestGroup::addPreDownloadHandler(const PreDownloadHandlerHandle& handler)
{
  _preDownloadHandlers.push_back(handler);
}

void RequestGroup::clearPostDowloadHandler()
{
  _postDownloadHandlers.clear();
}

void RequestGroup::clearPreDowloadHandler()
{
  _preDownloadHandlers.clear();
}

void RequestGroup::setPieceStorage(const PieceStorageHandle& pieceStorage)
{
  _pieceStorage = pieceStorage;
}

void RequestGroup::setProgressInfoFile(const BtProgressInfoFileHandle& progressInfoFile)
{
  _progressInfoFile = progressInfoFile;
}

bool RequestGroup::needsFileAllocation() const
{
  return isFileAllocationEnabled() &&
    (uint64_t)_option->getAsLLInt(PREF_NO_FILE_ALLOCATION_LIMIT) <= getTotalLength() &&
    !_pieceStorage->getDiskAdaptor()->fileAllocationIterator()->finished();
}

DownloadResultHandle RequestGroup::createDownloadResult() const
{
  uint64_t sessionDownloadLength = 0;

#ifdef ENABLE_BITTORRENT
  if(!_peerStorage.isNull()) {
    sessionDownloadLength +=
      _peerStorage->calculateStat().getSessionDownloadLength();
  }
#endif // ENABLE_BITTORRENT
  if(!_segmentMan.isNull()) {
    sessionDownloadLength +=
      _segmentMan->calculateSessionDownloadLength();
  }
  return
    SharedHandle<DownloadResult>
    (new DownloadResult(_gid,
			_downloadContext->getFileEntries(),
			_inMemoryDownload,
			sessionDownloadLength,
			_downloadContext->calculateSessionTime(),
			downloadResult()));
}
  
void RequestGroup::reportDownloadFinished()
{
  _logger->notice(MSG_FILE_DOWNLOAD_COMPLETED,
		  _downloadContext->getBasePath().c_str());
  _uriSelector->resetCounters();
#ifdef ENABLE_BITTORRENT
  if(_downloadContext->hasAttribute(bittorrent::BITTORRENT)) {
    TransferStat stat = calculateStat();
    double shareRatio = ((stat.getAllTimeUploadLength()*10)/getCompletedLength())/10.0;
    _logger->notice(MSG_SHARE_RATIO_REPORT,
		    shareRatio,
		    util::abbrevSize(stat.getAllTimeUploadLength()).c_str(),
		    util::abbrevSize(getCompletedLength()).c_str());
  }
#endif // ENABLE_BITTORRENT
}

void RequestGroup::addAcceptType(const std::string& type)
{
  if(std::find(_acceptTypes.begin(), _acceptTypes.end(), type) == _acceptTypes.end()) {
    _acceptTypes.push_back(type);
  }
}

void RequestGroup::removeAcceptType(const std::string& type)
{
  _acceptTypes.erase(std::remove(_acceptTypes.begin(), _acceptTypes.end(), type),
		     _acceptTypes.end());
}

void RequestGroup::setURISelector(const SharedHandle<URISelector>& uriSelector)
{
  _uriSelector = uriSelector;
}

void RequestGroup::applyLastModifiedTimeToLocalFiles()
{
  if(!_pieceStorage.isNull() && _lastModifiedTime.good()) {
    time_t t = _lastModifiedTime.getTime();
    _logger->info("Applying Last-Modified time: %s in local time zone",
		  ctime(&t));
    size_t n =
      _pieceStorage->getDiskAdaptor()->utime(Time(), _lastModifiedTime);
    _logger->info("Last-Modified attrs of %lu files were updated.",
		  static_cast<unsigned long>(n));
  }
}

void RequestGroup::updateLastModifiedTime(const Time& time)
{
  if(time.good() && _lastModifiedTime < time) {
    _lastModifiedTime = time;
  }
}

void RequestGroup::increaseAndValidateFileNotFoundCount()
{
  ++_fileNotFoundCount;
  const unsigned int maxCount = _option->getAsInt(PREF_MAX_FILE_NOT_FOUND);
  if(maxCount > 0 && _fileNotFoundCount >= maxCount &&
     _segmentMan->calculateSessionDownloadLength() == 0) {
    throw DOWNLOAD_FAILURE_EXCEPTION2
      (StringFormat("Reached max-file-not-found count=%u", maxCount).str(),
       downloadresultcode::MAX_FILE_NOT_FOUND);
  }
}

void RequestGroup::markInMemoryDownload()
{
  _inMemoryDownload = true;
}

void RequestGroup::setTimeout(time_t timeout)
{
  _timeout = timeout;
}

bool RequestGroup::doesDownloadSpeedExceed()
{
  return _maxDownloadSpeedLimit > 0 &&
    _maxDownloadSpeedLimit < calculateStat().getDownloadSpeed();
}

bool RequestGroup::doesUploadSpeedExceed()
{
  return _maxUploadSpeedLimit > 0 &&
    _maxUploadSpeedLimit < calculateStat().getUploadSpeed();
}

void RequestGroup::setLastUriResult
(const std::string uri, downloadresultcode::RESULT result)
{
  _lastUriResult.reset(new URIResult(uri, result));
}

void RequestGroup::saveControlFile() const
{
  if(_saveControlFile) {
    _progressInfoFile->save();
  }
}

void RequestGroup::removeControlFile() const
{
  _progressInfoFile->removeFile();
}

} // namespace aria2
