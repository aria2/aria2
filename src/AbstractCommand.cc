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
#include "StreamCheckIntegrityEntry.h"
#include "PieceStorage.h"
#include "Socket.h"
#include "message.h"
#include "prefs.h"
#include "fmt.h"
#include "ServerStat.h"
#include "RequestGroupMan.h"
#include "A2STR.h"
#include "util.h"
#include "LogFactory.h"
#include "DownloadContext.h"
#include "wallclock.h"
#include "NameResolver.h"
#include "uri.h"
#include "FileEntry.h"
#include "error_code.h"
#include "SocketRecvBuffer.h"
#ifdef ENABLE_ASYNC_DNS
#include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS
#ifdef ENABLE_MESSAGE_DIGEST
# include "ChecksumCheckIntegrityEntry.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

AbstractCommand::AbstractCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 DownloadEngine* e,
 const SocketHandle& s,
 const SharedHandle<SocketRecvBuffer>& socketRecvBuffer,
 bool incNumConnection)
  : Command(cuid), checkPoint_(global::wallclock()),
    timeout_(requestGroup->getTimeout()),
    requestGroup_(requestGroup),
    req_(req), fileEntry_(fileEntry), e_(e), socket_(s),
    socketRecvBuffer_(socketRecvBuffer),
    checkSocketIsReadable_(false), checkSocketIsWritable_(false),
    nameResolverCheck_(false),
    incNumConnection_(incNumConnection),
    serverStatTimer_(global::wallclock())
{
  if(socket_ && socket_->isOpen()) {
    setReadCheckSocket(socket_);
  }
  if(incNumConnection_) {
    requestGroup->increaseStreamConnection();
  }
  requestGroup_->increaseStreamCommand();
  requestGroup_->increaseNumCommand();
}

AbstractCommand::~AbstractCommand() {
  disableReadCheckSocket();
  disableWriteCheckSocket();
#ifdef ENABLE_ASYNC_DNS
  disableNameResolverCheck(asyncNameResolver_);
#endif // ENABLE_ASYNC_DNS
  requestGroup_->decreaseNumCommand();
  requestGroup_->decreaseStreamCommand();
  if(incNumConnection_) {
    requestGroup_->decreaseStreamConnection();
  }
}

void AbstractCommand::useFasterRequest
(const SharedHandle<Request>& fasterRequest)
{
  A2_LOG_INFO(fmt("CUID#%" PRId64 " - Use faster Request hostname=%s, port=%u",
                  getCuid(),
                  fasterRequest->getHost().c_str(),
                  fasterRequest->getPort()));
  // Cancel current Request object and use faster one.
  fileEntry_->removeRequest(req_);
  Command* command =
    InitiateConnectionCommandFactory::createInitiateConnectionCommand
    (getCuid(), fasterRequest, fileEntry_, requestGroup_, e_);
  e_->setNoWait(true);
  e_->addCommand(command);
}

