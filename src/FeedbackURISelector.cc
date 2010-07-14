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
#include "FeedbackURISelector.h"

#include <algorithm>

#include "ServerStatMan.h"
#include "ServerStat.h"
#include "Request.h"
#include "A2STR.h"
#include "FileEntry.h"

namespace aria2 {

FeedbackURISelector::FeedbackURISelector
(const SharedHandle<ServerStatMan>& serverStatMan):
  serverStatMan_(serverStatMan) {}

FeedbackURISelector::~FeedbackURISelector() {}

class ServerStatFaster {
public:
  bool operator()(const std::pair<SharedHandle<ServerStat>, std::string> lhs,
                  const std::pair<SharedHandle<ServerStat>, std::string> rhs)
    const
  {
    return lhs.first->getDownloadSpeed() > rhs.first->getDownloadSpeed();
  }
};

std::string FeedbackURISelector::select
(FileEntry* fileEntry, const std::vector<std::string>& usedHosts)
{
  if(fileEntry->getRemainingUris().empty()) {
    return A2STR::NIL;
  }
  // Select URI with usedHosts first. If no URI is selected, then do
  // it again without usedHosts.
  std::string uri = selectInternal(fileEntry->getRemainingUris(), usedHosts);
  if(uri.empty()) {
    uri = selectInternal
      (fileEntry->getRemainingUris(), std::vector<std::string>());
  } 
  if(!uri.empty()) {
    std::deque<std::string>& uris = fileEntry->getRemainingUris();
    uris.erase(std::find(uris.begin(), uris.end(), uri));
  }
  return uri;
}

std::string FeedbackURISelector::selectInternal
(const std::deque<std::string>& uris,
 const std::vector<std::string>& usedHosts)
{
  // Use first 10 good URIs to introduce some randomness.
  const size_t NUM_URI = 10;
  // Ignore low speed server
  const unsigned int SPEED_THRESHOLD = 20*1024;
  std::vector<std::pair<SharedHandle<ServerStat>, std::string> > fastCands;
  std::vector<std::string> normCands;
  for(std::deque<std::string>::const_iterator i = uris.begin(),
        eoi = uris.end(); i != eoi && fastCands.size() < NUM_URI; ++i) {
    Request r;
    r.setUri(*i);
    if(std::find(usedHosts.begin(), usedHosts.end(), r.getHost())
       != usedHosts.end()) {
      continue;
    }
    SharedHandle<ServerStat> ss =
      serverStatMan_->find(r.getHost(), r.getProtocol());
    if(!ss.isNull() && ss->isOK() && ss->getDownloadSpeed() > SPEED_THRESHOLD) {
      fastCands.push_back(std::make_pair(ss, *i));
    }
    if(ss.isNull() || ss->isOK()) {
      normCands.push_back(*i);
    }
  }
  if(fastCands.empty()) {
    if(normCands.empty()) {
      if(usedHosts.empty()) {
        // All URIs are inspected but aria2 cannot find usable one.
        // Return first URI anyway in this case.
        return uris.front();
      } else {
        // If usedHosts is not empty, there is a possibility it
        // includes usable host.
        return A2STR::NIL;
      }
    } else {
      return normCands.front();
    }
  } else {
    std::sort(fastCands.begin(), fastCands.end(), ServerStatFaster());
    return fastCands.front().second;
  }
}

} // namespace aria2
