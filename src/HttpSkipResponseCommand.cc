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
#include "HttpSkipResponseCommand.h"
#include "HttpConnection.h"
#include "HttpResponse.h"
#include "message.h"
#include "SocketCore.h"
#include "DlRetryEx.h"
#include "Request.h"
#include "DownloadEngine.h"
#include "Logger.h"
#include "LogFactory.h"
#include "HttpRequest.h"
#include "Segment.h"
#include "util.h"
#include "fmt.h"
#include "DlAbortEx.h"
#include "HttpHeader.h"
#include "prefs.h"
#include "Option.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "DownloadContext.h"
#include "StreamFilter.h"
#include "BinaryStream.h"
#include "NullSinkStreamFilter.h"
#include "SinkStreamFilter.h"
#include "error_code.h"
#include "SocketRecvBuffer.h"

namespace aria2 {

HttpSkipResponseCommand::HttpSkipResponseCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    const std::shared_ptr<HttpConnection>& httpConnection,
    std::unique_ptr<HttpResponse> httpResponse, DownloadEngine* e,
    const std::shared_ptr<SocketCore>& s)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e, s,
                      httpConnection->getSocketRecvBuffer()),
      sinkFilterOnly_(true),
      totalLength_(httpResponse->getEntityLength()),
      receivedBytes_(0),
      httpConnection_(httpConnection),
      httpResponse_(std::move(httpResponse)),
      streamFilter_(make_unique<NullSinkStreamFilter>())
{
  checkSocketRecvBuffer();
}

HttpSkipResponseCommand::~HttpSkipResponseCommand() {}

void HttpSkipResponseCommand::installStreamFilter(
    std::unique_ptr<StreamFilter> streamFilter)
{
  if (!streamFilter) {
    return;
  }
  streamFilter->installDelegate(std::move(streamFilter_));
  streamFilter_ = std::move(streamFilter);
  const std::string& name = streamFilter_->getName();
  sinkFilterOnly_ = util::endsWith(name, SinkStreamFilter::NAME);
}

bool HttpSkipResponseCommand::executeInternal()
{
  if (getRequest()->getMethod() == Request::METHOD_HEAD ||
      (totalLength_ == 0 && sinkFilterOnly_)) {
    // If request method is HEAD or content-length header is present and
    // it's value is 0, then pool socket for reuse.
    // If content-length header is not present, then EOF is expected in the end.
    // In this case, the content is thrown away and socket cannot be pooled.
    if (getRequest()->getMethod() == Request::METHOD_HEAD ||
        httpResponse_->getHttpHeader()->defined(HttpHeader::CONTENT_LENGTH)) {
      poolConnection();
    }
    return processResponse();
  }
  bool eof = false;
  try {
    size_t bufSize;
    if (getSocketRecvBuffer()->bufferEmpty()) {
      eof = getSocketRecvBuffer()->recv() == 0 && !getSocket()->wantRead() &&
            !getSocket()->wantWrite();
    }
    if (!eof) {
      if (sinkFilterOnly_) {
        if (totalLength_ > 0) {
          bufSize = std::min(
              totalLength_ - receivedBytes_,
              static_cast<int64_t>(getSocketRecvBuffer()->getBufferLength()));
        }
        else {
          bufSize = getSocketRecvBuffer()->getBufferLength();
        }
        receivedBytes_ += bufSize;
      }
      else {
        // receivedBytes_ is not updated if transferEncoding is set.
        // The return value is safely ignored here.
        streamFilter_->transform(std::shared_ptr<BinaryStream>(),
                                 std::shared_ptr<Segment>(),
                                 getSocketRecvBuffer()->getBuffer(),
                                 getSocketRecvBuffer()->getBufferLength());
        bufSize = streamFilter_->getBytesProcessed();
      }
      getSocketRecvBuffer()->drain(bufSize);
    }
    if (totalLength_ != 0 && eof) {
      throw DL_RETRY_EX(EX_GOT_EOF);
    }
  }
  catch (RecoverableException& e) {
    A2_LOG_DEBUG_EX(EX_EXCEPTION_CAUGHT, e)
    return processResponse();
  }

  if (eof) {
    // we may get EOF before non-sink streamFilter reports its
    // completion. There are some broken servers to prevent
    // streamFilter from completion. Since we just discard the
    // response body anyway, so we assume that the response is
    // completed.
    return processResponse();
  }
  bool finished = false;
  if (sinkFilterOnly_) {
    finished = (totalLength_ == receivedBytes_);
  }
  else {
    finished = streamFilter_->finished();
  }
  if (finished) {
    if (getSegments().size() <= 1) {
      // Don't pool connection if the command has multiple
      // segments. This means it did HTTP pipelined request. If this
      // response is for the first request, then successive response
      // may arrived to the socket.
      poolConnection();
    }
    return processResponse();
  }
  else {
    setWriteCheckSocketIf(getSocket(), getSocket()->wantWrite());
    addCommandSelf();
    return false;
  }
}