bool AbstractCommand::execute() {
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - socket: read:%d, write:%d, hup:%d, err:%d",
                   getCuid(),
                   readEventEnabled(),
                   writeEventEnabled(),
                   hupEventEnabled(),
                   errorEventEnabled()));
  try {
    if(requestGroup_->downloadFinished() || requestGroup_->isHaltRequested()) {
      return true;
    }
    if(req_ && req_->removalRequested()) {
      A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Discard original URI=%s because it is"
                       " requested.",
                       getCuid(), req_->getUri().c_str()));
      return prepareForRetry(0);
    }
    if(getPieceStorage()) {
      segments_.clear();
      getSegmentMan()->getInFlightSegment(segments_, getCuid());
      if(req_ && segments_.empty()) {
        // This command previously has assigned segments, but it is
        // canceled. So discard current request chain.  Plus, if no
        // segment is available when http pipelining is used.
        A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - It seems previously assigned segments"
                         " are canceled. Restart.",
                         getCuid()));
        // Request::isPipeliningEnabled() == true means aria2
        // accessed the remote server and discovered that the server
        // supports pipelining.
        if(req_ && req_->isPipeliningEnabled()) {
          e_->poolSocket(req_, createProxyRequest(), socket_);
        }
        return prepareForRetry(0);
      }
      // TODO it is not needed to check other PeerStats every time.
      // Find faster Request when no segment is available.
      if(req_ && fileEntry_->countPooledRequest() > 0 &&
         !getPieceStorage()->hasMissingUnusedPiece()) {
        SharedHandle<Request> fasterRequest = fileEntry_->findFasterRequest(req_);
        if(fasterRequest) {
          useFasterRequest(fasterRequest);
          return true;
        }
      }
      // Don't use this feature if PREF_MAX_{OVERALL_}DOWNLOAD_LIMIT
      // is used or total length is unknown.
      if(req_ && fileEntry_->getLength() > 0 &&
         e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit() == 0 &&
         requestGroup_->getMaxDownloadSpeedLimit() == 0 &&
         serverStatTimer_.difference(global::wallclock()) >= 10) {
        serverStatTimer_ = global::wallclock();
        std::vector<std::pair<size_t, std::string> > usedHosts;
        if(getOption()->getAsBool(PREF_SELECT_LEAST_USED_HOST)) {
          getDownloadEngine()->getRequestGroupMan()->getUsedHosts(usedHosts);
        }
        SharedHandle<Request> fasterRequest =
          fileEntry_->findFasterRequest
          (req_, usedHosts, e_->getRequestGroupMan()->getServerStatMan());
        if(fasterRequest) {
          useFasterRequest(fasterRequest);
          return true;
        }
      }
    }
    if((checkSocketIsReadable_ &&
        (readEventEnabled() ||
         (socketRecvBuffer_ && !socketRecvBuffer_->bufferEmpty()))) ||
       (checkSocketIsWritable_ && writeEventEnabled()) ||
       hupEventEnabled() ||
#ifdef ENABLE_ASYNC_DNS
       (nameResolverCheck_ && nameResolveFinished()) ||
#endif // ENABLE_ASYNC_DNS
       (!checkSocketIsReadable_ && !checkSocketIsWritable_ &&
        !nameResolverCheck_)) {
      checkPoint_ = global::wallclock();
      if(getPieceStorage()) {
        if(!req_ || req_->getMaxPipelinedRequest() == 1 ||
           // Why the following condition is necessary? That's because
           // For single file download, SegmentMan::getSegment(cuid)
           // is more efficient.
           getDownloadContext()->getFileEntries().size() == 1) {
          size_t maxSegments = req_?req_->getMaxPipelinedRequest():1;
          size_t minSplitSize = calculateMinSplitSize();
          while(segments_.size() < maxSegments) {
            SharedHandle<Segment> segment =
              getSegmentMan()->getSegment(getCuid(), minSplitSize);
            if(!segment) {
              break;
            } else {
              segments_.push_back(segment);
            }
          }
          if(segments_.empty()) {
            // TODO socket could be pooled here if pipelining is
            // enabled...  Hmm, I don't think if pipelining is enabled
            // it does not go here.
            A2_LOG_INFO(fmt(MSG_NO_SEGMENT_AVAILABLE,
                            getCuid()));
            // When all segments are ignored in SegmentMan, there are
            // no URIs available, so don't retry.
            if(getSegmentMan()->allSegmentsIgnored()) {
              A2_LOG_DEBUG("All segments are ignored.");
              // This will execute other idle Commands and let them
              // finish quickly.
              e_->setRefreshInterval(0);
              return true;
            } else {
              return prepareForRetry(1);
            }
          }
        } else {
          // For multi-file downloads
          size_t minSplitSize = calculateMinSplitSize();
          size_t maxSegments = req_->getMaxPipelinedRequest();
          if(segments_.size() < maxSegments) {
            getSegmentMan()->getSegment
              (segments_, getCuid(), minSplitSize, fileEntry_, maxSegments);
          }
          if(segments_.empty()) {
            return prepareForRetry(0);
          }
        }
      }
      return executeInternal();
    } else if(errorEventEnabled()) {
      throw DL_RETRY_EX(fmt(MSG_NETWORK_PROBLEM,
                            socket_->getSocketError().c_str()));
    } else {
      if(checkPoint_.difference(global::wallclock()) >= timeout_) {
        // timeout triggers ServerStat error state.
        SharedHandle<ServerStat> ss =
          e_->getRequestGroupMan()->getOrCreateServerStat(req_->getHost(),
                                                          req_->getProtocol());
        ss->setError();
        // When DNS query was timeout, req_->getConnectedAddr() is
        // empty.
        if(!req_->getConnectedAddr().empty()) {
          // Purging IP address cache to renew IP address.
          A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Marking IP address %s as bad",
                           getCuid(),
                           req_->getConnectedAddr().c_str()));
          e_->markBadIPAddress(req_->getConnectedHostname(),
                               req_->getConnectedAddr(),
                               req_->getConnectedPort());
        }
        if(e_->findCachedIPAddress
           (req_->getConnectedHostname(), req_->getConnectedPort()).empty()) {
          A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - All IP addresses were marked bad."
                           " Removing Entry.",
                           getCuid()));
          e_->removeCachedIPAddress
            (req_->getConnectedHostname(), req_->getConnectedPort());
        }
        throw DL_RETRY_EX2(EX_TIME_OUT, error_code::TIME_OUT);
      }
      e_->addCommand(this);
      return false;
    }
  } catch(DlAbortEx& err) {
    requestGroup_->setLastErrorCode(err.getErrorCode());
    if(req_) {
      A2_LOG_ERROR_EX(fmt(MSG_DOWNLOAD_ABORTED,
                          getCuid(),
                          req_->getUri().c_str()),
                      DL_ABORT_EX2(fmt("URI=%s", req_->getCurrentUri().c_str()),
                                   err));
      fileEntry_->addURIResult(req_->getUri(), err.getErrorCode());
      if(err.getErrorCode() == error_code::CANNOT_RESUME) {
        requestGroup_->increaseResumeFailureCount();
      }
    } else {
      A2_LOG_DEBUG_EX(EX_EXCEPTION_CAUGHT, err);
    }
    onAbort();
    tryReserved();
    return true;
  } catch(DlRetryEx& err) {
    assert(req_);
    A2_LOG_INFO_EX(fmt(MSG_RESTARTING_DOWNLOAD,
                       getCuid(), req_->getUri().c_str()),
                   DL_RETRY_EX2(fmt("URI=%s", req_->getCurrentUri().c_str()),
                                err));
    req_->addTryCount();
    req_->resetRedirectCount();
    req_->resetUri();
    const int maxTries = getOption()->getAsInt(PREF_MAX_TRIES);
    bool isAbort = maxTries != 0 && req_->getTryCount() >= maxTries;
    if(isAbort) {
      A2_LOG_INFO(fmt(MSG_MAX_TRY,
                      getCuid(),
                      req_->getTryCount()));
      A2_LOG_ERROR_EX(fmt(MSG_DOWNLOAD_ABORTED,
                          getCuid(),
                          req_->getUri().c_str()),
                      err);
      fileEntry_->addURIResult(req_->getUri(), err.getErrorCode());
      requestGroup_->setLastErrorCode(err.getErrorCode());
      if(err.getErrorCode() == error_code::CANNOT_RESUME) {
        requestGroup_->increaseResumeFailureCount();
      }
      onAbort();
      tryReserved();
      return true;
    } else {
      Timer wakeTime(global::wallclock());
      wakeTime.advance(getOption()->getAsInt(PREF_RETRY_WAIT));
      req_->setWakeTime(wakeTime);
      return prepareForRetry(0);
    }
  } catch(DownloadFailureException& err) {
    requestGroup_->setLastErrorCode(err.getErrorCode());
    if(req_) {
      A2_LOG_ERROR_EX(fmt(MSG_DOWNLOAD_ABORTED,
                          getCuid(),
                          req_->getUri().c_str()),
                      DL_ABORT_EX2(fmt("URI=%s", req_->getCurrentUri().c_str()),
                                   err));
      fileEntry_->addURIResult(req_->getUri(), err.getErrorCode());
    } else {
      A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, err);
    }
    requestGroup_->setHaltRequested(true);
    getDownloadEngine()->setRefreshInterval(0);
    return true;
  }
}

