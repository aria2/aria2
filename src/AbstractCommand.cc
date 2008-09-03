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
#include "RequestGroup.h"
#include "Request.h"
#include "DownloadEngine.h"
#include "Option.h"
#include "PeerStat.h"
#include "SegmentMan.h"
#include "Logger.h"
#include "Segment.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "DownloadFailureException.h"
#include "InitiateConnectionCommandFactory.h"
#include "SleepCommand.h"
#ifdef ENABLE_ASYNC_DNS
#include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS
#include "StreamCheckIntegrityEntry.h"
#include "PieceStorage.h"
#include "Socket.h"
#include "message.h"
#include "prefs.h"
#include "StringFormat.h"

namespace aria2 {

AbstractCommand::AbstractCommand(int32_t cuid,
				 const SharedHandle<Request>& req,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  Command(cuid), RequestGroupAware(requestGroup),
  req(req), e(e), socket(s),
  checkSocketIsReadable(false), checkSocketIsWritable(false),
  nameResolverCheck(false)
{
  if(!socket.isNull() && socket->isOpen()) {
    setReadCheckSocket(socket);
  }
  timeout = this->e->option->getAsInt(PREF_TIMEOUT);
  _requestGroup->increaseStreamConnection();
}

AbstractCommand::~AbstractCommand() {
  disableReadCheckSocket();
  disableWriteCheckSocket();
#ifdef ENABLE_ASYNC_DNS
  disableNameResolverCheck(_asyncNameResolver);
#endif // ENABLE_ASYNC_DNS
  _requestGroup->decreaseStreamConnection();
}

bool AbstractCommand::execute() {
  try {
    if(_requestGroup->downloadFinished() || _requestGroup->isHaltRequested()) {
      //logger->debug("CUID#%d - finished.", cuid);
      return true;
    }
    PeerStatHandle peerStat;
    if(!_requestGroup->getSegmentMan().isNull()) {
      peerStat = _requestGroup->getSegmentMan()->getPeerStat(cuid);
    }
    if(!peerStat.isNull()) {
      if(peerStat->getStatus() == PeerStat::REQUEST_IDLE) {
	logger->info(MSG_ABORT_REQUESTED, cuid);
	onAbort();
	req->resetUrl();
	tryReserved();
	return true;
      }
    }
    if((checkSocketIsReadable && _readEvent) ||
       (checkSocketIsWritable && _writeEvent) ||
       _errorEvent ||
#ifdef ENABLE_ASYNC_DNS
       (nameResolverCheck && nameResolveFinished()) ||
#endif // ENABLE_ASYNC_DNS
       (!checkSocketIsReadable && !checkSocketIsWritable && !nameResolverCheck)) {
      checkPoint.reset();
      if(!_requestGroup->getPieceStorage().isNull()) {
	_segments.clear();
	_requestGroup->getSegmentMan()->getInFlightSegment(_segments, cuid);
	size_t maxSegments;
	if(req->isPipeliningEnabled()) {
	  maxSegments = e->option->getAsInt(PREF_MAX_HTTP_PIPELINING);
	} else {
	  maxSegments = 1;
	}
	while(_segments.size() < maxSegments) {
	  SegmentHandle segment = _requestGroup->getSegmentMan()->getSegment(cuid);
	  if(segment.isNull()) {
	    break;
	  }
	  _segments.push_back(segment);
	}
	if(_segments.empty()) {
	  // TODO socket could be pooled here if pipelining is enabled...
	  logger->info(MSG_NO_SEGMENT_AVAILABLE, cuid);
	  return prepareForRetry(1);
	}
      }
      return executeInternal();
    } else {
      if(checkPoint.elapsed(timeout)) {
	throw DlRetryEx(EX_TIME_OUT);
      }
      e->commands.push_back(this);
      return false;
    }
  } catch(DlAbortEx& err) {
    logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
    onAbort();
    req->resetUrl();
    tryReserved();
    return true;
  } catch(DlRetryEx& err) {
    logger->info(MSG_RESTARTING_DOWNLOAD, err, cuid, req->getUrl().c_str());
    req->addTryCount();
    req->resetRedirectCount();
    bool isAbort = e->option->getAsInt(PREF_MAX_TRIES) != 0 &&
      req->getTryCount() >= (unsigned int)e->option->getAsInt(PREF_MAX_TRIES);
    if(isAbort) {
      onAbort();
    }
    // In case where Request::getCurrentUrl() is not a valid URI.
    req->resetUrl();
    if(isAbort) {
      logger->info(MSG_MAX_TRY, cuid, req->getTryCount());
      logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
      tryReserved();
      return true;
    } else {
      return prepareForRetry(e->option->getAsInt(PREF_RETRY_WAIT));
    }
  } catch(DownloadFailureException& err) {
    logger->error(EX_EXCEPTION_CAUGHT, err);
    _requestGroup->setHaltRequested(true);
    return true;
  }
}

void AbstractCommand::tryReserved() {
  _requestGroup->removeServerHost(cuid);
  Commands commands;
  _requestGroup->createNextCommand(commands, e, 1);
  e->addCommand(commands);
}

bool AbstractCommand::prepareForRetry(time_t wait) {
  if(!_requestGroup->getPieceStorage().isNull()) {
    _requestGroup->getSegmentMan()->cancelSegment(cuid);
  }
  Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, _requestGroup, e);
  if(wait == 0) {
    e->setNoWait(true);
    e->commands.push_back(command);
  } else {
    SleepCommand* scom = new SleepCommand(cuid, e, command, wait);
    e->commands.push_back(scom);
  }
  return true;
}

void AbstractCommand::onAbort() {
  logger->debug(MSG_UNREGISTER_CUID, cuid);
  //_segmentMan->unregisterId(cuid);
  if(!_requestGroup->getPieceStorage().isNull()) {
    _requestGroup->getSegmentMan()->cancelSegment(cuid);
  }
  _requestGroup->removeIdenticalURI(req->getUrl());
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

#ifdef ENABLE_ASYNC_DNS

bool AbstractCommand::isAsyncNameResolverInitialized() const
{
  return !_asyncNameResolver.isNull();
}

void AbstractCommand::initAsyncNameResolver(const std::string& hostname)
{
  _asyncNameResolver.reset(new AsyncNameResolver());
  logger->info(MSG_RESOLVING_HOSTNAME, cuid, hostname.c_str());
  _asyncNameResolver->resolve(hostname);
  setNameResolverCheck(_asyncNameResolver);
}

bool AbstractCommand::asyncResolveHostname()
{
  switch(_asyncNameResolver->getStatus()) {
  case AsyncNameResolver::STATUS_SUCCESS:
    return true;
  case AsyncNameResolver::STATUS_ERROR:
    throw DlAbortEx(StringFormat(MSG_NAME_RESOLUTION_FAILED, cuid,
				 _asyncNameResolver->getHostname().c_str(),
				 _asyncNameResolver->getError().c_str()).str());
  default:
    return false;
  }
}

const std::deque<std::string>& AbstractCommand::getResolvedAddresses()
{
  return _asyncNameResolver->getResolvedAddresses();
}

void AbstractCommand::setNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver) {
  if(!resolver.isNull()) {
    nameResolverCheck = true;
    e->addNameResolverCheck(resolver, this);
  }
}

void AbstractCommand::disableNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver) {
  if(!resolver.isNull()) {
    nameResolverCheck = false;
    e->deleteNameResolverCheck(resolver, this);
  }
}

bool AbstractCommand::nameResolveFinished() const {
  return
    _asyncNameResolver->getStatus() ==  AsyncNameResolver::STATUS_SUCCESS ||
    _asyncNameResolver->getStatus() == AsyncNameResolver::STATUS_ERROR;
}
#endif // ENABLE_ASYNC_DNS

void AbstractCommand::prepareForNextAction(Command* nextCommand)
{
  CheckIntegrityEntryHandle entry(new StreamCheckIntegrityEntry(req, _requestGroup, nextCommand));

  std::deque<Command*> commands;
  _requestGroup->processCheckIntegrityEntry(commands, entry, e);

  e->addCommand(commands);
  e->setNoWait(true);
}

} // namespace aria2
