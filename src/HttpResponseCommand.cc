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
#include "SocketCore.h"
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
#include "NullProgressInfoFile.h"
#include "Checksum.h"
#include "ChecksumCheckIntegrityEntry.h"
#ifdef HAVE_ZLIB
#  include "GZipDecodingStreamFilter.h"
#endif // HAVE_ZLIB

namespace aria2 {

namespace {

std::unique_ptr<StreamFilter> getTransferEncodingStreamFilter(
    HttpResponse* httpResponse,
    std::unique_ptr<StreamFilter> delegate = nullptr)
{
  if (httpResponse->isTransferEncodingSpecified()) {
    auto filter = httpResponse->getTransferEncodingStreamFilter();
    if (!filter) {
      throw DL_ABORT_EX(fmt(EX_TRANSFER_ENCODING_NOT_SUPPORTED,
                            httpResponse->getTransferEncoding().c_str()));
    }
    filter->init();
    filter->installDelegate(std::move(delegate));
    return filter;
  }

  return delegate;
}

std::unique_ptr<StreamFilter>
getContentEncodingStreamFilter(HttpResponse* httpResponse,
                               std::unique_ptr<StreamFilter> delegate = nullptr)
{
  if (httpResponse->isContentEncodingSpecified()) {
    auto filter = httpResponse->getContentEncodingStreamFilter();
    if (!filter) {
      A2_LOG_INFO(fmt("Content-Encoding %s is specified, but the current "
                      "implementation doesn't support it. The decoding "
                      "process is skipped and the downloaded content will be "
                      "still encoded.",
                      httpResponse->getContentEncoding().c_str()));
    }
    else {
      filter->init();
      filter->installDelegate(std::move(delegate));
      return filter;
    }
  }
  return delegate;
}

} // namespace

HttpResponseCommand::HttpResponseCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    const std::shared_ptr<HttpConnection>& httpConnection, DownloadEngine* e,
    const std::shared_ptr<SocketCore>& s)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e, s,
                      httpConnection->getSocketRecvBuffer()),
      httpConnection_(httpConnection)
{
  checkSocketRecvBuffer();
}

HttpResponseCommand::~HttpResponseCommand() = default;

