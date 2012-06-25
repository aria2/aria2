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
#include "FtpNegotiationCommand.h"

#include <stdint.h>
#include <cassert>
#include <utility>
#include <map>

#include "Request.h"
#include "DownloadEngine.h"
#include "FtpConnection.h"
#include "RequestGroup.h"
#include "PieceStorage.h"
#include "FtpDownloadCommand.h"
#include "FileEntry.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "util.h"
#include "Option.h"
#include "Logger.h"
#include "LogFactory.h"
#include "Segment.h"
#include "DownloadContext.h"
#include "DefaultBtProgressInfoFile.h"
#include "RequestGroupMan.h"
#include "DownloadFailureException.h"
#include "Socket.h"
#include "fmt.h"
#include "DiskAdaptor.h"
#include "SegmentMan.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "a2functional.h"
#include "URISelector.h"
#include "HttpConnection.h"
#include "HttpHeader.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "DlRetryEx.h"
#include "CheckIntegrityEntry.h"
#include "error_code.h"
#include "SocketRecvBuffer.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "ChecksumCheckIntegrityEntry.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

FtpNegotiationCommand::FtpNegotiationCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 DownloadEngine* e,
 const SocketHandle& socket,
 Seq seq,
 const std::string& baseWorkingDir):
  AbstractCommand(cuid, req, fileEntry, requestGroup, e, socket),
  sequence_(seq),
  ftp_(new FtpConnection(cuid, socket, req,
                        e->getAuthConfigFactory()->createAuthConfig
                        (req, requestGroup->getOption().get()),
                         getOption().get())),
  pasvPort_(0)
{
  ftp_->setBaseWorkingDir(baseWorkingDir);
  if(seq == SEQ_RECV_GREETING) {
    setTimeout(getOption()->getAsInt(PREF_CONNECT_TIMEOUT));
  }
  disableReadCheckSocket();
  setWriteCheckSocket(getSocket());
}

FtpNegotiationCommand::~FtpNegotiationCommand() {}

bool FtpNegotiationCommand::executeInternal() {
  while(processSequence(getSegments().front()));
  if(sequence_ == SEQ_RETRY) {
    return prepareForRetry(0);
  } else if(sequence_ == SEQ_NEGOTIATION_COMPLETED) {
    FtpDownloadCommand* command =
      new FtpDownloadCommand
      (getCuid(), getRequest(), getFileEntry(), getRequestGroup(), ftp_,
       getDownloadEngine(), dataSocket_, getSocket());
    command->setStartupIdleTime(getOption()->getAsInt(PREF_STARTUP_IDLE_TIME));
    command->setLowestDownloadSpeedLimit
      (getOption()->getAsInt(PREF_LOWEST_SPEED_LIMIT));
    if(getFileEntry()->isUniqueProtocol()) {
      getFileEntry()->removeURIWhoseHostnameIs(getRequest()->getHost());
    }
    getRequestGroup()->getURISelector()->tuneDownloadCommand
      (getFileEntry()->getRemainingUris(), command);
    getDownloadEngine()->addCommand(command);
    return true;
  } else if(sequence_ == SEQ_HEAD_OK ||
            sequence_ == SEQ_DOWNLOAD_ALREADY_COMPLETED) {
    return true;
  } else if(sequence_ == SEQ_FILE_PREPARATION) {
    if(getOption()->getAsBool(PREF_FTP_PASV)) {
      sequence_ = SEQ_PREPARE_PASV;
    } else {
      sequence_ = SEQ_PREPARE_PORT;
    }
    return false;
  } else if(sequence_ == SEQ_EXIT) {
    return true;
  } else {
    getDownloadEngine()->addCommand(this);
    return false;
  }
}

bool FtpNegotiationCommand::recvGreeting() {
  if(!checkIfConnectionEstablished
     (getSocket(), getRequest()->getConnectedHostname(),
      getRequest()->getConnectedAddr(), getRequest()->getConnectedPort())) {
    sequence_ = SEQ_EXIT;
    return false;
  }
  setTimeout(getRequestGroup()->getTimeout());
  //socket->setBlockingMode();
  disableWriteCheckSocket();
  setReadCheckSocket(getSocket());

  int status = ftp_->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 220) {
    throw DL_ABORT_EX2(EX_CONNECTION_FAILED, error_code::FTP_PROTOCOL_ERROR);
  }
  sequence_ = SEQ_SEND_USER;

  return true;
}

