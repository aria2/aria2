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
#include "AbstractCommand.h"
#include "SegmentMan.h"
#include "NameResolver.h"
#include "CUIDCounter.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "FatalException.h"
#include "InitiateConnectionCommandFactory.h"
#include "Util.h"
#include "message.h"
#include "SleepCommand.h"
#include "prefs.h"
#include "DNSCache.h"
#include "SingleFileDownloadContext.h"
#include "DefaultPieceStorage.h"
#include "UnknownLengthPieceStorage.h"
#include "File.h"
#include "StreamCheckIntegrityEntry.h"
#include "DefaultBtProgressInfoFile.h"
#include "CheckIntegrityCommand.h"
#include "DiskAdaptor.h"
#include "PeerStat.h"
#include "Segment.h"
#include "DiskWriterFactory.h"
#include "Option.h"

AbstractCommand::AbstractCommand(int32_t cuid,
				 const RequestHandle& req,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  Command(cuid), RequestGroupAware(requestGroup),
  req(req), e(e), socket(s),
  segment(0),
  checkSocketIsReadable(false), checkSocketIsWritable(false),
  nameResolverCheck(false)
{ 
  setReadCheckSocket(socket);
  timeout = this->e->option->getAsInt(PREF_TIMEOUT);
  _requestGroup->increaseStreamConnection();
}

AbstractCommand::~AbstractCommand() {
  disableReadCheckSocket();
  disableWriteCheckSocket();
  _requestGroup->decreaseStreamConnection();
}

bool AbstractCommand::execute() {
  try {
    if(_requestGroup->downloadFinished() || _requestGroup->isHaltRequested()) {
      //logger->debug("CUID#%d - finished.", cuid);
      return true;
    }
    PeerStatHandle peerStat = 0;
    if(!_requestGroup->getSegmentMan().isNull()) {
      peerStat = _requestGroup->getSegmentMan()->getPeerStat(cuid);
    }
    if(!peerStat.isNull()) {
      if(peerStat->getStatus() == PeerStat::REQUEST_IDLE) {
	logger->info(MSG_ABORT_REQUESTED, cuid);
	onAbort(0);
	req->resetUrl();
	tryReserved();
	return true;
      }
    }
    if(checkSocketIsReadable && readCheckTarget->isReadable(0) ||
       checkSocketIsWritable && writeCheckTarget->isWritable(0) ||
#ifdef ENABLE_ASYNC_DNS
       nameResolverCheck && nameResolveFinished() ||
#endif // ENABLE_ASYNC_DNS
       !checkSocketIsReadable && !checkSocketIsWritable && !nameResolverCheck) {
      checkPoint.reset();
      if(!_requestGroup->getPieceStorage().isNull()) {
	if(segment.isNull()) {
	  segment = _requestGroup->getSegmentMan()->getSegment(cuid);
	  if(segment.isNull()) {
	    logger->info(MSG_NO_SEGMENT_AVAILABLE, cuid);
	    return prepareForRetry(1);
	  }
	}
      }
      return executeInternal();
    } else {
      if(checkPoint.elapsed(timeout)) {
	throw new DlRetryEx(EX_TIME_OUT);
      }
      e->commands.push_back(this);
      return false;
    }
  } catch(DlAbortEx* err) {
    logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
    onAbort(err);
    delete(err);
    req->resetUrl();
    tryReserved();
    return true;
  } catch(DlRetryEx* err) {
    logger->info(MSG_RESTARTING_DOWNLOAD, err, cuid, req->getUrl().c_str());
    req->addTryCount();
    bool isAbort = e->option->getAsInt(PREF_MAX_TRIES) != 0 &&
      req->getTryCount() >= e->option->getAsInt(PREF_MAX_TRIES);
    if(isAbort) {
      onAbort(err);
    }
    if(isAbort) {
      logger->info(MSG_MAX_TRY, cuid, req->getTryCount());
      logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
      delete(err);
      tryReserved();
      return true;
    } else {
      delete(err);
      return prepareForRetry(e->option->getAsInt(PREF_RETRY_WAIT));
    }
  } catch(FatalException* err) {
    delete(err);
    _requestGroup->setHaltRequested(true);
    return true;
  }
}

void AbstractCommand::tryReserved() {
  Commands commands = _requestGroup->createNextCommand(e, 1);
  e->addCommand(commands);
}

bool AbstractCommand::prepareForRetry(int32_t wait) {
  if(!_requestGroup->getPieceStorage().isNull()) {
    _requestGroup->getSegmentMan()->cancelSegment(cuid);
  }
  Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, _requestGroup, e);
  if(wait == 0) {
    e->commands.push_back(command);
  } else {
    SleepCommand* scom = new SleepCommand(cuid, e, command, wait);
    e->commands.push_back(scom);
  }
  return true;
}

void AbstractCommand::onAbort(Exception* ex) {
  logger->debug(MSG_UNREGISTER_CUID, cuid);
  //_segmentMan->unregisterId(cuid);
  if(!_requestGroup->getPieceStorage().isNull()) {
    _requestGroup->getSegmentMan()->cancelSegment(cuid);
  }
}