void HttpSkipResponseCommand::poolConnection() const
{
  if (getRequest()->supportsPersistentConnection()) {
    getDownloadEngine()->poolSocket(getRequest(), createProxyRequest(),
                                    getSocket());
  }
}

bool HttpSkipResponseCommand::processResponse()
{
  if (httpResponse_->isRedirect()) {
    int rnum =
        httpResponse_->getHttpRequest()->getRequest()->getRedirectCount();
    if (rnum >= Request::MAX_REDIRECT) {
      throw DL_ABORT_EX2(fmt("Too many redirects: count=%u", rnum),
                         error_code::HTTP_TOO_MANY_REDIRECTS);
    }
    httpResponse_->processRedirect();
    return prepareForRetry(0);
  }

  auto statusCode = httpResponse_->getStatusCode();
  if (statusCode >= 400) {
    switch (statusCode) {
    case 401:
      if (getOption()->getAsBool(PREF_HTTP_AUTH_CHALLENGE) &&
          !httpResponse_->getHttpRequest()->authenticationUsed() &&
          getDownloadEngine()->getAuthConfigFactory()->activateBasicCred(
              getRequest()->getHost(), getRequest()->getPort(),
              getRequest()->getDir(), getOption().get())) {
        return prepareForRetry(0);
      }
      throw DL_ABORT_EX2(EX_AUTH_FAILED, error_code::HTTP_AUTH_FAILED);
    case 404:
      if (getOption()->getAsInt(PREF_MAX_FILE_NOT_FOUND) == 0) {
        throw DL_ABORT_EX2(MSG_RESOURCE_NOT_FOUND,
                           error_code::RESOURCE_NOT_FOUND);
      }
      throw DL_RETRY_EX2(MSG_RESOURCE_NOT_FOUND,
                         error_code::RESOURCE_NOT_FOUND);
    case 503:
      // Only retry if pretry-wait > 0. Hammering 'busy' server is not
      // a good idea.
      if (getOption()->getAsInt(PREF_RETRY_WAIT) > 0) {
        throw DL_RETRY_EX2(fmt(EX_BAD_STATUS, statusCode),
                           error_code::HTTP_SERVICE_UNAVAILABLE);
      }
      throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, statusCode),
                         error_code::HTTP_SERVICE_UNAVAILABLE);
    case 504:
      // This is Gateway Timeout, so try again
      throw DL_RETRY_EX2(fmt(EX_BAD_STATUS, statusCode),
                         error_code::HTTP_SERVICE_UNAVAILABLE);
    };

    throw DL_ABORT_EX2(fmt(EX_BAD_STATUS, statusCode),
                       error_code::HTTP_PROTOCOL_ERROR);
  }

  return prepareForRetry(0);
}

void HttpSkipResponseCommand::disableSocketCheck()
{
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

} // namespace aria2
