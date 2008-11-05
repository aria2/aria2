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
#include "Util.h"
#include "Option.h"
#include "Logger.h"
#include "Segment.h"
#include "SingleFileDownloadContext.h"
#include "DefaultBtProgressInfoFile.h"
#include "RequestGroupMan.h"
#include "DownloadFailureException.h"
#include "ServerHost.h"
#include "Socket.h"
#include "StringFormat.h"
#include "DiskAdaptor.h"
#include "SegmentMan.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"

namespace aria2 {

FtpNegotiationCommand::FtpNegotiationCommand(int32_t cuid,
					     const RequestHandle& req,
					     RequestGroup* requestGroup,
					     DownloadEngine* e,
					     const SocketHandle& s,
					     Seq seq,
					     const std::string& baseWorkingDir):
  AbstractCommand(cuid, req, requestGroup, e, s), sequence(seq),
  ftp(new FtpConnection(cuid, socket, req,
			e->getAuthConfigFactory()->createAuthConfig(req),
			e->option))
{
  ftp->setBaseWorkingDir(baseWorkingDir);
  if(seq == SEQ_RECV_GREETING) {
    setTimeout(e->option->getAsInt(PREF_CONNECT_TIMEOUT));
  }
  disableReadCheckSocket();
  setWriteCheckSocket(socket);
}

FtpNegotiationCommand::~FtpNegotiationCommand() {}

bool FtpNegotiationCommand::executeInternal() {
  while(processSequence(_segments.front()));
  if(sequence == SEQ_RETRY) {
    return prepareForRetry(0);
  } else if(sequence == SEQ_NEGOTIATION_COMPLETED) {
    FtpDownloadCommand* command =
      new FtpDownloadCommand(cuid, req, _requestGroup, ftp, e, dataSocket, socket);
    command->setMaxDownloadSpeedLimit(e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
    command->setStartupIdleTime(e->option->getAsInt(PREF_STARTUP_IDLE_TIME));
    command->setLowestDownloadSpeedLimit(e->option->getAsInt(PREF_LOWEST_SPEED_LIMIT));
    if(!_requestGroup->isSingleHostMultiConnectionEnabled()) {
      SharedHandle<ServerHost> sv =
	_requestGroup->searchServerHost(req->getHost());
      if(!sv.isNull()) {
	_requestGroup->removeURIWhoseHostnameIs(sv->getHostname());
      }
    }
    e->commands.push_back(command);
    return true;
  } else if(sequence == SEQ_HEAD_OK || sequence == SEQ_DOWNLOAD_ALREADY_COMPLETED) {
    return true;
  } else if(sequence == SEQ_FILE_PREPARATION) {
    if(e->option->getAsBool(PREF_FTP_PASV)) {
      sequence = SEQ_SEND_PASV;
    } else {
      sequence = SEQ_PREPARE_SERVER_SOCKET;
    }
    return false;
  } else {
    e->commands.push_back(this);
    return false;
  }
}

bool FtpNegotiationCommand::recvGreeting() {
  checkIfConnectionEstablished(socket);
  setTimeout(e->option->getAsInt(PREF_TIMEOUT));
  //socket->setBlockingMode();
  disableWriteCheckSocket();
  setReadCheckSocket(socket);

  unsigned int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 220) {
    throw DlAbortEx(EX_CONNECTION_FAILED);
  }
  sequence = SEQ_SEND_USER;

  return true;
}

bool FtpNegotiationCommand::sendUser() {
  if(ftp->sendUser()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_USER;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvUser() {
  unsigned int status = ftp->receiveResponse();
  switch(status) {
  case 0:
    return false;
  case 230:
    sequence = SEQ_SEND_TYPE;
    break;
  case 331:
    sequence = SEQ_SEND_PASS;
    break;
  default:
    throw DlAbortEx(StringFormat(EX_BAD_STATUS, status).str());
  }
  return true;
}

bool FtpNegotiationCommand::sendPass() {
  if(ftp->sendPass()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_PASS;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvPass() {
  unsigned int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 230) {
    throw DlAbortEx(StringFormat(EX_BAD_STATUS, status).str());
  }
  sequence = SEQ_SEND_TYPE;
  return true;
}

bool FtpNegotiationCommand::sendType() {
  if(ftp->sendType()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_TYPE;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvType() {
  unsigned int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 200) {
    throw DlAbortEx(StringFormat(EX_BAD_STATUS, status).str());
  }
  sequence = SEQ_SEND_PWD;
  return true;
}

bool FtpNegotiationCommand::sendPwd()
{
  if(ftp->sendPwd()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_PWD;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvPwd()
{
  std::string pwd;
  unsigned int status = ftp->receivePwdResponse(pwd);
  if(status == 0) {
    return false;
  }
  if(status != 257) {
    throw DlAbortEx(StringFormat(EX_BAD_STATUS, status).str());
  }
  ftp->setBaseWorkingDir(pwd);
  logger->info("CUID#%d - base working directory is '%s'", cuid, pwd.c_str());
  sequence = SEQ_SEND_CWD;
  return true;
}

bool FtpNegotiationCommand::sendCwd() {
  // Calling setReadCheckSocket() is needed when the socket is reused, 
  setReadCheckSocket(socket);
  if(ftp->sendCwd()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_CWD;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvCwd() {
  unsigned int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 250) {
    poolConnection();
    _requestGroup->increaseAndValidateFileNotFoundCount();
    throw DlAbortEx(StringFormat(EX_BAD_STATUS, status).str());
  }
  if(e->option->getAsBool(PREF_REMOTE_TIME)) {
    sequence = SEQ_SEND_MDTM;
  } else {
    sequence = SEQ_SEND_SIZE;
  }
  return true;
}

bool FtpNegotiationCommand::sendMdtm()
{
  if(ftp->sendMdtm()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_MDTM;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvMdtm()
{
  Time lastModifiedTime = Time::null();
  unsigned int status = ftp->receiveMdtmResponse(lastModifiedTime);
  if(status == 0) {
    return false;
  }
  if(status == 213) {
    if(lastModifiedTime.good()) {
      _requestGroup->updateLastModifiedTime(lastModifiedTime);
      time_t t = lastModifiedTime.getTime();
      struct tm* tms = gmtime(&t); // returned struct is statically allocated.
      if(tms) {
	logger->debug("MDTM result was parsed as: %s GMT", asctime(tms));
      } else {
	logger->debug("gmtime() failed for MDTM result.");
      }
    } else {
      logger->debug("MDTM response was returned, but it seems not to be a time"
		    " value as in specified in RFC3659.");
    }
  } else {
    logger->info("CUID#%d - MDTM command failed.", cuid);
  }
  sequence = SEQ_SEND_SIZE;
  return true;  
}

bool FtpNegotiationCommand::sendSize() {
  if(ftp->sendSize()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_SIZE;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::onFileSizeDetermined(uint64_t totalLength)
{
  SingleFileDownloadContextHandle dctx =
    dynamic_pointer_cast<SingleFileDownloadContext>(_requestGroup->getDownloadContext());
  dctx->setTotalLength(totalLength);
  dctx->setFilename(Util::urldecode(req->getFile()));
  _requestGroup->preDownloadProcessing();
  if(e->_requestGroupMan->isSameFileBeingDownloaded(_requestGroup)) {
    throw DownloadFailureException
      (StringFormat(EX_DUPLICATE_FILE_DOWNLOAD,
		    _requestGroup->getFilePath().c_str()).str());
  }
  if(totalLength == 0) {

    _requestGroup->initPieceStorage();
    _requestGroup->shouldCancelDownloadForSafety();
    _requestGroup->getPieceStorage()->getDiskAdaptor()->initAndOpenFile();

    return true;
  } else {
    _requestGroup->initPieceStorage();

    // TODO Is this really necessary?
    if(req->getMethod() == Request::METHOD_HEAD) {
      sequence = SEQ_HEAD_OK;
      return false;
    }
    
    BtProgressInfoFileHandle infoFile(new DefaultBtProgressInfoFile(_requestGroup->getDownloadContext(), _requestGroup->getPieceStorage(), e->option));
    if(!infoFile->exists() && _requestGroup->downloadFinishedByFileLength()) {
      sequence = SEQ_DOWNLOAD_ALREADY_COMPLETED;
      
      poolConnection();
      
      return false;
    }
    _requestGroup->loadAndOpenFile(infoFile);

    prepareForNextAction(this);

    disableReadCheckSocket();
  }
  return false;
}

bool FtpNegotiationCommand::recvSize() {
  uint64_t size = 0;
  unsigned int status = ftp->receiveSizeResponse(size);
  if(status == 0) {
    return false;
  }
  if(status == 213) {

    if(size > INT64_MAX) {
      throw DlAbortEx
	(StringFormat(EX_TOO_LARGE_FILE, Util::uitos(size, true).c_str()).str());
    }
    if(_requestGroup->getPieceStorage().isNull()) {

      sequence = SEQ_FILE_PREPARATION;
      return onFileSizeDetermined(size);

    } else {
      _requestGroup->validateTotalLength(size);
    }

  } else {
    
    logger->info("CUID#%d - The remote FTP Server doesn't recognize SIZE command. Continue.", cuid);

    // Even if one of the other servers waiting in the queue supports SIZE
    // command, resuming and segmented downloading are disabled when the first
    // contacted FTP server doesn't support it.
    if(_requestGroup->getPieceStorage().isNull()) {

      if(e->option->getAsBool(PREF_FTP_PASV)) {
	sequence = SEQ_SEND_PASV;
      } else {
	sequence = SEQ_PREPARE_SERVER_SOCKET;
      }
      return onFileSizeDetermined(0);

    }
    // TODO Skipping RequestGroup::validateTotalLength(0) here will allow
    // wrong file to be downloaded if user-specified URL is wrong.
  }
  if(e->option->getAsBool(PREF_FTP_PASV)) {
    sequence = SEQ_SEND_PASV;
  } else {
    sequence = SEQ_PREPARE_SERVER_SOCKET;
  }
  return true;
}

void FtpNegotiationCommand::afterFileAllocation()
{
  setReadCheckSocket(socket);
}

bool FtpNegotiationCommand::prepareServerSocket()
{
  serverSocket = ftp->createServerSocket();
  sequence = SEQ_SEND_PORT;
  return true;
}

bool FtpNegotiationCommand::sendPort() {
  afterFileAllocation();
  if(ftp->sendPort(serverSocket)) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_PORT;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvPort() {
  unsigned int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 200) {
    throw DlAbortEx(StringFormat(EX_BAD_STATUS, status).str());
  }
  sequence = SEQ_SEND_REST;
  return true;
}

bool FtpNegotiationCommand::sendPasv() {
  afterFileAllocation();
  if(ftp->sendPasv()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_PASV;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvPasv() {
  std::pair<std::string, uint16_t> dest;
  unsigned int status = ftp->receivePasvResponse(dest);
  if(status == 0) {
    return false;
  }
  if(status != 227) {
    throw DlAbortEx(StringFormat(EX_BAD_STATUS, status).str());
  }
  // make a data connection to the server.
  logger->info(MSG_CONNECTING_TO_SERVER, cuid,
	       dest.first.c_str(),
	       dest.second);
  dataSocket.reset(new SocketCore());
  dataSocket->establishConnection(dest.first, dest.second);

  disableReadCheckSocket();
  setWriteCheckSocket(dataSocket);

  sequence = SEQ_SEND_REST_PASV;
  return false;
}

bool FtpNegotiationCommand::sendRestPasv(const SegmentHandle& segment) {
  //dataSocket->setBlockingMode();
  setReadCheckSocket(socket);
  disableWriteCheckSocket();
  return sendRest(segment);
}

bool FtpNegotiationCommand::sendRest(const SegmentHandle& segment) {
  if(ftp->sendRest(segment)) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_REST;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvRest(const SharedHandle<Segment>& segment) {
  unsigned int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  // If we recieve negative response and requested file position is not 0,
  // then throw exception here.
  if(status != 350) {
    if(!segment.isNull() && segment->getPositionToWrite() != 0) {
      throw DlAbortEx("FTP server doesn't support resuming.");
    }
  }
  sequence = SEQ_SEND_RETR;
  return true;
}

bool FtpNegotiationCommand::sendRetr() {
  if(ftp->sendRetr()) {
    disableWriteCheckSocket();
    sequence = SEQ_RECV_RETR;
  } else {
    setWriteCheckSocket(socket);
  }
  return false;
}

bool FtpNegotiationCommand::recvRetr() {
  unsigned int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 150 && status != 125) {
    _requestGroup->increaseAndValidateFileNotFoundCount();
    throw DlAbortEx(StringFormat(EX_BAD_STATUS, status).str());
  }
  if(e->option->getAsBool(PREF_FTP_PASV)) {
    sequence = SEQ_NEGOTIATION_COMPLETED;
    return false;
  } else {
    disableReadCheckSocket();
    setReadCheckSocket(serverSocket);
    sequence = SEQ_WAIT_CONNECTION;
    return false;
  }
}

bool FtpNegotiationCommand::waitConnection()
{
  disableReadCheckSocket();
  setReadCheckSocket(socket);
  dataSocket.reset(serverSocket->acceptConnection());
  //dataSocket->setBlockingMode();
  sequence = SEQ_NEGOTIATION_COMPLETED;
  return false;
}

bool FtpNegotiationCommand::processSequence(const SegmentHandle& segment) {
  bool doNextSequence = true;
  switch(sequence) {
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
  case SEQ_PREPARE_SERVER_SOCKET:
    return prepareServerSocket();
  case SEQ_SEND_PORT:
    return sendPort();
  case SEQ_RECV_PORT:
    return recvPort();
  case SEQ_SEND_PASV:
    return sendPasv();
  case SEQ_RECV_PASV:
    return recvPasv();
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
  if(!isProxyDefined() && e->option->getAsBool(PREF_FTP_REUSE_CONNECTION)) {
    std::pair<std::string, uint16_t> peerInfo;
    socket->getPeerInfo(peerInfo);
    std::map<std::string, std::string> options;
    options["baseWorkingDir"] = ftp->getBaseWorkingDir();
    e->poolSocket(peerInfo.first, peerInfo.second, socket, options);
  }
}

} // namespace aria2
