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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include "ServerStat.h"
#include "RequestGroupMan.h"
#include "A2STR.h"
#include "util.h"
#include "LogFactory.h"
#include "DownloadContext.h"

namespace aria2 {

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
  if(logger->debug()) {
    logger->debug("CUID#%d - socket: read:%d, write:%d, hup:%d, err:%d",
                  cuid, _readEvent, _writeEvent, _hupEvent, _errorEvent);
  }
  try {
    if(_requestGroup->downloadFinished() || _requestGroup->isHaltRequested()) {
      //logger->debug("CUID#%d - finished.", cuid);
      return true;
    }
    // TODO it is not needed to check other PeerStats every time.
    // Find faster Request when no segment is available.
    if(!req.isNull() && _fileEntry->countPooledRequest() > 0 &&
       !_requestGroup->getPieceStorage()->hasMissingUnusedPiece()) {
      SharedHandle<Request> fasterRequest = _fileEntry->findFasterRequest(req);
      if(!fasterRequest.isNull()) {
        logger->info("CUID#%d - Use faster Request hostname=%s, port=%u",
                     cuid,
                     fasterRequest->getHost().c_str(),
                     fasterRequest->getPort());

        // Cancel current Request object and use faster one.
        _fileEntry->removeRequest(req);
        Command* command =
          InitiateConnectionCommandFactory::createInitiateConnectionCommand
          (cuid, fasterRequest, _fileEntry, _requestGroup, e);
        e->setNoWait(true);
        e->commands.push_back(command);
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
        if(req.isNull() || req->getMaxPipelinedRequest() == 1 ||
           _requestGroup->getDownloadContext()->getFileEntries().size() == 1) {
          if(_segments.empty()) {
            SharedHandle<Segment> segment =
              _requestGroup->getSegmentMan()->getSegment(cuid);
            if(!segment.isNull()) {
              _segments.push_back(segment);
            }
          }
          if(_segments.empty()) {
            // TODO socket could be pooled here if pipelining is enabled...
            logger->info(MSG_NO_SEGMENT_AVAILABLE, cuid);
            // When all segments are ignored in SegmentMan, there are
            // no URIs available, so don't retry.
            if(_requestGroup->getSegmentMan()->allSegmentsIgnored()) {
              return true;
            } else {
              return prepareForRetry(1);
            }
          }
        } else {
          size_t maxSegments = req->getMaxPipelinedRequest();
          if(_segments.size() < maxSegments) {
            _requestGroup->getSegmentMan()->getSegment
              (_segments, cuid, _fileEntry, maxSegments);
          }
          if(_segments.empty()) {
            return prepareForRetry(0);
          }
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
      if(logger->debug()) {
        logger->debug(EX_EXCEPTION_CAUGHT, err);
      }
    } else {
      logger->error(MSG_DOWNLOAD_ABORTED,
                    DL_ABORT_EX2(StringFormat
                                 ("URI=%s", req->getCurrentUrl().c_str()).str(),err),
                    cuid, req->getUrl().c_str());
      _fileEntry->addURIResult(req->getUrl(), err.getCode());
      _requestGroup->setLastUriResult(req->getUrl(), err.getCode());
    }
    onAbort();
    tryReserved();
    return true;
  } catch(DlRetryEx& err) {
    assert(!req.isNull());
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
      logger->info(MSG_MAX_TRY, cuid, req->getTryCount());
      logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
      _fileEntry->addURIResult(req->getUrl(), err.getCode());
      _requestGroup->setLastUriResult(req->getUrl(), err.getCode());
      tryReserved();
      return true;
    } else {
      return prepareForRetry(0);
    }
  } catch(DownloadFailureException& err) {
    logger->error(EX_EXCEPTION_CAUGHT, err);
    if(!req.isNull()) {
      _fileEntry->addURIResult(req->getUrl(), err.getCode());
      _requestGroup->setLastUriResult(req->getUrl(), err.getCode());
    }
    _requestGroup->setHaltRequested(true);
    return true;
  }
}

void AbstractCommand::tryReserved() {
  if(_requestGroup->getDownloadContext()->getFileEntries().size() == 1) {
    const SharedHandle<FileEntry>& entry =
      _requestGroup->getDownloadContext()->getFirstFileEntry();
    // Don't create new command if currently file length is unknown
    // and there are no URI left. Because file length is unknown, we
    // can assume that there are no in-flight request object.
    if(entry->getLength() == 0 && entry->getRemainingUris().empty()) {
      if(logger->debug()) {
        logger->debug("CUID#%d - Not trying next request."
                      " No reserved/pooled request is remaining and"
                      " total length is still unknown.", cuid);
      }
      return;
    }
  }
  if(logger->debug()) {
    logger->debug("CUID#%d - Trying reserved/pooled request.", cuid);
  }
  std::vector<Command*> commands;
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
    if(logger->debug()) {
      logger->debug("CUID#%d - Pooling request URI=%s",
                    cuid, req->getUrl().c_str());
    }
    if(!_requestGroup->getSegmentMan().isNull()) {
      _requestGroup->getSegmentMan()->recognizeSegmentFor(_fileEntry);
    }
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
    // TODO This might be a problem if the failure is caused by proxy.
    e->_requestGroupMan->getOrCreateServerStat(req->getHost(),
                                               req->getProtocol())->setError();
    _fileEntry->removeIdenticalURI(req->getUrl());
    _fileEntry->removeRequest(req);
  }
  if(logger->debug()) {
    logger->debug("CUID#%d - Aborting download", cuid);
  }
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

// Returns proxy option value for the given protocol.
static const std::string& getProxyOptionFor
(const std::string& proxyPref, const SharedHandle<Option>& option)
{
  if(option->defined(proxyPref)) {
    return option->get(proxyPref);
  } else {
    return option->get(PREF_ALL_PROXY);
  }
}

// Returns proxy URI for given protocol.  If no proxy URI is defined,
// then returns an empty string.
static const std::string& getProxyUri
(const std::string& protocol, const SharedHandle<Option>& option)
{
  if(protocol == Request::PROTO_HTTP) {
    return getProxyOptionFor(PREF_HTTP_PROXY, option);
  } else if(protocol == Request::PROTO_HTTPS) {
    return getProxyOptionFor(PREF_HTTPS_PROXY, option);
  } else if(protocol == Request::PROTO_FTP) {
    return getProxyOptionFor(PREF_FTP_PROXY, option);
  } else {
    return A2STR::NIL;
  }
}

// Returns true if proxy is defined for the given protocol. Otherwise
// returns false.
static bool isProxyRequest
(const std::string& protocol, const SharedHandle<Option>& option)
{
  const std::string& proxyUri = getProxyUri(protocol, option);
  return !proxyUri.empty() && Request().setUrl(proxyUri);
}

class DomainMatch {
private:
  std::string _hostname;
public:
  DomainMatch(const std::string& hostname):_hostname(hostname) {}

