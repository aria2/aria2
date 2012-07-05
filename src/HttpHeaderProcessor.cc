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
    result_(new HttpHeader())
{}

HttpHeaderProcessor::~HttpHeaderProcessor() {}

namespace {
size_t getToken(std::string& buf,
                const unsigned char* data, size_t length, size_t off)
{
  size_t j;
  for(j = off; j < length && !util::isLws(data[j]) && !util::isCRLF(data[j]);
      ++j);
  buf.append(&data[off], &data[j]);
  return j-1;
}
} // namespace

namespace {
size_t getFieldNameToken(std::string& buf,
                         const unsigned char* data, size_t length, size_t off)
{
  size_t j;
  for(j = off; j < length && data[j] != ':' &&
        !util::isLws(data[j]) && !util::isCRLF(data[j]); ++j);
  buf.append(&data[off], &data[j]);
  return j-1;
}
} // namespace

namespace {
size_t getText(std::string& buf,
               const unsigned char* data, size_t length, size_t off)
{
  size_t j;
  for(j = off; j < length && !util::isCRLF(data[j]); ++j);
  buf.append(&data[off], &data[j]);
  return j-1;
}
} // namespace

bool HttpHeaderProcessor::parse(const unsigned char* data, size_t length)
{
  size_t i;
  lastBytesProcessed_ = 0;
  for(i = 0; i < length; ++i) {
    unsigned char c = data[i];
    switch(state_) {
    case PREV_METHOD:
      if(util::isLws(c) || util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing method");
      } else {
        i = getToken(buf_, data, length, i);
        state_ = METHOD;
      }
      break;
    case METHOD:
      if(util::isLws(c)) {
        result_->setMethod(buf_);
        buf_.clear();
        state_ = PREV_PATH;
      } else if(util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing request-target");
      } else {
        i = getToken(buf_, data, length, i);
      }
      break;
    case PREV_PATH:
      if(util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing request-target");
      } else if(!util::isLws(c)) {
        i = getToken(buf_, data, length, i);
        state_ = PATH;
      }
      break;
    case PATH:
      if(util::isLws(c)) {
        result_->setRequestPath(buf_);
        buf_.clear();
        state_ = PREV_REQ_VERSION;
      } else if(util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing HTTP-version");
      } else {
        i = getToken(buf_, data, length, i);
      }
      break;
    case PREV_REQ_VERSION:
      if(util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Request-Line: missing HTTP-version");
      } else if(!util::isLws(c)) {
        i = getToken(buf_, data, length, i);
        state_ = REQ_VERSION;
      }
      break;
    case REQ_VERSION:
      if(util::isCRLF(c)) {
        result_->setVersion(buf_);
        buf_.clear();
        if(c == '\n') {
          state_ = PREV_FIELD_NAME;
        } else {
          state_ = PREV_EOL;
        }
      } else if(util::isLws(c)) {
        throw DL_ABORT_EX("Bad Request-Line: LWS after HTTP-version");
      } else {
        i = getToken(buf_, data, length, i);
      }
      break;
    case PREV_RES_VERSION:
      if(util::isLws(c) || util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Status-Line: missing HTTP-version");
      } else {
        i = getToken(buf_, data, length, i);
        state_ = RES_VERSION;
      }
      break;
    case RES_VERSION:
      if(util::isLws(c)) {
        result_->setVersion(buf_);
        buf_.clear();
        state_ = PREV_STATUS_CODE;
      } else if(util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Status-Line: missing status-code");
      }
      break;
    case PREV_STATUS_CODE:
      if(util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad Status-Line: missing status-code");
      } else if(!util::isLws(c)) {
        state_ = STATUS_CODE;
        i = getToken(buf_, data, length, i);
      }
      break;
    case STATUS_CODE:
      if(util::isLws(c) || util::isCRLF(c)) {
        int statusCode = -1;
        if(buf_.size() == 3 && util::isNumber(buf_.begin(), buf_.end())) {
          statusCode = (buf_[0]-'0')*100 + (buf_[1]-'0')*10 + (buf_[2]-'0');
        }
        if(statusCode >= 100) {
          result_->setStatusCode(statusCode);
          buf_.clear();
        } else {
          throw DL_ABORT_EX("Bad status code: bad status-code");
        }
        if(c == '\r') {
          state_ = PREV_EOL;
        } else if(c == '\n') {
          state_ = PREV_FIELD_NAME;
        } else {
          state_ = PREV_REASON_PHRASE;
        }
      } else {
        i = getToken(buf_, data, length, i);
      }
      break;
    case PREV_REASON_PHRASE:
      if(util::isCRLF(c)) {
        // The reason-phrase is completely optional.
        if(c == '\n') {
          state_ = PREV_FIELD_NAME;
        } else {
          state_ = PREV_EOL;
        }
      } else if(!util::isLws(c)) {
        state_ = REASON_PHRASE;
        i = getText(buf_, data, length, i);
      }
      break;
    case REASON_PHRASE:
      if(util::isCRLF(c)) {
        result_->setReasonPhrase(buf_);
        buf_.clear();
        if(c == '\n') {
          state_ = PREV_FIELD_NAME;
        } else {
          state_ = PREV_EOL;
        }
      } else {
        i = getText(buf_, data, length, i);
      }
      break;
    case PREV_EOL:
      if(c == '\n') {
        state_ = PREV_FIELD_NAME;
      } else {
        throw DL_ABORT_EX("Bad HTTP header: missing LF");
      }
      break;
    case PREV_FIELD_NAME:
      if(util::isLws(c)) {
        if(lastFieldName_.empty()) {
          throw DL_ABORT_EX("Bad HTTP header: field name starts with LWS");
        }
        // Evil Multi-line header field
        state_ = FIELD_VALUE;
      } else {
        if(!lastFieldName_.empty()) {
          util::lowercase(lastFieldName_);
          result_->put(lastFieldName_, util::strip(buf_));
          lastFieldName_.clear();
          buf_.clear();
        }
        if(c == '\n') {
          state_ = HEADERS_COMPLETE;
        } else if(c == '\r') {
          state_ = PREV_EOH;
        } else if(c == ':') {
          throw DL_ABORT_EX("Bad HTTP header: field name starts with ':'");
        } else {
          state_ = FIELD_NAME;
          i = getFieldNameToken(lastFieldName_, data, length, i);
        }
      }
      break;
    case FIELD_NAME:
      if(util::isLws(c) || util::isCRLF(c)) {
        throw DL_ABORT_EX("Bad HTTP header: missing ':'");
      } else if(c == ':') {
        state_ = PREV_FIELD_VALUE;
      } else {
        i = getFieldNameToken(lastFieldName_, data, length, i);
      }
      break;
    case PREV_FIELD_VALUE:
      if(c == '\r') {
        state_ = PREV_EOL;
      } else if(c == '\n') {
        state_ = PREV_FIELD_NAME;
      } else if(!util::isLws(c)) {
        state_ = FIELD_VALUE;
        i = getText(buf_, data, length, i);
      }
      break;
    case FIELD_VALUE:
      if(c == '\r') {
        state_ = PREV_EOL;
      } else if(c == '\n') {
        state_ = PREV_FIELD_NAME;
      } else {
        i = getText(buf_, data, length, i);
      }
      break;
    case PREV_EOH:
      if(c == '\n') {
        state_ = HEADERS_COMPLETE;
      } else {
        throw DL_ABORT_EX("Bad HTTP header: "
                          "missing LF at the end of the header");
      }
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
  if(lastFieldName_.size() > 1024 || buf_.size() > 8192) {
    throw DL_ABORT_EX("Too large HTTP header");
  }
  lastBytesProcessed_ = i;
  headers_.append(&data[0], &data[i]);
  return state_ == HEADERS_COMPLETE;
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
  result_.reset(new HttpHeader());
  headers_.clear();
}

const SharedHandle<HttpHeader>& HttpHeaderProcessor::getResult() const
{
  return result_;
}

std::string HttpHeaderProcessor::getHeaderString() const
{
  return headers_;
}

} // namespace aria2
