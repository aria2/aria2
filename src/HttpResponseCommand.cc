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
#include "HttpInitiateConnectionCommand.h"
#include "message.h"
#include "Util.h"
#include "prefs.h"
#include "File.h"
#include "FatalException.h"
#include <sys/types.h>
#include <unistd.h>

HttpResponseCommand::HttpResponseCommand(int cuid,
					 const RequestHandle req,
					 DownloadEngine* e,
					 const SocketHandle& s)
  :AbstractCommand(cuid, req, e, s) {
  http = new HttpConnection(cuid, socket, req, e->option);
}

HttpResponseCommand::~HttpResponseCommand() {
  delete http;
}

bool HttpResponseCommand::executeInternal(Segment& segment) {
  if(req->segment != segment) {
    logger->info(MSG_SEGMENT_CHANGED, cuid);
    return prepareForRetry(0);
  }
  HttpHeader headers;
  int status = http->receiveResponse(headers);
  if(status == 0) {
    // didn't receive header fully
    e->commands.push_back(this);
    return false;
  }
  // check HTTP status number
  checkResponse(status, segment);
  retrieveCookie(headers);
  // check whether the server supports persistent connections.
  if(Util::toLower(headers.getFirst("Connection")).find("close") != string::npos) {
    req->setKeepAlive(false);
  }
  // check whether Location header exists. If it does, update request object
  // with redirected URL.
  // then establish a connection to the new host and port
  if(headers.defined("Location")) {
    return handleRedirect(headers.getFirst("Location"), headers);
  }
  if(!e->segmentMan->downloadStarted) {
    string transferEncoding;
    if(headers.defined("Transfer-Encoding")) {
      return handleOtherEncoding(headers.getFirst("Transfer-Encoding"),
				 headers);
    } else {
      return handleDefaultEncoding(headers);
    }
  } else {
    if(determinFilename(headers) != e->segmentMan->filename) {
      throw new DlAbortEx(EX_FILENAME_MISMATCH, req->getFile().c_str(), e->segmentMan->filename.c_str());
    }
    createHttpDownloadCommand();
    return true;
  }
}

void HttpResponseCommand::checkResponse(int status, const Segment& segment) {
  if(status == 401) {
    throw new DlAbortEx(EX_AUTH_FAILED);
  }
  if(!(300 <= status && status < 400 ||
       (segment.getPosition()+segment.writtenLength == 0 && (status == 200 || status == 206)) ||
       (segment.getPosition()+segment.writtenLength > 0 &&  status == 206))) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
}

bool HttpResponseCommand::handleRedirect(const string& url, const HttpHeader& headers) {
  req->redirectUrl(url);
  logger->info(MSG_REDIRECT, cuid, url.c_str());
  e->noWait = true;
  return prepareForRetry(0);
}

string HttpResponseCommand::determinFilename(const HttpHeader& headers) {
  string contentDisposition =
    Util::getContentDispositionFilename(headers.getFirst("Content-Disposition"));
  if(contentDisposition.empty()) {
    return Util::urldecode(req->getFile());
  } else {
    logger->info("CUID#%d - Content-Disposition Detected. Use %s as filename",
		 cuid, contentDisposition.c_str());
    return Util::urldecode(contentDisposition);
  }
}

bool HttpResponseCommand::handleDefaultEncoding(const HttpHeader& headers) {
  // TODO quick and dirty way 
  if(req->isTorrent) {
    long long int size = headers.getFirstAsLLInt("Content-Length");
    e->segmentMan->totalSize = size;
    if(size > 0) {
      e->segmentMan->initBitfield(e->option->getAsInt(PREF_SEGMENT_SIZE),
				  e->segmentMan->totalSize);
    }
    // disable keep-alive
    req->setKeepAlive(false);
    e->segmentMan->isSplittable = false;
    e->segmentMan->downloadStarted = true;
    e->segmentMan->diskWriter->initAndOpenFile("/tmp/aria2"+Util::itos(getpid()));
    createHttpDownloadCommand();
    return true;
  }

  long long int size = headers.getFirstAsLLInt("Content-Length");
  if(size == LONG_LONG_MAX || size < 0) {
    throw new DlAbortEx(EX_TOO_LARGE_FILE, size);
  }
  e->segmentMan->isSplittable = !(size == 0);
  e->segmentMan->filename = determinFilename(headers);
  
  // quick hack for method 'head'
  if(req->getMethod() == Request::METHOD_HEAD) {
    e->segmentMan->downloadStarted = true;
    e->segmentMan->totalSize = size;
    e->segmentMan->initBitfield(e->option->getAsInt(PREF_SEGMENT_SIZE),
				e->segmentMan->totalSize);
    e->segmentMan->markAllPiecesDone();
    e->segmentMan->isSplittable = false; // TODO because we don't want segment file to be saved.
    return true;
  }
  bool segFileExists = e->segmentMan->segmentFileExists();
  e->segmentMan->downloadStarted = true;
  if(segFileExists) {
    e->segmentMan->load();
    e->segmentMan->diskWriter->openExistingFile(e->segmentMan->getFilePath());
    // send request again to the server with Range header
    return prepareForRetry(0);
  } else {
    e->segmentMan->totalSize = size;
    e->segmentMan->initBitfield(e->option->getAsInt(PREF_SEGMENT_SIZE),
				e->segmentMan->totalSize);
    e->segmentMan->diskWriter->initAndOpenFile(e->segmentMan->getFilePath(),
					       size);
    return prepareForRetry(0);
  }
}

bool HttpResponseCommand::handleOtherEncoding(const string& transferEncoding, const HttpHeader& headers) {
  if(e->segmentMan->shouldCancelDownloadForSafety()) {
    throw new FatalException(EX_FILE_ALREADY_EXISTS,
			     e->segmentMan->getFilePath().c_str(),
			     e->segmentMan->getSegmentFilePath().c_str());
  }
  // we ignore content-length when transfer-encoding is set
  e->segmentMan->downloadStarted = true;
  e->segmentMan->isSplittable = false;
  e->segmentMan->filename = determinFilename(headers);
  e->segmentMan->totalSize = 0;
  // disable keep-alive
  req->setKeepAlive(false);
  Segment segment;
  e->segmentMan->getSegment(segment, cuid);	
  e->segmentMan->diskWriter->initAndOpenFile(e->segmentMan->getFilePath());
  createHttpDownloadCommand(transferEncoding);
  return true;
}

void HttpResponseCommand::createHttpDownloadCommand(const string& transferEncoding) {
  HttpDownloadCommand* command = new HttpDownloadCommand(cuid, req, e, socket);
  command->setMaxDownloadSpeedLimit(e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
  command->setStartupIdleTime(e->option->getAsInt(PREF_STARTUP_IDLE_TIME));
  command->setLowestDownloadSpeedLimit(e->option->getAsInt(PREF_LOWEST_SPEED_LIMIT));
  TransferEncoding* enc = NULL;
  if(transferEncoding.size() && (enc = command->getTransferEncoding(transferEncoding)) == NULL) {
    delete(command);
    throw new DlAbortEx(EX_TRANSFER_ENCODING_NOT_SUPPORTED, transferEncoding.c_str());
  } else {
    if(enc != NULL) {
      command->transferEncoding = transferEncoding;
      enc->init();
    }
    e->commands.push_back(command);
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
  
