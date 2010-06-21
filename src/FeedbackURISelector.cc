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

std::string FeedbackURISelector::select(FileEntry* fileEntry)
{
  std::deque<std::string>& uris = fileEntry->getRemainingUris();
  if(uris.empty()) {
    return A2STR::NIL;
  }
  // Use first 10 URIs to introduce some randomness.
  const int NUM_URI = 10;
  // Ignore low speed server
  const unsigned int SPEED_THRESHOLD = 20*1024;
  size_t max = std::min(uris.size(), static_cast<size_t>(NUM_URI));
  std::deque<std::string>::iterator urisLast = uris.begin()+max;
  std::deque<std::pair<SharedHandle<ServerStat>, std::string> > cands;
  for(std::deque<std::string>::iterator i = uris.begin(), eoi = urisLast;
      i != eoi; ++i) {
    Request r;
    r.setUri(*i);
    SharedHandle<ServerStat> ss = serverStatMan_->find(r.getHost(),
                                                       r.getProtocol());
    if(!ss.isNull() && ss->isOK() && ss->getDownloadSpeed() > SPEED_THRESHOLD) {
      cands.push_back(std::pair<SharedHandle<ServerStat>, std::string>(ss, *i));
    }
  }
  if(cands.empty()) {
    for(std::deque<std::string>::iterator i = uris.begin(), eoi = uris.end();
        i != eoi; ++i) {
      Request r;
      r.setUri(*i);
      SharedHandle<ServerStat> ss = serverStatMan_->find(r.getHost(),
                                                         r.getProtocol());
      // Skip ERROR state URI
      if(ss.isNull() || ss->isOK()) {
        std::string nextURI = *i;
        uris.erase(uris.begin(), i+1);
        return nextURI;
      }
    }
    // All URIs are inspected but aria2 cannot find usable one.
    // Return first URI anyway in this case.
    std::string nextURI = uris.front();
    uris.pop_front();
    return nextURI;
  } else {
    std::sort(cands.begin(), cands.end(), ServerStatFaster());
    uris.erase(std::find(uris.begin(), uris.end(), cands.front().second));
    return cands.front().second;
  }
}

} // namespace aria2