bool FtpNegotiationCommand::sendUser() {
  if(ftp_->sendUser()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_USER;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvUser() {
  int status = ftp_->receiveResponse();
  switch(status) {
  case 0:
    return false;
  case 230:
    sequence_ = SEQ_SEND_TYPE;
    break;
  case 331:
    sequence_ = SEQ_SEND_PASS;
    break;
  default:
    throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, status),
                       error_code::FTP_PROTOCOL_ERROR);
  }
  return true;
}

bool FtpNegotiationCommand::sendPass() {
  if(ftp_->sendPass()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_PASS;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvPass() {
  int status = ftp_->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 230) {
    throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, status),
                       error_code::FTP_PROTOCOL_ERROR);
  }
  sequence_ = SEQ_SEND_TYPE;
  return true;
}

bool FtpNegotiationCommand::sendType() {
  if(ftp_->sendType()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_TYPE;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvType() {
  int status = ftp_->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 200) {
    throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, status),
                       error_code::FTP_PROTOCOL_ERROR);
  }
  sequence_ = SEQ_SEND_PWD;
  return true;
}

bool FtpNegotiationCommand::sendPwd()
{
  if(ftp_->sendPwd()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_PWD;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvPwd()
{
  std::string pwd;
  int status = ftp_->receivePwdResponse(pwd);
  if(status == 0) {
    return false;
  }
  if(status != 257) {
    throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, status),
                       error_code::FTP_PROTOCOL_ERROR);
  }
  ftp_->setBaseWorkingDir(pwd);
  A2_LOG_INFO(fmt("CUID#%" PRId64 " - base working directory is '%s'",
                  getCuid(), pwd.c_str()));
  sequence_ = SEQ_SEND_CWD_PREP;
  return true;
}

bool FtpNegotiationCommand::sendCwdPrep()
{
  // Calling setReadCheckSocket() is needed when the socket is reused, 
  setReadCheckSocket(getSocket());
  cwdDirs_.push_front(ftp_->getBaseWorkingDir());
  util::split(getRequest()->getDir().begin(), getRequest()->getDir().end(),
              std::back_inserter(cwdDirs_), '/');
  sequence_ = SEQ_SEND_CWD;
  return true;
}

bool FtpNegotiationCommand::sendCwd()
{
  if(ftp_->sendCwd(cwdDirs_.front())) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_CWD;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvCwd()
{
  int status = ftp_->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 250) {
    poolConnection();
    getRequestGroup()->increaseAndValidateFileNotFoundCount();
    if (status == 550)
      throw DL_ABORT_EX2(MSG_RESOURCE_NOT_FOUND,
                         error_code::RESOURCE_NOT_FOUND);
    else
      throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, status),
                         error_code::FTP_PROTOCOL_ERROR);
  }
  cwdDirs_.pop_front();
  if(cwdDirs_.empty()) {
    if(getOption()->getAsBool(PREF_REMOTE_TIME)) {
      sequence_ = SEQ_SEND_MDTM;
    } else {
      sequence_ = SEQ_SEND_SIZE;
    }
  } else {
    sequence_ = SEQ_SEND_CWD;
  }
  return true;
}

