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
#include "Request.h"

#include <cassert>
#include <utility>

#include "util.h"
#include "fmt.h"
#include "A2STR.h"
#include "uri.h"
#include "PeerStat.h"
#include "wallclock.h"

namespace aria2 {

const std::string Request::METHOD_GET = "GET";

const std::string Request::METHOD_HEAD = "HEAD";

const std::string Request::DEFAULT_FILE = "index.html";

Request::Request()
    : method_(METHOD_GET),
      tryCount_(0),
      redirectCount_(0),
      supportsPersistentConnection_(true),
      keepAliveHint_(false),
      pipeliningHint_(false),
      maxPipelinedRequest_(1),
      removalRequested_(false),
      connectedPort_(0),
      wakeTime_(global::wallclock())
{
}

Request::~Request() = default;

namespace {
std::string removeFragment(const std::string& uri)
{
  std::string::size_type sharpIndex = uri.find("#");
  if (sharpIndex == std::string::npos) {
    return uri;
  }
  else {
    return uri.substr(0, sharpIndex);
  }
}
} // namespace

bool Request::setUri(const std::string& uri)
{
  supportsPersistentConnection_ = true;
  uri_ = uri;
  return parseUri(uri_);
}

bool Request::resetUri()
{
  supportsPersistentConnection_ = true;
  setConnectedAddrInfo(A2STR::NIL, A2STR::NIL, 0);
  return parseUri(uri_);
}

void Request::setReferer(const std::string& uri)
{
  referer_ = removeFragment(uri);
}

bool Request::redirectUri(const std::string& uri)
{
  supportsPersistentConnection_ = true;
  ++redirectCount_;
  if (uri.empty()) {
    return false;
  }
  std::string redirectedUri;
  if (util::startsWith(uri, "//")) {
    // Network-path reference (according to RFC 3986, Section 4.2)
    // Just complement current protocol.
    redirectedUri = getProtocol();
    redirectedUri += ":";
    redirectedUri += uri;
  }
  else {
    std::string::size_type schemeEnd = uri.find("://");
    bool absUri;
    if (schemeEnd == std::string::npos) {
      absUri = false;
    }
    else {
      absUri = true;
      // Check that scheme is acceptable one.
      for (size_t i = 0; i < schemeEnd; ++i) {
        char c = uri[i];
        if (!util::isAlpha(c) && !util::isDigit(c) && c != '+' && c != '-' &&
            c != '.') {
          absUri = false;
          break;
        }
      }
    }
    if (absUri) {
      redirectedUri = uri;
    }
    else {
      // rfc2616 requires absolute URI should be provided by Location header
      // field, but some servers don't obey this rule.
      // UPDATE: draft-ietf-httpbis-p2-semantics-18 now allows this.
      redirectedUri = uri::joinUri(currentUri_, uri);
    }
  }
  return parseUri(redirectedUri);
}

bool Request::parseUri(const std::string& srcUri)
{
  currentUri_ = removeFragment(srcUri);
  uri::UriStruct us;
  if (uri::parse(us, currentUri_)) {
    us_.swap(us);
    return true;
  }
  else {
    return false;
  }
}

void Request::resetRedirectCount() { redirectCount_ = 0; }

void Request::setMaxPipelinedRequest(int num) { maxPipelinedRequest_ = num; }

const std::shared_ptr<PeerStat>& Request::initPeerStat()
{
  // Use host and protocol in original URI, because URI selector
  // selects URI based on original URI, not redirected one.
  uri_split_result us;
  int v = uri_split(&us, uri_.c_str());
  assert(v == 0);
  std::string host = uri::getFieldString(us, USR_HOST, uri_.c_str());
  std::string protocol = uri::getFieldString(us, USR_SCHEME, uri_.c_str());
  peerStat_ = std::make_shared<PeerStat>(0, host, protocol);
  return peerStat_;
}

std::string Request::getURIHost() const
{
  if (isIPv6LiteralAddress()) {
    std::string s = "[";
    s += getHost();
    s += "]";
    return s;
  }
  else {
    return getHost();
  }
}

void Request::setMethod(const std::string& method) { method_ = method; }

void Request::setConnectedAddrInfo(const std::string& hostname,
                                   const std::string& addr, uint16_t port)
{
  connectedHostname_ = hostname;
  connectedAddr_ = addr;
  connectedPort_ = port;
}

} // namespace aria2
