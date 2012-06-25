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
#include "HttpResponseCommand.h"
#include "DownloadEngine.h"
#include "DownloadContext.h"
#include "FileEntry.h"
#include "RequestGroup.h"
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
#include "util.h"
#include "File.h"
#include "Option.h"
#include "Logger.h"
#include "Socket.h"
#include "message.h"
#include "prefs.h"
#include "fmt.h"
#include "HttpSkipResponseCommand.h"
#include "HttpHeader.h"
#include "LogFactory.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "a2functional.h"
#include "URISelector.h"
#include "CheckIntegrityEntry.h"
#include "StreamFilter.h"
#include "SinkStreamFilter.h"
#include "ChunkedDecodingStreamFilter.h"
#include "uri.h"
#include "SocketRecvBuffer.h"
#include "MetalinkHttpEntry.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChecksumCheckIntegrityEntry.h"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef HAVE_ZLIB
# include "GZipDecodingStreamFilter.h"
#endif // HAVE_ZLIB

namespace aria2 {

namespace {
SharedHandle<StreamFilter> getTransferEncodingStreamFilter
(const SharedHandle<HttpResponse>& httpResponse,
 const SharedHandle<StreamFilter>& delegate = SharedHandle<StreamFilter>())
{
  SharedHandle<StreamFilter> filter;
  if(httpResponse->isTransferEncodingSpecified()) {
    filter = httpResponse->getTransferEncodingStreamFilter();
    if(!filter) {
      throw DL_ABORT_EX
        (fmt(EX_TRANSFER_ENCODING_NOT_SUPPORTED,
             httpResponse->getTransferEncoding().c_str()));
    }
    filter->init();
    filter->installDelegate(delegate);
  }
  if(!filter) {
    filter = delegate;
  }
  return filter;
}
} // namespace

namespace {
SharedHandle<StreamFilter> getContentEncodingStreamFilter
(const SharedHandle<HttpResponse>& httpResponse,
 const SharedHandle<StreamFilter>& delegate = SharedHandle<StreamFilter>())
{
  SharedHandle<StreamFilter> filter;
  if(httpResponse->isContentEncodingSpecified()) {
    filter = httpResponse->getContentEncodingStreamFilter();
    if(!filter) {
      A2_LOG_INFO
        (fmt("Content-Encoding %s is specified, but the current implementation"
             "doesn't support it. The decoding process is skipped and the"
             "downloaded content will be still encoded.",
             httpResponse->getContentEncoding().c_str()));
    } else {
      filter->init();
      filter->installDelegate(delegate);
    }
  }
  if(!filter) {
    filter = delegate;
  }
  return filter;
}
} // namespace

HttpResponseCommand::HttpResponseCommand
(cuid_t cuid,
 const SharedHandle<Request>& req,
 const SharedHandle<FileEntry>& fileEntry,
 RequestGroup* requestGroup,
 const HttpConnectionHandle& httpConnection,
 DownloadEngine* e,
 const SocketHandle& s)
  : AbstractCommand(cuid, req, fileEntry, requestGroup, e, s,
                    httpConnection->getSocketRecvBuffer()),
    httpConnection_(httpConnection)
{
  checkSocketRecvBuffer();
}

HttpResponseCommand::~HttpResponseCommand() {}

bool HttpResponseCommand::executeInternal()
{
  SharedHandle<HttpRequest> httpRequest =httpConnection_->getFirstHttpRequest();
  SharedHandle<HttpResponse> httpResponse = httpConnection_->receiveResponse();
  if(!httpResponse) {
    // The server has not responded to our request yet.
    // For socket->wantRead() == true, setReadCheckSocket(socket) is already
    // done in the constructor.
    setWriteCheckSocketIf(getSocket(), getSocket()->wantWrite());
    getDownloadEngine()->addCommand(this);
    return false;
  }
  // check HTTP status number
  httpResponse->validateResponse();
  httpResponse->retrieveCookie();

  SharedHandle<HttpHeader> httpHeader = httpResponse->getHttpHeader();
  // Disable persistent connection if:
  //   Connection: close is received or the remote server is not HTTP/1.1.
  // We don't care whether non-HTTP/1.1 server returns Connection: keep-alive.
  getRequest()->supportsPersistentConnection
    (httpResponse->supportsPersistentConnection());
  if(getRequest()->isPipeliningEnabled()) {
    getRequest()->setMaxPipelinedRequest
      (getOption()->getAsInt(PREF_MAX_HTTP_PIPELINING));
  } else {
    getRequest()->setMaxPipelinedRequest(1);
  }

  int statusCode = httpResponse->getStatusCode();

  if(statusCode == 304) {
    int64_t totalLength = httpResponse->getEntityLength();
    getFileEntry()->setLength(totalLength);
    getRequestGroup()->initPieceStorage();
    getPieceStorage()->markAllPiecesDone();
    // Just set checksum verification done.
    getDownloadContext()->setChecksumVerified(true);
    A2_LOG_NOTICE
      (fmt(MSG_DOWNLOAD_ALREADY_COMPLETED,
           getRequestGroup()->getGID(),
           getRequestGroup()->getFirstFilePath().c_str()));
    poolConnection();
    getFileEntry()->poolRequest(getRequest());
    return true;
  }
  if(!getPieceStorage()) {
    // Metalink/HTTP
    if(!getDownloadContext()->getMetalinkServerContacted()) {
      if(httpHeader->defined(HttpHeader::LINK)) {
        getDownloadContext()->setMetalinkServerContacted(true);
        std::vector<MetalinkHttpEntry> entries;
        httpResponse->getMetalinKHttpEntries(entries, getOption());
        for(std::vector<MetalinkHttpEntry>::iterator i = entries.begin(),
              eoi = entries.end(); i != eoi; ++i) {
          getFileEntry()->addUri((*i).uri);
          A2_LOG_DEBUG(fmt("Adding URI=%s", (*i).uri.c_str()));
        }
      }
    }
#ifdef ENABLE_MESSAGE_DIGEST
    if(httpHeader->defined(HttpHeader::DIGEST)) {
      std::vector<Checksum> checksums;
      httpResponse->getDigest(checksums);
      for(std::vector<Checksum>::iterator i = checksums.begin(),
            eoi = checksums.end(); i != eoi; ++i) {
        if(getDownloadContext()->getHashType().empty()) {
          A2_LOG_DEBUG(fmt("Setting digest: type=%s, digest=%s",
                           (*i).getHashType().c_str(),
                           (*i).getDigest().c_str()));
          getDownloadContext()->setDigest((*i).getHashType(), (*i).getDigest());
          break;
        } else {
          if(checkChecksum(getDownloadContext(), *i)) {
            break;
          }
        }
      }
    }
#endif // ENABLE_MESSAGE_DIGEST
  }
  if(statusCode >= 300) {
    if(statusCode == 404) {
      getRequestGroup()->increaseAndValidateFileNotFoundCount();
    }
    return skipResponseBody(httpResponse);
  }
  if(getFileEntry()->isUniqueProtocol()) {
    // Redirection should be considered here. We need to parse
    // original URI to get hostname.
    uri::UriStruct us;
    if(uri::parse(us, getRequest()->getUri())) {
      getFileEntry()->removeURIWhoseHostnameIs(us.host);
    }
  }
  if(!getPieceStorage()) {
    util::removeMetalinkContentTypes(getRequestGroup());
    int64_t totalLength = httpResponse->getEntityLength();
    getFileEntry()->setLength(totalLength);
    if(getFileEntry()->getPath().empty()) {
      getFileEntry()->setPath
        (util::createSafePath
         (getOption()->get(PREF_DIR), httpResponse->determinFilename()));
    }
    getFileEntry()->setContentType(httpResponse->getContentType());
    getRequestGroup()->preDownloadProcessing();
    if(getDownloadEngine()->getRequestGroupMan()->
       isSameFileBeingDownloaded(getRequestGroup())) {
      throw DOWNLOAD_FAILURE_EXCEPTION2
        (fmt(EX_DUPLICATE_FILE_DOWNLOAD,
             getRequestGroup()->getFirstFilePath().c_str()),
         error_code::DUPLICATE_DOWNLOAD);
    }
    // update last modified time
    updateLastModifiedTime(httpResponse->getLastModifiedTime());

    // If both transfer-encoding and total length is specified, we
    // assume we can do segmented downloading
    if(totalLength == 0 || shouldInflateContentEncoding(httpResponse)) {
      // we ignore content-length when inflate is required
      getFileEntry()->setLength(0);
      if(getRequest()->getMethod() == Request::METHOD_GET &&
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
#ifdef ENABLE_MESSAGE_DIGEST
    if(!getDownloadContext()->getHashType().empty() &&
       httpHeader->defined(HttpHeader::DIGEST)) {
      std::vector<Checksum> checksums;
      httpResponse->getDigest(checksums);
      for(std::vector<Checksum>::iterator i = checksums.begin(),
            eoi = checksums.end(); i != eoi; ++i) {
        if(checkChecksum(getDownloadContext(), *i)) {
          break;
        }
      }
    }
#endif // ENABLE_MESSAGE_DIGEST
    // validate totalsize
    getRequestGroup()->validateTotalLength(getFileEntry()->getLength(),
                                       httpResponse->getEntityLength());
    // update last modified time
    updateLastModifiedTime(httpResponse->getLastModifiedTime());
    if(getRequestGroup()->getTotalLength() == 0) {
      // Since total length is unknown, the file size in previously
      // failed download could be larger than the size this time.
      // Also we can't resume in this case too.  So truncate the file
      // anyway.
      getPieceStorage()->getDiskAdaptor()->truncate(0);
      getDownloadEngine()->addCommand
        (createHttpDownloadCommand
         (httpResponse,
          getTransferEncodingStreamFilter
          (httpResponse,
           getContentEncodingStreamFilter(httpResponse))));
    } else {
      getDownloadEngine()->addCommand
        (createHttpDownloadCommand
         (httpResponse,
          getTransferEncodingStreamFilter(httpResponse)));
    }
    return true;
  }
}

void HttpResponseCommand::updateLastModifiedTime(const Time& lastModified)
{
  if(getOption()->getAsBool(PREF_REMOTE_TIME)) {
    getRequestGroup()->updateLastModifiedTime(lastModified);
  }
}

bool HttpResponseCommand::shouldInflateContentEncoding
(const SharedHandle<HttpResponse>& httpResponse)
{
  // Basically, on the fly inflation cannot be made with segment
  // download, because in each segment we don't know where the date
  // should be written.  So turn off segmented downloading.
  // Meanwhile, Some server returns content-encoding: gzip for .tgz
  // files.  I think those files should not be inflated by clients,
  // because it is the original format of those files. Current
  // implementation just inflates these files nonetheless.
  const std::string& ce = httpResponse->getContentEncoding();
  return httpResponse->getHttpRequest()->acceptGZip() &&
    (ce == "gzip" || ce == "deflate");
}

bool HttpResponseCommand::handleDefaultEncoding
(const SharedHandle<HttpResponse>& httpResponse)
{
  SharedHandle<HttpRequest> httpRequest = httpResponse->getHttpRequest();
  SharedHandle<BtProgressInfoFile> progressInfoFile
    (new DefaultBtProgressInfoFile
     (getDownloadContext(), SharedHandle<PieceStorage>(), getOption().get()));
  getRequestGroup()->adjustFilename(progressInfoFile);
  getRequestGroup()->initPieceStorage();

  if(getOption()->getAsBool(PREF_DRY_RUN)) {
    onDryRunFileFound();
    return true;
  }

  SharedHandle<CheckIntegrityEntry> checkEntry =
    getRequestGroup()->createCheckIntegrityEntry();
  if(!checkEntry) {
    return true;
  }
  File file(getRequestGroup()->getFirstFilePath());
  // We have to make sure that command that has Request object must
  // have segment after PieceStorage is initialized. See
  // AbstractCommand::execute()
  SharedHandle<Segment> segment =
    getSegmentMan()->getSegmentWithIndex(getCuid(), 0);
  // pipelining requires implicit range specified. But the request for
  // this response most likely dones't contains range header. This means
  // we can't continue to use this socket because server sends all entity
  // body instead of a segment.
  // Therefore, we shutdown the socket here if pipelining is enabled.
  DownloadCommand* command = 0;
  if(getRequest()->getMethod() == Request::METHOD_GET &&
     segment && segment->getPositionToWrite() == 0 &&
     !getRequest()->isPipeliningEnabled()) {
    command = createHttpDownloadCommand
      (httpResponse,
       getTransferEncodingStreamFilter(httpResponse));
  } else {
    getSegmentMan()->cancelSegment(getCuid());
    getFileEntry()->poolRequest(getRequest());
  }
  // After command is passed to prepareForNextAction(), it is managed
  // by CheckIntegrityEntry.
  checkEntry->pushNextCommand(command);
  command = 0;

  prepareForNextAction(checkEntry);

  if(getRequest()->getMethod() == Request::METHOD_HEAD) {
    poolConnection();
    getRequest()->setMethod(Request::METHOD_GET);
  }
  return true;
}

bool HttpResponseCommand::handleOtherEncoding
(const SharedHandle<HttpResponse>& httpResponse) {
  // We assume that RequestGroup::getTotalLength() == 0 here
  SharedHandle<HttpRequest> httpRequest = httpResponse->getHttpRequest();

  if(getOption()->getAsBool(PREF_DRY_RUN)) {
    getRequestGroup()->initPieceStorage();
    onDryRunFileFound();
    return true;
  }

  if(getRequest()->getMethod() == Request::METHOD_HEAD) {
    poolConnection();
    getRequest()->setMethod(Request::METHOD_GET);
    return prepareForRetry(0);
  }

  // In this context, knowsTotalLength() is true only when the file is
  // really zero-length.

  SharedHandle<StreamFilter> streamFilter =
    getTransferEncodingStreamFilter
    (httpResponse,
     getContentEncodingStreamFilter(httpResponse));
  // If chunked transfer-encoding is specified, we have to read end of
  // chunk markers(0\r\n\r\n, for example).
  bool chunkedUsed = streamFilter &&
    streamFilter->getName() == ChunkedDecodingStreamFilter::NAME;

  // For zero-length file, check existing file comparing its size
  if(!chunkedUsed && getDownloadContext()->knowsTotalLength() &&
     getRequestGroup()->downloadFinishedByFileLength()) {
    getRequestGroup()->initPieceStorage();
#ifdef ENABLE_MESSAGE_DIGEST
    // TODO Known issue: if .aria2 file exists, it will not be deleted
    // on successful verification, because .aria2 file is not loaded.
    // See also FtpNegotiationCommand::onFileSizeDetermined()
    if(getDownloadContext()->isChecksumVerificationNeeded()) {
      A2_LOG_DEBUG("Zero length file exists. Verify checksum.");
      SharedHandle<ChecksumCheckIntegrityEntry> entry
        (new ChecksumCheckIntegrityEntry(getRequestGroup()));
      entry->initValidator();
      getPieceStorage()->getDiskAdaptor()->openExistingFile();
      getDownloadEngine()->getCheckIntegrityMan()->pushEntry(entry);
    } else
#endif // ENABLE_MESSAGE_DIGEST
      {
        getPieceStorage()->markAllPiecesDone();
        getDownloadContext()->setChecksumVerified(true);
        A2_LOG_NOTICE
          (fmt(MSG_DOWNLOAD_ALREADY_COMPLETED,
               getRequestGroup()->getGID(),
               getRequestGroup()->getFirstFilePath().c_str()));
      }
    poolConnection();
    return true;
  }

  getRequestGroup()->shouldCancelDownloadForSafety();
  getRequestGroup()->initPieceStorage();
  getPieceStorage()->getDiskAdaptor()->initAndOpenFile();

  // Local file size becomes zero when DiskAdaptor::initAndOpenFile()
  // is called. So zero-length file is complete if chunked encoding is
  // not used.
  if(!chunkedUsed && getDownloadContext()->knowsTotalLength()) {
    A2_LOG_DEBUG("File length becomes zero and it means download completed.");
    // TODO Known issue: if .aria2 file exists, it will not be deleted
    // on successful verification, because .aria2 file is not loaded.
    // See also FtpNegotiationCommand::onFileSizeDetermined()
#ifdef ENABLE_MESSAGE_DIGEST
    if(getDownloadContext()->isChecksumVerificationNeeded()) {
      A2_LOG_DEBUG("Verify checksum for zero-length file");
      SharedHandle<ChecksumCheckIntegrityEntry> entry
        (new ChecksumCheckIntegrityEntry(getRequestGroup()));
      entry->initValidator();
      getDownloadEngine()->getCheckIntegrityMan()->pushEntry(entry);
    } else
#endif // ENABLE_MESSAGE_DIGEST
      {
        getRequestGroup()->getPieceStorage()->markAllPiecesDone();
      }
    poolConnection();
    return true;
  }
  // We have to make sure that command that has Request object must
  // have segment after PieceStorage is initialized. See
  // AbstractCommand::execute()
  getSegmentMan()->getSegmentWithIndex(getCuid(), 0);

  getDownloadEngine()->addCommand
    (createHttpDownloadCommand(httpResponse, streamFilter));
  return true;
}

bool HttpResponseCommand::skipResponseBody
(const SharedHandle<HttpResponse>& httpResponse)
{
  SharedHandle<StreamFilter> filter =
    getTransferEncodingStreamFilter(httpResponse);
  // We don't use Content-Encoding here because this response body is just
  // thrown away.

  HttpSkipResponseCommand* command = new HttpSkipResponseCommand
    (getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
     httpConnection_, httpResponse,
     getDownloadEngine(), getSocket());
  command->installStreamFilter(filter);

  // If request method is HEAD or the response body is zero-length,
  // set command's status to real time so that avoid read check blocking
  if(getRequest()->getMethod() == Request::METHOD_HEAD ||
     (httpResponse->getEntityLength() == 0 &&
      !httpResponse->isTransferEncodingSpecified())) {
    command->setStatusRealtime();
    // If entity length == 0, then socket read/write check must be disabled.
    command->disableSocketCheck();
    getDownloadEngine()->setNoWait(true);
  }

  getDownloadEngine()->addCommand(command);
  return true;
}

namespace {
bool decideFileAllocation
(const SharedHandle<StreamFilter>& filter)
{
#ifdef HAVE_ZLIB
  for(SharedHandle<StreamFilter> f = filter; f; f = f->getDelegate()){
    // Since the compressed file's length are returned in the response header
    // and the decompressed file size is unknown at this point, disable file
    // allocation here.
    if(f->getName() == GZipDecodingStreamFilter::NAME) {
      return false;
    }
  }
#endif // HAVE_ZLIB
  return true;
}
} // namespace

HttpDownloadCommand* HttpResponseCommand::createHttpDownloadCommand
(const SharedHandle<HttpResponse>& httpResponse,
 const SharedHandle<StreamFilter>& filter)
{

  HttpDownloadCommand* command =
    new HttpDownloadCommand(getCuid(), getRequest(), getFileEntry(),
                            getRequestGroup(),
                            httpResponse, httpConnection_,
                            getDownloadEngine(), getSocket());
  command->setStartupIdleTime(getOption()->getAsInt(PREF_STARTUP_IDLE_TIME));
  command->setLowestDownloadSpeedLimit
    (getOption()->getAsInt(PREF_LOWEST_SPEED_LIMIT));
  command->installStreamFilter(filter);
  if(getRequestGroup()->isFileAllocationEnabled() &&
     !decideFileAllocation(filter)) {
    getRequestGroup()->setFileAllocationEnabled(false);    
  }
  getRequestGroup()->getURISelector()->tuneDownloadCommand
    (getFileEntry()->getRemainingUris(), command);

  return command;
}

void HttpResponseCommand::poolConnection()
{
  if(getRequest()->supportsPersistentConnection()) {
    getDownloadEngine()->poolSocket(getRequest(), createProxyRequest(),
                                    getSocket());
  }
}

void HttpResponseCommand::onDryRunFileFound()
{
  getPieceStorage()->markAllPiecesDone();
  getDownloadContext()->setChecksumVerified(true);
  poolConnection();
}

#ifdef ENABLE_MESSAGE_DIGEST
bool HttpResponseCommand::checkChecksum
(const SharedHandle<DownloadContext>& dctx,
 const Checksum& checksum)
{
  if(dctx->getHashType() == checksum.getHashType()) {
    if(dctx->getDigest() == checksum.getDigest()) {
      A2_LOG_INFO("Valid hash found in Digest header field.");
      return true;
    } else {
      throw DL_ABORT_EX("Invalid hash found in Digest header field.");
    }
  }
  return false;
}
#endif // ENABLE_MESSAGE_DIGEST

} // namespace aria2