void AbstractCommand::tryReserved() {
  if(getDownloadContext()->getFileEntries().size() == 1) {
    const SharedHandle<FileEntry>& entry =
      getDownloadContext()->getFirstFileEntry();
    // Don't create new command if currently file length is unknown
    // and there are no URI left. Because file length is unknown, we
    // can assume that there are no in-flight request object.
    if(entry->getLength() == 0 && entry->getRemainingUris().empty()) {
      A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Not trying next request."
                       " No reserved/pooled request is remaining and"
                       " total length is still unknown.",
                       getCuid()));
      return;
    }
  }
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Trying reserved/pooled request.",
                   getCuid()));
  std::vector<Command*> commands;
  requestGroup_->createNextCommand(commands, e_, 1);
  e_->setNoWait(true);
  e_->addCommand(commands);
}

bool AbstractCommand::prepareForRetry(time_t wait) {
  if(getPieceStorage()) {
    getSegmentMan()->cancelSegment(getCuid());
  }
  if(req_) {
    // Reset persistentConnection and maxPipelinedRequest to handle
    // the situation where remote server returns Connection: close
    // after several pipelined requests.
    req_->supportsPersistentConnection(true);
    req_->setMaxPipelinedRequest(1);

    fileEntry_->poolRequest(req_);
    A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Pooling request URI=%s",
                     getCuid(), req_->getUri().c_str()));
    if(getSegmentMan()) {
      getSegmentMan()->recognizeSegmentFor(fileEntry_);
    }
  }

  Command* command = new CreateRequestCommand(getCuid(), requestGroup_, e_);
  if(wait == 0) {
    e_->setNoWait(true);
    e_->addCommand(command);
  } else {
    // We don't use wait so that Command can be executed by
    // DownloadEngine::setRefreshInterval(0).
    command->setStatus(Command::STATUS_INACTIVE);
    e_->addCommand(command);
  }
  return true;
}