bool FtpNegotiationCommand::sendMdtm()
{
  if(ftp_->sendMdtm()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_MDTM;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvMdtm()
{
  Time lastModifiedTime = Time::null();
  int status = ftp_->receiveMdtmResponse(lastModifiedTime);
  if(status == 0) {
    return false;
  }
  if(status == 213) {
    if(lastModifiedTime.good()) {
      getRequestGroup()->updateLastModifiedTime(lastModifiedTime);
      A2_LOG_DEBUG(fmt("MDTM result was parsed as: %s",
                       lastModifiedTime.toHTTPDate().c_str()));
    } else {
      A2_LOG_DEBUG("MDTM response was returned, but it seems not to be"
                   " a time value as in specified in RFC3659.");
    }
  } else {
    A2_LOG_INFO(fmt("CUID#%" PRId64 " - MDTM command failed.",
                    getCuid()));
  }
  sequence_ = SEQ_SEND_SIZE;
  return true;  
}

bool FtpNegotiationCommand::sendSize() {
  if(ftp_->sendSize()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_SIZE;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::onFileSizeDetermined(int64_t totalLength)
{
  getFileEntry()->setLength(totalLength);
  if(getFileEntry()->getPath().empty()) {
    getFileEntry()->setPath
      (util::createSafePath
       (getOption()->get(PREF_DIR),
        util::percentDecode(getRequest()->getFile().begin(),
                            getRequest()->getFile().end())));
  }
  getRequestGroup()->preDownloadProcessing();
  if(getDownloadEngine()->getRequestGroupMan()->
     isSameFileBeingDownloaded(getRequestGroup())) {
    throw DOWNLOAD_FAILURE_EXCEPTION2
      (fmt(EX_DUPLICATE_FILE_DOWNLOAD,
           getRequestGroup()->getFirstFilePath().c_str()),
       error_code::DUPLICATE_DOWNLOAD);
  }
  if(totalLength == 0) {

    if(getOption()->getAsBool(PREF_FTP_PASV)) {
      sequence_ = SEQ_PREPARE_PASV;
    } else {
      sequence_ = SEQ_PREPARE_PORT;
    }

    if(getOption()->getAsBool(PREF_DRY_RUN)) {
      getRequestGroup()->initPieceStorage();
      onDryRunFileFound();
      return false;
    }

    if(getDownloadContext()->knowsTotalLength() &&
       getRequestGroup()->downloadFinishedByFileLength()) {
#ifdef ENABLE_MESSAGE_DIGEST
      // TODO Known issue: if .aria2 file exists, it will not be
      // deleted on successful verification, because .aria2 file is
      // not loaded.  See also
      // HttpResponseCommand::handleOtherEncoding()
      getRequestGroup()->initPieceStorage();
      if(getDownloadContext()->isChecksumVerificationNeeded()) {
        A2_LOG_DEBUG("Zero length file exists. Verify checksum.");
        SharedHandle<ChecksumCheckIntegrityEntry> entry
          (new ChecksumCheckIntegrityEntry(getRequestGroup()));
        entry->initValidator();
        getPieceStorage()->getDiskAdaptor()->openExistingFile();
        getDownloadEngine()->getCheckIntegrityMan()->pushEntry(entry);
        sequence_ = SEQ_EXIT;
      } else
#endif // ENABLE_MESSAGE_DIGEST
        {
          getPieceStorage()->markAllPiecesDone();
          getDownloadContext()->setChecksumVerified(true);
          sequence_ = SEQ_DOWNLOAD_ALREADY_COMPLETED;
          A2_LOG_NOTICE
            (fmt(MSG_DOWNLOAD_ALREADY_COMPLETED,
                 getRequestGroup()->getGID(),
                 getRequestGroup()->getFirstFilePath().c_str()));
        }
      poolConnection();
      return false;
    }

    getRequestGroup()->shouldCancelDownloadForSafety();
    getRequestGroup()->initPieceStorage();
    getPieceStorage()->getDiskAdaptor()->initAndOpenFile();

    if(getDownloadContext()->knowsTotalLength()) {
      A2_LOG_DEBUG("File length becomes zero and it means download completed.");
#ifdef ENABLE_MESSAGE_DIGEST
      // TODO Known issue: if .aria2 file exists, it will not be
      // deleted on successful verification, because .aria2 file is
      // not loaded.  See also
      // HttpResponseCommand::handleOtherEncoding()
      if(getDownloadContext()->isChecksumVerificationNeeded()) {
        A2_LOG_DEBUG("Verify checksum for zero-length file");
        SharedHandle<ChecksumCheckIntegrityEntry> entry
          (new ChecksumCheckIntegrityEntry(getRequestGroup()));
        entry->initValidator();
        getDownloadEngine()->getCheckIntegrityMan()->pushEntry(entry);
        sequence_ = SEQ_EXIT;
      } else
#endif // ENABLE_MESSAGE_DIGEST
        {
          sequence_ = SEQ_DOWNLOAD_ALREADY_COMPLETED;
          getPieceStorage()->markAllPiecesDone();
        }
      poolConnection();
      return false;
    }
    // We have to make sure that command that has Request object must
    // have segment after PieceStorage is initialized. See
    // AbstractCommand::execute()
    getSegmentMan()->getSegmentWithIndex(getCuid(), 0);
    return true;
  } else {
    SharedHandle<BtProgressInfoFile> progressInfoFile
      (new DefaultBtProgressInfoFile
       (getDownloadContext(), SharedHandle<PieceStorage>(), getOption().get()));
    getRequestGroup()->adjustFilename(progressInfoFile);
    getRequestGroup()->initPieceStorage();

    if(getOption()->getAsBool(PREF_DRY_RUN)) {
      onDryRunFileFound();
      return false;
    }

    SharedHandle<CheckIntegrityEntry> checkIntegrityEntry =
      getRequestGroup()->createCheckIntegrityEntry();
    if(!checkIntegrityEntry) {
      sequence_ = SEQ_DOWNLOAD_ALREADY_COMPLETED;
      poolConnection();
      return false;
    }
    checkIntegrityEntry->pushNextCommand(this);
    // We have to make sure that command that has Request object must
    // have segment after PieceStorage is initialized. See
    // AbstractCommand::execute()
    getSegmentMan()->getSegmentWithIndex(getCuid(), 0);

    prepareForNextAction(checkIntegrityEntry);

    disableReadCheckSocket();
  }
  return false;
}

bool FtpNegotiationCommand::recvSize() {
  int64_t size = 0;
  int status = ftp_->receiveSizeResponse(size);
  if(status == 0) {
    return false;
  }
  if(status == 213) {
    if(size > std::numeric_limits<off_t>::max()) {
      throw DL_ABORT_EX2(fmt(EX_TOO_LARGE_FILE, size),
                         error_code::FTP_PROTOCOL_ERROR);
    }
    if(!getPieceStorage()) {

      sequence_ = SEQ_FILE_PREPARATION;
      return onFileSizeDetermined(size);

    } else {
      getRequestGroup()->validateTotalLength(getFileEntry()->getLength(), size);
    }

  } else {
    A2_LOG_INFO(fmt("CUID#%" PRId64 " - The remote FTP Server doesn't recognize SIZE"
                    " command. Continue.", getCuid()));
    // Even if one of the other servers waiting in the queue supports SIZE
    // command, resuming and segmented downloading are disabled when the first
    // contacted FTP server doesn't support it.
    if(!getPieceStorage()) {
      getDownloadContext()->markTotalLengthIsUnknown();
      return onFileSizeDetermined(0);

    }
    // TODO Skipping RequestGroup::validateTotalLength(0) here will allow
    // wrong file to be downloaded if user-specified URL is wrong.
  }
  if(getOption()->getAsBool(PREF_FTP_PASV)) {
    sequence_ = SEQ_PREPARE_PASV;
  } else {
    sequence_ = SEQ_PREPARE_PORT;
  }
  return true;
}

void FtpNegotiationCommand::afterFileAllocation()
{
  setReadCheckSocket(getSocket());
}

bool FtpNegotiationCommand::preparePort() {
  afterFileAllocation();
  if(getSocket()->getAddressFamily() == AF_INET6) {
    sequence_ = SEQ_PREPARE_SERVER_SOCKET_EPRT;
  } else {
    sequence_ = SEQ_PREPARE_SERVER_SOCKET;
  }
  return true;
}

bool FtpNegotiationCommand::prepareServerSocketEprt() {
  serverSocket_ = ftp_->createServerSocket();
  sequence_ = SEQ_SEND_EPRT;
  return true;
}

bool FtpNegotiationCommand::prepareServerSocket()
{
  serverSocket_ = ftp_->createServerSocket();
  sequence_ = SEQ_SEND_PORT;
  return true;
}

bool FtpNegotiationCommand::sendEprt() {
  if(ftp_->sendEprt(serverSocket_)) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_EPRT;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvEprt() {
  int status = ftp_->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status == 200) {
    sequence_ = SEQ_SEND_REST;
  } else {
    sequence_ = SEQ_PREPARE_SERVER_SOCKET;
  }
  return true;
}

bool FtpNegotiationCommand::sendPort() {
  if(ftp_->sendPort(serverSocket_)) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_PORT;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvPort() {
  int status = ftp_->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 200) {
    throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, status),
                       error_code::FTP_PROTOCOL_ERROR);
  }
  sequence_ = SEQ_SEND_REST;
  return true;
}

bool FtpNegotiationCommand::preparePasv() {
  afterFileAllocation();
  if(getSocket()->getAddressFamily() == AF_INET6) {
    sequence_ = SEQ_SEND_EPSV;
  } else {
    sequence_ = SEQ_SEND_PASV;
  }
  return true;
}

bool FtpNegotiationCommand::sendEpsv() {
  if(ftp_->sendEpsv()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_EPSV;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return true;
}

bool FtpNegotiationCommand::recvEpsv() {
  uint16_t port;
  int status = ftp_->receiveEpsvResponse(port);
  if(status == 0) {
    return false;
  }
  if(status == 229) {
    pasvPort_ = port;
    return preparePasvConnect();
  } else {
    sequence_ = SEQ_SEND_PASV;
    return true;
  }
}

bool FtpNegotiationCommand::sendPasv() {
  if(ftp_->sendPasv()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_PASV;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvPasv() {
  std::pair<std::string, uint16_t> dest;
  int status = ftp_->receivePasvResponse(dest);
  if(status == 0) {
    return false;
  }
  if(status != 227) {
    throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, status),
                       error_code::FTP_PROTOCOL_ERROR);
  }
  pasvPort_ = dest.second;;
  return preparePasvConnect();
}

bool FtpNegotiationCommand::preparePasvConnect() {
  if(isProxyDefined()) {
    sequence_ = SEQ_RESOLVE_PROXY;
    return true;
  } else {
    std::pair<std::string, uint16_t> dataAddr;
    getSocket()->getPeerInfo(dataAddr);
    // make a data connection to the server.
    A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER,
                    getCuid(),
                    dataAddr.first.c_str(),
                    pasvPort_));
    dataSocket_.reset(new SocketCore());
    dataSocket_->establishConnection(dataAddr.first, pasvPort_);
    disableReadCheckSocket();
    setWriteCheckSocket(dataSocket_);
    sequence_ = SEQ_SEND_REST_PASV;
    return false;
  }
}

bool FtpNegotiationCommand::resolveProxy()
{
  SharedHandle<Request> proxyReq = createProxyRequest();
  std::vector<std::string> addrs;
  proxyAddr_ = resolveHostname
    (addrs, proxyReq->getHost(), proxyReq->getPort());
  if(proxyAddr_.empty()) {
    return false;
  }
  A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER,
                  getCuid(),
                  proxyAddr_.c_str(), proxyReq->getPort()));
  dataSocket_.reset(new SocketCore());                  
  dataSocket_->establishConnection(proxyAddr_, proxyReq->getPort());
  disableReadCheckSocket();
  setWriteCheckSocket(dataSocket_);
  SharedHandle<SocketRecvBuffer> socketRecvBuffer
    (new SocketRecvBuffer(dataSocket_));
  http_.reset(new HttpConnection(getCuid(), dataSocket_, socketRecvBuffer));
  sequence_ = SEQ_SEND_TUNNEL_REQUEST;
  return false;
}

