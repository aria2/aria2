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
#ifndef D_COOKIE_STORAGE_H
#define D_COOKIE_STORAGE_H

#include "common.h"

#include <string>
#include <deque>
#include <vector>
#include <set>
#include <algorithm>
#include <unordered_map>

#include "a2time.h"
#include "Cookie.h"
#include "a2functional.h"

namespace aria2 {

class BufferedFile;

// This object represents one domain label.
class DomainNode {
public:
  DomainNode(std::string label, DomainNode* parent);
  // Stores the matching cookies in |out|. The |now| is used to update
  // the last access time of this node.
  void findCookie(std::vector<const Cookie*>& out,
                  const std::string& requestHost,
                  const std::string& requestPath, time_t now, bool secure);
  // Returns the number of cookies this node has.
  size_t countCookie() const;
  // Add |cookie| using update time |now|. Returns true if the
  // function succeeds.
  bool addCookie(std::unique_ptr<Cookie> cookie, time_t now);
  // Sets the last access time of this node.
  void setLastAccessTime(time_t lastAccessTime);
  // Returns the last access time of this node.
  time_t getLastAccessTime() const;
  // Sets the time |t| as a time used as key in LRU tracker.
  void setLruAccessTime(time_t t);
  time_t getLruAccessTime() const;

  bool writeCookie(BufferedFile& fp) const;
  // Returns true if this node contains the |cookie|.
  bool contains(const Cookie& cookie) const;
  // Returns true if this node contains no cookie.
  bool empty() const;
  // Returns true if this node has any next nodes.
  bool hasNext() const;
  // Returns the parent node. If this is the root node, returns
  // nullptr.
  DomainNode* getParent() const;
  // Removes the child node |node|. Nothing happens if |node| is not a
  // child of this node.
  void removeNode(DomainNode* node);
  // Returns the child node having label |label. Returns nullptr if
  // there is no such node.
  DomainNode* findNext(const std::string& label) const;
  // Add the |node| as a child using label |label and returns the raw
  // pointer of |node|.
  DomainNode* addNext(std::string label, std::unique_ptr<DomainNode> node);
  // Returns the |label|.
  const std::string& getLabel() const;
  // Deletes all cookies this node has.
  void clearCookie();
  // Returns value set by setInLru(). This is typically used to know
  // this node is tracked by LRU tracker or not.
  bool getInLru() const;
  void setInLru(bool f);

  template <typename OutputIterator>
  OutputIterator dumpCookie(OutputIterator out) const
  {
    if (cookies_) {
      for (auto& c : *cookies_) {
        out++ = c.get();
      }
    }
    return out;
  }

private:
  std::string label_;
  DomainNode* parent_;
  time_t lastAccessTime_;
  time_t lruAccessTime_;
  bool inLru_;
  std::unique_ptr<std::deque<std::unique_ptr<Cookie>>> cookies_;
  // domain label string to DomainNode
  // e.g. net, sourceforge
  // For numerical addresses, this is address itself.
  // e.g. 192.168.0.1
  std::unordered_map<std::string, std::unique_ptr<DomainNode>> next_;
};

class CookieStorage {
public:
  static const size_t MAX_COOKIE_PER_DOMAIN = 50;

private:
  // typedef std::set<std::shared_ptr<DomainEntry>,
  //                  DerefLess<std::shared_ptr<DomainEntry> > > DomainEntrySet;
  // DomainEntrySet domains_;

public:
  CookieStorage();

  // Returns true if cookie is stored or updated existing cookie.
  // Returns false if cookie is expired. now is used as last access
  // time.
  bool store(std::unique_ptr<Cookie> cookie, time_t now);

  // Returns true if cookie is stored or updated existing cookie.
  // Otherwise, returns false. now is used as creation time and last
  // access time.
  bool parseAndStore(const std::string& setCookieString,
                     const std::string& requestHost,
                     const std::string& requestPath, time_t now);

  // Finds cookies matched with given criteria and returns them.
  // Matched cookies' lastAccess_ property is updated.
  std::vector<const Cookie*> criteriaFind(const std::string& requestHost,
                                          const std::string& requestPath,
                                          time_t now, bool secure);

  // Loads Cookies from file denoted by filename.  If compiled with
  // libsqlite3, this method automatically detects the specified file
  // is sqlite3 or just plain text file and calls appropriate parser
  // implementation class.  If Cookies are successfully loaded, this
  // method returns true.  Otherwise, this method returns false.  now
  // is used as creation time and last access time.
  bool load(const std::string& filename, time_t now);

  // Saves Cookies in Netspace format which is used in
  // Firefox1.2/Netscape/Mozilla.  If Cookies are successfully saved,
  // this method returns true, otherwise returns false.
  bool saveNsFormat(const std::string& filename);

  // Returns the number of cookies this object stores.
  size_t size() const;

  // Returns true if this object contains a cookie x where x == cookie
  // satisfies.
  bool contains(const Cookie& cookie) const;

  template <typename OutputIterator>
  OutputIterator dumpCookie(OutputIterator out) const
  {
    for (auto& i : lruTracker_) {
      out = i.second->dumpCookie(out);
    }
    return out;
  }

  // Force eviction of delnum nodes. Exposed for unittest
  void evictNode(size_t delnum);
  // Returns size of LRU tracker. Exposed for unittest
  size_t getLruTrackerSize() const;
  // Returns root node. Exposed for unittest
  const DomainNode* getRootNode() const;

private:
  template <typename InputIterator>
  void storeCookies(InputIterator first, InputIterator last, time_t now)
  {
    for (; first != last; ++first) {
      store(*first, now);
    }
  }

  void updateLru(DomainNode* node, time_t now);

  // rootNode_ is a root node of tree structure of reversed domain
  // labels.  rootNode_ always contains no cookie. It has the child
  // nodes of the top level domain label (e.g., net, com and org). And
  // those top level domain nodes have 2nd domain label (e.g.,
  // sourceforge, github), and so on. The numeric host name are always
  // stored as a child node of rootNode_. So the domain name of a
  // particular node is constructed as follows. First traverse the
  // target node from root node. The concatenation of the visited
  // node's label in the reverse order, delimited by ".", is the
  std::unique_ptr<DomainNode> rootNode_;
  // This object tracks the node which has cookies or it once had. The
  // order is sorted by the least recent updated node first. This
  // object does not track the node which has not contain cookie. For
  // example, adding cookies in aria2.sourceforge.net, and no node
  // labeled "sourceforge" is present, only node labeled "aria2" is
  // tracked and node labeled "sourceforge" and "net" are not.
  std::set<std::pair<time_t, DomainNode*>> lruTracker_;
};

} // namespace aria2

#endif // D_COOKIE_STORAGE_H