void AbstractCommand::disableReadCheckSocket() {
  if(checkSocketIsReadable) {
    e->deleteSocketForReadCheck(readCheckTarget, this);
    checkSocketIsReadable = false;
    readCheckTarget = SocketHandle();
  }  
}

void AbstractCommand::setReadCheckSocket(const SocketHandle& socket) {
  if(!socket->isOpen()) {
    disableReadCheckSocket();
  } else {
    if(checkSocketIsReadable) {
      if(readCheckTarget != socket) {
	e->deleteSocketForReadCheck(readCheckTarget, this);
	e->addSocketForReadCheck(socket, this);
	readCheckTarget = socket;
      }
    } else {
      e->addSocketForReadCheck(socket, this);
      checkSocketIsReadable = true;
      readCheckTarget = socket;
    }
  }
}

void AbstractCommand::disableWriteCheckSocket() {
  if(checkSocketIsWritable) {
    e->deleteSocketForWriteCheck(writeCheckTarget, this);
    checkSocketIsWritable = false;
    writeCheckTarget = SocketHandle();
  }
}

void AbstractCommand::setWriteCheckSocket(const SocketHandle& socket) {
  if(!socket->isOpen()) {
    disableWriteCheckSocket();
  } else {
    if(checkSocketIsWritable) {
      if(writeCheckTarget != socket) {
	e->deleteSocketForWriteCheck(writeCheckTarget, this);
	e->addSocketForWriteCheck(socket, this);
	writeCheckTarget = socket;
      }
    } else {
      e->addSocketForWriteCheck(socket, this);
      checkSocketIsWritable = true;
      writeCheckTarget = socket;
    }
  }
}

bool AbstractCommand::resolveHostname(const string& hostname,
				      const NameResolverHandle& resolver) {
  string ipaddr = DNSCacheSingletonHolder::instance()->find(hostname);
  if(ipaddr.empty()) {
#ifdef ENABLE_ASYNC_DNS
    switch(resolver->getStatus()) {
    case NameResolver::STATUS_READY:
      logger->info(MSG_RESOLVING_HOSTNAME, cuid, hostname.c_str());
      resolver->resolve(hostname);
      setNameResolverCheck(resolver);
      return false;
    case NameResolver::STATUS_SUCCESS:
      logger->info(MSG_NAME_RESOLUTION_COMPLETE, cuid,
		   hostname.c_str(), resolver->getAddrString().c_str());
      DNSCacheSingletonHolder::instance()->put(hostname, resolver->getAddrString());
      return true;
      break;
    case NameResolver::STATUS_ERROR:
      throw new DlAbortEx(MSG_NAME_RESOLUTION_FAILED, cuid,
			  hostname.c_str(),
			  resolver->getError().c_str());
    default:
      return false;
    }
#else
    logger->info(MSG_RESOLVING_HOSTNAME, cuid, hostname.c_str());
    resolver->resolve(hostname);
    logger->info(MSG_NAME_RESOLUTION_COMPLETE, cuid,
		 hostname.c_str(), resolver->getAddrString().c_str());
    DNSCacheSingletonHolder::instance()->put(hostname, resolver->getAddrString());
    return true;
#endif // ENABLE_ASYNC_DNS
  } else {
    logger->info(MSG_DNS_CACHE_HIT, cuid,
		 hostname.c_str(), ipaddr.c_str());
    resolver->setAddr(ipaddr);
    return true;
  }
}

#ifdef ENABLE_ASYNC_DNS
void AbstractCommand::setNameResolverCheck(const NameResolverHandle& resolver) {
  nameResolverCheck = true;
  e->addNameResolverCheck(resolver, this);
}

void AbstractCommand::disableNameResolverCheck(const NameResolverHandle& resolver) {
  nameResolverCheck = false;
  e->deleteNameResolverCheck(resolver, this);
}

bool AbstractCommand::nameResolveFinished() const {
  return false;
}
#endif // ENABLE_ASYNC_DNS