bool FtpNegotiationCommand::sendTunnelRequest()
{
  if(http_->sendBufferIsEmpty()) {
    if(dataSocket_->isReadable(0)) {
      std::string error = getSocket()->getSocketError();
      if(!error.empty()) {
        SharedHandle<Request> proxyReq = createProxyRequest();
        getDownloadEngine()->markBadIPAddress(proxyReq->getHost(),
                                              proxyAddr_,proxyReq->getPort());
        std::string nextProxyAddr = getDownloadEngine()->findCachedIPAddress
          (proxyReq->getHost(), proxyReq->getPort());
        if(nextProxyAddr.empty()) {
          getDownloadEngine()->removeCachedIPAddress(proxyReq->getHost(),
                                                     proxyReq->getPort());
          throw DL_RETRY_EX
            (fmt(MSG_ESTABLISHING_CONNECTION_FAILED,
                 error.c_str()));
        } else {
          A2_LOG_INFO(fmt(MSG_CONNECT_FAILED_AND_RETRY,
                          getCuid(),
                          proxyAddr_.c_str(), proxyReq->getPort()));
          proxyAddr_ = nextProxyAddr;
          A2_LOG_INFO(fmt(MSG_CONNECTING_TO_SERVER,
                          getCuid(),
                          proxyAddr_.c_str(), proxyReq->getPort()));
          dataSocket_->establishConnection(proxyAddr_, proxyReq->getPort());
          return false;
        }
      }
    }      
    SharedHandle<HttpRequest> httpRequest(new HttpRequest());
    httpRequest->setUserAgent(getOption()->get(PREF_USER_AGENT));
    SharedHandle<Request> req(new Request());
    // Construct fake URI in order to use HttpRequest
    std::pair<std::string, uint16_t> dataAddr;
    uri::UriStruct us;
    us.protocol = "ftp";
    us.host = getRequest()->getHost();
    us.port = pasvPort_;
    us.ipv6LiteralAddress = getRequest()->isIPv6LiteralAddress();
    if(!req->setUri(uri::construct(us))) {
      throw DL_RETRY_EX("Something wrong with FTP URI");
    }
    httpRequest->setRequest(req);
    httpRequest->setProxyRequest(createProxyRequest());
    http_->sendProxyRequest(httpRequest);
  } else {
    http_->sendPendingData();
  }
  if(http_->sendBufferIsEmpty()) {
    disableWriteCheckSocket();
    setReadCheckSocket(dataSocket_);
    sequence_ = SEQ_RECV_TUNNEL_RESPONSE;
    return false;
  } else {
    setWriteCheckSocket(dataSocket_);
    return false;
  }
}

