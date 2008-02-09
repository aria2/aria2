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
#include "HttpResponseCommand.h"
#include "DownloadEngine.h"
#include "HttpResponse.h"
#include "HttpConnection.h"
#include "SegmentMan.h"
#include "DlAbortEx.h"
#include "HttpDownloadCommand.h"
#include "message.h"
#include "Util.h"
#include "prefs.h"
#include "File.h"
#include "InitiateConnectionCommandFactory.h"
#include "SingleFileDownloadContext.h"
#include "DiskAdaptor.h"
#include "PieceStorage.h"
#include "DefaultBtProgressInfoFile.h"
#include "RequestGroupMan.h"
#include "DownloadFailureException.h"
#include <sys/types.h>
#include <unistd.h>

HttpResponseCommand::HttpResponseCommand(int32_t cuid,
					 const RequestHandle& req,
					 RequestGroup* requestGroup,
					 const HttpConnectionHandle& httpConnection,
					 DownloadEngine* e,
					 const SocketHandle& s)
  :AbstractCommand(cuid, req, requestGroup, e, s),
   httpConnection(httpConnection) {}

HttpResponseCommand::~HttpResponseCommand() {}

bool HttpResponseCommand::executeInternal()
{
  HttpRequestHandle httpRequest = httpConnection->getFirstHttpRequest();
  HttpResponseHandle httpResponse = httpConnection->receiveResponse();
  if(httpResponse.isNull()) {
    // The server has not responded to our request yet.
    e->commands.push_back(this);
    return false;
  }
  // check HTTP status number
  httpResponse->validateResponse();
  httpResponse->retrieveCookie();
  // check whether Location header exists. If it does, update request object
  // with redirected URL.
  // then establish a connection to the new host and port
  if(httpResponse->isRedirect()) {
    httpResponse->processRedirect();
    logger->info(MSG_REDIRECT, cuid, httpResponse->getRedirectURI().c_str());
    e->noWait = true;
    return prepareForRetry(0);
  }
  if(_requestGroup->getPieceStorage().isNull()) {
    int64_t totalLength = httpResponse->getEntityLength();
    SingleFileDownloadContextHandle dctx = _requestGroup->getDownloadContext();
    dctx->setTotalLength(totalLength);
    dctx->setFilename(httpResponse->determinFilename());
    dctx->setContentType(httpResponse->getContentType());
    _requestGroup->preDownloadProcessing();
    if(e->_requestGroupMan->isSameFileBeingDownloaded(_requestGroup)) {
      throw new DownloadFailureException(EX_DUPLICATE_FILE_DOWNLOAD,
					 _requestGroup->getFilePath().c_str());
    }
    if(totalLength == 0 || httpResponse->isTransferEncodingSpecified()) {
      // we ignore content-length when transfer-encoding is set
      dctx->setTotalLength(0);
      return handleOtherEncoding(httpResponse);
    } else {
      return handleDefaultEncoding(httpResponse);
    }
  } else {
    // validate totalsize
    _requestGroup->validateTotalLength(httpResponse->getEntityLength());
    e->commands.push_back(createHttpDownloadCommand(httpResponse));
    return true;
  }
}

bool HttpResponseCommand::handleDefaultEncoding(const HttpResponseHandle& httpResponse)
{
  HttpRequestHandle httpRequest = httpResponse->getHttpRequest();
  _requestGroup->initPieceStorage();

  // quick hack for method 'head',, is it necessary?
  if(httpRequest->getMethod() == Request::METHOD_HEAD) {
    // TODO because we don't want segment file to be saved.
    return true;
  }

  BtProgressInfoFileHandle infoFile = new DefaultBtProgressInfoFile(_requestGroup->getDownloadContext(), _requestGroup->getPieceStorage(), e->option);
  if(!infoFile->exists() && _requestGroup->downloadFinishedByFileLength()) {
    return true;
  }

  DownloadCommand* command = 0;
  try {
    _requestGroup->loadAndOpenFile(infoFile);
    File file(_requestGroup->getFilePath());

    SegmentHandle segment = _requestGroup->getSegmentMan()->getSegment(cuid, 0);
    if(!segment.isNull() && segment->getPositionToWrite() == 0) {
      command = createHttpDownloadCommand(httpResponse);
    } else {
      _requestGroup->getSegmentMan()->cancelSegment(cuid);
    }
    prepareForNextAction(command);
    e->noWait = true;
  } catch(Exception* e) {
    delete command;
    throw;
  }
  return true;
}

bool HttpResponseCommand::handleOtherEncoding(const HttpResponseHandle& httpResponse) {
  HttpRequestHandle httpRequest = httpResponse->getHttpRequest();
  // quick hack for method 'head',, is it necessary?
  if(httpRequest->getMethod() == Request::METHOD_HEAD) {
    return true;
  }
  // disable keep-alive
  req->setKeepAlive(false);
  _requestGroup->initPieceStorage();
  _requestGroup->shouldCancelDownloadForSafety();
  _requestGroup->getPieceStorage()->getDiskAdaptor()->initAndOpenFile();
  e->commands.push_back(createHttpDownloadCommand(httpResponse));
  return true;
}

HttpDownloadCommand* HttpResponseCommand::createHttpDownloadCommand(const HttpResponseHandle& httpResponse)
{
  TransferEncodingHandle enc = 0;
  if(httpResponse->isTransferEncodingSpecified()) {
    enc = httpResponse->getTransferDecoder();
    if(enc.isNull()) {
      throw new DlAbortEx(EX_TRANSFER_ENCODING_NOT_SUPPORTED,
			  httpResponse->getTransferEncoding().c_str());
    }
    enc->init();
  }
  HttpDownloadCommand* command =
    new HttpDownloadCommand(cuid, req, _requestGroup, httpConnection, e, socket);
  command->setMaxDownloadSpeedLimit(e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
  command->setStartupIdleTime(e->option->getAsInt(PREF_STARTUP_IDLE_TIME));
  command->setLowestDownloadSpeedLimit(e->option->getAsInt(PREF_LOWEST_SPEED_LIMIT));
  command->setTransferDecoder(enc);

  return command;
}
