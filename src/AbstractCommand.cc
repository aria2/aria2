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
#include "wallclock.h"
#include "NameResolver.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"

namespace aria2 {

AbstractCommand::AbstractCommand(cuid_t cuid,
                                 const SharedHandle<Request>& req,
                                 const SharedHandle<FileEntry>& fileEntry,
                                 RequestGroup* requestGroup,
                                 DownloadEngine* e,
                                 const SocketHandle& s):
  Command(cuid), checkPoint(global::wallclock), _requestGroup(requestGroup),
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
    logger->debug("CUID#%s - socket: read:%d, write:%d, hup:%d, err:%d",
                  util::itos(cuid).c_str(), _readEvent, _writeEvent, _hupEvent,
                  _errorEvent);
  }
  try {
    if(_requestGroup->downloadFinished() || _requestGroup->isHaltRequested()) {
      return true;
    }
    if(!req.isNull() && req->removalRequested()) {
      if(logger->debug()) {
        logger->debug
          ("CUID#%s - Discard original URI=%s because it is requested.",
           util::itos(cuid).c_str(), req->getUri().c_str());
      }
      return prepareForRetry(0);
    }
    // TODO it is not needed to check other PeerStats every time.
    // Find faster Request when no segment is available.
    if(!req.isNull() && _fileEntry->countPooledRequest() > 0 &&
       !_requestGroup->getPieceStorage()->hasMissingUnusedPiece()) {
      SharedHandle<Request> fasterRequest = _fileEntry->findFasterRequest(req);
      if(!fasterRequest.isNull()) {
        if(logger->info()) {
          logger->info("CUID#%s - Use faster Request hostname=%s, port=%u",
                       util::itos(cuid).c_str(),
                       fasterRequest->getHost().c_str(),
                       fasterRequest->getPort());
        }
        // Cancel current Request object and use faster one.
        _fileEntry->removeRequest(req);
        Command* command =
          InitiateConnectionCommandFactory::createInitiateConnectionCommand
          (cuid, fasterRequest, _fileEntry, _requestGroup, e);
        e->setNoWait(true);
        e->addCommand(command);
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
      checkPoint = global::wallclock;
      if(!_requestGroup->getPieceStorage().isNull()) {
        _segments.clear();
        _requestGroup->getSegmentMan()->getInFlightSegment(_segments, cuid);
        if(!req.isNull() && _segments.empty()) {
          // This command previously has assigned segments, but it is
          // canceled. So discard current request chain.
          if(logger->debug()) {
            logger->debug("CUID#%s - It seems previously assigned segments are"
                          " canceled. Restart.", util::itos(cuid).c_str());
          }
          return prepareForRetry(0);
        }
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
            if(logger->info()) {
              logger->info(MSG_NO_SEGMENT_AVAILABLE, util::itos(cuid).c_str());
            }
            // When all segments are ignored in SegmentMan, there are
            // no URIs available, so don't retry.
            if(_requestGroup->getSegmentMan()->allSegmentsIgnored()) {
              if(logger->debug()) {
                logger->debug("All segments are ignored.");
              }
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
      if(checkPoint.difference(global::wallclock) >= timeout) {
        // timeout triggers ServerStat error state.

        SharedHandle<ServerStat> ss =
          e->getRequestGroupMan()->getOrCreateServerStat(req->getHost(),
                                                         req->getProtocol());
        ss->setError();

        throw DL_RETRY_EX2(EX_TIME_OUT, downloadresultcode::TIME_OUT);
      }
      e->addCommand(this);
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
                                 ("URI=%s", req->getCurrentUri().c_str()).str(),err),
                    util::itos(cuid).c_str(), req->getUri().c_str());
      _fileEntry->addURIResult(req->getUri(), err.getCode());
      _requestGroup->setLastUriResult(req->getUri(), err.getCode());
      if(err.getCode() == downloadresultcode::CANNOT_RESUME) {
        _requestGroup->increaseResumeFailureCount();
      }
    }
    onAbort();
    tryReserved();
    return true;
  } catch(DlRetryEx& err) {
    assert(!req.isNull());
    if(logger->info()) {
      logger->info(MSG_RESTARTING_DOWNLOAD,
                   DL_RETRY_EX2(StringFormat
                                ("URI=%s", req->getCurrentUri().c_str()).str(),
                                err),
                   util::itos(cuid).c_str(), req->getUri().c_str());
    }
    req->addTryCount();
    req->resetRedirectCount();
    const unsigned int maxTries = getOption()->getAsInt(PREF_MAX_TRIES);
    bool isAbort = maxTries != 0 && req->getTryCount() >= maxTries;
    if(isAbort) {
      if(logger->info()) {
        logger->info(MSG_MAX_TRY, util::itos(cuid).c_str(), req->getTryCount());
      }
      logger->error(MSG_DOWNLOAD_ABORTED, err, util::itos(cuid).c_str(),
                    req->getUri().c_str());
      _fileEntry->addURIResult(req->getUri(), err.getCode());
      _requestGroup->setLastUriResult(req->getUri(), err.getCode());
      if(err.getCode() == downloadresultcode::CANNOT_RESUME) {
        _requestGroup->increaseResumeFailureCount();
      }
      onAbort();
      tryReserved();
      return true;
    } else {
      return prepareForRetry(0);
    }
  } catch(DownloadFailureException& err) {
    logger->error(EX_EXCEPTION_CAUGHT, err);
    if(!req.isNull()) {
      _fileEntry->addURIResult(req->getUri(), err.getCode());
      _requestGroup->setLastUriResult(req->getUri(), err.getCode());
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
        logger->debug("CUID#%s - Not trying next request."
                      " No reserved/pooled request is remaining and"
                      " total length is still unknown.",
                      util::itos(cuid).c_str());
      }
      return;
    }
  }
  if(logger->debug()) {
    logger->debug("CUID#%s - Trying reserved/pooled request.",
                  util::itos(cuid).c_str());
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
      logger->debug("CUID#%s - Pooling request URI=%s",
                    util::itos(cuid).c_str(), req->getUri().c_str());
    }
    if(!_requestGroup->getSegmentMan().isNull()) {
      _requestGroup->getSegmentMan()->recognizeSegmentFor(_fileEntry);
    }
  }

  Command* command = new CreateRequestCommand(cuid, _requestGroup, e);
  if(wait == 0) {
    e->setNoWait(true);
    e->addCommand(command);
  } else {
    SleepCommand* scom = new SleepCommand(cuid, e, _requestGroup,
                                          command, wait);
    e->addCommand(scom);
  }
  return true;
}

