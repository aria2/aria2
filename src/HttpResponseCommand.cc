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
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "HttpDownloadCommand.h"
#include "message.h"
#include "Util.h"
#include "prefs.h"
#include "File.h"
#include "InitiateConnectionCommandFactory.h"
#include "FatalException.h"
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
  if(!(httpRequest->getSegment() == segment)) {
    logger->info(MSG_SEGMENT_CHANGED, cuid);
    return prepareForRetry(0);
  }
  HttpResponseHandle httpResponse = httpConnection->receiveResponse();
  if(httpResponse.isNull()) {
    // The server has not responded to our request yet.
    e->commands.push_back(this);
    return false;
  }
  // check HTTP status number
  httpResponse->validateResponse();
  httpResponse->retrieveCookie();
  // check whether the server supports persistent connections.
  /*
  if(Util::toLower(headers.getFirst("Connection")).find("close") != string::npos) {
    req->setKeepAlive(false);
  }
  */
  // check whether Location header exists. If it does, update request object
  // with redirected URL.
  // then establish a connection to the new host and port
  if(httpResponse->isRedirect()) {
    httpResponse->processRedirect();
    logger->info(MSG_REDIRECT, cuid, httpResponse->getRedirectURI().c_str());
    e->noWait = true;
    return prepareForRetry(0);
  }
  if(_requestGroup->getSegmentMan()->downloadStarted) {
    // validate totalsize
    _requestGroup->validateFilename(httpResponse->determinFilename());
    _requestGroup->validateTotalLength(httpResponse->getEntityLength());

    e->commands.push_back(createHttpDownloadCommand(httpResponse));
    return true;
  } else {
    // validate totalsize against hintTotalSize if it is provided.
    _requestGroup->validateFilenameByHint(httpResponse->determinFilename());
    _requestGroup->validateTotalLengthByHint(httpResponse->getEntityLength());

    _requestGroup->getSegmentMan()->filename = httpResponse->determinFilename();
    if(e->_requestGroupMan->isSameFileBeingDownloaded(_requestGroup)) {
      throw new FatalException(EX_DUPLICATE_FILE_DOWNLOAD, _requestGroup->getFilePath().c_str());
    }

    if(httpResponse->isTransferEncodingSpecified()) {
      return handleOtherEncoding(httpResponse);
    } else {
      return handleDefaultEncoding(httpResponse);
    }
  }
}

bool HttpResponseCommand::handleDefaultEncoding(const HttpResponseHandle& httpResponse)
{
  HttpRequestHandle httpRequest = httpResponse->getHttpRequest();
  // TODO quick and dirty way 
  if(_requestGroup->isTorrent) {
    return doTorrentStuff(httpResponse);
  }
  int64_t size = httpResponse->getEntityLength();
  if(size == INT64_MAX || size < 0) {
    throw new DlAbortEx(EX_TOO_LARGE_FILE, Util::llitos(size, true).c_str());
  }
  _requestGroup->getSegmentMan()->isSplittable = !(size == 0);
  _requestGroup->getSegmentMan()->downloadStarted = true;
  _requestGroup->getSegmentMan()->totalSize = size;
  
  // quick hack for method 'head'
  if(httpRequest->getMethod() == Request::METHOD_HEAD) {
    // TODO because we don't want segment file to be saved.
    _requestGroup->getSegmentMan()->isSplittable = false;
    return true;
  }

  if(e->option->get(PREF_CHECK_INTEGRITY) != V_TRUE) {
    if(_requestGroup->downloadFinishedByFileLength()) {
      logger->notice(MSG_DOWNLOAD_ALREADY_COMPLETED, cuid, _requestGroup->getFilePath().c_str());
      return true;
    }
  }

  DownloadCommand* command = 0;
  try {
    File file(_requestGroup->getFilePath());
    if(_requestGroup->getRemainingUris().empty() && !file.exists()) {
      command = createHttpDownloadCommand(httpResponse);
    }
    _requestGroup->loadAndOpenFile();
    _requestGroup->prepareForNextAction(cuid, req, e, command);
    e->noWait = true;
  } catch(Exception* e) {
    delete command;
    throw;
  }
  return true;
}

bool HttpResponseCommand::handleOtherEncoding(const HttpResponseHandle& httpResponse) {
  HttpRequestHandle httpRequest = httpResponse->getHttpRequest();
  // we ignore content-length when transfer-encoding is set
  _requestGroup->getSegmentMan()->downloadStarted = true;
  _requestGroup->getSegmentMan()->isSplittable = false;
  _requestGroup->getSegmentMan()->totalSize = 0;
  // quick hack for method 'head'
  if(httpRequest->getMethod() == Request::METHOD_HEAD) {
    return true;
  }
  // disable keep-alive
  req->setKeepAlive(false);
  segment = _requestGroup->getSegmentMan()->getSegment(cuid);	
  _requestGroup->shouldCancelDownloadForSafety();
  _requestGroup->getSegmentMan()->diskWriter->initAndOpenFile(_requestGroup->getSegmentMan()->getFilePath());
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
    new HttpDownloadCommand(cuid, req, _requestGroup, e, socket);
  command->setMaxDownloadSpeedLimit(e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
  command->setStartupIdleTime(e->option->getAsInt(PREF_STARTUP_IDLE_TIME));
  command->setLowestDownloadSpeedLimit(e->option->getAsInt(PREF_LOWEST_SPEED_LIMIT));
  command->setTransferDecoder(enc);

  return command;
}

bool HttpResponseCommand::doTorrentStuff(const HttpResponseHandle& httpResponse)
{
  int64_t size = httpResponse->getEntityLength();
  _requestGroup->getSegmentMan()->totalSize = size;
  if(size > 0) {
    _requestGroup->getSegmentMan()->initBitfield(e->option->getAsInt(PREF_SEGMENT_SIZE),
			      _requestGroup->getSegmentMan()->totalSize);
  }
  // disable keep-alive
  httpResponse->getHttpRequest()->getRequest()->setKeepAlive(false);
  _requestGroup->getSegmentMan()->isSplittable = false;
  _requestGroup->getSegmentMan()->downloadStarted = true;
  _requestGroup->getSegmentMan()->diskWriter->initAndOpenFile("/tmp/aria2"+Util::itos((int32_t)getpid()));
  e->commands.push_back(createHttpDownloadCommand(httpResponse));
  return true;
}