void AbstractCommand::loadAndOpenFile()
{
  if(!_requestGroup->isPreLocalFileCheckEnabled()) {
    _requestGroup->getPieceStorage()->getDiskAdaptor()->initAndOpenFile();
    return;
  }

  //_requestGroup->setProgressInfoFile(new DefaultBtProgressInfoFile(_requestGroup->getDownloadContext(), _requestGroup->getPieceStorage(), e->option));
  BtProgressInfoFileHandle progressInfoFile =
    new DefaultBtProgressInfoFile(_requestGroup->getDownloadContext(), _requestGroup->getPieceStorage(), e->option);
  if(progressInfoFile->exists()) {
    progressInfoFile->load();
    _requestGroup->getPieceStorage()->getDiskAdaptor()->openExistingFile();
  } else {
    File outfile(_requestGroup->getFilePath());    
    if(outfile.exists() && e->option->get(PREF_CONTINUE) == V_TRUE) {
      if(_requestGroup->getTotalLength() < outfile.size()) {
	throw new FatalException(EX_FILE_LENGTH_MISMATCH_BETWEEN_LOCAL_AND_REMOTE,
				 _requestGroup->getFilePath().c_str(),
				 Util::llitos(outfile.size()).c_str(),
				 Util::llitos(_requestGroup->getTotalLength()).c_str());
      }
      _requestGroup->getPieceStorage()->getDiskAdaptor()->openExistingFile();
      _requestGroup->getPieceStorage()->markPiecesDone(outfile.size());
    } else {
#ifdef ENABLE_MESSAGE_DIGEST
      if(outfile.exists() && e->option->get(PREF_CHECK_INTEGRITY) == V_TRUE) {
	_requestGroup->getPieceStorage()->getDiskAdaptor()->openExistingFile();
      } else {
	shouldCancelDownloadForSafety();
	_requestGroup->getPieceStorage()->getDiskAdaptor()->initAndOpenFile();
      }
#else // ENABLE_MESSAGE_DIGEST
      shouldCancelDownloadForSafety();
      _requestGroup->getPieceStorage()->getDiskAdaptor()->initAndOpenFile();
#endif // ENABLE_MESSAGE_DIGEST
    }
  }
  _requestGroup->setProgressInfoFile(progressInfoFile);
}

void AbstractCommand::shouldCancelDownloadForSafety()
{
  File outfile(_requestGroup->getFilePath());
  if(outfile.exists() && !_requestGroup->getProgressInfoFile()->exists()) {
    if(e->option->get(PREF_AUTO_FILE_RENAMING) == V_TRUE) {
      if(tryAutoFileRenaming()) {
	logger->notice("File already exists. Renamed to %s.",
		       _requestGroup->getFilePath().c_str());
      } else {
	logger->notice("File renaming failed: %s",
		       _requestGroup->getFilePath().c_str());
	throw new FatalException(EX_DOWNLOAD_ABORTED);
      }
    } else if(e->option->get(PREF_ALLOW_OVERWRITE) != V_TRUE) {
      logger->notice(MSG_FILE_ALREADY_EXISTS,
		     _requestGroup->getFilePath().c_str(),
		     _requestGroup->getProgressInfoFile()->getFilename().c_str());
      throw new FatalException(EX_DOWNLOAD_ABORTED);
    }
  }
}

bool AbstractCommand::tryAutoFileRenaming()
{
  string filepath = _requestGroup->getFilePath();
  if(filepath.empty()) {
    return false;
  }
  for(int32_t i = 1; i < 10000; ++i) {
    File newfile(filepath+"."+Util::itos(i));
    if(!newfile.exists()) {
      SingleFileDownloadContextHandle(_requestGroup->getDownloadContext())->setUFilename(newfile.getBasename());
      return true;
    }
  }
  return false;
}

void AbstractCommand::initPieceStorage()
{
  if(_requestGroup->getDownloadContext()->getTotalLength() == 0) {
    UnknownLengthPieceStorageHandle ps = new UnknownLengthPieceStorage(_requestGroup->getDownloadContext(), e->option);
    if(!_requestGroup->getDiskWriterFactory().isNull()) {
      ps->setDiskWriterFactory(_requestGroup->getDiskWriterFactory());
    }
    _requestGroup->setPieceStorage(ps);
  } else {
    DefaultPieceStorageHandle ps = new DefaultPieceStorage(_requestGroup->getDownloadContext(), e->option);
    if(!_requestGroup->getDiskWriterFactory().isNull()) {
      ps->setDiskWriterFactory(_requestGroup->getDiskWriterFactory());
    }
    _requestGroup->setPieceStorage(ps);
  }
  _requestGroup->getPieceStorage()->initStorage();
  _requestGroup->initSegmentMan();
}

bool AbstractCommand::downloadFinishedByFileLength()
{
  // check existence of control file using ProgressInfoFile class.
  if(_requestGroup->getProgressInfoFile()->exists()) {
    return false;
  }
  // TODO consider the case when the getFilePath() returns dir path. 
  File outfile(_requestGroup->getFilePath());
  if(outfile.exists() &&
     _requestGroup->getTotalLength() == outfile.size()) {
    _requestGroup->getPieceStorage()->markAllPiecesDone();
    return true;
  } else {
    return false;
  }
}

void AbstractCommand::prepareForNextAction(Command* nextCommand)
{
  CheckIntegrityEntryHandle entry =
    new StreamCheckIntegrityEntry(req, _requestGroup, nextCommand);
#ifdef ENABLE_MESSAGE_DIGEST
  if(File(_requestGroup->getFilePath()).size() > 0 &&
     e->option->get(PREF_CHECK_INTEGRITY) == V_TRUE &&
     entry->isValidationReady()) {
    entry->initValidator();
    logger->debug("Issuing CheckIntegrityCommand.");
    CheckIntegrityCommand* command =
      new CheckIntegrityCommand(CUIDCounterSingletonHolder::instance()->newID(), _requestGroup, e, entry);
    e->commands.push_back(command);
  } else
#endif // ENABLE_MESSAGE_DIGEST
    {
      e->addCommand(entry->prepareForNextAction(e));
    }
}
