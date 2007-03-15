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
#include "FtpDownloadCommand.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "message.h"
#include "prefs.h"
#include "Util.h"
#include "FatalException.h"

FtpNegotiationCommand::FtpNegotiationCommand(int cuid,
					     const RequestHandle& req,
					     DownloadEngine* e,
					     const SocketHandle& s):
  AbstractCommand(cuid, req, e, s), sequence(SEQ_RECV_GREETING)
{
  ftp = new FtpConnection(cuid, socket, req, e->option);
  disableReadCheckSocket();
  setWriteCheckSocket(socket);
}

FtpNegotiationCommand::~FtpNegotiationCommand() {
  delete ftp;
}

bool FtpNegotiationCommand::executeInternal() {
  while(processSequence(segment));
  if(sequence == SEQ_RETRY) {
    return prepareForRetry(0);
  } else if(sequence == SEQ_NEGOTIATION_COMPLETED) {
    FtpDownloadCommand* command =
      new FtpDownloadCommand(cuid, req, e, dataSocket, socket);
    command->setMaxDownloadSpeedLimit(e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
    command->setStartupIdleTime(e->option->getAsInt(PREF_STARTUP_IDLE_TIME));
    command->setLowestDownloadSpeedLimit(e->option->getAsInt(PREF_LOWEST_SPEED_LIMIT));
    e->commands.push_back(command);
    return true;
  } else if(sequence == SEQ_HEAD_OK) {
    return true;
  } else {
    e->commands.push_back(this);
    return false;
  }
}

bool FtpNegotiationCommand::recvGreeting() {
  socket->setBlockingMode();
  disableWriteCheckSocket();
  setReadCheckSocket(socket);

  int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 220) {
    throw new DlRetryEx(EX_CONNECTION_FAILED);
  }
  sequence = SEQ_SEND_USER;

  return true;
}

bool FtpNegotiationCommand::sendUser() {
  ftp->sendUser();
  sequence = SEQ_RECV_USER;
  return false;
}

bool FtpNegotiationCommand::recvUser() {
  int status = ftp->receiveResponse();
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
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  return true;
}

bool FtpNegotiationCommand::sendPass() {
  ftp->sendPass();
  sequence = SEQ_RECV_PASS;
  return false;
}

bool FtpNegotiationCommand::recvPass() {
  int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 230) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  sequence = SEQ_SEND_TYPE;
  return true;
}

bool FtpNegotiationCommand::sendType() {
  ftp->sendType();
  sequence = SEQ_RECV_TYPE;
  return false;
}

bool FtpNegotiationCommand::recvType() {
  int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 200) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  sequence = SEQ_SEND_CWD;
  return true;
}

bool FtpNegotiationCommand::sendCwd() {
  ftp->sendCwd();
  sequence = SEQ_RECV_CWD;
  return false;
}

bool FtpNegotiationCommand::recvCwd() {
  int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 250) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  sequence = SEQ_SEND_SIZE;
  return true;
}

bool FtpNegotiationCommand::sendSize() {
  ftp->sendSize();
  sequence = SEQ_RECV_SIZE;
  return false;
}

bool FtpNegotiationCommand::recvSize() {
  long long int size = 0;
  int status = ftp->receiveSizeResponse(size);
  if(status == 0) {
    return false;
  }
  if(status != 213) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  if(size == INT64_MAX || size < 0) {
    throw new DlAbortEx(EX_TOO_LARGE_FILE, size);
  }
  if(!e->segmentMan->downloadStarted) {
    e->segmentMan->downloadStarted = true;
    e->segmentMan->totalSize = size;
    e->segmentMan->initBitfield(e->option->getAsInt(PREF_SEGMENT_SIZE),
				e->segmentMan->totalSize);
    e->segmentMan->filename = Util::urldecode(req->getFile());
    if(req->getMethod() == Request::METHOD_HEAD) {
      e->segmentMan->isSplittable = false; // TODO because we don't want segment file to be saved.
      sequence = SEQ_HEAD_OK;
      return false;
    }
    bool segFileExists = e->segmentMan->segmentFileExists();
    if(segFileExists) {
      e->segmentMan->load();
      e->segmentMan->diskWriter->openExistingFile(e->segmentMan->getFilePath());
    } else {
      e->segmentMan->diskWriter->initAndOpenFile(e->segmentMan->getFilePath(), size);
    }
  } else if(e->segmentMan->totalSize != size) {
    throw new DlAbortEx(EX_SIZE_MISMATCH, e->segmentMan->totalSize, size);
  }
  if(e->option->get(PREF_FTP_PASV_ENABLED) == V_TRUE) {
    sequence = SEQ_SEND_PASV;
  } else {
    sequence = SEQ_SEND_PORT;
  }
  return true;
}

bool FtpNegotiationCommand::sendPort() {
  serverSocket = ftp->sendPort();
  sequence = SEQ_RECV_PORT;
  return false;
}

bool FtpNegotiationCommand::recvPort() {
  int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 200) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  sequence = SEQ_SEND_REST;
  return true;
}

bool FtpNegotiationCommand::sendPasv() {
  ftp->sendPasv();
  sequence = SEQ_RECV_PASV;
  return false;
}

bool FtpNegotiationCommand::recvPasv() {
  pair<string, int> dest;
  int status = ftp->receivePasvResponse(dest);
  if(status == 0) {
    return false;
  }
  if(status != 227) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  // make a data connection to the server.
  logger->info(MSG_CONNECTING_TO_SERVER, cuid,
	       dest.first.c_str(),
	       dest.second);
  dataSocket->establishConnection(dest.first, dest.second);

  disableReadCheckSocket();
  setWriteCheckSocket(dataSocket);

  sequence = SEQ_SEND_REST_PASV;
  return false;
}

bool FtpNegotiationCommand::sendRestPasv(const SegmentHandle& segment) {
  dataSocket->setBlockingMode();
  setReadCheckSocket(socket);
  disableWriteCheckSocket();
  return sendRest(segment);
}

bool FtpNegotiationCommand::sendRest(const SegmentHandle& segment) {
  ftp->sendRest(segment);
  sequence = SEQ_RECV_REST;
  return false;
}

bool FtpNegotiationCommand::recvRest() {
  int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  // TODO if we recieve negative response, then we set e->segmentMan->splittable = false, and continue.
  if(status != 350) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  sequence = SEQ_SEND_RETR;
  return true;
}

bool FtpNegotiationCommand::sendRetr() {
  ftp->sendRetr();
  sequence = SEQ_RECV_RETR;
  return false;
}

bool FtpNegotiationCommand::recvRetr() {
  int status = ftp->receiveResponse();
  if(status == 0) {
    return false;
  }
  if(status != 150 && status != 125) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  if(e->option->get(PREF_FTP_PASV_ENABLED) != V_TRUE) {
    assert(serverSocket->getSockfd() != -1);
    dataSocket = serverSocket->acceptConnection();
  }
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
  case SEQ_SEND_CWD:
    return sendCwd();
  case SEQ_RECV_CWD:
    return recvCwd();
  case SEQ_SEND_SIZE:
    return sendSize();
  case SEQ_RECV_SIZE:
    return recvSize();
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
    return recvRest();
  case SEQ_SEND_RETR:
    return sendRetr();
  case SEQ_RECV_RETR:
    return recvRetr();
  default:
    abort();
  }
  return doNextSequence;
}
