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
#include "InitiateConnectionCommandFactory.h"
#include "File.h"
#include "message.h"
#include "Util.h"
#include "BtRegistry.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DiskAdaptor.h"
#include "DiskWriterFactory.h"
#include "RecoverableException.h"
#include "StreamCheckIntegrityEntry.h"
#include "CheckIntegrityCommand.h"
#include "UnknownLengthPieceStorage.h"
#include "BtContext.h"
#include "SingleFileDownloadContext.h"
#include "DlAbortEx.h"
#include "DownloadFailureException.h"
#include "RequestGroupMan.h"
#include "DefaultBtProgressInfoFile.h"
#include "DefaultPieceStorage.h"
#include "DownloadHandlerFactory.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "DownloadHandlerConstants.h"
#include "ServerHost.h"
#include "Option.h"
#include "FileEntry.h"
#include "Request.h"
#include "FileAllocationIterator.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "URISelector.h"
#include "InOrderURISelector.h"
#include "PieceSelector.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "CheckIntegrityCommand.h"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_BITTORRENT
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

RequestGroup::RequestGroup(const Option* option,
			   const std::deque<std::string>& uris):
  _gid(++_gidCounter),
  _uris(uris),
  _numConcurrentCommand(option->getAsInt(PREF_SPLIT)),
  _numStreamConnection(0),
  _numCommand(0),
  _segmentManFactory(new DefaultSegmentManFactory(option)),
  _progressInfoFile(new NullProgressInfoFile()),
  _preLocalFileCheckEnabled(true),
  _haltRequested(false),
  _forceHaltRequested(false),
  _singleHostMultiConnectionEnabled(true),
  _uriSelector(new InOrderURISelector()),
  _lastModifiedTime(Time::null()),
  _fileNotFoundCount(0),
  _inMemoryDownload(false),
  _option(option),
  _logger(LogFactory::getInstance())
{
  if(_option->get(PREF_FILE_ALLOCATION) == V_PREALLOC) {
    _fileAllocationEnabled = true;
  } else {
    _fileAllocationEnabled = false;
  }
  // Add types to be sent as a Accept header value here.
  // It would be good to put this value in Option so that user can tweak
  // and add this list.
  // ACCEPT_METALINK is used for `transparent metalink'.
  addAcceptType(ACCEPT_METALINK);

  initializePreDownloadHandler();
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

bool RequestGroup::allDownloadFinished() const
{
  if(_pieceStorage.isNull()) {
    return false;
  } else {
    return _pieceStorage->allDownloadFinished();
  }
}

DownloadResult::RESULT RequestGroup::downloadResult() const
{
  if (downloadFinished())
    return DownloadResult::FINISHED;
  else {
    if (_uriResults.empty()) {
      return DownloadResult::UNKNOWN_ERROR;
    } else {
      return _uriResults.back().getResult();
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
					DownloadEngine* e,
					const std::string& method)
{
#ifdef ENABLE_BITTORRENT
  {
    BtContextHandle btContext = dynamic_pointer_cast<BtContext>(_downloadContext);
    if(!btContext.isNull()) {
      if(e->_requestGroupMan->isSameFileBeingDownloaded(this)) {
	throw DownloadFailureException
	  (StringFormat(EX_DUPLICATE_FILE_DOWNLOAD,
			getFilePath().c_str()).str());
      }
      initPieceStorage();
      if(btContext->getFileEntries().size() > 1) {
	// this is really multi file torrent.
	// clear http/ftp uris because the current implementation does not
	// allow integrating multi-file torrent and http/ftp.
	_logger->debug("Clearing http/ftp URIs because the current implementation does not allow integrating multi-file torrent and http/ftp.");
	_uris.clear();

	_pieceStorage->setFileFilter(Util::parseIntRange(_option->get(PREF_SELECT_FILE)));
      }
      
      SharedHandle<DefaultBtProgressInfoFile>
	progressInfoFile(new DefaultBtProgressInfoFile(_downloadContext,
						       _pieceStorage,
						       _option));
      
      SharedHandle<BtRegistry> btRegistry = e->getBtRegistry();

      btRegistry->registerBtContext(btContext->getInfoHashAsString(),
				    btContext);
      btRegistry->registerPieceStorage(btContext->getInfoHashAsString(),
				       _pieceStorage);
      btRegistry->registerBtProgressInfoFile(btContext->getInfoHashAsString(),
					     progressInfoFile);

  
      BtRuntimeHandle btRuntime(new BtRuntime());
      btRuntime->setListenPort(_option->getAsInt(PREF_LISTEN_PORT));
      btRuntime->setMaxPeers(_option->getAsInt(PREF_BT_MAX_PEERS));
      btRegistry->registerBtRuntime(btContext->getInfoHashAsString(),
				    btRuntime);
      _btRuntime = btRuntime;
      progressInfoFile->setBtRuntime(btRuntime);

      SharedHandle<DefaultPeerStorage> peerStorage
	(new DefaultPeerStorage(btContext, _option));
      peerStorage->setBtRuntime(btRuntime);
      peerStorage->setPieceStorage(_pieceStorage);
      btRegistry->registerPeerStorage(btContext->getInfoHashAsString(),
				      peerStorage);
      _peerStorage = peerStorage;
      progressInfoFile->setPeerStorage(peerStorage);

      SharedHandle<DefaultBtAnnounce> btAnnounce
	(new DefaultBtAnnounce(btContext, _option));
      btAnnounce->setBtRuntime(btRuntime);
      btAnnounce->setPieceStorage(_pieceStorage);
      btAnnounce->setPeerStorage(peerStorage);
      btRegistry->registerBtAnnounce(btContext->getInfoHashAsString(),
				     btAnnounce);
      btAnnounce->shuffleAnnounce();
      
      // Remove the control file if download file doesn't exist
      if(progressInfoFile->exists() && !_pieceStorage->getDiskAdaptor()->fileExists()) {
	progressInfoFile->removeFile();
	_logger->notice(MSG_REMOVED_DEFUNCT_CONTROL_FILE,
			progressInfoFile->getFilename().c_str(),
			_pieceStorage->getDiskAdaptor()->getFilePath().c_str());
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
	    throw DownloadFailureException
	      (StringFormat
	       (MSG_FILE_ALREADY_EXISTS,
		_pieceStorage->getDiskAdaptor()->getFilePath().c_str()).str());
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

      if(!btContext->isPrivate() && _option->getAsBool(PREF_ENABLE_DHT)) {
	std::deque<Command*> commands;
	DHTSetup().setup(commands, e, _option);
	e->addCommand(commands);
	if(!btContext->getNodes().empty() && DHTSetup::initialized()) {
	  DHTEntryPointNameResolveCommand* command =
	    new DHTEntryPointNameResolveCommand(e->newCUID(), e,
						btContext->getNodes());
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
  // TODO I assume here when totallength is set to DownloadContext and it is
  // not 0, then filepath is also set DownloadContext correctly....
  if(_downloadContext->getTotalLength() == 0) {
    createNextCommand(commands, e, 1, method);
  }else {
    if(e->_requestGroupMan->isSameFileBeingDownloaded(this)) {
      throw DownloadFailureException
	(StringFormat(EX_DUPLICATE_FILE_DOWNLOAD,
		      getFilePath().c_str()).str());
    }
    initPieceStorage();
    BtProgressInfoFileHandle infoFile
      (new DefaultBtProgressInfoFile(_downloadContext, _pieceStorage, _option));
    if(infoFile->exists() || !downloadFinishedByFileLength()) {
      loadAndOpenFile(infoFile);
      SharedHandle<CheckIntegrityEntry> checkIntegrityEntry
	(new StreamCheckIntegrityEntry(SharedHandle<Request>(), this));
      processCheckIntegrityEntry(commands, checkIntegrityEntry, e);
    }
  }
}

void RequestGroup::processCheckIntegrityEntry(std::deque<Command*>& commands,
					      const CheckIntegrityEntryHandle& entry,
					      DownloadEngine* e)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(e->option->getAsBool(PREF_CHECK_INTEGRITY) &&
     entry->isValidationReady()) {
    entry->initValidator();
    entry->cutTrailingGarbage();    
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
    SharedHandle<DefaultPieceStorage> ps;
    SharedHandle<PieceSelector> selector;
    // Use LongestSequencePieceSelector when HTTP/FTP/BitTorrent integrated
    // downloads. Currently multi-file integrated download is not supported.
    if(!_uris.empty() &&
       _downloadContext->getFileEntries().size() == 1 &&
       !dynamic_pointer_cast<BtContext>(_downloadContext).isNull()) {
      _logger->debug("Using LongestSequencePieceSelector");
      selector.reset(new LongestSequencePieceSelector());
    }
    ps.reset(new DefaultPieceStorage(_downloadContext, _option, selector));
#else // !ENABLE_BITTORRENT
    SharedHandle<DefaultPieceStorage> ps
      (new DefaultPieceStorage(_downloadContext, _option));
#endif // !ENABLE_BITTORRENT
    if(!_diskWriterFactory.isNull()) {
      ps->setDiskWriterFactory(_diskWriterFactory);
    }
    _pieceStorage = ps;
  } else {
    UnknownLengthPieceStorageHandle ps
      (new UnknownLengthPieceStorage(_downloadContext, _option));
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
  // TODO consider the case when the getFilePath() returns dir path. 
  File outfile(getFilePath());
  if(outfile.exists() && getTotalLength() == outfile.size()) {
    _pieceStorage->markAllPiecesDone();
    _logger->notice(MSG_DOWNLOAD_ALREADY_COMPLETED, _gid, getFilePath().c_str());
    return true;
  } else {
    return false;
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
		      _pieceStorage->getDiskAdaptor()->getFilePath().c_str());
    }
    while(1) {
      if(progressInfoFile->exists()) {
	progressInfoFile->load();
	_pieceStorage->getDiskAdaptor()->openExistingFile();
      } else {
	File outfile(getFilePath());    
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
	    shouldCancelDownloadForSafety();
	    // call updateFilename here in case when filename is renamed
	    // by tryAutoFileRenaming()
	    progressInfoFile->updateFilename();
	    if(progressInfoFile->exists()) {
	      // Close DiskAdaptor here. Renmaed file will be opened in the
	      // next loop .
	      _pieceStorage->getDiskAdaptor()->closeFile();
	      continue;
	    }
	    _pieceStorage->getDiskAdaptor()->initAndOpenFile();
#ifdef ENABLE_MESSAGE_DIGEST
	  }
#endif // ENABLE_MESSAGE_DIGEST
	}
      }
      setProgressInfoFile(progressInfoFile);
      break;
    }
  } catch(RecoverableException& e) {
    throw DownloadFailureException
      (StringFormat(EX_DOWNLOAD_ABORTED).str(), e);
  }
}

// assuming that a control file does not exist
void RequestGroup::shouldCancelDownloadForSafety()
{
  if(_option->getAsBool(PREF_ALLOW_OVERWRITE)) {
    return;
  }
  File outfile(getFilePath());
  if(outfile.exists()) {
    if(_option->getAsBool(PREF_AUTO_FILE_RENAMING)) {
      if(tryAutoFileRenaming()) {
	_logger->notice(MSG_FILE_RENAMED, getFilePath().c_str());
      } else {
	throw DownloadFailureException
	  (StringFormat("File renaming failed: %s",
			getFilePath().c_str()).str());
      }
    } else {
      throw DownloadFailureException
	(StringFormat(MSG_FILE_ALREADY_EXISTS,
		      getFilePath().c_str()).str());
    }
  }
}

bool RequestGroup::tryAutoFileRenaming()
{
  std::string filepath = getFilePath();
  if(filepath.empty()) {
    return false;
  }
  SingleFileDownloadContextHandle ctx =
    dynamic_pointer_cast<SingleFileDownloadContext>(_downloadContext);
  // Make a copy of ctx.
  SingleFileDownloadContextHandle tempCtx(new SingleFileDownloadContext(*ctx.get()));

  DefaultBtProgressInfoFile tempInfoFile(tempCtx, SharedHandle<PieceStorage>(), 0);

  for(unsigned int i = 1; i < 10000; ++i) {
    File newfile(filepath+"."+Util::uitos(i));
    std::string newFilename = newfile.getBasename();
    tempCtx->setUFilename(newFilename);
    tempInfoFile.updateFilename();
    if(!newfile.exists() || (newfile.exists() && tempInfoFile.exists())) {
      ctx->setUFilename(newFilename);
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
    if(_numConcurrentCommand == 0) {
      // TODO remove _uris.size() support
      numCommand = _uris.size();
    } else {
      numCommand = _numConcurrentCommand;
    }
    numCommand = std::min(static_cast<int>(_downloadContext->getNumPieces()),
			  numCommand);
    numCommand += numAdj;
  }
  if(numCommand > 0) {
    createNextCommand(commands, e, numCommand);
  }
}

void RequestGroup::createNextCommand(std::deque<Command*>& commands,
				     DownloadEngine* e,
				     unsigned int numCommand,
				     const std::string& method)
{
  std::deque<std::string> pendingURIs;
  for(; !_uris.empty() && numCommand--; ) {    
    std::string uri = _uriSelector->select(_uris);
    if(uri.empty())
      continue;
    RequestHandle req(new Request());
    if(req->setUrl(uri)) {
      ServerHostHandle sv;
      if(!_singleHostMultiConnectionEnabled){
	sv = searchServerHost(req->getHost());
      }
      if(sv.isNull()) {
	_spentUris.push_back(uri);
	req->setReferer(_option->get(PREF_REFERER));
	req->setMethod(method);

	Command* command =
	  InitiateConnectionCommandFactory::createInitiateConnectionCommand
	  (e->newCUID(), req, this, e);
	ServerHostHandle sv(new ServerHost(command->getCuid(), req->getHost()));
	registerServerHost(sv);
	// give a chance to be executed in the next loop in DownloadEngine
	command->setStatus(Command::STATUS_ONESHOT_REALTIME);
	commands.push_back(command);
      } else {
	pendingURIs.push_back(uri);
      }
    } else {
      _logger->error(MSG_UNRECOGNIZED_URI, req->getUrl().c_str());
    }
  }
  _uris.insert(_uris.begin(), pendingURIs.begin(), pendingURIs.end());
}

std::string RequestGroup::getFilePath() const
{
  assert(!_downloadContext.isNull());
  if(inMemoryDownload()) {
    static const std::string DIR_MEMORY("[MEMORY]");
    return DIR_MEMORY+File(_downloadContext->getActualBasePath()).getBasename();
  } else {
    return _downloadContext->getActualBasePath();
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
    throw DlAbortEx(StringFormat(EX_FILENAME_MISMATCH,
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
    throw DlAbortEx
      (StringFormat(EX_SIZE_MISMATCH,
		    Util::itos(expectedTotalLength, true).c_str(),
		    Util::itos(actualTotalLength, true).c_str()).str());
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

void RequestGroup::setHaltRequested(bool f)
{
  _haltRequested = f;
#ifdef ENABLE_BITTORRENT
  if(!_btRuntime.isNull()) {
    _btRuntime->setHalt(f);
  }
#endif // ENABLE_BITTORRENT
}

void RequestGroup::setForceHaltRequested(bool f)
{
  setHaltRequested(f);
  _forceHaltRequested = f;
}

void RequestGroup::releaseRuntimeResource(DownloadEngine* e)
{
#ifdef ENABLE_BITTORRENT
  BtContextHandle btContext = dynamic_pointer_cast<BtContext>(_downloadContext);
  if(!btContext.isNull()) {
    SharedHandle<BtRegistry> btRegistry = e->getBtRegistry();
    BtContextHandle btContextInReg =
      btRegistry->getBtContext(btContext->getInfoHashAsString());
    if(!btContextInReg.isNull() &&
       btContextInReg->getOwnerRequestGroup()->getGID() ==
	btContext->getOwnerRequestGroup()->getGID()) {
      btRegistry->unregister(btContext->getInfoHashAsString());
      if(!DHTRegistry::_peerAnnounceStorage.isNull()) {
	DHTRegistry::_peerAnnounceStorage->
	  removeLocalPeerAnnounce(btContext->getInfoHash());
      }
    }
  }
#endif // ENABLE_BITTORRENT
  if(!_pieceStorage.isNull()) {
    _pieceStorage->removeAdvertisedPiece(0);
  }
}

void RequestGroup::preDownloadProcessing()
{
  _logger->debug("Finding PreDownloadHandler for path %s.", getFilePath().c_str());
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
  _logger->debug("Finding PostDownloadHandler for path %s.", getFilePath().c_str());
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

void RequestGroup::getURIs(std::deque<std::string>& uris) const
{
  uris.insert(uris.end(), _spentUris.begin(), _spentUris.end());
  uris.insert(uris.end(), _uris.begin(), _uris.end());
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
    (uint64_t)_option->getAsLLInt(PREF_NO_FILE_ALLOCATION_LIMIT) <= getTotalLength() &&
    !_pieceStorage->getDiskAdaptor()->fileAllocationIterator()->finished();
}

DownloadResultHandle RequestGroup::createDownloadResult() const
{
  std::deque<std::string> uris;
  getURIs(uris);

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
			getFilePath(),
			getTotalLength(),
			uris.empty() ? A2STR::NIL:uris.front(),
			uris.size(),
			sessionDownloadLength,
			_downloadContext->calculateSessionTime(),
			downloadResult()));
}

void RequestGroup::registerServerHost(const ServerHostHandle& serverHost)
{
  _serverHosts.push_back(serverHost);
}

class FindServerHostByCUID
{
private:
  int32_t _cuid;
public:
  FindServerHostByCUID(int32_t cuid):_cuid(cuid) {}

  bool operator()(const ServerHostHandle& sv) const
  {
    return sv->getCuid() == _cuid;
  }
};

ServerHostHandle RequestGroup::searchServerHost(int32_t cuid) const
{
  std::deque<SharedHandle<ServerHost> >::const_iterator itr =
    std::find_if(_serverHosts.begin(), _serverHosts.end(), FindServerHostByCUID(cuid));
  if(itr == _serverHosts.end()) {
    return SharedHandle<ServerHost>();
  } else {
    return *itr;
  }
}

class FindServerHostByHostname
{
private:
  std::string _hostname;
public:
  FindServerHostByHostname(const std::string& hostname):_hostname(hostname) {}

  bool operator()(const ServerHostHandle& sv) const
  {
    return sv->getHostname() == _hostname;
  }
};

ServerHostHandle RequestGroup::searchServerHost(const std::string& hostname) const
{
  std::deque<SharedHandle<ServerHost> >::const_iterator itr =
    std::find_if(_serverHosts.begin(), _serverHosts.end(), FindServerHostByHostname(hostname));
  if(itr == _serverHosts.end()) {
    return SharedHandle<ServerHost>();
  } else {
    return *itr;
  }
}

void RequestGroup::removeServerHost(int32_t cuid)
{
  _serverHosts.erase(std::remove_if(_serverHosts.begin(), _serverHosts.end(), FindServerHostByCUID(cuid)), _serverHosts.end());
}
  
void RequestGroup::removeURIWhoseHostnameIs(const std::string& hostname)
{
  std::deque<std::string> newURIs;
  Request req;
  for(std::deque<std::string>::const_iterator itr = _uris.begin(); itr != _uris.end(); ++itr) {
    if(((*itr).find(hostname) == std::string::npos) ||
       (req.setUrl(*itr) && (req.getHost() != hostname))) {
      newURIs.push_back(*itr);
    }
  }
  _logger->debug("GUID#%d - Removed %d duplicate hostname URIs",
		 _gid, _uris.size()-newURIs.size());
  _uris = newURIs;
}

void RequestGroup::removeIdenticalURI(const std::string& uri)
{
  _uris.erase(std::remove(_uris.begin(), _uris.end(), uri), _uris.end());
}

void RequestGroup::reportDownloadFinished()
{
  _logger->notice(MSG_FILE_DOWNLOAD_COMPLETED,
		  getFilePath().c_str());
  _uriSelector->resetCounters();
#ifdef ENABLE_BITTORRENT
  SharedHandle<BtContext> ctx = dynamic_pointer_cast<BtContext>(_downloadContext);
  if(!ctx.isNull()) {
    TransferStat stat = calculateStat();
    double shareRatio = ((stat.getAllTimeUploadLength()*10)/getCompletedLength())/10.0;
    _logger->notice(MSG_SHARE_RATIO_REPORT,
		    shareRatio,
		    Util::abbrevSize(stat.getAllTimeUploadLength()).c_str(),
		    Util::abbrevSize(getCompletedLength()).c_str());
  }
#endif // ENABLE_BITTORRENT
}

const std::deque<std::string>& RequestGroup::getAcceptFeatures() const
{
  return _acceptFeatures;
}

void RequestGroup::addAcceptFeatureHeader(const std::string& feature)
{
  if(std::find(_acceptFeatures.begin(), _acceptFeatures.end(), feature) == _acceptFeatures.end()) {
    _acceptFeatures.push_back(feature);
  }
}

void RequestGroup::removeAcceptFeatureHeader(const std::string& feature)
{
  std::deque<std::string>::iterator i = std::find(_acceptFeatures.begin(), _acceptFeatures.end(), feature);
  if(i != _acceptFeatures.end()) {
    _acceptFeatures.erase(i);
  }
}

const std::deque<std::string>& RequestGroup::getAcceptTypes() const
{
  return _acceptTypes;
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
    throw DownloadFailureException
      (StringFormat("Reached max-file-not-found count=%u", maxCount).str(),
       DownloadResult::MAX_FILE_NOT_FOUND);
  }
}

unsigned int RequestGroup::getNumConcurrentCommand() const
{
  return _numConcurrentCommand;
}

void RequestGroup::markInMemoryDownload()
{
  _inMemoryDownload = true;
}

bool RequestGroup::inMemoryDownload() const
{
  return _inMemoryDownload;
}

void RequestGroup::tuneDownloadCommand(DownloadCommand* command)
{
  _uriSelector->tuneDownloadCommand(_uris, command);
}

void RequestGroup::addURIResult(std::string uri, DownloadResult::RESULT result)
{
  _uriResults.push_back(URIResult(uri, result));
}

const std::deque<URIResult>& RequestGroup::getURIResults() const
{
  return _uriResults;
}

} // namespace aria2