void AbstractCommand::onAbort() {
  if(req_) {
    fileEntry_->removeIdenticalURI(req_->getUri());
    fileEntry_->removeRequest(req_);
  }
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Aborting download",
                   getCuid()));
  if(getPieceStorage()) {
    getSegmentMan()->cancelSegment(getCuid());
    // Don't do following process if BitTorrent is involved or files
    // in DownloadContext is more than 1. The latter condition is
    // limitation of current implementation.
    if(!getOption()->getAsBool(PREF_ALWAYS_RESUME) &&
       fileEntry_ &&
       getSegmentMan()->calculateSessionDownloadLength() == 0 &&
       !requestGroup_->p2pInvolved() &&
       getDownloadContext()->getFileEntries().size() == 1) {
      const int maxTries = getOption()->getAsInt(PREF_MAX_RESUME_FAILURE_TRIES);
      if((maxTries > 0 && requestGroup_->getResumeFailureCount() >= maxTries)||
         fileEntry_->emptyRequestUri()) {
        // Local file exists, but given servers(or at least contacted
        // ones) doesn't support resume. Let's restart download from
        // scratch.
        A2_LOG_NOTICE(fmt(_("CUID#%" PRId64 " - Failed to resume download."
                            " Download from scratch."),
                          getCuid()));
        A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Gathering URIs that has CANNOT_RESUME"
                         " error",
                         getCuid()));
        // Set PREF_ALWAYS_RESUME to A2_V_TRUE to avoid repeating this
        // process.
        getOption()->put(PREF_ALWAYS_RESUME, A2_V_TRUE);
        std::deque<URIResult> res;
        fileEntry_->extractURIResult(res, error_code::CANNOT_RESUME);
        if(!res.empty()) {
          getSegmentMan()->cancelAllSegments();
          getSegmentMan()->eraseSegmentWrittenLengthMemo();
          getPieceStorage()->markPiecesDone(0);
          std::vector<std::string> uris;
          uris.reserve(res.size());
          std::transform(res.begin(), res.end(), std::back_inserter(uris),
                         std::mem_fun_ref(&URIResult::getURI));
          A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - %lu URIs found.",
                           getCuid(),
                           static_cast<unsigned long int>(uris.size())));
          fileEntry_->addUris(uris.begin(), uris.end());
          getSegmentMan()->recognizeSegmentFor(fileEntry_);
        }
      }
    }
  }
}

void AbstractCommand::disableReadCheckSocket() {
  if(checkSocketIsReadable_) {
    e_->deleteSocketForReadCheck(readCheckTarget_, this);
    checkSocketIsReadable_ = false;
    readCheckTarget_.reset();
  }  
}

