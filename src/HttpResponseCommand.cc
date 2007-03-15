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
#include <sys/types.h>
#include <unistd.h>

HttpResponseCommand::HttpResponseCommand(int32_t cuid,
					 const RequestHandle& req,
					 const HttpConnectionHandle& httpConnection,
					 DownloadEngine* e,
					 const SocketHandle& s)
  :AbstractCommand(cuid, req, e, s),
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
  httpResponse->validateFilename(e->segmentMan->filename);
  if(e->segmentMan->downloadStarted) {
    createHttpDownloadCommand(httpResponse);
    return true;
  } else {
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
  if(httpRequest->getRequest()->isTorrent) {
    return doTorrentStuff(httpResponse);
  }
  int64_t size = httpResponse->getEntityLength();
  if(size == INT64_MAX || size < 0) {
    throw new DlAbortEx(EX_TOO_LARGE_FILE, size);
  }
  e->segmentMan->isSplittable = !(size == 0);
  e->segmentMan->filename = httpResponse->determinFilename();
  e->segmentMan->downloadStarted = true;
    e->segmentMan->totalSize = size;
  
  // quick hack for method 'head'
  if(httpRequest->getMethod() == Request::METHOD_HEAD) {
    // TODO because we don't want segment file to be saved.
    e->segmentMan->isSplittable = false;
    return true;
  }
  bool segFileExists = e->segmentMan->segmentFileExists();
  if(segFileExists) {
    e->segmentMan->load();
    e->segmentMan->diskWriter->openExistingFile(e->segmentMan->getFilePath());
    // send request again to the server with Range header
    return prepareForRetry(0);
  } else {
    e->segmentMan->initBitfield(e->option->getAsInt(PREF_SEGMENT_SIZE),
				e->segmentMan->totalSize);
    e->segmentMan->diskWriter->initAndOpenFile(e->segmentMan->getFilePath(),
					       size);
    return prepareForRetry(0);
  }
}

bool HttpResponseCommand::handleOtherEncoding(const HttpResponseHandle& httpResponse) {
  HttpRequestHandle httpRequest = httpResponse->getHttpRequest();
  // we ignore content-length when transfer-encoding is set
  e->segmentMan->downloadStarted = true;
  e->segmentMan->isSplittable = false;
  e->segmentMan->filename = httpResponse->determinFilename();
  e->segmentMan->totalSize = 0;
  // quick hack for method 'head'
  if(httpRequest->getMethod() == Request::METHOD_HEAD) {
    return true;
  }
  // disable keep-alive
  req->setKeepAlive(false);
  segment = e->segmentMan->getSegment(cuid);	
  e->segmentMan->diskWriter->initAndOpenFile(e->segmentMan->getFilePath());
  createHttpDownloadCommand(httpResponse);
  return true;
}

void HttpResponseCommand::createHttpDownloadCommand(const HttpResponseHandle& httpResponse)
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
  HttpDownloadCommand* command = new HttpDownloadCommand(cuid, req, e, socket);
  command->setMaxDownloadSpeedLimit(e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
  command->setStartupIdleTime(e->option->getAsInt(PREF_STARTUP_IDLE_TIME));
  command->setLowestDownloadSpeedLimit(e->option->getAsInt(PREF_LOWEST_SPEED_LIMIT));
  command->setTransferDecoder(enc);

  e->commands.push_back(command);
}

bool HttpResponseCommand::doTorrentStuff(const HttpResponseHandle& httpResponse)
{
  int64_t size = httpResponse->getEntityLength();
  e->segmentMan->totalSize = size;
  if(size > 0) {
    e->segmentMan->initBitfield(e->option->getAsInt(PREF_SEGMENT_SIZE),
				e->segmentMan->totalSize);
  }
  // disable keep-alive
  httpResponse->getHttpRequest()->getRequest()->setKeepAlive(false);
  e->segmentMan->isSplittable = false;
  e->segmentMan->downloadStarted = true;
  e->segmentMan->diskWriter->initAndOpenFile("/tmp/aria2"+Util::itos((int32_t)getpid()));
  createHttpDownloadCommand(httpResponse);
  return true;
}