bool HttpResponseCommand::executeInternal()
{
  auto httpResponse = httpConnection_->receiveResponse();
  if (!httpResponse) {
    // The server has not responded to our request yet.
    // For socket->wantRead() == true, setReadCheckSocket(socket) is already
    // done in the constructor.
    setWriteCheckSocketIf(getSocket(), getSocket()->wantWrite());
    addCommandSelf();
    return false;
  }

  // check HTTP status code
  httpResponse->validateResponse();
  httpResponse->retrieveCookie();

  const auto& httpHeader = httpResponse->getHttpHeader();
  // Disable persistent connection if:
  //   Connection: close is received or the remote server is not HTTP/1.1.
  // We don't care whether non-HTTP/1.1 server returns Connection: keep-alive.
  auto& req = getRequest();
  req->supportsPersistentConnection(
      httpResponse->supportsPersistentConnection());
  if (req->isPipeliningEnabled()) {
    req->setMaxPipelinedRequest(
        getOption()->getAsInt(PREF_MAX_HTTP_PIPELINING));
  }
  else {
    req->setMaxPipelinedRequest(1);
  }

  auto statusCode = httpResponse->getStatusCode();
  auto& ctx = getDownloadContext();
  auto grp = getRequestGroup();
  auto& fe = getFileEntry();

  if (statusCode == 304) {
    int64_t totalLength = httpResponse->getEntityLength();
    fe->setLength(totalLength);
    grp->initPieceStorage();
    getPieceStorage()->markAllPiecesDone();
    // Just set checksum verification done.
    ctx->setChecksumVerified(true);

    if (fe->getPath().empty()) {
      // If path is empty, set default file name or file portion of
      // URI.  This is the file we used to get modified date.
      auto& file = getRequest()->getFile();
      auto suffixPath = util::createSafePath(
          getRequest()->getFile().empty()
              ? Request::DEFAULT_FILE
              : util::percentDecode(std::begin(file), std::end(file)));
      fe->setPath(util::applyDir(getOption()->get(PREF_DIR), suffixPath));
      fe->setSuffixPath(suffixPath);
    }

    A2_LOG_NOTICE(fmt(MSG_DOWNLOAD_ALREADY_COMPLETED,
                      GroupId::toHex(grp->getGID()).c_str(),
                      grp->getFirstFilePath().c_str()));
    poolConnection();
    fe->poolRequest(req);
    return true;
  }

  if (!getPieceStorage()) {
    // Metalink/HTTP
    if (ctx->getAcceptMetalink()) {
      if (httpHeader->defined(HttpHeader::LINK)) {
        ctx->setAcceptMetalink(false);
        std::vector<MetalinkHttpEntry> entries;
        httpResponse->getMetalinKHttpEntries(entries, getOption());
        for (const auto& e : entries) {
          fe->addUri(e.uri);
          A2_LOG_DEBUG(fmt("Adding URI=%s", e.uri.c_str()));
        }
      }
    }

    if (httpHeader->defined(HttpHeader::DIGEST)) {
      std::vector<Checksum> checksums;
      httpResponse->getDigest(checksums);
      for (const auto& checksum : checksums) {
        if (ctx->getHashType().empty()) {
          A2_LOG_DEBUG(fmt("Setting digest: type=%s, digest=%s",
                           checksum.getHashType().c_str(),
                           checksum.getDigest().c_str()));
          ctx->setDigest(checksum.getHashType(), checksum.getDigest());
          break;
        }

        if (checkChecksum(ctx, checksum)) {
          break;
        }
      }
    }
  }

  if (statusCode >= 300) {
    if (statusCode == 404) {
      grp->increaseAndValidateFileNotFoundCount();
    }
    return skipResponseBody(std::move(httpResponse));
  }

  if (fe->isUniqueProtocol()) {
    // Redirection should be considered here. We need to parse
    // original URI to get hostname.
    const std::string& uri = getRequest()->getUri();
    uri_split_result us;
    if (uri_split(&us, uri.c_str()) == 0) {
      std::string host = uri::getFieldString(us, USR_HOST, uri.c_str());
      fe->removeURIWhoseHostnameIs(host);
    }
  }

  if (!getPieceStorage()) {
    ctx->setAcceptMetalink(false);
    int64_t totalLength = httpResponse->getEntityLength();
    fe->setLength(totalLength);
    if (fe->getPath().empty()) {
      auto suffixPath = util::createSafePath(httpResponse->determineFilename(
          getOption()->getAsBool(PREF_CONTENT_DISPOSITION_DEFAULT_UTF8)));
      fe->setPath(util::applyDir(getOption()->get(PREF_DIR), suffixPath));
      fe->setSuffixPath(suffixPath);
    }
    fe->setContentType(httpResponse->getContentType());
    grp->preDownloadProcessing();

    // update last modified time
    updateLastModifiedTime(httpResponse->getLastModifiedTime());

    // If both transfer-encoding and total length is specified, we
    // should have ignored total length.  In this case, we can not do
    // segmented downloading
    if (totalLength == 0 || shouldInflateContentEncoding(httpResponse.get())) {
      // we ignore content-length when inflate is required
      fe->setLength(0);
      if (req->getMethod() == Request::METHOD_GET &&
          (totalLength != 0 || !httpResponse->getHttpHeader()->defined(
                                   HttpHeader::CONTENT_LENGTH))) {
        // DownloadContext::knowsTotalLength() == true only when
        // server says the size of file is 0 explicitly.
        getDownloadContext()->markTotalLengthIsUnknown();
      }
      return handleOtherEncoding(std::move(httpResponse));
    }

    return handleDefaultEncoding(std::move(httpResponse));
  }

  if (!ctx->getHashType().empty() && httpHeader->defined(HttpHeader::DIGEST)) {
    std::vector<Checksum> checksums;
    httpResponse->getDigest(checksums);
    for (const auto& checksum : checksums) {
      if (checkChecksum(ctx, checksum)) {
        break;
      }
    }
  }

  // validate totalsize
  grp->validateTotalLength(fe->getLength(), httpResponse->getEntityLength());
  // update last modified time
  updateLastModifiedTime(httpResponse->getLastModifiedTime());

  if (grp->getTotalLength() == 0) {
    // Since total length is unknown, the file size in previously
    // failed download could be larger than the size this time.
    // Also we can't resume in this case too.  So truncate the file
    // anyway.
    getPieceStorage()->getDiskAdaptor()->truncate(0);
    auto teFilter = getTransferEncodingStreamFilter(
        httpResponse.get(), getContentEncodingStreamFilter(httpResponse.get()));
    getDownloadEngine()->addCommand(createHttpDownloadCommand(
        std::move(httpResponse), std::move(teFilter)));
  }
  else {
    auto teFilter = getTransferEncodingStreamFilter(httpResponse.get());
    getDownloadEngine()->addCommand(createHttpDownloadCommand(
        std::move(httpResponse), std::move(teFilter)));
  }

  return true;
}

void HttpResponseCommand::updateLastModifiedTime(const Time& lastModified)
{
  if (getOption()->getAsBool(PREF_REMOTE_TIME)) {
    getRequestGroup()->updateLastModifiedTime(lastModified);
  }
}