void AbstractCommand::setReadCheckSocket(const SocketHandle& socket) {
  if(!socket->isOpen()) {
    disableReadCheckSocket();
  } else {
    if(checkSocketIsReadable_) {
      if(*readCheckTarget_ != *socket) {
        e_->deleteSocketForReadCheck(readCheckTarget_, this);
        e_->addSocketForReadCheck(socket, this);
        readCheckTarget_ = socket;
      }
    } else {
      e_->addSocketForReadCheck(socket, this);
      checkSocketIsReadable_ = true;
      readCheckTarget_ = socket;
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
  if(checkSocketIsWritable_) {
    e_->deleteSocketForWriteCheck(writeCheckTarget_, this);
    checkSocketIsWritable_ = false;
    writeCheckTarget_.reset();
  }
}

void AbstractCommand::setWriteCheckSocket(const SocketHandle& socket) {
  if(!socket->isOpen()) {
    disableWriteCheckSocket();
  } else {
    if(checkSocketIsWritable_) {
      if(*writeCheckTarget_ != *socket) {
        e_->deleteSocketForWriteCheck(writeCheckTarget_, this);
        e_->addSocketForWriteCheck(socket, this);
        writeCheckTarget_ = socket;
      }
    } else {
      e_->addSocketForWriteCheck(socket, this);
      checkSocketIsWritable_ = true;
      writeCheckTarget_ = socket;
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

namespace {
// Constructs proxy URI, merging username and password if they are
// defined.
std::string makeProxyUri
(const Pref* proxyPref,
 const Pref* proxyUser,
 const Pref* proxyPasswd,
 const Option* option)
{
  uri::UriStruct us;
  if(uri::parse(us, option->get(proxyPref))) {
    if(option->defined(proxyUser)) {
      us.username = option->get(proxyUser);
    }
    if(option->defined(proxyPasswd)) {
      us.password = option->get(proxyPasswd);
      us.hasPassword = true;
    }
    return uri::construct(us);
  } else {
    return "";
  }
}
} // namespace

namespace {
// Returns proxy option value for the given protocol.
std::string getProxyOptionFor
(const Pref* proxyPref,
 const Pref* proxyUser,
 const Pref* proxyPasswd,
 const Option* option)
{
  std::string uri = makeProxyUri(proxyPref, proxyUser, proxyPasswd, option);
  if(uri.empty()) {
    return makeProxyUri(PREF_ALL_PROXY, PREF_ALL_PROXY_USER,
                        PREF_ALL_PROXY_PASSWD, option);
  } else {
    return uri;
  }
}
} // namespace

// Returns proxy URI for given protocol.  If no proxy URI is defined,
// then returns an empty string.
std::string getProxyUri
(const std::string& protocol, const Option* option)
{
  if(protocol == Request::PROTO_HTTP) {
    return getProxyOptionFor(PREF_HTTP_PROXY,
                             PREF_HTTP_PROXY_USER,
                             PREF_HTTP_PROXY_PASSWD,
                             option);
  } else if(protocol == Request::PROTO_HTTPS) {
    return getProxyOptionFor(PREF_HTTPS_PROXY,
                             PREF_HTTPS_PROXY_USER,
                             PREF_HTTPS_PROXY_PASSWD,
                             option);
  } else if(protocol == Request::PROTO_FTP) {
    return getProxyOptionFor(PREF_FTP_PROXY,
                             PREF_FTP_PROXY_USER,
                             PREF_FTP_PROXY_PASSWD,
                             option);
  } else {
    return A2STR::NIL;
  }
}

namespace {
// Returns true if proxy is defined for the given protocol. Otherwise
// returns false.
bool isProxyRequest
(const std::string& protocol, const SharedHandle<Option>& option)
{
  std::string proxyUri = getProxyUri(protocol, option.get());
  return !proxyUri.empty();
}
} // namespace

namespace {
bool inNoProxy(const SharedHandle<Request>& req,
               const std::string& noProxy)
{
  std::vector<Scip> entries;
  util::splitIter(noProxy.begin(), noProxy.end(), std::back_inserter(entries),
                  ',', true);
  if(entries.empty()) {
    return false;
  }
  for(std::vector<Scip>::const_iterator i = entries.begin(),
        eoi = entries.end(); i != eoi; ++i) {
    std::string::const_iterator slashpos =
      std::find((*i).first, (*i).second, '/');
    if(slashpos == (*i).second) {
      if(util::noProxyDomainMatch
         (req->getHost(), std::string((*i).first, (*i).second))) {
        return true;
      }
    } else {
      // TODO We don't resolve hostname here.  More complete
      // implementation is that we should first resolve
      // hostname(which may result in several IP addresses) and
      // evaluates against all of them
      std::string ip((*i).first, slashpos);
      uint32_t bits;
      if(!util::parseUIntNoThrow(bits, std::string(slashpos+1, (*i).second))) {
        continue;
      }
      if(util::inSameCidrBlock(ip, req->getHost(), bits)) {
        return true;
      }
    }
  }
  return false;
}
} // namespace

bool AbstractCommand::isProxyDefined() const
{
  return isProxyRequest(req_->getProtocol(), getOption()) &&
    !inNoProxy(req_, getOption()->get(PREF_NO_PROXY));
}

SharedHandle<Request> AbstractCommand::createProxyRequest() const
{
  SharedHandle<Request> proxyRequest;
  if(inNoProxy(req_, getOption()->get(PREF_NO_PROXY))) {
    return proxyRequest;
  }
  std::string proxy = getProxyUri(req_->getProtocol(), getOption().get());
  if(!proxy.empty()) {
    proxyRequest.reset(new Request());
    if(proxyRequest->setUri(proxy)) {
      A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Using proxy", getCuid()));
    } else {
      A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Failed to parse proxy string",
                       getCuid()));
      proxyRequest.reset();
    }
  }
  return proxyRequest;
}

#ifdef ENABLE_ASYNC_DNS

bool AbstractCommand::isAsyncNameResolverInitialized() const
{
  return asyncNameResolver_;
}

void AbstractCommand::initAsyncNameResolver(const std::string& hostname)
{
  int family;
  if(getOption()->getAsBool(PREF_ENABLE_ASYNC_DNS6)) {
    family = AF_UNSPEC;
  } else {
    family = AF_INET;
  }
  asyncNameResolver_.reset
    (new AsyncNameResolver(family
#ifdef HAVE_ARES_ADDR_NODE
                           ,
                           e_->getAsyncDNSServers()
#endif // HAVE_ARES_ADDR_NODE
                           ));
  A2_LOG_INFO(fmt(MSG_RESOLVING_HOSTNAME,
                  getCuid(),
                  hostname.c_str()));
  asyncNameResolver_->resolve(hostname);
  setNameResolverCheck(asyncNameResolver_);
}

bool AbstractCommand::asyncResolveHostname()
{
  switch(asyncNameResolver_->getStatus()) {
  case AsyncNameResolver::STATUS_SUCCESS:
    disableNameResolverCheck(asyncNameResolver_);
    return true;
  case AsyncNameResolver::STATUS_ERROR:
    disableNameResolverCheck(asyncNameResolver_);
    if(!isProxyRequest(req_->getProtocol(), getOption())) {
      e_->getRequestGroupMan()->getOrCreateServerStat
        (req_->getHost(), req_->getProtocol())->setError();
    }
    throw DL_ABORT_EX2
      (fmt(MSG_NAME_RESOLUTION_FAILED,
           getCuid(),
           asyncNameResolver_->getHostname().c_str(),
           asyncNameResolver_->getError().c_str()),
       error_code::NAME_RESOLVE_ERROR);
  default:
    return false;
  }
}

const std::vector<std::string>& AbstractCommand::getResolvedAddresses()
{
  return asyncNameResolver_->getResolvedAddresses();
}

void AbstractCommand::setNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver) {
  if(resolver) {
    nameResolverCheck_ = true;
    e_->addNameResolverCheck(resolver, this);
  }
}

void AbstractCommand::disableNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver) {
  if(resolver) {
    nameResolverCheck_ = false;
    e_->deleteNameResolverCheck(resolver, this);
  }
}

bool AbstractCommand::nameResolveFinished() const {
  return
    asyncNameResolver_->getStatus() ==  AsyncNameResolver::STATUS_SUCCESS ||
    asyncNameResolver_->getStatus() == AsyncNameResolver::STATUS_ERROR;
}
#endif // ENABLE_ASYNC_DNS

std::string AbstractCommand::resolveHostname
(std::vector<std::string>& addrs, const std::string& hostname, uint16_t port)
{
  if(util::isNumericHost(hostname)) {
    addrs.push_back(hostname);
    return hostname;
  }
  e_->findAllCachedIPAddresses(std::back_inserter(addrs), hostname, port);
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
        if(e_->getOption()->getAsBool(PREF_DISABLE_IPV6)) {
          res.setFamily(AF_INET);
        }
        res.resolve(addrs, hostname);
      }
    A2_LOG_INFO(fmt(MSG_NAME_RESOLUTION_COMPLETE,
                    getCuid(),
                    hostname.c_str(),
                    strjoin(addrs.begin(), addrs.end(), ", ").c_str()));
    for(std::vector<std::string>::const_iterator i = addrs.begin(),
          eoi = addrs.end(); i != eoi; ++i) {
      e_->cacheIPAddress(hostname, *i, port);
    }
    ipaddr = e_->findCachedIPAddress(hostname, port);
  } else {
    ipaddr = addrs.front();
    A2_LOG_INFO(fmt(MSG_DNS_CACHE_HIT,
                    getCuid(),
                    hostname.c_str(),
                    strjoin(addrs.begin(), addrs.end(), ", ").c_str()));
  }
  return ipaddr;
}

