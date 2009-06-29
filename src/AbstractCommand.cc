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

#include <algorithm>

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
#include "CreateRequestCommand.h"
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
#include "ServerStat.h"
#include "RequestGroupMan.h"
#include "A2STR.h"
#include "Util.h"
#include "LogFactory.h"
#include "DownloadContext.h"

namespace aria2 {

// TODO1.5 Remove this
AbstractCommand::AbstractCommand(int32_t cuid,
				 const SharedHandle<Request>& req,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  Command(cuid), _requestGroup(requestGroup),
  req(req), e(e), socket(s),
  checkSocketIsReadable(false), checkSocketIsWritable(false),
  nameResolverCheck(false)
{
  if(!socket.isNull() && socket->isOpen()) {
    setReadCheckSocket(socket);
  }
  timeout = _requestGroup->getTimeout();
  _requestGroup->increaseStreamConnection();
  _requestGroup->increaseNumCommand();
}

AbstractCommand::AbstractCommand(int32_t cuid,
				 const SharedHandle<Request>& req,
				 const SharedHandle<FileEntry>& fileEntry,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  Command(cuid), _requestGroup(requestGroup),
  req(req), _fileEntry(fileEntry), e(e), socket(s),
  checkSocketIsReadable(false), checkSocketIsWritable(false),
  nameResolverCheck(false)
{
  if(!socket.isNull() && socket->isOpen()) {
    setReadCheckSocket(socket);
  }
  timeout = _requestGroup->getTimeout();
  _requestGroup->increaseStreamConnection();
  _requestGroup->increaseNumCommand();
}

AbstractCommand::~AbstractCommand() {
  disableReadCheckSocket();
  disableWriteCheckSocket();
#ifdef ENABLE_ASYNC_DNS
  disableNameResolverCheck(_asyncNameResolver);
#endif // ENABLE_ASYNC_DNS
  _requestGroup->decreaseNumCommand();
  _requestGroup->decreaseStreamConnection();
}

bool AbstractCommand::execute() {
  logger->debug("CUID#%d - socket: read:%d, write:%d, hup:%d, err:%d",
		cuid, _readEvent, _writeEvent, _hupEvent, _errorEvent);
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
       _hupEvent ||
#ifdef ENABLE_ASYNC_DNS
       (nameResolverCheck && nameResolveFinished()) ||
#endif // ENABLE_ASYNC_DNS
       (!checkSocketIsReadable && !checkSocketIsWritable && !nameResolverCheck)) {
      checkPoint.reset();
      if(!_requestGroup->getPieceStorage().isNull()) {
	_segments.clear();
	_requestGroup->getSegmentMan()->getInFlightSegment(_segments, cuid);
	size_t maxSegments = req.isNull()?1:req->getMaxPipelinedRequest();
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
	  return true;
	}
      }
      return executeInternal();
    } else if(_errorEvent) {
      throw DL_RETRY_EX
	(StringFormat(MSG_NETWORK_PROBLEM,
		      socket->getSocketError().c_str()).str());
    } else {
      if(checkPoint.elapsed(timeout)) {
	// timeout triggers ServerStat error state.

	SharedHandle<ServerStat> ss =
	  e->_requestGroupMan->getOrCreateServerStat(req->getHost(),
						     req->getProtocol());
	ss->setError();

	throw DL_RETRY_EX2(EX_TIME_OUT, downloadresultcode::TIME_OUT);
      }
      e->commands.push_back(this);
      return false;
    }
  } catch(DlAbortEx& err) {
    if(req.isNull()) {
      logger->debug(EX_EXCEPTION_CAUGHT, err);
    } else {
      logger->error(MSG_DOWNLOAD_ABORTED,
		    DL_ABORT_EX2(StringFormat
				 ("URI=%s", req->getCurrentUrl().c_str()).str(),err),
		    cuid, req->getUrl().c_str());
      _fileEntry->addURIResult(req->getUrl(), err.getCode());
      _requestGroup->setLastUriResult(req->getUrl(), err.getCode());
    }
    onAbort();
    // TODO Do we need this?
    //req->resetUrl();
    tryReserved();
    return true;
  } catch(DlRetryEx& err) {
    // TODO1.5 Consider the case when req is null
    logger->info(MSG_RESTARTING_DOWNLOAD,
		 DL_RETRY_EX2(StringFormat
			     ("URI=%s", req->getCurrentUrl().c_str()).str(),err),
		 cuid, req->getUrl().c_str());
    req->addTryCount();
    req->resetRedirectCount();
    const unsigned int maxTries = getOption()->getAsInt(PREF_MAX_TRIES);
    bool isAbort = maxTries != 0 && req->getTryCount() >= maxTries;
    if(isAbort) {
      onAbort();
      req->resetUrl();
    } else {
      if(getOption()->getAsBool(PREF_RESET_URI)) {
	req->resetUrl();
      }
    }
    if(isAbort) {
      logger->info(MSG_MAX_TRY, cuid, req->getTryCount());
      logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
      _fileEntry->addURIResult(req->getUrl(), err.getCode());
      _requestGroup->setLastUriResult(req->getUrl(), err.getCode());
      tryReserved();
      return true;
    } else {
      return prepareForRetry(getOption()->getAsInt(PREF_RETRY_WAIT));
    }
  } catch(DownloadFailureException& err) {
    // TODO1.5 Consider the case when req is null
    logger->error(EX_EXCEPTION_CAUGHT, err);
    _fileEntry->addURIResult(req->getUrl(), err.getCode());
    _requestGroup->setLastUriResult(req->getUrl(), err.getCode());
    _requestGroup->setHaltRequested(true);
    return true;
  }
}