void AbstractCommand::onAbort() {
  if(!req.isNull()) {
    // TODO This might be a problem if the failure is caused by proxy.
    e->getRequestGroupMan()->getOrCreateServerStat(req->getHost(),
                                                   req->getProtocol())->setError();
    _fileEntry->removeIdenticalURI(req->getUri());
    _fileEntry->removeRequest(req);
  }
  if(logger->debug()) {
    logger->debug("CUID#%s - Aborting download", util::itos(cuid).c_str());
  }
  if(!_requestGroup->getPieceStorage().isNull()) {
    SharedHandle<SegmentMan> segmentMan = _requestGroup->getSegmentMan();
    segmentMan->cancelSegment(cuid);
    // Don't do following process if BitTorrent is involved or files
    // in DownloadContext is more than 1. The latter condition is
    // limitation of current implementation.
    if(!getOption()->getAsBool(PREF_ALWAYS_RESUME) &&
       !_fileEntry.isNull() &&
       segmentMan->calculateSessionDownloadLength() == 0 &&
       !_requestGroup->p2pInvolved() &&
       _requestGroup->getDownloadContext()->getFileEntries().size() == 1) {
      const int maxTries = getOption()->getAsInt(PREF_MAX_RESUME_FAILURE_TRIES);
      if((maxTries > 0 && _requestGroup->getResumeFailureCount() >= maxTries)||
         _fileEntry->emptyRequestUri()) {
        // Local file exists, but given servers(or at least contacted
        // ones) doesn't support resume. Let's restart download from
        // scratch.
        logger->notice("CUID#%s - Failed to resume download."
                       " Download from scratch.",
                       util::itos(cuid).c_str());
        if(logger->debug()) {
          logger->debug("CUID#%s - Gathering URIs that has CANNOT_RESUME error",
                        util::itos(cuid).c_str());
        }
        // Set PREF_ALWAYS_RESUME to V_TRUE to avoid repeating this
        // process.
        getOption()->put(PREF_ALWAYS_RESUME, V_TRUE);
        std::deque<URIResult> res;
        _fileEntry->extractURIResult(res, downloadresultcode::CANNOT_RESUME);
        if(!res.empty()) {
          segmentMan->cancelAllSegments();
          segmentMan->eraseSegmentWrittenLengthMemo();
          _requestGroup->getPieceStorage()->markPiecesDone(0);
          std::vector<std::string> uris;
          uris.reserve(res.size());
          std::transform(res.begin(), res.end(), std::back_inserter(uris),
                         std::mem_fun_ref(&URIResult::getURI));
          if(logger->debug()) {
            logger->debug("CUID#%s - %lu URIs found.",
                          util::itos(cuid).c_str(),
                          static_cast<unsigned long int>(uris.size()));
          }
          _fileEntry->addUris(uris.begin(), uris.end());
          segmentMan->recognizeSegmentFor(_fileEntry);
        }
      }
    }
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
  return !proxyUri.empty() && Request().setUri(proxyUri);
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
  DomainMatch domainMatch(A2STR::DOT_C+req->getHost());
  for(std::vector<std::string>::const_iterator i = entries.begin(),
        eoi = entries.end(); i != eoi; ++i) {
    std::string::size_type slashpos = (*i).find('/');
    if(slashpos == std::string::npos) {
      if(util::isNumericHost(*i)) {
        if(req->getHost() == *i) {
          return true;
        }
      } else if(domainMatch(*i)) {
        return true;
      }
    } else {
      if(!util::isNumericHost(req->getHost())) {
        // TODO We don't resolve hostname here.  More complete
        // implementation is that we should first resolve
        // hostname(which may result in several IP addresses) and
        // evaluates against all of them
        continue;
      }
      std::string ip = (*i).substr(0, slashpos);
      uint32_t bits;
      if(!util::parseUIntNoThrow(bits, (*i).substr(slashpos+1))) {
        continue;
      }
      if(util::inSameCidrBlock(ip, req->getHost(), bits)) {
        return true;
      }
    }
  }
  return false;
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
    if(proxyRequest->setUri(proxy)) {
      if(logger->debug()) {
        logger->debug("CUID#%s - Using proxy", util::itos(cuid).c_str());
      }
    } else {
      if(logger->debug()) {
        logger->debug("CUID#%s - Failed to parse proxy string",
                      util::itos(cuid).c_str());
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
  if(logger->info()) {
    logger->info(MSG_RESOLVING_HOSTNAME,
                 util::itos(cuid).c_str(), hostname.c_str());
  }
  _asyncNameResolver->resolve(hostname);
  setNameResolverCheck(_asyncNameResolver);
}

bool AbstractCommand::asyncResolveHostname()
{
  switch(_asyncNameResolver->getStatus()) {
  case AsyncNameResolver::STATUS_SUCCESS:
    disableNameResolverCheck(_asyncNameResolver);
    return true;
  case AsyncNameResolver::STATUS_ERROR:
    disableNameResolverCheck(_asyncNameResolver);
    if(!isProxyRequest(req->getProtocol(), getOption())) {
      e->getRequestGroupMan()->getOrCreateServerStat
        (req->getHost(), req->getProtocol())->setError();
    }
    throw DL_ABORT_EX(StringFormat(MSG_NAME_RESOLUTION_FAILED,
                                   util::itos(cuid).c_str(),
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

std::string AbstractCommand::resolveHostname
(std::vector<std::string>& addrs, const std::string& hostname, uint16_t port)
{
  e->findAllCachedIPAddresses(std::back_inserter(addrs), hostname, port);
  std::string ipaddr;
  if(addrs.empty()) {
#ifdef ENABLE_ASYNC_DNS
    if(getOption()->getAsBool(PREF_ASYNC_DNS)) {
      if(!isAsyncNameResolverInitialized()) {
        initAsyncNameResolver(hostname);
      }
      if(asyncResolveHostname()) {
        addrs = getResolvedAddresses();
      } else {
        return A2STR::NIL;
      }
    } else
#endif // ENABLE_ASYNC_DNS
      {
        NameResolver res;
        res.setSocktype(SOCK_STREAM);
        if(e->getOption()->getAsBool(PREF_DISABLE_IPV6)) {
          res.setFamily(AF_INET);
        }
        res.resolve(addrs, hostname);
      }
    if(logger->info()) {
      logger->info(MSG_NAME_RESOLUTION_COMPLETE, util::itos(cuid).c_str(),
                   hostname.c_str(),
                   strjoin(addrs.begin(), addrs.end(), ", ").c_str());
    }
    for(std::vector<std::string>::const_iterator i = addrs.begin(),
          eoi = addrs.end(); i != eoi; ++i) {
      e->cacheIPAddress(hostname, *i, port);
    }
    ipaddr = e->findCachedIPAddress(hostname, port);
  } else {
    ipaddr = addrs.front();
    if(logger->info()) {
      logger->info(MSG_DNS_CACHE_HIT,
                   util::itos(cuid).c_str(), hostname.c_str(),
                   strjoin(addrs.begin(), addrs.end(), ", ").c_str());
    }
  }
  return ipaddr;
}

// nextCommand is going to be managed by CheckIntegrityEntry which is
// created in side this function. Don't release nextCommand after this
// function call.
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
        if(logger->info()) {
          logger->info(MSG_CONNECT_FAILED_AND_RETRY,
                       util::itos(cuid).c_str(),
                       connectedAddr.c_str(), connectedPort);
        }
        Command* command =
          InitiateConnectionCommandFactory::createInitiateConnectionCommand
          (cuid, req, _fileEntry, _requestGroup, e);
        e->setNoWait(true);
        e->addCommand(command);
        return false;
      }
      e->removeCachedIPAddress(connectedHostname, connectedPort);
      // Don't set error if proxy server is used and its method is GET.
      if(resolveProxyMethod(req->getProtocol()) != V_GET ||
         !isProxyRequest(req->getProtocol(), getOption())) {
        e->getRequestGroupMan()->getOrCreateServerStat
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