void AbstractCommand::prepareForNextAction
(const SharedHandle<CheckIntegrityEntry>& checkEntry)
{
  std::vector<Command*>* commands = new std::vector<Command*>();
  auto_delete_container<std::vector<Command*> > commandsDel(commands);
  requestGroup_->processCheckIntegrityEntry(*commands, checkEntry, e_);
  e_->addCommand(*commands);
  commands->clear();
  e_->setNoWait(true);
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
      e_->markBadIPAddress(connectedHostname, connectedAddr, connectedPort);
      if(!e_->findCachedIPAddress(connectedHostname, connectedPort).empty()) {
        A2_LOG_INFO(fmt(MSG_CONNECT_FAILED_AND_RETRY,
                        getCuid(),
                        connectedAddr.c_str(), connectedPort));
        Command* command =
          InitiateConnectionCommandFactory::createInitiateConnectionCommand
          (getCuid(), req_, fileEntry_, requestGroup_, e_);
        e_->setNoWait(true);
        e_->addCommand(command);
        return false;
      }
      e_->removeCachedIPAddress(connectedHostname, connectedPort);
      // Don't set error if proxy server is used and its method is GET.
      if(resolveProxyMethod(req_->getProtocol()) != V_GET ||
         !isProxyRequest(req_->getProtocol(), getOption())) {
        e_->getRequestGroupMan()->getOrCreateServerStat
          (req_->getHost(), req_->getProtocol())->setError();
      }
      throw DL_RETRY_EX
        (fmt(MSG_ESTABLISHING_CONNECTION_FAILED, error.c_str()));
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
  return requestGroup_->getOption();
}

