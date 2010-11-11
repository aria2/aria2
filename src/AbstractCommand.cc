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
#include "uri.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "ChecksumCheckIntegrityEntry.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

AbstractCommand::AbstractCommand(cuid_t cuid,
                                 const SharedHandle<Request>& req,
                                 const SharedHandle<FileEntry>& fileEntry,
                                 RequestGroup* requestGroup,
                                 DownloadEngine* e,
                                 const SocketHandle& s,
                                 bool incNumConnection):
  Command(cuid), checkPoint_(global::wallclock),
  timeout_(requestGroup->getTimeout()),
  requestGroup_(requestGroup),
  req_(req), fileEntry_(fileEntry), e_(e), socket_(s),
  checkSocketIsReadable_(false), checkSocketIsWritable_(false),
  nameResolverCheck_(false),
  incNumConnection_(incNumConnection)
{
  if(!socket_.isNull() && socket_->isOpen()) {
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

bool AbstractCommand::execute() {
  if(getLogger()->debug()) {
    getLogger()->debug("CUID#%s - socket: read:%d, write:%d, hup:%d, err:%d",
                       util::itos(getCuid()).c_str(),
                       readEventEnabled(),
                       writeEventEnabled(),
                       hupEventEnabled(),
                       errorEventEnabled());
  }
  try {
    if(requestGroup_->downloadFinished() || requestGroup_->isHaltRequested()) {
      return true;
    }
    if(!req_.isNull() && req_->removalRequested()) {
      if(getLogger()->debug()) {
        getLogger()->debug
          ("CUID#%s - Discard original URI=%s because it is requested.",
           util::itos(getCuid()).c_str(), req_->getUri().c_str());
      }
      return prepareForRetry(0);
    }
    if(!getPieceStorage().isNull()) {
      segments_.clear();
      getSegmentMan()->getInFlightSegment(segments_, getCuid());
      if(!req_.isNull() && segments_.empty()) {
        // This command previously has assigned segments, but it is
        // canceled. So discard current request chain.  Plus, if no
        // segment is available when http pipelining is used.
        if(getLogger()->debug()) {
          getLogger()->debug("CUID#%s - It seems previously assigned segments"
                             " are canceled. Restart.",
                             util::itos(getCuid()).c_str());
        }
        // Request::isPipeliningEnabled() == true means aria2
        // accessed the remote server and discovered that the server
        // supports pipelining.
        if(!req_.isNull() && req_->isPipeliningEnabled()) {
          e_->poolSocket(req_, createProxyRequest(), socket_);
        }
        return prepareForRetry(0);
      }
      // TODO it is not needed to check other PeerStats every time.
      // Find faster Request when no segment is available.
      if(!req_.isNull() && fileEntry_->countPooledRequest() > 0 &&
         !getPieceStorage()->hasMissingUnusedPiece()) {
        SharedHandle<Request> fasterRequest = fileEntry_->findFasterRequest(req_);
        if(!fasterRequest.isNull()) {
          if(getLogger()->info()) {
            getLogger()->info("CUID#%s - Use faster Request hostname=%s, port=%u",
                              util::itos(getCuid()).c_str(),
                              fasterRequest->getHost().c_str(),
                              fasterRequest->getPort());
          }
          // Cancel current Request object and use faster one.
          fileEntry_->removeRequest(req_);
          Command* command =
            InitiateConnectionCommandFactory::createInitiateConnectionCommand
            (getCuid(), fasterRequest, fileEntry_, requestGroup_, e_);
          e_->setNoWait(true);
          e_->addCommand(command);
          return true;
        }
      }
    }
    if((checkSocketIsReadable_ && readEventEnabled()) ||
       (checkSocketIsWritable_ && writeEventEnabled()) ||
       hupEventEnabled() ||
#ifdef ENABLE_ASYNC_DNS
       (nameResolverCheck_ && nameResolveFinished()) ||
#endif // ENABLE_ASYNC_DNS
       (!checkSocketIsReadable_ && !checkSocketIsWritable_ &&
        !nameResolverCheck_)) {
      checkPoint_ = global::wallclock;
      if(!getPieceStorage().isNull()) {
        if(req_.isNull() || req_->getMaxPipelinedRequest() == 1 ||
           // Why the following condition is necessary? That's because
           // For single file download, SegmentMan::getSegment(cuid)
           // is more efficient.
           getDownloadContext()->getFileEntries().size() == 1) {
          size_t maxSegments = req_.isNull()?1:req_->getMaxPipelinedRequest();
          size_t minSplitSize = calculateMinSplitSize();
          while(segments_.size() < maxSegments) {
            SharedHandle<Segment> segment =
              getSegmentMan()->getSegment(getCuid(), minSplitSize);
            if(segment.isNull()) {
              break;
            } else {
              segments_.push_back(segment);
            }
          }
          if(segments_.empty()) {
            // TODO socket could be pooled here if pipelining is
            // enabled...  Hmm, I don't think if pipelining is enabled
            // it does not go here.
            if(getLogger()->info()) {
              getLogger()->info(MSG_NO_SEGMENT_AVAILABLE,
                                util::itos(getCuid()).c_str());
            }
            // When all segments are ignored in SegmentMan, there are
            // no URIs available, so don't retry.
            if(getSegmentMan()->allSegmentsIgnored()) {
              if(getLogger()->debug()) {
                getLogger()->debug("All segments are ignored.");
              }
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
      throw DL_RETRY_EX
        (StringFormat(MSG_NETWORK_PROBLEM,
                      socket_->getSocketError().c_str()).str());
    } else {
      if(checkPoint_.difference(global::wallclock) >= timeout_) {
        // timeout triggers ServerStat error state.
        SharedHandle<ServerStat> ss =
          e_->getRequestGroupMan()->getOrCreateServerStat(req_->getHost(),
                                                          req_->getProtocol());
        ss->setError();
        // Purging IP address cache to renew IP address.
        if(getLogger()->debug()) {
          getLogger()->debug("CUID#%s - Marking IP address %s as bad",
                             util::itos(getCuid()).c_str(),
                             req_->getConnectedAddr().c_str());
        }
        e_->markBadIPAddress(req_->getConnectedHostname(),
                             req_->getConnectedAddr(),
                             req_->getConnectedPort());
        if(e_->findCachedIPAddress
           (req_->getConnectedHostname(), req_->getConnectedPort()).empty()) {
          if(getLogger()->debug()) {
            getLogger()->debug("CUID#%s - All IP addresses were marked bad."
                               " Removing Entry.",
                               util::itos(getCuid()).c_str());
          }
          e_->removeCachedIPAddress
            (req_->getConnectedHostname(), req_->getConnectedPort());
        }
        throw DL_RETRY_EX2(EX_TIME_OUT, downloadresultcode::TIME_OUT);
      }
      e_->addCommand(this);
      return false;
    }
  } catch(DlAbortEx& err) {
    if(req_.isNull()) {
      if(getLogger()->debug()) {
        getLogger()->debug(EX_EXCEPTION_CAUGHT, err);
      }
    } else {
      getLogger()->error
        (MSG_DOWNLOAD_ABORTED,
         DL_ABORT_EX2(StringFormat
                      ("URI=%s", req_->getCurrentUri().c_str()).str(),err),
         util::itos(getCuid()).c_str(), req_->getUri().c_str());
      fileEntry_->addURIResult(req_->getUri(), err.getCode());
      requestGroup_->setLastUriResult(req_->getUri(), err.getCode());
      if(err.getCode() == downloadresultcode::CANNOT_RESUME) {
        requestGroup_->increaseResumeFailureCount();
      }
    }
    onAbort();
    tryReserved();
    return true;
  } catch(DlRetryEx& err) {
    assert(!req_.isNull());
    if(getLogger()->info()) {
      getLogger()->info
        (MSG_RESTARTING_DOWNLOAD,
         DL_RETRY_EX2(StringFormat
                      ("URI=%s", req_->getCurrentUri().c_str()).str(),
                      err),
         util::itos(getCuid()).c_str(), req_->getUri().c_str());
    }
    req_->addTryCount();
    req_->resetRedirectCount();
    req_->resetUri();
    const unsigned int maxTries = getOption()->getAsInt(PREF_MAX_TRIES);
    bool isAbort = maxTries != 0 && req_->getTryCount() >= maxTries;
    if(isAbort) {
      if(getLogger()->info()) {
        getLogger()->info(MSG_MAX_TRY,
                          util::itos(getCuid()).c_str(), req_->getTryCount());
      }
      getLogger()->error(MSG_DOWNLOAD_ABORTED, err,
                         util::itos(getCuid()).c_str(),
                         req_->getUri().c_str());
      fileEntry_->addURIResult(req_->getUri(), err.getCode());
      requestGroup_->setLastUriResult(req_->getUri(), err.getCode());
      if(err.getCode() == downloadresultcode::CANNOT_RESUME) {
        requestGroup_->increaseResumeFailureCount();
      }
      onAbort();
      tryReserved();
      return true;
    } else {
      return prepareForRetry(0);
    }
  } catch(DownloadFailureException& err) {
    getLogger()->error(EX_EXCEPTION_CAUGHT, err);
    if(!req_.isNull()) {
      fileEntry_->addURIResult(req_->getUri(), err.getCode());
      requestGroup_->setLastUriResult(req_->getUri(), err.getCode());
    }
    requestGroup_->setHaltRequested(true);
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
      if(getLogger()->debug()) {
        getLogger()->debug("CUID#%s - Not trying next request."
                           " No reserved/pooled request is remaining and"
                           " total length is still unknown.",
                           util::itos(getCuid()).c_str());
      }
      return;
    }
  }
  if(getLogger()->debug()) {
    getLogger()->debug("CUID#%s - Trying reserved/pooled request.",
                       util::itos(getCuid()).c_str());
  }
  std::vector<Command*> commands;
  requestGroup_->createNextCommand(commands, e_, 1);
  e_->setNoWait(true);
  e_->addCommand(commands);
}

bool AbstractCommand::prepareForRetry(time_t wait) {
  if(!getPieceStorage().isNull()) {
    getSegmentMan()->cancelSegment(getCuid());
  }
  if(!req_.isNull()) {
    fileEntry_->poolRequest(req_);
    if(getLogger()->debug()) {
      getLogger()->debug("CUID#%s - Pooling request URI=%s",
                         util::itos(getCuid()).c_str(), req_->getUri().c_str());
    }
    if(!getSegmentMan().isNull()) {
      getSegmentMan()->recognizeSegmentFor(fileEntry_);
    }
  }

  Command* command = new CreateRequestCommand(getCuid(), requestGroup_, e_);
  if(wait == 0) {
    e_->setNoWait(true);
    e_->addCommand(command);
  } else {
    SleepCommand* scom = new SleepCommand(getCuid(), e_, requestGroup_,
                                          command, wait);
    e_->addCommand(scom);
  }
  return true;
}

void AbstractCommand::onAbort() {
  if(!req_.isNull()) {
    fileEntry_->removeIdenticalURI(req_->getUri());
    fileEntry_->removeRequest(req_);
  }
  if(getLogger()->debug()) {
    getLogger()->debug("CUID#%s - Aborting download",
                       util::itos(getCuid()).c_str());
  }
  if(!getPieceStorage().isNull()) {
    getSegmentMan()->cancelSegment(getCuid());
    // Don't do following process if BitTorrent is involved or files
    // in DownloadContext is more than 1. The latter condition is
    // limitation of current implementation.
    if(!getOption()->getAsBool(PREF_ALWAYS_RESUME) &&
       !fileEntry_.isNull() &&
       getSegmentMan()->calculateSessionDownloadLength() == 0 &&
       !requestGroup_->p2pInvolved() &&
       getDownloadContext()->getFileEntries().size() == 1) {
      const int maxTries = getOption()->getAsInt(PREF_MAX_RESUME_FAILURE_TRIES);
      if((maxTries > 0 && requestGroup_->getResumeFailureCount() >= maxTries)||
         fileEntry_->emptyRequestUri()) {
        // Local file exists, but given servers(or at least contacted
        // ones) doesn't support resume. Let's restart download from
        // scratch.
        getLogger()->notice("CUID#%s - Failed to resume download."
                            " Download from scratch.",
                            util::itos(getCuid()).c_str());
        if(getLogger()->debug()) {
          getLogger()->debug
            ("CUID#%s - Gathering URIs that has CANNOT_RESUME error",
             util::itos(getCuid()).c_str());
        }
        // Set PREF_ALWAYS_RESUME to A2_V_TRUE to avoid repeating this
        // process.
        getOption()->put(PREF_ALWAYS_RESUME, A2_V_TRUE);
        std::deque<URIResult> res;
        fileEntry_->extractURIResult(res, downloadresultcode::CANNOT_RESUME);
        if(!res.empty()) {
          getSegmentMan()->cancelAllSegments();
          getSegmentMan()->eraseSegmentWrittenLengthMemo();
          getPieceStorage()->markPiecesDone(0);
          std::vector<std::string> uris;
          uris.reserve(res.size());
          std::transform(res.begin(), res.end(), std::back_inserter(uris),
                         std::mem_fun_ref(&URIResult::getURI));
          if(getLogger()->debug()) {
            getLogger()->debug("CUID#%s - %lu URIs found.",
                               util::itos(getCuid()).c_str(),
                               static_cast<unsigned long int>(uris.size()));
          }
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
// Returns proxy option value for the given protocol.
const std::string& getProxyOptionFor
(const std::string& proxyPref, const SharedHandle<Option>& option)
{
  if(option->defined(proxyPref)) {
    return option->get(proxyPref);
  } else {
    return option->get(PREF_ALL_PROXY);
  }
}
} // namespace

namespace {
// Returns proxy URI for given protocol.  If no proxy URI is defined,
// then returns an empty string.
const std::string& getProxyUri
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
} // namespace

namespace {
// Returns true if proxy is defined for the given protocol. Otherwise
// returns false.
bool isProxyRequest
(const std::string& protocol, const SharedHandle<Option>& option)
{
  const std::string& proxyUri = getProxyUri(protocol, option);
  uri::UriStruct us;
  return !proxyUri.empty() && uri::parse(us, proxyUri);
}
} // namespace

namespace {
class DomainMatch {
private:
  std::string hostname_;
public:
  DomainMatch(const std::string& hostname):hostname_(hostname) {}

  bool operator()(const std::string& domain) const
  {
    if(util::startsWith(domain, A2STR::DOT_C)) {
      return util::endsWith(hostname_, domain);
    } else {
      return util::endsWith(hostname_, A2STR::DOT_C+domain);
    }
  }
};
} // namespace

namespace {
bool inNoProxy(const SharedHandle<Request>& req,
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
  std::string proxy = getProxyUri(req_->getProtocol(), getOption());
  if(!proxy.empty()) {
    proxyRequest.reset(new Request());
    if(proxyRequest->setUri(proxy)) {
      if(getLogger()->debug()) {
        getLogger()->debug("CUID#%s - Using proxy",
                           util::itos(getCuid()).c_str());
      }
    } else {
      if(getLogger()->debug()) {
        getLogger()->debug("CUID#%s - Failed to parse proxy string",
                           util::itos(getCuid()).c_str());
      }
      proxyRequest.reset();
    }
  }
  return proxyRequest;
}

#ifdef ENABLE_ASYNC_DNS

bool AbstractCommand::isAsyncNameResolverInitialized() const
{
  return !asyncNameResolver_.isNull();
}

void AbstractCommand::initAsyncNameResolver(const std::string& hostname)
{
  int family;
  if(getOption()->getAsBool(PREF_ENABLE_ASYNC_DNS6)) {
    family = AF_UNSPEC;
  } else {
    family = AF_INET;
  }
  asyncNameResolver_.reset(new AsyncNameResolver(family));
  if(getLogger()->info()) {
    getLogger()->info(MSG_RESOLVING_HOSTNAME,
                      util::itos(getCuid()).c_str(), hostname.c_str());
  }
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
    throw DL_ABORT_EX
      (StringFormat(MSG_NAME_RESOLUTION_FAILED,
                    util::itos(getCuid()).c_str(),
                    asyncNameResolver_->getHostname().c_str(),
                    asyncNameResolver_->getError().c_str()).str());
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
  if(!resolver.isNull()) {
    nameResolverCheck_ = true;
    e_->addNameResolverCheck(resolver, this);
  }
}

void AbstractCommand::disableNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver) {
  if(!resolver.isNull()) {
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
    if(getLogger()->info()) {
      getLogger()->info(MSG_NAME_RESOLUTION_COMPLETE,
                        util::itos(getCuid()).c_str(),
                        hostname.c_str(),
                        strjoin(addrs.begin(), addrs.end(), ", ").c_str());
    }
    for(std::vector<std::string>::const_iterator i = addrs.begin(),
          eoi = addrs.end(); i != eoi; ++i) {
      e_->cacheIPAddress(hostname, *i, port);
    }
    ipaddr = e_->findCachedIPAddress(hostname, port);
  } else {
    ipaddr = addrs.front();
    if(getLogger()->info()) {
      getLogger()->info(MSG_DNS_CACHE_HIT,
                        util::itos(getCuid()).c_str(), hostname.c_str(),
                        strjoin(addrs.begin(), addrs.end(), ", ").c_str());
    }
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
        if(getLogger()->info()) {
          getLogger()->info(MSG_CONNECT_FAILED_AND_RETRY,
                            util::itos(getCuid()).c_str(),
                            connectedAddr.c_str(), connectedPort);
        }
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
  return requestGroup_->getOption();
}

void AbstractCommand::createSocket()
{
  socket_.reset(new SocketCore());
}

size_t AbstractCommand::calculateMinSplitSize() const
{
  if(!req_.isNull() && req_->isPipeliningEnabled()) {
    return getDownloadContext()->getPieceLength();
  } else {
    return getOption()->getAsInt(PREF_MIN_SPLIT_SIZE);
  }
}

} // namespace aria2
