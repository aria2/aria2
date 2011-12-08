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

#include <cassert>
#include <algorithm>

#include "ServerStatMan.h"
#include "ServerStat.h"
#include "A2STR.h"
#include "FileEntry.h"
#include "Logger.h"
#include "LogFactory.h"
#include "a2algo.h"
#include "uri.h"
#include "fmt.h"

namespace aria2 {

FeedbackURISelector::FeedbackURISelector
(const SharedHandle<ServerStatMan>& serverStatMan)
  : serverStatMan_(serverStatMan)
{}

FeedbackURISelector::~FeedbackURISelector() {}

std::string FeedbackURISelector::select
(FileEntry* fileEntry,
 const std::vector<std::pair<size_t, std::string> >& usedHosts)
{
  if(A2_LOG_DEBUG_ENABLED) {
    for(std::vector<std::pair<size_t, std::string> >::const_iterator i =
          usedHosts.begin(), eoi = usedHosts.end(); i != eoi; ++i) {
      A2_LOG_DEBUG(fmt("UsedHost=%lu, %s",
                       static_cast<unsigned long>((*i).first),
                       (*i).second.c_str()));
    }
  }
  if(fileEntry->getRemainingUris().empty()) {
    return A2STR::NIL;
  }
  // Select URI with usedHosts first. If no URI is selected, then do
  // it again without usedHosts.
  std::string uri = selectFaster(fileEntry->getRemainingUris(), usedHosts);
  if(uri.empty()) {
    A2_LOG_DEBUG("No URI returned from selectFaster()");
    uri = selectRarer(fileEntry->getRemainingUris(), usedHosts);
  } 
  if(!uri.empty()) {
    std::deque<std::string>& uris = fileEntry->getRemainingUris();
    uris.erase(std::find(uris.begin(), uris.end(), uri));
  }
  A2_LOG_DEBUG(fmt("FeedbackURISelector selected %s", uri.c_str()));
  return uri;
}

std::string FeedbackURISelector::selectRarer
(const std::deque<std::string>& uris,
 const std::vector<std::pair<size_t, std::string> >& usedHosts)
{
  // pair of host and URI
  std::vector<std::pair<std::string, std::string> > cands;
  for(std::deque<std::string>::const_iterator i = uris.begin(),
        eoi = uris.end(); i != eoi; ++i) {
    uri::UriStruct us;
    if(!uri::parse(us, *i)) {
      continue;
    }
    SharedHandle<ServerStat> ss =
      serverStatMan_->find(us.host, us.protocol);
    if(ss && ss->isError()) {
      A2_LOG_DEBUG(fmt("Error not considered: %s", (*i).c_str()));
      continue;
    }
    cands.push_back(std::make_pair(us.host, *i));
  }
  for(std::vector<std::pair<size_t, std::string> >::const_iterator i =
        usedHosts.begin(), eoi = usedHosts.end(); i != eoi; ++i) {
    for(std::vector<std::pair<std::string, std::string> >::const_iterator j =
          cands.begin(), eoj = cands.end(); j != eoj; ++j) {
      if((*i).second == (*j).first) {
        return (*j).second;
      }
    }
  }
  assert(!uris.empty());
  return uris.front();
}

std::string FeedbackURISelector::selectFaster
(const std::deque<std::string>& uris,
 const std::vector<std::pair<size_t, std::string> >& usedHosts)
{
  // Use first 10 good URIs to introduce some randomness.
  const size_t NUM_URI = 10;
  // Ignore low speed server
  const int SPEED_THRESHOLD = 20*1024;
  std::vector<std::pair<SharedHandle<ServerStat>, std::string> > fastCands;
  std::vector<std::string> normCands;
  for(std::deque<std::string>::const_iterator i = uris.begin(),
        eoi = uris.end(); i != eoi && fastCands.size() < NUM_URI; ++i) {
    uri::UriStruct us;
    if(!uri::parse(us, *i)) {
      continue;
    }
    if(findSecond(usedHosts.begin(), usedHosts.end(), us.host) !=
       usedHosts.end()) {
      A2_LOG_DEBUG(fmt("%s is in usedHosts, not considered", (*i).c_str()));
      continue;
    }
    SharedHandle<ServerStat> ss =
      serverStatMan_->find(us.host, us.protocol);
    if(!ss) {
      normCands.push_back(*i);
    } else if(ss->isOK()) {
      if(ss->getDownloadSpeed() > SPEED_THRESHOLD) {
        fastCands.push_back(std::make_pair(ss, *i));
      } else {
        normCands.push_back(*i);
      }
    }
  }
  if(fastCands.empty()) {
    if(normCands.empty()) {
      return A2STR::NIL;
    } else {
      A2_LOG_DEBUG("Selected from normCands");
      return normCands.front();
    }
  } else {
    A2_LOG_DEBUG("Selected from fastCands");
    std::sort(fastCands.begin(), fastCands.end(), ServerStatFaster());
    return fastCands.front().second;
  }
}

} // namespace aria2
