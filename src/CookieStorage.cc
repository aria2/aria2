/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "CookieStorage.h"

#include <cstring>
#include <cstdio>
#include <algorithm>

#include "util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "NsCookieParser.h"
#include "File.h"
#include "a2functional.h"
#include "A2STR.h"
#include "message.h"
#include "cookie_helper.h"
#include "BufferedFile.h"
#ifdef HAVE_SQLITE3
#  include "Sqlite3CookieParserImpl.h"
#endif // HAVE_SQLITE3

namespace aria2 {

DomainNode::DomainNode(std::string label, DomainNode* parent)
    : label_{std::move(label)},
      parent_{parent},
      lastAccessTime_{0},
      lruAccessTime_{0},
      inLru_{false}
{
}

void DomainNode::findCookie(std::vector<const Cookie*>& out,
                            const std::string& requestHost,
                            const std::string& requestPath, time_t now,
                            bool secure)
{
  if (cookies_) {
    for (auto& c : *cookies_) {
      if (c->match(requestHost, requestPath, now, secure)) {
        c->setLastAccessTime(now);
        out.push_back(c.get());
      }
    }
  }
}

bool DomainNode::addCookie(std::unique_ptr<Cookie> cookie, time_t now)
{
  using namespace std::placeholders;
  setLastAccessTime(now);
  if (!cookies_) {
    if (cookie->isExpired(now)) {
      return false;
    }
    else {
      cookies_ = make_unique<std::deque<std::unique_ptr<Cookie>>>();
      cookies_->push_back(std::move(cookie));
      return true;
    }
  }

  auto i = std::find_if(
      std::begin(*cookies_), std::end(*cookies_),
      [&](const std::unique_ptr<Cookie>& c) { return *c == *cookie; });
  if (i == std::end(*cookies_)) {
    if (cookie->isExpired(now)) {
      return false;
    }
    else {
      if (cookies_->size() >= CookieStorage::MAX_COOKIE_PER_DOMAIN) {
        cookies_->erase(std::remove_if(std::begin(*cookies_),
                                       std::end(*cookies_),
                                       std::bind(&Cookie::isExpired, _1, now)),
                        std::end(*cookies_));
        if (cookies_->size() >= CookieStorage::MAX_COOKIE_PER_DOMAIN) {
          auto m = std::min_element(std::begin(*cookies_), std::end(*cookies_),
                                    [](const std::unique_ptr<Cookie>& lhs,
                                       const std::unique_ptr<Cookie>& rhs) {
                                      return lhs->getLastAccessTime() <
                                             rhs->getLastAccessTime();
                                    });
          *m = std::move(cookie);
        }
        else {
          cookies_->push_back(std::move(cookie));
        }
      }
      else {
        cookies_->push_back(std::move(cookie));
      }
      return true;
    }
  }
  else if (cookie->isExpired(now)) {
    cookies_->erase(i);
    return false;
  }
  else {
    cookie->setCreationTime((*i)->getCreationTime());
    *i = std::move(cookie);
    return true;
  }
}

bool DomainNode::contains(const Cookie& cookie) const
{
  if (cookies_) {
    for (auto& i : *cookies_) {
      if (*i == cookie) {
        return true;
      }
    }
  }
  return false;
}

bool DomainNode::writeCookie(BufferedFile& fp) const
{
  if (cookies_) {
    for (const auto& c : *cookies_) {
      std::string data = c->toNsCookieFormat();
      data += "\n";
      if (fp.write(data.data(), data.size()) != data.size()) {
        return false;
      }
    }
  }
  return true;
}

size_t DomainNode::countCookie() const
{
  if (cookies_) {
    return cookies_->size();
  }
  else {
    return 0;
  }
}

void DomainNode::clearCookie() { cookies_->clear(); }

void DomainNode::setLastAccessTime(time_t lastAccessTime)
{
  lastAccessTime_ = lastAccessTime;
}

time_t DomainNode::getLastAccessTime() const { return lastAccessTime_; }

void DomainNode::setLruAccessTime(time_t t) { lruAccessTime_ = t; }

time_t DomainNode::getLruAccessTime() const { return lruAccessTime_; }

bool DomainNode::empty() const { return !cookies_ || cookies_->empty(); }

bool DomainNode::hasNext() const { return !next_.empty(); }

DomainNode* DomainNode::getParent() const { return parent_; }

void DomainNode::removeNode(DomainNode* node) { next_.erase(node->getLabel()); }

DomainNode* DomainNode::findNext(const std::string& label) const
{
  auto i = next_.find(label);
  if (i == std::end(next_)) {
    return nullptr;
  }
  else {
    return (*i).second.get();
  }
}

DomainNode* DomainNode::addNext(std::string label,
                                std::unique_ptr<DomainNode> node)
{
  auto& res = next_[std::move(label)] = std::move(node);
  return res.get();
}

const std::string& DomainNode::getLabel() const { return label_; }

bool DomainNode::getInLru() const { return inLru_; }

void DomainNode::setInLru(bool f) { inLru_ = f; }

CookieStorage::CookieStorage() : rootNode_{make_unique<DomainNode>("", nullptr)}
{
}

namespace {
// See CookieStorageTest::testDomainIsFull() in CookieStorageTest.cc
const size_t DOMAIN_EVICTION_TRIGGER = 2000;

const double DOMAIN_EVICTION_RATE = 0.1;
} // namespace

namespace {
std::vector<std::string> splitDomainLabel(const std::string& domain)
{
  auto labels = std::vector<std::string>{};
  if (util::isNumericHost(domain)) {
    labels.push_back(domain);
  }
  else {
    util::split(std::begin(domain), std::end(domain),
                std::back_inserter(labels), '.');
  }
  return labels;
}
} // namespace

size_t CookieStorage::getLruTrackerSize() const { return lruTracker_.size(); }

void CookieStorage::evictNode(size_t delnum)
{
  for (; delnum > 0 && !lruTracker_.empty(); --delnum) {
    auto node = (*lruTracker_.begin()).second;
    lruTracker_.erase(lruTracker_.begin());
    node->setInLru(false);
    node->clearCookie();
    while (node->empty() && !node->hasNext()) {
      auto parent = node->getParent();
      parent->removeNode(node);
      if (!parent->empty() || parent->hasNext() || parent == rootNode_.get()) {
        break;
      }
      node = parent;
      if (node->getInLru()) {
        lruTracker_.erase({node->getLruAccessTime(), node});
        node->setInLru(false);
      }
    }
  }
}

const DomainNode* CookieStorage::getRootNode() const { return rootNode_.get(); }

bool CookieStorage::store(std::unique_ptr<Cookie> cookie, time_t now)
{
  if (lruTracker_.size() >= DOMAIN_EVICTION_TRIGGER) {
    auto delnum = size_t(lruTracker_.size() * DOMAIN_EVICTION_RATE);
    evictNode(delnum);
  }
  auto labels = splitDomainLabel(cookie->getDomain());
  auto node = rootNode_.get();
  for (auto i = labels.rbegin(), eoi = labels.rend(); i != eoi; ++i) {
    auto nextNode = node->findNext(*i);
    if (nextNode) {
      node = nextNode;
    }
    else {
      node = node->addNext(*i, make_unique<DomainNode>(*i, node));
    }
  }
  bool ok = node->addCookie(std::move(cookie), now);
  if (ok) {
    updateLru(node, now);
  }
  return ok;
}

void CookieStorage::updateLru(DomainNode* node, time_t now)
{
  if (node->getInLru()) {
    lruTracker_.erase({node->getLruAccessTime(), node});
  }
  else {
    node->setInLru(true);
  }
  node->setLruAccessTime(now);
  lruTracker_.insert({node->getLruAccessTime(), node});
}

bool CookieStorage::parseAndStore(const std::string& setCookieString,
                                  const std::string& requestHost,
                                  const std::string& defaultPath, time_t now)
{
  auto cookie = cookie::parse(setCookieString, requestHost, defaultPath, now);
  return cookie && store(std::move(cookie), now);
}

namespace {
struct CookiePathDivider {
  const Cookie* cookie_;
  int pathDepth_;
  CookiePathDivider(const Cookie* cookie) : cookie_(cookie), pathDepth_(0)
  {
    const std::string& path = cookie_->getPath();
    if (!path.empty()) {
      for (size_t i = 1, len = path.size(); i < len; ++i) {
        if (path[i] == '/' && path[i - 1] != '/') {
          ++pathDepth_;
        }
      }
      if (path[path.size() - 1] != '/') {
        ++pathDepth_;
      }
    }
  }
};
} // namespace

namespace {
class CookiePathDividerConverter {
public:
  CookiePathDivider operator()(const Cookie* cookie) const
  {
    return CookiePathDivider(cookie);
  }

