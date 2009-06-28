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
#include "DownloadContext.h"
#include "FileEntry.h"
#include "RequestGroup.h"
#include "ServerHost.h"
#include "RequestGroupMan.h"
#include "Request.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpConnection.h"
#include "SegmentMan.h"
#include "Segment.h"
#include "HttpDownloadCommand.h"
#include "DiskAdaptor.h"
#include "PieceStorage.h"
#include "DefaultBtProgressInfoFile.h"
#include "DownloadFailureException.h"
#include "DlAbortEx.h"
#include "Util.h"
#include "File.h"
#include "Option.h"
#include "Logger.h"
#include "Socket.h"
#include "message.h"
#include "prefs.h"
#include "StringFormat.h"
#include "HttpSkipResponseCommand.h"
#include "HttpHeader.h"
#include "LogFactory.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "a2functional.h"

namespace aria2 {

static SharedHandle<Decoder> getTransferEncodingDecoder
(const SharedHandle<HttpResponse>& httpResponse);

static SharedHandle<Decoder> getContentEncodingDecoder
(const SharedHandle<HttpResponse>& httpResponse);

HttpResponseCommand::HttpResponseCommand
(int32_t cuid,
 const RequestHandle& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 const HttpConnectionHandle& httpConnection,
 DownloadEngine* e,
 const SocketHandle& s)
  :AbstractCommand(cuid, req, fileEntry, requestGroup, e, s),
   httpConnection(httpConnection)
{}

HttpResponseCommand::~HttpResponseCommand() {}

bool HttpResponseCommand::executeInternal()
{
  HttpRequestHandle httpRequest = httpConnection->getFirstHttpRequest();
  HttpResponseHandle httpResponse = httpConnection->receiveResponse();
  if(httpResponse.isNull()) {
    // The server has not responded to our request yet.
    // For socket->wantRead() == true, setReadCheckSocket(socket) is already
    // done in the constructor.
    setWriteCheckSocketIf(socket, socket->wantWrite());
    e->commands.push_back(this);
    return false;
  }
  // check HTTP status number
  httpResponse->validateResponse();
  httpResponse->retrieveCookie();

  SharedHandle<HttpHeader> httpHeader = httpResponse->getHttpHeader();
  // Disable persistent connection if:
  //   Connection: close is received or the remote server is not HTTP/1.1.
  // We don't care whether non-HTTP/1.1 server returns Connection: keep-alive.
  req->supportsPersistentConnection
    (httpResponse->supportsPersistentConnection());
  if(req->isPipeliningEnabled()) {
    req->setMaxPipelinedRequest(getOption()->getAsInt(PREF_MAX_HTTP_PIPELINING));
  }

  if(httpResponse->getResponseStatus() >= HttpHeader::S300) {
    if(httpResponse->getResponseStatus() == HttpHeader::S404) {
      _requestGroup->increaseAndValidateFileNotFoundCount();
    }
    return skipResponseBody(httpResponse);
  }
  if(!_requestGroup->isSingleHostMultiConnectionEnabled()) {
    // Query by hostname. Searching by CUID may returns NULL.  In case
    // when resuming download, ServerHost is registered with CUID A.
    // Then if requested range is not equal to saved one,
    // StreamFileAllocationEntry is created with _nextCommand NULL.
    // This results creating new command CUID, say B and same URI. So
    // searching ServerHost by CUID B fails.
    SharedHandle<ServerHost> sv =
      _requestGroup->searchServerHost(req->getHost());
    if(!sv.isNull()) {
      _requestGroup->removeURIWhoseHostnameIs(sv->getHostname());
    }
  }
  if(_requestGroup->getPieceStorage().isNull()) {
    uint64_t totalLength = httpResponse->getEntityLength();
    _fileEntry->setLength(totalLength);
    // We assume that in this context
    // DownloadContext::getFileEntries().size() == 1
    if(getOption()->get(PREF_OUT).empty()) {
      _fileEntry->setPath
	(strconcat(getDownloadContext()->getDir(),
		   "/", httpResponse->determinFilename()));
    }
    _fileEntry->setContentType(httpResponse->getContentType());
    _requestGroup->preDownloadProcessing();
    if(e->_requestGroupMan->isSameFileBeingDownloaded(_requestGroup)) {
      throw DOWNLOAD_FAILURE_EXCEPTION
	(StringFormat(EX_DUPLICATE_FILE_DOWNLOAD,
		      _requestGroup->getFirstFilePath().c_str()).str());
    }
    // update last modified time
    updateLastModifiedTime(httpResponse->getLastModifiedTime());

    // If both transfer-encoding and total length is specified, we
    // assume we can do segmented downloading
    if(totalLength == 0 || shouldInflateContentEncoding(httpResponse)) {
      // we ignore content-length when inflate is required
      _fileEntry->setLength(0);
      if(req->getMethod() == Request::METHOD_GET &&
	 (totalLength != 0 ||
	  !httpResponse->getHttpHeader()->defined(HttpHeader::CONTENT_LENGTH))){
	// DownloadContext::knowsTotalLength() == true only when
	// server says the size of file is 0 explicitly.
	getDownloadContext()->markTotalLengthIsUnknown();
      }
      return handleOtherEncoding(httpResponse);
    } else {
      return handleDefaultEncoding(httpResponse);
    }
  } else {
    // validate totalsize
    _requestGroup->validateTotalLength(_fileEntry->getLength(),
				       httpResponse->getEntityLength());
    // update last modified time
    updateLastModifiedTime(httpResponse->getLastModifiedTime());
    if(_requestGroup->getTotalLength() == 0) {
      // Since total length is unknown, the file size in previously
      // failed download could be larger than the size this time.
      // Also we can't resume in this case too.  So truncate the file
      // anyway.
      _requestGroup->getPieceStorage()->getDiskAdaptor()->truncate(0);
      e->commands.push_back
	(createHttpDownloadCommand(httpResponse,
				   getTransferEncodingDecoder(httpResponse),
				   getContentEncodingDecoder(httpResponse)));
    } else {
      e->commands.push_back(createHttpDownloadCommand(httpResponse,
						      getTransferEncodingDecoder(httpResponse)));
    }
    return true;
  }
}

void HttpResponseCommand::updateLastModifiedTime(const Time& lastModified)
{
  if(getOption()->getAsBool(PREF_REMOTE_TIME)) {
    _requestGroup->updateLastModifiedTime(lastModified);
  }
}

static bool fileIsGzipped(const SharedHandle<HttpResponse>& httpResponse)
{
  std::string filename =
    Util::toLower(httpResponse->getHttpRequest()->getRequest()->getFile());
  return Util::endsWith(filename, ".gz") || Util::endsWith(filename, ".tgz");
}

bool HttpResponseCommand::shouldInflateContentEncoding
(const SharedHandle<HttpResponse>& httpResponse)
{
  // Basically, on the fly inflation cannot be made with segment download,
  // because in each segment we don't know where the date should be written.
  // So turn off segmented downloading.
  // Meanwhile, Some server returns content-encoding: gzip for .tgz files.
  // I think those files should not be inflated by clients, because it is the
  // original format of those files. So I made filename ending ".gz" or ".tgz"
  // (case-insensitive) not inflated.
  return httpResponse->isContentEncodingSpecified() &&
    !fileIsGzipped(httpResponse);
}

bool HttpResponseCommand::handleDefaultEncoding(const HttpResponseHandle& httpResponse)
{
  HttpRequestHandle httpRequest = httpResponse->getHttpRequest();
  _requestGroup->adjustFilename
    (SharedHandle<BtProgressInfoFile>(new DefaultBtProgressInfoFile
				      (_requestGroup->getDownloadContext(),
				       SharedHandle<PieceStorage>(),
				       getOption().get())));
  _requestGroup->initPieceStorage();

  if(getOption()->getAsBool(PREF_DRY_RUN)) {
    onDryRunFileFound();
    return true;
  }

  BtProgressInfoFileHandle infoFile(new DefaultBtProgressInfoFile(_requestGroup->getDownloadContext(), _requestGroup->getPieceStorage(), getOption().get()));
  if(!infoFile->exists() && _requestGroup->downloadFinishedByFileLength()) {
    _requestGroup->getPieceStorage()->markAllPiecesDone();

    logger->notice(MSG_DOWNLOAD_ALREADY_COMPLETED,
		   _requestGroup->getGID(),
		   _requestGroup->getFirstFilePath().c_str());

    return true;
  }

  DownloadCommand* command = 0;
  try {
    _requestGroup->loadAndOpenFile(infoFile);
    File file(_requestGroup->getFirstFilePath());

    SegmentHandle segment = _requestGroup->getSegmentMan()->getSegment(cuid, 0);
    // pipelining requires implicit range specified. But the request for
    // this response most likely dones't contains range header. This means
    // we can't continue to use this socket because server sends all entity
    // body instead of a segment.
    // Therefore, we shutdown the socket here if pipelining is enabled.
    if(req->getMethod() == Request::METHOD_GET &&
       !segment.isNull() && segment->getPositionToWrite() == 0 &&
       !req->isPipeliningEnabled()) {
      command = createHttpDownloadCommand(httpResponse);
    } else {
      _requestGroup->getSegmentMan()->cancelSegment(cuid);
      _fileEntry->poolRequest(req);
    }
    prepareForNextAction(command);
    if(req->getMethod() == Request::METHOD_HEAD) {
      poolConnection();
      req->setMethod(Request::METHOD_GET);
    }
  } catch(Exception& e) {
    delete command;
    throw;
  }
  return true;
}

static SharedHandle<Decoder> getTransferEncodingDecoder
(const SharedHandle<HttpResponse>& httpResponse)
{
  SharedHandle<Decoder> decoder;
  if(httpResponse->isTransferEncodingSpecified()) {
    decoder = httpResponse->getTransferEncodingDecoder();
    if(decoder.isNull()) {
      throw DL_ABORT_EX
	(StringFormat(EX_TRANSFER_ENCODING_NOT_SUPPORTED,
		      httpResponse->getTransferEncoding().c_str()).str());
    }
    decoder->init();
  }
  return decoder;
}

static SharedHandle<Decoder> getContentEncodingDecoder
(const SharedHandle<HttpResponse>& httpResponse)
{
  SharedHandle<Decoder> decoder;
  if(httpResponse->isContentEncodingSpecified()) {
    decoder = httpResponse->getContentEncodingDecoder();
    if(decoder.isNull()) {
      LogFactory::getInstance()->info
	("Content-Encoding %s is specified, but the current implementation"
	 "doesn't support it. The decoding process is skipped and the"
	 "downloaded content will be still encoded.",
	 httpResponse->getContentEncoding().c_str());
    } else {
      decoder->init();
    }
  }
  return decoder;
}

bool HttpResponseCommand::handleOtherEncoding(const HttpResponseHandle& httpResponse) {
  // We assume that RequestGroup::getTotalLength() == 0 here
  HttpRequestHandle httpRequest = httpResponse->getHttpRequest();

  if(getOption()->getAsBool(PREF_DRY_RUN)) {
    _requestGroup->initPieceStorage();
    onDryRunFileFound();
    return true;
  }

  if(req->getMethod() == Request::METHOD_HEAD) {
    poolConnection();
    req->setMethod(Request::METHOD_GET);
    return prepareForRetry(0);
  }

  // For zero-length file, check existing file comparing its size
  if(_requestGroup->downloadFinishedByFileLength()) {
    _requestGroup->initPieceStorage();
    _requestGroup->getPieceStorage()->markAllPiecesDone();

    logger->notice(MSG_DOWNLOAD_ALREADY_COMPLETED,
		   _requestGroup->getGID(),
		   _requestGroup->getFirstFilePath().c_str());

    poolConnection();
    return true;
  }

  _requestGroup->shouldCancelDownloadForSafety();
  _requestGroup->initPieceStorage();

  _requestGroup->getPieceStorage()->getDiskAdaptor()->initAndOpenFile();

  // In this context, knowsTotalLength() is true only when the file is
  // really zero-length.
  if(_requestGroup->getDownloadContext()->knowsTotalLength()) {
    poolConnection();
    return true;
  }
  e->commands.push_back
    (createHttpDownloadCommand(httpResponse,
			       getTransferEncodingDecoder(httpResponse),
			       getContentEncodingDecoder(httpResponse)));
  return true;
}

bool HttpResponseCommand::skipResponseBody
(const SharedHandle<HttpResponse>& httpResponse)
{
  SharedHandle<Decoder> decoder = getTransferEncodingDecoder(httpResponse);
  // We don't use Content-Encoding here because this response body is just
  // thrown away.

  HttpSkipResponseCommand* command = new HttpSkipResponseCommand
    (cuid, req, _fileEntry, _requestGroup, httpConnection, httpResponse, e, socket);
  command->setTransferEncodingDecoder(decoder);

  // If request method is HEAD or the response body is zero-length,
  // set command's status to real time so that avoid read check blocking
  if(req->getMethod() == Request::METHOD_HEAD ||
     (httpResponse->getEntityLength() == 0 &&
      !httpResponse->isTransferEncodingSpecified())) {
    command->setStatusRealtime();
    // If entity length == 0, then socket read/write check must be disabled.
    command->disableSocketCheck();
    e->setNoWait(true);
  }

  e->commands.push_back(command);
  return true;
}

HttpDownloadCommand* HttpResponseCommand::createHttpDownloadCommand
(const HttpResponseHandle& httpResponse,
 const SharedHandle<Decoder>& transferEncodingDecoder,
 const SharedHandle<Decoder>& contentEncodingDecoder)
{

  HttpDownloadCommand* command =
    new HttpDownloadCommand(cuid, req, _fileEntry, _requestGroup,
			    httpResponse, httpConnection, e, socket);
  command->setStartupIdleTime(getOption()->getAsInt(PREF_STARTUP_IDLE_TIME));
  command->setLowestDownloadSpeedLimit
    (getOption()->getAsInt(PREF_LOWEST_SPEED_LIMIT));
  command->setTransferEncodingDecoder(transferEncodingDecoder);

  if(!contentEncodingDecoder.isNull()) {
    command->setContentEncodingDecoder(contentEncodingDecoder);
    // Since the compressed file's length are returned in the response header
    // and the decompressed file size is unknown at this point, disable file
    // allocation here.
    _requestGroup->setFileAllocationEnabled(false);
  }

  _requestGroup->tuneDownloadCommand(command);

  return command;
}

void HttpResponseCommand::poolConnection()
{
  if(req->supportsPersistentConnection()) {
    e->poolSocket(req, isProxyDefined(), socket);
  }
}

void HttpResponseCommand::onDryRunFileFound()
{
  _requestGroup->getPieceStorage()->markAllPiecesDone();
  poolConnection();
}

} // namespace aria2
