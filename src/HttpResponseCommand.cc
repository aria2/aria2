/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "HttpResponseCommand.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "HttpDownloadCommand.h"
#include "HttpInitiateConnectionCommand.h"
#include "message.h"
#include "Util.h"
#include "TrackerDownloadCommand.h"
// TODO
#include "TorrentDownloadEngine.h"

HttpResponseCommand::HttpResponseCommand(int cuid, Request* req, DownloadEngine* e, const Socket* s):
  AbstractCommand(cuid, req, e, s) {
  http = new HttpConnection(cuid, socket, req, e->option);
}

HttpResponseCommand::~HttpResponseCommand() {
  delete http;
}

bool HttpResponseCommand::executeInternal(Segment seg) {
  if(req->seg.sp != seg.sp) {
    logger->info(MSG_SEGMENT_CHANGED, cuid);
    return prepareForRetry(0);
  }
  HttpHeader headers;
  int status = http->receiveResponse(headers);
  if(status == 0) {
    // didn't receive header fully
    e->commands.push(this);
    return false;
  }
  // check HTTP status number
  checkResponse(status, seg);
  retrieveCookie(headers);
  // check whether Location header exists. If it does, update request object
  // with redirected URL.
  // then establish a connection to the new host and port
  if(headers.defined("Location")) {
    return handleRedirect(headers.getFirst("Location"), headers);
  }
  if(!e->segmentMan->downloadStarted) {
    string transferEncoding;
    if(headers.defined("Transfer-Encoding")) {
      return handleOtherEncoding(headers.getFirst("Transfer-Encoding"), headers);
    } else {
      return handleDefaultEncoding(headers);
    }
  } else {
    if(req->getFile() != e->segmentMan->filename) {
      throw new DlAbortEx(EX_FILENAME_MISMATCH, req->getFile().c_str(), e->segmentMan->filename.c_str());
    }
    createHttpDownloadCommand();
    return true;
  }
}

void HttpResponseCommand::checkResponse(int status, const Segment& segment) {
    if(!(status < 400 && status >= 300 ||
	 (segment.sp+segment.ds == 0 && status == 200)
	 || (segment.sp+segment.ds > 0 &&  status == 206))) {
      throw new DlRetryEx(EX_BAD_STATUS, status);
    }
}

bool HttpResponseCommand::handleRedirect(string url, const HttpHeader& headers) {
  req->redirectUrl(url);
  logger->info(MSG_REDIRECT, cuid, url.c_str());
  e->noWait = true;
  return prepareForRetry(0);
}

bool HttpResponseCommand::handleDefaultEncoding(const HttpHeader& headers) {
  // TODO quick and dirty way 
  if(req->isTorrent) {
    long long int size = headers.getFirstAsLLInt("Content-Length");
    e->segmentMan->totalSize = size;
    e->segmentMan->isSplittable = false;
    createHttpDownloadCommand();
    return true;
  }

  long long int size = headers.getFirstAsLLInt("Content-Length");
  if(size == LONG_LONG_MAX || size < 0) {
    throw new DlAbortEx(EX_TOO_LARGE_FILE, size);
  }
  e->segmentMan->isSplittable = !(size == 0);
  e->segmentMan->filename = req->getFile();
  bool segFileExists = e->segmentMan->segmentFileExists();
  e->segmentMan->downloadStarted = true;
  if(segFileExists) {
    e->segmentMan->load();
    e->diskWriter->openExistingFile(e->segmentMan->getFilePath());
    // send request again to the server with Range header
    return prepareForRetry(0);
  } else {
    e->segmentMan->totalSize = size;
    Segment seg;
    e->segmentMan->getSegment(seg, cuid);	
    e->diskWriter->initAndOpenFile(e->segmentMan->getFilePath());
    createHttpDownloadCommand();
    return true;
  }
}

bool HttpResponseCommand::handleOtherEncoding(string transferEncoding, const HttpHeader& headers) {
  // we ignore content-length when transfer-encoding is set
  e->segmentMan->downloadStarted = true;
  e->segmentMan->isSplittable = false;
  e->segmentMan->filename = req->getFile();
  e->segmentMan->totalSize = 0;
  Segment seg;
  e->segmentMan->getSegment(seg, cuid);	
  e->diskWriter->initAndOpenFile(e->segmentMan->getFilePath());
  createHttpDownloadCommand(transferEncoding);
  return true;
}

void HttpResponseCommand::createHttpDownloadCommand(string transferEncoding) {
  if(!req->isTorrent) {
    HttpDownloadCommand* command = new HttpDownloadCommand(cuid, req, e, socket);
    TransferEncoding* enc = NULL;
    if(transferEncoding.size() && (enc = command->getTransferEncoding(transferEncoding)) == NULL) {
      delete(command);
      throw new DlAbortEx(EX_TRANSFER_ENCODING_NOT_SUPPORTED, transferEncoding.c_str());
    } else {
      if(enc != NULL) {
	command->transferEncoding = transferEncoding;
	enc->init();
      }
      e->commands.push(command);
    }
  } else {
    // TODO
    TrackerDownloadCommand* command = new TrackerDownloadCommand(cuid, req, (TorrentDownloadEngine*)e, socket);
    e->commands.push(command);
  }
}

void HttpResponseCommand::retrieveCookie(const HttpHeader& headers) {
  Strings v = headers.get("Set-Cookie");
  for(Strings::const_iterator itr = v.begin(); itr != v.end(); itr++) {
    Cookie c;
    req->cookieBox->parse(c, *itr);
    req->cookieBox->add(c);
  }
}
  