bool FtpNegotiationCommand::recvTunnelResponse()
{
  SharedHandle<HttpResponse> httpResponse = http_->receiveResponse();
  if(!httpResponse) {
    return false;
  }
  if(httpResponse->getStatusCode() != 200) {
    throw DL_RETRY_EX(EX_PROXY_CONNECTION_FAILED);
  }
  sequence_ = SEQ_SEND_REST_PASV;
  return true;
}

bool FtpNegotiationCommand::sendRestPasv(const SharedHandle<Segment>& segment) {
  //dataSocket_->setBlockingMode();
  // Check connection is made properly
  if(dataSocket_->isReadable(0)) {
    std::string error = dataSocket_->getSocketError();
    throw DL_ABORT_EX2
      (fmt(MSG_ESTABLISHING_CONNECTION_FAILED, error.c_str()),
       error_code::FTP_PROTOCOL_ERROR);
  }
  setReadCheckSocket(getSocket());
  disableWriteCheckSocket();
  return sendRest(segment);
}

bool FtpNegotiationCommand::sendRest(const SharedHandle<Segment>& segment) {
  if(ftp_->sendRest(segment)) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_REST;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvRest(const SharedHandle<Segment>& segment) {
  int status = ftp_->receiveResponse();
  if(status == 0) {
    return false;
  }
  // If we recieve negative response and requested file position is not 0,
  // then throw exception here.
  if(status != 350) {
    if(segment && segment->getPositionToWrite() != 0) {
      throw DL_ABORT_EX2("FTP server doesn't support resuming.",
                         error_code::CANNOT_RESUME);
    }
  }
  sequence_ = SEQ_SEND_RETR;
  return true;
}

bool FtpNegotiationCommand::sendRetr() {
  if(ftp_->sendRetr()) {
    disableWriteCheckSocket();
    sequence_ = SEQ_RECV_RETR;
  } else {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

bool FtpNegotiationCommand::recvRetr() {
  int status = ftp_->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 150 && status != 125) {
    getRequestGroup()->increaseAndValidateFileNotFoundCount();
    if (status == 550)
      throw DL_ABORT_EX2(MSG_RESOURCE_NOT_FOUND,
                         error_code::RESOURCE_NOT_FOUND);
    else
      throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, status),
                         error_code::FTP_PROTOCOL_ERROR);
  }
  if(getOption()->getAsBool(PREF_FTP_PASV)) {
    sequence_ = SEQ_NEGOTIATION_COMPLETED;
    return false;
  } else {
    disableReadCheckSocket();
    setReadCheckSocket(serverSocket_);
    sequence_ = SEQ_WAIT_CONNECTION;
    return false;
  }
}