bool HttpResponseCommand::shouldInflateContentEncoding(
    HttpResponse* httpResponse)
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

bool HttpResponseCommand::handleDefaultEncoding(
    std::unique_ptr<HttpResponse> httpResponse)
{
  auto progressInfoFile = std::make_shared<DefaultBtProgressInfoFile>(
      getDownloadContext(), std::shared_ptr<PieceStorage>{}, getOption().get());
  getRequestGroup()->adjustFilename(progressInfoFile);
  getRequestGroup()->initPieceStorage();

  if (getOption()->getAsBool(PREF_DRY_RUN)) {
    onDryRunFileFound();
    return true;
  }

  auto checkEntry = getRequestGroup()->createCheckIntegrityEntry();
  if (!checkEntry) {
    return true;
  }

  File file(getRequestGroup()->getFirstFilePath());
  // We have to make sure that command that has Request object must
  // have segment after PieceStorage is initialized. See
  // AbstractCommand::execute()
  auto segment = getSegmentMan()->getSegmentWithIndex(getCuid(), 0);
  // pipelining requires implicit range specified. But the request for
  // this response most likely doesn't contains range header. This means
  // we can't continue to use this socket because server sends all entity
  // body instead of a segment.
  // Therefore, we shutdown the socket here if pipelining is enabled.
  if (getRequest()->getMethod() == Request::METHOD_GET && segment &&
      segment->getPositionToWrite() == 0 &&
      !getRequest()->isPipeliningEnabled()) {
    auto teFilter = getTransferEncodingStreamFilter(httpResponse.get());
    checkEntry->pushNextCommand(createHttpDownloadCommand(
        std::move(httpResponse), std::move(teFilter)));
  }
  else {
    getSegmentMan()->cancelSegment(getCuid());
    getFileEntry()->poolRequest(getRequest());
  }

  prepareForNextAction(std::move(checkEntry));

  if (getRequest()->getMethod() == Request::METHOD_HEAD) {
    poolConnection();
    getRequest()->setMethod(Request::METHOD_GET);
  }

  return true;
}

bool HttpResponseCommand::handleOtherEncoding(
    std::unique_ptr<HttpResponse> httpResponse)
{
  // We assume that RequestGroup::getTotalLength() == 0 here
  if (getOption()->getAsBool(PREF_DRY_RUN)) {
    getRequestGroup()->initPieceStorage();
    onDryRunFileFound();
    return true;
  }

  if (getRequest()->getMethod() == Request::METHOD_HEAD) {
    poolConnection();
    getRequest()->setMethod(Request::METHOD_GET);
    return prepareForRetry(0);
  }

  // In this context, knowsTotalLength() is true only when the file is
  // really zero-length.

  auto streamFilter = getTransferEncodingStreamFilter(
      httpResponse.get(), getContentEncodingStreamFilter(httpResponse.get()));
  // If chunked transfer-encoding is specified, we have to read end of
  // chunk markers(0\r\n\r\n, for example).
  bool chunkedUsed = streamFilter && streamFilter->getName() ==
                                         ChunkedDecodingStreamFilter::NAME;

  // For zero-length file, check existing file comparing its size
  if (!chunkedUsed && getDownloadContext()->knowsTotalLength() &&
      getRequestGroup()->downloadFinishedByFileLength()) {
    getRequestGroup()->initPieceStorage();

    // TODO Known issue: if .aria2 file exists, it will not be deleted
    // on successful verification, because .aria2 file is not loaded.
    // See also FtpNegotiationCommand::onFileSizeDetermined()
    if (getDownloadContext()->isChecksumVerificationNeeded()) {
      A2_LOG_DEBUG("Zero length file exists. Verify checksum.");
      auto entry = make_unique<ChecksumCheckIntegrityEntry>(getRequestGroup());
      entry->initValidator();
      getPieceStorage()->getDiskAdaptor()->openExistingFile();
      getDownloadEngine()->getCheckIntegrityMan()->pushEntry(std::move(entry));
    }
    else {
      getPieceStorage()->markAllPiecesDone();
      getDownloadContext()->setChecksumVerified(true);
      A2_LOG_NOTICE(fmt(MSG_DOWNLOAD_ALREADY_COMPLETED,
                        GroupId::toHex(getRequestGroup()->getGID()).c_str(),
                        getRequestGroup()->getFirstFilePath().c_str()));
    }
    poolConnection();
    return true;
  }

  getRequestGroup()->adjustFilename(std::make_shared<NullProgressInfoFile>());
  getRequestGroup()->initPieceStorage();
  getPieceStorage()->getDiskAdaptor()->initAndOpenFile();

  // Local file size becomes zero when DiskAdaptor::initAndOpenFile()
  // is called. So zero-length file is complete if chunked encoding is
  // not used.
  if (!chunkedUsed && getDownloadContext()->knowsTotalLength()) {
    A2_LOG_DEBUG("File length becomes zero and it means download completed.");
    // TODO Known issue: if .aria2 file exists, it will not be deleted
    // on successful verification, because .aria2 file is not loaded.
    // See also FtpNegotiationCommand::onFileSizeDetermined()
    if (getDownloadContext()->isChecksumVerificationNeeded()) {
      A2_LOG_DEBUG("Verify checksum for zero-length file");
      auto entry = make_unique<ChecksumCheckIntegrityEntry>(getRequestGroup());
      entry->initValidator();
      getDownloadEngine()->getCheckIntegrityMan()->pushEntry(std::move(entry));
    }
    else {
      getRequestGroup()->getPieceStorage()->markAllPiecesDone();
    }
    poolConnection();
    return true;
  }

  // We have to make sure that command that has Request object must
  // have segment after PieceStorage is initialized. See
  // AbstractCommand::execute()
  getSegmentMan()->getSegmentWithIndex(getCuid(), 0);

  getDownloadEngine()->addCommand(createHttpDownloadCommand(
      std::move(httpResponse), std::move(streamFilter)));
  return true;
}