void AbstractCommand::createSocket()
{
  socket_.reset(new SocketCore());
}

int32_t AbstractCommand::calculateMinSplitSize() const
{
  if(req_ && req_->isPipeliningEnabled()) {
    return getDownloadContext()->getPieceLength();
  } else {
    return getOption()->getAsInt(PREF_MIN_SPLIT_SIZE);
  }
}

void AbstractCommand::setRequest(const SharedHandle<Request>& request)
{
  req_ = request;
}

void AbstractCommand::resetRequest()
{
  req_.reset();
}

void AbstractCommand::setFileEntry(const SharedHandle<FileEntry>& fileEntry)
{
  fileEntry_ = fileEntry;
}

void AbstractCommand::setSocket(const SharedHandle<SocketCore>& s)
{
  socket_ = s;
}

const SharedHandle<DownloadContext>& AbstractCommand::getDownloadContext() const
{
  return requestGroup_->getDownloadContext();
}

const SharedHandle<SegmentMan>& AbstractCommand::getSegmentMan() const
{
  return requestGroup_->getSegmentMan();
}

const SharedHandle<PieceStorage>& AbstractCommand::getPieceStorage() const
{
  return requestGroup_->getPieceStorage();
}

void AbstractCommand::checkSocketRecvBuffer()
{
  if(!socketRecvBuffer_->bufferEmpty()) {
    setStatus(Command::STATUS_ONESHOT_REALTIME);
    e_->setNoWait(true);
  }
}

} // namespace aria2