void AbstractCommand::tryReserved() {
  _requestGroup->removeServerHost(cuid);
  if(_requestGroup->getDownloadContext()->getFileEntries().size() == 1) {
    const SharedHandle<FileEntry>& entry =
      _requestGroup->getDownloadContext()->getFirstFileEntry();
    // Don't create new command if currently file length is unknown
    // and there are no URI left. Because file length is unknown, we
    // can assume that there are no in-flight request object.
    if(entry->getLength() == 0 && entry->getRemainingUris().size() == 0) {
      return;
    }
  }
  Commands commands;
  _requestGroup->createNextCommand(commands, e, 1);
  e->setNoWait(true);
  e->addCommand(commands);
}

bool AbstractCommand::prepareForRetry(time_t wait) {
  if(!_requestGroup->getPieceStorage().isNull()) {
    _requestGroup->getSegmentMan()->cancelSegment(cuid);
  }
  if(!req.isNull()) {
    _fileEntry->poolRequest(req);
  }
  if(!_segments.empty()) {
    // TODO1.5 subtract 1 from getPositionToWrite()
    SharedHandle<FileEntry> fileEntry = _requestGroup->getDownloadContext()->findFileEntryByOffset(_segments.front()->getPositionToWrite()-1);
    logger->debug("CUID#%d - Pooling request URI=%s",
		  cuid, req->getUrl().c_str());
    _requestGroup->getSegmentMan()->recognizeSegmentFor(_fileEntry);
  }

  Command* command = new CreateRequestCommand(cuid, _requestGroup, e);
  if(wait == 0) {
    e->setNoWait(true);
    e->commands.push_back(command);
  } else {
    SleepCommand* scom = new SleepCommand(cuid, e, _requestGroup,
					  command, wait);
    e->commands.push_back(scom);
  }
  return true;
}