  bool operator()(const std::string& domain) const
  {
    if(util::startsWith(domain, A2STR::DOT_C)) {
      return util::endsWith(_hostname, domain);
    } else {
      return util::endsWith(_hostname, A2STR::DOT_C+domain);
    }
  }
};

static bool inNoProxy(const SharedHandle<Request>& req,
                      const std::string& noProxy)
{
  std::vector<std::string> entries;
  util::split(noProxy, std::back_inserter(entries), ",", true);
  if(entries.empty()) {
    return false;
  }
  return
    std::find_if(entries.begin(), entries.end(),
                 DomainMatch(A2STR::DOT_C+req->getHost())) != entries.end();
}

bool AbstractCommand::isProxyDefined() const
{
  return isProxyRequest(req->getProtocol(), getOption()) &&
    !inNoProxy(req, getOption()->get(PREF_NO_PROXY));
}

SharedHandle<Request> AbstractCommand::createProxyRequest() const
{
  SharedHandle<Request> proxyRequest;
  if(inNoProxy(req, getOption()->get(PREF_NO_PROXY))) {
    return proxyRequest;
  }
  std::string proxy = getProxyUri(req->getProtocol(), getOption());
  if(!proxy.empty()) {
    proxyRequest.reset(new Request());
    if(proxyRequest->setUrl(proxy)) {
      if(logger->debug()) {
        logger->debug("CUID#%d - Using proxy", cuid);
      }
    } else {
      if(logger->debug()) {
        logger->debug("CUID#%d - Failed to parse proxy string", cuid);
      }
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

const std::vector<std::string>& AbstractCommand::getResolvedAddresses()
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
  SharedHandle<CheckIntegrityEntry> entry
    (new StreamCheckIntegrityEntry(_requestGroup, nextCommand));

  std::vector<Command*> commands;
  try {
    _requestGroup->processCheckIntegrityEntry(commands, entry, e);
  } catch(RecoverableException& e) {
    std::for_each(commands.begin(), commands.end(), Deleter());
    throw;
  }
  e->addCommand(commands);
  e->setNoWait(true);
}

bool AbstractCommand::checkIfConnectionEstablished
(const SharedHandle<SocketCore>& socket,
 const std::string& connectedHostname,
 const std::string& connectedAddr,
 uint16_t connectedPort)
{
  if(socket->isReadable(0)) {
    std::string error = socket->getSocketError();
    if(!error.empty()) {
      // See also InitiateConnectionCommand::executeInternal()
      e->markBadIPAddress(connectedHostname, connectedAddr, connectedPort);
      if(!e->findCachedIPAddress(connectedHostname, connectedPort).empty()) {
        logger->info(MSG_CONNECT_FAILED_AND_RETRY,
                     cuid, connectedAddr.c_str(), connectedPort);
        Command* command =
          InitiateConnectionCommandFactory::createInitiateConnectionCommand
          (cuid, req, _fileEntry, _requestGroup, e);
        e->setNoWait(true);
        e->commands.push_back(command);
        return false;
      }
      e->removeCachedIPAddress(connectedHostname, connectedPort);
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
  return true;
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
