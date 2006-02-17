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
#include "HttpDownloadCommand.h"
#include "HttpInitiateConnectionCommand.h"
#include "message.h"
#include "Util.h"

HttpResponseCommand::HttpResponseCommand(int cuid, Request* req, DownloadEngine* e, Socket* s):
  AbstractCommand(cuid, req, e, s) {
  AbstractCommand::checkSocketIsReadable = true;
}

HttpResponseCommand::~HttpResponseCommand() {}

bool HttpResponseCommand::executeInternal(Segment seg) {
  if(SEGMENT_EQUAL(req->seg, seg) == false) {
    e->logger->info(MSG_SEGMENT_CHANGED, cuid);
    return prepareForRetry();
  }
  HttpHeader headers;
  HttpConnection httpConnection(cuid, socket, e->option, e->logger);
  int status = httpConnection.receiveResponse(headers);

  // check HTTP status number
  checkResponse(status, seg);
  retrieveCookie(headers);
  // check whether Location header exists. If it does, update request object
  // with redirected URL.
  // then establish a connection to the new host and port
  if(headers.count("Location")) {
    return handleRedirect((*headers.find("Location")).second, headers);
  }
  if(!e->segmentMan->downloadStarted) {
    string transferEncoding;
    headers.find("Transfer-Encoding");
    if(headers.count("Transfer-Encoding")) {
      return handleOtherEncoding((*headers.find("Transfer-Encoding")).second, headers);
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
      throw new DlAbortEx(EX_BAD_STATUS, status);
    }
}

bool HttpResponseCommand::handleRedirect(string url, const HttpHeader& headers) {
  req->redirectUrl(url);
  e->logger->info(MSG_REDIRECT, cuid, url.c_str());
  e->noWait = true;
  return prepareForRetry();
}

bool HttpResponseCommand::handleDefaultEncoding(const HttpHeader& headers) {
  long long int size;
  if(headers.count("Content-Length") == 0) {
    size = 0;
  } else {
    size = STRTOLL((*headers.find("Content-Length")).second.c_str());
    if(size == LONG_LONG_MAX || size == LONG_LONG_MIN) {
      throw new DlAbortEx(EX_TOO_LARGE_FILE, size);
    }
  }
  e->segmentMan->isSplittable = !(size == 0);
  e->segmentMan->filename = req->getFile();
  e->segmentMan->totalSize = size;
  bool segFileExists = e->segmentMan->segmentFileExists();
  e->segmentMan->downloadStarted = true;
  if(segFileExists) {
    e->segmentMan->load();
    e->diskWriter->openExistingFile(e->segmentMan->getFilePath());
    // send request again to the server with Range header
    return prepareForRetry();
  } else {
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
}

void HttpResponseCommand::retrieveCookie(const HttpHeader& headers) {
  for(HttpHeader::const_iterator itr = headers.find("Set-Cookie"); itr != headers.end(); itr++) {
    Cookie c;
    req->cookieBox->parse(c, (*itr).second);
    req->cookieBox->add(c);
  }
}