  const Cookie* operator()(const CookiePathDivider& cookiePathDivider) const
  {
    return cookiePathDivider.cookie_;
  }
};
} // namespace

namespace {
class OrderByPathDepthDesc : public std::binary_function<Cookie, Cookie, bool> {
public:
  bool operator()(const CookiePathDivider& lhs,
                  const CookiePathDivider& rhs) const
  {
    // From http://tools.ietf.org/html/rfc6265#section-5.4:
    // 2.  The user agent SHOULD sort the cookie-list in the following
    //    order:
    //
    //    *  Cookies with longer paths are listed before cookies with
    //       shorter paths.
    //
    //    *  Among cookies that have equal-length path fields, cookies with
    //       earlier creation-times are listed before cookies with later
    //       creation-times.
    return lhs.pathDepth_ > rhs.pathDepth_ ||
           (!(rhs.pathDepth_ > lhs.pathDepth_) &&
            lhs.cookie_->getCreationTime() < rhs.cookie_->getCreationTime());
  }
};
} // namespace

namespace {
DomainNode* findNode(const std::string& domain, DomainNode* node)
{
  auto labels = splitDomainLabel(domain);
  for (auto i = labels.rbegin(), eoi = labels.rend(); i != eoi && node; ++i) {
    node = node->findNext(*i);
  }
  return node;
}
} // namespace

bool CookieStorage::contains(const Cookie& cookie) const
{
  auto node = findNode(cookie.getDomain(), rootNode_.get());
  return node && node->contains(cookie);
}

std::vector<const Cookie*>
CookieStorage::criteriaFind(const std::string& requestHost,
                            const std::string& requestPath, time_t now,
                            bool secure)
{
  auto res = std::vector<const Cookie*>{};
  if (requestPath.empty()) {
    return res;
  }
  auto labels = splitDomainLabel(requestHost);
  auto node = rootNode_.get();
  for (auto i = labels.rbegin(), eoi = labels.rend(); i != eoi; ++i) {
    auto nextNode = node->findNext(*i);
    if (!nextNode) {
      break;
    }
    nextNode->setLastAccessTime(now);
    if (nextNode->getInLru()) {
      updateLru(nextNode, now);
    }
    nextNode->findCookie(res, requestHost, requestPath, now, secure);
    node = nextNode;
  }
  auto divs = std::vector<CookiePathDivider>{};
  std::transform(std::begin(res), std::end(res), std::back_inserter(divs),
                 CookiePathDividerConverter{});
  std::sort(std::begin(divs), std::end(divs), OrderByPathDepthDesc{});
  std::transform(std::begin(divs), std::end(divs), std::begin(res),
                 CookiePathDividerConverter{});
  return res;
}

size_t CookieStorage::size() const
{
  size_t n = 0;
  for (auto& p : lruTracker_) {
    n += p.second->countCookie();
  }
  return n;
}

bool CookieStorage::load(const std::string& filename, time_t now)
{
  char header[16]; // "SQLite format 3" plus \0
  size_t headlen;
  {
    BufferedFile fp{filename.c_str(), BufferedFile::READ};
    if (!fp) {
      A2_LOG_ERROR(fmt("Failed to open cookie file %s", filename.c_str()));
      return false;
    }
    headlen = fp.read(header, sizeof(header));
  }
  try {
    if (headlen == 16 && memcmp(header, "SQLite format 3\0", 16) == 0) {
#ifdef HAVE_SQLITE3
      try {
        auto cookies = Sqlite3MozCookieParser(filename).parse();
        storeCookies(std::make_move_iterator(std::begin(cookies)),
                     std::make_move_iterator(std::end(cookies)), now);
      }
      catch (RecoverableException& e) {
        A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
        A2_LOG_INFO("This does not look like Firefox3 cookie file."
                    " Retrying, assuming it is Chromium cookie file.");
        // Try chrome cookie format
        auto cookies = Sqlite3ChromiumCookieParser(filename).parse();
        storeCookies(std::make_move_iterator(std::begin(cookies)),
                     std::make_move_iterator(std::end(cookies)), now);
      }
#else  // !HAVE_SQLITE3
      throw DL_ABORT_EX(
          "Cannot read SQLite3 database because SQLite3 support is disabled by"
          " configuration.");
#endif // !HAVE_SQLITE3
    }
    else {
      auto cookies = NsCookieParser().parse(filename, now);
      storeCookies(std::make_move_iterator(std::begin(cookies)),
                   std::make_move_iterator(std::end(cookies)), now);
    }
    return true;
  }
  catch (RecoverableException& e) {
    A2_LOG_ERROR(fmt("Failed to load cookies from %s", filename.c_str()));
    return false;
  }
}

bool CookieStorage::saveNsFormat(const std::string& filename)
{
  auto tempfilename = filename;
  tempfilename += "__temp";
  {
    BufferedFile fp{tempfilename.c_str(), BufferedFile::WRITE};
    if (!fp) {
      A2_LOG_ERROR(fmt("Cannot create cookie file %s", filename.c_str()));
      return false;
    }
    for (auto& p : lruTracker_) {
      if (!p.second->writeCookie(fp)) {
        A2_LOG_ERROR(fmt("Failed to save cookies to %s", filename.c_str()));
        return false;
      }
    }
    if (fp.close() == EOF) {
      A2_LOG_ERROR(fmt("Failed to save cookies to %s", filename.c_str()));
      return false;
    }
  }
  if (File(tempfilename).renameTo(filename)) {
    return true;
  }
  else {
    A2_LOG_ERROR(fmt("Could not rename file %s as %s", tempfilename.c_str(),
                     filename.c_str()));
    return false;
  }
}

} // namespace aria2
