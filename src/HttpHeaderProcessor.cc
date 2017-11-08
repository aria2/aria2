/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#include "HttpHeaderProcessor.h"

#include <vector>

#include "HttpHeader.h"
#include "message.h"
#include "util.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "A2STR.h"
#include "error_code.h"

namespace aria2 {

namespace {
enum {
  // Server mode
  PREV_METHOD,
  METHOD,
  PREV_PATH,
  PATH,
  PREV_REQ_VERSION,
  REQ_VERSION,
  // Client mode,
  PREV_RES_VERSION,
  RES_VERSION,
  PREV_STATUS_CODE,
  STATUS_CODE,
  PREV_REASON_PHRASE,
  REASON_PHRASE,
  // name/value header fields
  PREV_EOL,
  PREV_FIELD_NAME,
  FIELD_NAME,
  PREV_FIELD_VALUE,
  FIELD_VALUE,
  // End of header
  PREV_EOH,
  HEADERS_COMPLETE
};
} // namespace

HttpHeaderProcessor::HttpHeaderProcessor(ParserMode mode)
    : mode_(mode),
      state_(mode == CLIENT_PARSER ? PREV_RES_VERSION : PREV_METHOD),
      lastBytesProcessed_(0),
      lastFieldHdKey_(HttpHeader::MAX_INTERESTING_HEADER),
      result_(make_unique<HttpHeader>())
{
}

HttpHeaderProcessor::~HttpHeaderProcessor() = default;

namespace {
size_t getToken(std::string& buf, const unsigned char* data, size_t length,
                size_t off)
{
  size_t j = off;
  while (j < length && !util::isLws(data[j]) && !util::isCRLF(data[j])) {
    ++j;
  }
  buf.append(&data[off], &data[j]);
  return j - 1;
}
} // namespace

namespace {
size_t getFieldNameToken(std::string& buf, const unsigned char* data,
                         size_t length, size_t off)
{
  size_t j = off;
  while (j < length && data[j] != ':' && !util::isLws(data[j]) &&
         !util::isCRLF(data[j])) {
    ++j;
  }
  buf.append(&data[off], &data[j]);
  return j - 1;
}
} // namespace

namespace {
size_t getText(std::string& buf, const unsigned char* data, size_t length,
               size_t off)
{
  size_t j = off;
  while (j < length && !util::isCRLF(data[j])) {
    ++j;
  }
  buf.append(&data[off], &data[j]);
  return j - 1;
}
} // namespace

namespace {
size_t ignoreText(std::string& buf, const unsigned char* data, size_t length,
                  size_t off)
{
  size_t j = off;
  while (j < length && !util::isCRLF(data[j])) {
    ++j;
  }
  return j - 1;
}
} // namespace

bool HttpHeaderProcessor::parse(const unsigned char* data, size_t length)
{
  size_t i;
  lastBytesProcessed_ = 0;
  for (i = 0; i < length; ++i) {
    unsigned char c = data[i];
    switch (state_) {
    case PREV_METHOD:
      if (util::isLws(c) || util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing method");
      }

      i = getToken(buf_, data, length, i);
      state_ = METHOD;
      break;

    case METHOD:
      if (util::isLws(c)) {
        result_->setMethod(buf_);
        buf_.clear();
        state_ = PREV_PATH;
        break;
      }

      if (util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing request-target");
      }

      i = getToken(buf_, data, length, i);
      break;

    case PREV_PATH:
      if (util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing request-target");
      }

      if (util::isLws(c)) {
        break;
      }

      i = getToken(buf_, data, length, i);
      state_ = PATH;
      break;

    case PATH:
      if (util::isLws(c)) {
        result_->setRequestPath(buf_);
        buf_.clear();
        state_ = PREV_REQ_VERSION;
        break;
      }

      if (util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing HTTP-version");
      }

      i = getToken(buf_, data, length, i);
      break;

    case PREV_REQ_VERSION:
      if (util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing HTTP-version");
      }

      if (util::isLws(c)) {
        break;
      }

      i = getToken(buf_, data, length, i);
      state_ = REQ_VERSION;
      break;

    case REQ_VERSION:
      if (util::isCRLF(c)) {
        result_->setVersion(buf_);
        buf_.clear();
        state_ = c == '\n' ? PREV_FIELD_NAME : PREV_EOL;
        break;
      }

      if (util::isLws(c)) {
        throw DL_ABORT_EX("Bad Request-Line: LWS after HTTP-version");
      }

      i = getToken(buf_, data, length, i);
      break;

    case PREV_RES_VERSION:
      if (util::isLws(c) || util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Status-Line: missing HTTP-version");
      }

      i = getToken(buf_, data, length, i);
      state_ = RES_VERSION;
      break;

    case RES_VERSION:
      if (util::isLws(c)) {
        result_->setVersion(buf_);
        buf_.clear();
        state_ = PREV_STATUS_CODE;
        break;
      }

      if (util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Status-Line: missing status-code");
      }

      break;

    case PREV_STATUS_CODE:
      if (util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Status-Line: missing status-code");
      }

      if (!util::isLws(c)) {
        state_ = STATUS_CODE;
        i = getToken(buf_, data, length, i);
      }

      break;

    case STATUS_CODE:
      if (!util::isLws(c) && !util::isCRLF(c)) {
        i = getToken(buf_, data, length, i);
        break;
      }

      {
        int statusCode = -1;
        if (buf_.size() == 3 && util::isNumber(buf_.begin(), buf_.end())) {
          statusCode =
              (buf_[0] - '0') * 100 + (buf_[1] - '0') * 10 + (buf_[2] - '0');
        }
        if (statusCode < 100) {
          throw DL_ABORT_EX("Bad status code: bad status-code");
        }
        result_->setStatusCode(statusCode);
        buf_.clear();
      }
      if (c == '\r') {
        state_ = PREV_EOL;
        break;
      }

      if (c == '\n') {
        state_ = PREV_FIELD_NAME;
        break;
      }

      state_ = PREV_REASON_PHRASE;
      break;

    case PREV_REASON_PHRASE:
      if (util::isCRLF(c)) {
        // The reason-phrase is completely optional.
        state_ = c == '\n' ? PREV_FIELD_NAME : PREV_EOL;
        break;
      }

      if (util::isLws(c)) {
        break;
      }

      state_ = REASON_PHRASE;
      i = getText(buf_, data, length, i);
      break;

    case REASON_PHRASE:
      if (util::isCRLF(c)) {
        result_->setReasonPhrase(buf_);
        buf_.clear();
        state_ = c == '\n' ? PREV_FIELD_NAME : PREV_EOL;
        break;
      }

      i = getText(buf_, data, length, i);
      break;

    case PREV_EOL:
      if (c != '\n') {
        throw DL_ABORT_EX("Bad HTTP header: missing LF");
      }

      state_ = PREV_FIELD_NAME;
      break;

    case PREV_FIELD_NAME:
      if (util::isLws(c)) {
        if (lastFieldName_.empty()) {
          throw DL_ABORT_EX("Bad HTTP header: field name starts with LWS");
        }
        // Evil Multi-line header field
        state_ = FIELD_VALUE;
        break;
      }

      if (!lastFieldName_.empty()) {
        if (lastFieldHdKey_ != HttpHeader::MAX_INTERESTING_HEADER) {
          result_->put(lastFieldHdKey_, util::strip(buf_));
        }
        lastFieldName_.clear();
        lastFieldHdKey_ = HttpHeader::MAX_INTERESTING_HEADER;
        buf_.clear();
      }
      if (c == '\n') {
        state_ = HEADERS_COMPLETE;
        break;
      }

      if (c == '\r') {
        state_ = PREV_EOH;
        break;
      }

      if (c == ':') {
        throw DL_ABORT_EX("Bad HTTP header: field name starts with ':'");
      }

      state_ = FIELD_NAME;
      i = getFieldNameToken(lastFieldName_, data, length, i);
      break;

    case FIELD_NAME:
      if (util::isLws(c) || util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad HTTP header: missing ':'");
      }

      if (c == ':') {
        util::lowercase(lastFieldName_);
        lastFieldHdKey_ = idInterestingHeader(lastFieldName_.c_str());
        state_ = PREV_FIELD_VALUE;
        break;
      }

      i = getFieldNameToken(lastFieldName_, data, length, i);
      break;

    case PREV_FIELD_VALUE:
      if (c == '\r') {
        state_ = PREV_EOL;
        break;
      }

      if (c == '\n') {
        state_ = PREV_FIELD_NAME;
        break;
      }

      if (util::isLws(c)) {
        break;
      }

      state_ = FIELD_VALUE;
      if (lastFieldHdKey_ == HttpHeader::MAX_INTERESTING_HEADER) {
        i = ignoreText(buf_, data, length, i);
        break;
      }

      i = getText(buf_, data, length, i);
      break;

    case FIELD_VALUE:
      if (c == '\r') {
        state_ = PREV_EOL;
        break;
      }

      if (c == '\n') {
        state_ = PREV_FIELD_NAME;
        break;
      }

      if (lastFieldHdKey_ == HttpHeader::MAX_INTERESTING_HEADER) {
        i = ignoreText(buf_, data, length, i);
        break;
      }

      i = getText(buf_, data, length, i);
      break;

    case PREV_EOH:
      if (c != '\n') {
        throw DL_ABORT_EX("Bad HTTP header: "
                          "missing LF at the end of the header");
      }

      state_ = HEADERS_COMPLETE;
      break;

    case HEADERS_COMPLETE:
      goto fin;
    }
  }

fin:
  // See Apache's documentation
  // http://httpd.apache.org/docs/2.2/en/mod/core.html about size
  // limit of HTTP headers. The page states that the number of request
  // fields rarely exceeds 20.
  if (lastFieldName_.size() > 1024 || buf_.size() > 8_k) {
    throw DL_ABORT_EX("Too large HTTP header");
  }

  lastBytesProcessed_ = i;
  headers_.append(&data[0], &data[i]);

  if (state_ != HEADERS_COMPLETE) {
    return false;
  }

  // If both transfer-encoding and (content-length or content-range)
  // are present, delete content-length and content-range.  RFC 7230
  // says that sender must not send both transfer-encoding and
  // content-length.  If both present, transfer-encoding overrides
  // content-length.  There is no text about transfer-encoding and
  // content-range.  But there is no reason to send transfer-encoding
  // when range is set.
  if (result_->defined(HttpHeader::TRANSFER_ENCODING)) {
    result_->remove(HttpHeader::CONTENT_LENGTH);
    result_->remove(HttpHeader::CONTENT_RANGE);
  }

  return true;
}

bool HttpHeaderProcessor::parse(const std::string& data)
{
  return parse(reinterpret_cast<const unsigned char*>(data.c_str()),
               data.size());
}

size_t HttpHeaderProcessor::getLastBytesProcessed() const
{
  return lastBytesProcessed_;
}

void HttpHeaderProcessor::clear()
{
  state_ = (mode_ == CLIENT_PARSER ? PREV_RES_VERSION : PREV_METHOD);
  lastBytesProcessed_ = 0;
  buf_.clear();
  lastFieldName_.clear();
  lastFieldHdKey_ = HttpHeader::MAX_INTERESTING_HEADER;
  result_ = make_unique<HttpHeader>();
  headers_.clear();
}

std::unique_ptr<HttpHeader> HttpHeaderProcessor::getResult()
{
  return std::move(result_);
}

std::string HttpHeaderProcessor::getHeaderString() const { return headers_; }

} // namespace aria2