bool HttpResponseCommand::skipResponseBody(
    std::unique_ptr<HttpResponse> httpResponse)
{
  auto filter = getTransferEncodingStreamFilter(httpResponse.get());
  // We don't use Content-Encoding here because this response body is just
  // thrown away.
  auto httpResponsePtr = httpResponse.get();
  auto command = make_unique<HttpSkipResponseCommand>(
      getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
      httpConnection_, std::move(httpResponse), getDownloadEngine(),
      getSocket());
  command->installStreamFilter(std::move(filter));

  // If request method is HEAD or the response body is zero-length,
  // set command's status to real time so that avoid read check blocking
  if (getRequest()->getMethod() == Request::METHOD_HEAD ||
      (httpResponsePtr->getEntityLength() == 0 &&
       !httpResponsePtr->isTransferEncodingSpecified())) {
    command->setStatusRealtime();
    // If entity length == 0, then socket read/write check must be disabled.
    command->disableSocketCheck();
    getDownloadEngine()->setNoWait(true);
  }

  getDownloadEngine()->addCommand(std::move(command));
  return true;
}

namespace {

bool decideFileAllocation(StreamFilter* filter)
{
#ifdef HAVE_ZLIB
  for (StreamFilter* f = filter; f; f = f->getDelegate().get()) {
    // Since the compressed file's length are returned in the response header
    // and the decompressed file size is unknown at this point, disable file
    // allocation here.
    if (f->getName() == GZipDecodingStreamFilter::NAME) {
      return false;
    }
  }
#endif // HAVE_ZLIB

  return true;
}

} // namespace

std::unique_ptr<HttpDownloadCommand>
HttpResponseCommand::createHttpDownloadCommand(
    std::unique_ptr<HttpResponse> httpResponse,
    std::unique_ptr<StreamFilter> filter)
{

  auto command = make_unique<HttpDownloadCommand>(
      getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
      std::move(httpResponse), httpConnection_, getDownloadEngine(),
      getSocket());
  command->setStartupIdleTime(
      std::chrono::seconds(getOption()->getAsInt(PREF_STARTUP_IDLE_TIME)));
  command->setLowestDownloadSpeedLimit(
      getOption()->getAsInt(PREF_LOWEST_SPEED_LIMIT));
  if (getRequestGroup()->isFileAllocationEnabled() &&
      !decideFileAllocation(filter.get())) {
    getRequestGroup()->setFileAllocationEnabled(false);
  }
  command->installStreamFilter(std::move(filter));
  getRequestGroup()->getURISelector()->tuneDownloadCommand(
      getFileEntry()->getRemainingUris(), command.get());

  return command;
}

void HttpResponseCommand::poolConnection()
{
  if (getRequest()->supportsPersistentConnection()) {
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

bool HttpResponseCommand::checkChecksum(
    const std::shared_ptr<DownloadContext>& dctx, const Checksum& checksum)
{
  if (dctx->getHashType() == checksum.getHashType()) {
    if (dctx->getDigest() != checksum.getDigest()) {
      throw DL_ABORT_EX("Invalid hash found in Digest header field.");
    }
    A2_LOG_INFO("Valid hash found in Digest header field.");
    return true;
  }

  return false;
}

} // namespace aria2