void AbstractCommand::onAbort() {
  if(!req.isNull()) {
    logger->debug(req->getCurrentUrl().c_str());
    // TODO This might be a problem if the failure is caused by proxy.
    e->_requestGroupMan->getOrCreateServerStat(req->getHost(),
					       req->getProtocol())->setError();
    _fileEntry->removeIdenticalURI(req->getUrl());
    _fileEntry->removeRequest(req);
  }

  logger->debug(MSG_UNREGISTER_CUID, cuid);
  //_segmentMan->unregisterId(cuid);
  if(!_requestGroup->getPieceStorage().isNull()) {
    _requestGroup->getSegmentMan()->cancelSegment(cuid);
  }
  // TODO1.5 Should be moved to FileEntry
  // _requestGroup->removeIdenticalURI(req->getUrl());
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

void AbstractCommand::setReadCheckSocketIf
(const SharedHandle<SocketCore>& socket, bool pred)
{
  if(pred) {
    setReadCheckSocket(socket);
  } else {
    disableReadCheckSocket();
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

void AbstractCommand::setWriteCheckSocketIf
(const SharedHandle<SocketCore>& socket, bool pred)
{
  if(pred) {
    setWriteCheckSocket(socket);
  } else {
    disableWriteCheckSocket();
  }
}

static const std::string& getProxyStringFor(const std::string& proxyPref,
					    const SharedHandle<Option>& option)
{
  if(option->defined(proxyPref)) {
    return option->get(proxyPref);
  } else {
    return option->get(PREF_ALL_PROXY);
  }
}

static bool isProxyUsed(const std::string& proxyPref,
			const SharedHandle<Option>& option)
{
  std::string proxy = getProxyStringFor(proxyPref, option);
  if(proxy.empty()) {
    return false;
  } else {
    return Request().setUrl(proxy);
  }
}

static bool isProxyRequest(const std::string& protocol,
			   const SharedHandle<Option>& option)
{
  return
    (protocol == Request::PROTO_HTTP && isProxyUsed(PREF_HTTP_PROXY, option)) ||
    (protocol == Request::PROTO_HTTPS && isProxyUsed(PREF_HTTPS_PROXY,option))||
    (protocol == Request::PROTO_FTP && isProxyUsed(PREF_FTP_PROXY, option));
}

class DomainMatch {
private:
  std::string _hostname;
public:
  DomainMatch(const std::string& hostname):_hostname(hostname) {}

  bool operator()(const std::string& domain) const
  {
    if(Util::startsWith(domain, ".")) {
      return Util::endsWith(_hostname, domain);
    } else {
      return Util::endsWith(_hostname, "."+domain);
    }
  }
};

static bool inNoProxy(const SharedHandle<Request>& req,
		      const std::string& noProxy)
{
  std::deque<std::string> entries;
  Util::slice(entries, noProxy, ',', true);
  if(entries.empty()) {
    return false;
  }
  return
    std::find_if(entries.begin(), entries.end(),
		 DomainMatch("."+req->getHost())) != entries.end();
}

bool AbstractCommand::isProxyDefined() const
{
  return isProxyRequest(req->getProtocol(), getOption()) &&
    !inNoProxy(req, getOption()->get(PREF_NO_PROXY));
}

static const std::string& getProxyString(const SharedHandle<Request>& req,
					 const SharedHandle<Option>& option)
{
  if(req->getProtocol() == Request::PROTO_HTTP) {
    return getProxyStringFor(PREF_HTTP_PROXY, option);
  } else if(req->getProtocol() == Request::PROTO_HTTPS) {
    return getProxyStringFor(PREF_HTTPS_PROXY, option);
  } else if(req->getProtocol() == Request::PROTO_FTP) {
    return getProxyStringFor(PREF_FTP_PROXY, option);
  } else {
    return A2STR::NIL;
  }
}

SharedHandle<Request> AbstractCommand::createProxyRequest() const
{
  SharedHandle<Request> proxyRequest;
  if(inNoProxy(req, getOption()->get(PREF_NO_PROXY))) {
    return proxyRequest;
  }
  std::string proxy = getProxyString(req, getOption());
  if(!proxy.empty()) {
    proxyRequest.reset(new Request());
    if(proxyRequest->setUrl(proxy)) {
      logger->debug("CUID#%d - Using proxy", cuid);      
    } else {
      logger->debug("CUID#%d - Failed to parse proxy string", cuid);
      proxyRequest.reset();
    }
  }
  return proxyRequest;
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
    if(!isProxyRequest(req->getProtocol(), getOption())) {
      e->_requestGroupMan->getOrCreateServerStat
	(req->getHost(), req->getProtocol())->setError();
    }
    throw DL_ABORT_EX(StringFormat(MSG_NAME_RESOLUTION_FAILED, cuid,
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
  CheckIntegrityEntryHandle entry
    (new StreamCheckIntegrityEntry(_requestGroup, nextCommand));

  std::deque<Command*> commands;
  _requestGroup->processCheckIntegrityEntry(commands, entry, e);

  e->addCommand(commands);
  e->setNoWait(true);
}

void AbstractCommand::checkIfConnectionEstablished
(const SharedHandle<SocketCore>& socket)
{
  if(socket->isReadable(0)) {
    std::string error = socket->getSocketError();
    if(!error.empty()) {
      // Don't set error if proxy server is used and its method is GET.
      if(resolveProxyMethod(req->getProtocol()) != V_GET ||
	 !isProxyRequest(req->getProtocol(), getOption())) {
	e->_requestGroupMan->getOrCreateServerStat
	  (req->getHost(), req->getProtocol())->setError();
      }
      throw DL_RETRY_EX
	(StringFormat(MSG_ESTABLISHING_CONNECTION_FAILED, error.c_str()).str());
    }
  }
}

const std::string& AbstractCommand::resolveProxyMethod
(const std::string& protocol) const
{
  if(getOption()->get(PREF_PROXY_METHOD) == V_TUNNEL ||
     Request::PROTO_HTTPS == protocol) {
    return V_TUNNEL;
  } else {
    return V_GET;
  }
}

const SharedHandle<Option>& AbstractCommand::getOption() const
{
  return _requestGroup->getOption();
}

} // namespace aria2