bool FtpNegotiationCommand::waitConnection()
{
  disableReadCheckSocket();
  setReadCheckSocket(getSocket());
  dataSocket_.reset(serverSocket_->acceptConnection());
  dataSocket_->setNonBlockingMode();
  sequence_ = SEQ_NEGOTIATION_COMPLETED;
  return false;
}

bool FtpNegotiationCommand::processSequence
(const SharedHandle<Segment>& segment) {
  bool doNextSequence = true;
  switch(sequence_) {
  case SEQ_RECV_GREETING:
    return recvGreeting();
  case SEQ_SEND_USER:
    return sendUser();
  case SEQ_RECV_USER:
    return recvUser();
  case SEQ_SEND_PASS:
    return sendPass();
  case SEQ_RECV_PASS:
    return recvPass();
  case SEQ_SEND_TYPE:
    return sendType();
  case SEQ_RECV_TYPE:
    return recvType();
  case SEQ_SEND_PWD:
    return sendPwd();
  case SEQ_RECV_PWD:
    return recvPwd();
  case SEQ_SEND_CWD_PREP:
    return sendCwdPrep();
  case SEQ_SEND_CWD:
    return sendCwd();
  case SEQ_RECV_CWD:
    return recvCwd();
  case SEQ_SEND_MDTM:
    return sendMdtm();
  case SEQ_RECV_MDTM:
    return recvMdtm();
  case SEQ_SEND_SIZE:
    return sendSize();
  case SEQ_RECV_SIZE:
    return recvSize();
  case SEQ_PREPARE_PORT:
    return preparePort();
  case SEQ_PREPARE_SERVER_SOCKET_EPRT:
    return prepareServerSocketEprt();
  case SEQ_SEND_EPRT:
    return sendEprt();
  case SEQ_RECV_EPRT:
    return recvEprt();
  case SEQ_PREPARE_SERVER_SOCKET:
    return prepareServerSocket();
  case SEQ_SEND_PORT:
    return sendPort();
  case SEQ_RECV_PORT:
    return recvPort();
  case SEQ_PREPARE_PASV:
    return preparePasv();
  case SEQ_SEND_EPSV:
    return sendEpsv();
  case SEQ_RECV_EPSV:
    return recvEpsv();
  case SEQ_SEND_PASV:
    return sendPasv();
  case SEQ_RECV_PASV:
    return recvPasv();
  case SEQ_RESOLVE_PROXY:
    return resolveProxy();
  case SEQ_SEND_TUNNEL_REQUEST:
    return sendTunnelRequest();
  case SEQ_RECV_TUNNEL_RESPONSE:
    return recvTunnelResponse();
  case SEQ_SEND_REST_PASV:
    return sendRestPasv(segment);
  case SEQ_SEND_REST:
    return sendRest(segment);
  case SEQ_RECV_REST:
    return recvRest(segment);
  case SEQ_SEND_RETR:
    return sendRetr();
  case SEQ_RECV_RETR:
    return recvRetr();
  case SEQ_WAIT_CONNECTION:
    return waitConnection();
  default:
    abort();
  }
  return doNextSequence;
}

void FtpNegotiationCommand::poolConnection() const
{
  if(getOption()->getAsBool(PREF_FTP_REUSE_CONNECTION)) {
    std::map<std::string, std::string> options;
    options["baseWorkingDir"] = ftp_->getBaseWorkingDir();
    getDownloadEngine()->poolSocket(getRequest(), ftp_->getUser(),
                                    createProxyRequest(), getSocket(), options);
  }
}

void FtpNegotiationCommand::onDryRunFileFound()
{
  getPieceStorage()->markAllPiecesDone();
  getDownloadContext()->setChecksumVerified(true);
  poolConnection();
  sequence_ = SEQ_HEAD_OK;
}

} // namespace aria2
