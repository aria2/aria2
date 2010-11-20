/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "UTMetadataRequestTracker.h"

#include <algorithm>

#include "Logger.h"
#include "LogFactory.h"
#include "fmt.h"

namespace aria2 {

UTMetadataRequestTracker::UTMetadataRequestTracker() {}

void UTMetadataRequestTracker::add(size_t index)
{
  trackedRequests_.push_back(RequestEntry(index));
}

bool UTMetadataRequestTracker::tracks(size_t index)
{
  return std::find(trackedRequests_.begin(), trackedRequests_.end(),
                   RequestEntry(index)) != trackedRequests_.end();
}

void UTMetadataRequestTracker::remove(size_t index)
{
  std::vector<RequestEntry>::iterator i =
    std::find(trackedRequests_.begin(), trackedRequests_.end(),
              RequestEntry(index));
  if(i != trackedRequests_.end()) {
    trackedRequests_.erase(i);
  }
}

std::vector<size_t> UTMetadataRequestTracker::removeTimeoutEntry()
{
  std::vector<size_t> indexes;
  const time_t TIMEOUT = 20;
  for(std::vector<RequestEntry>::iterator i = trackedRequests_.begin(),
        eoi = trackedRequests_.end(); i != eoi;) {
    if((*i).elapsed(TIMEOUT)) {
      A2_LOG_DEBUG(fmt("ut_metadata request timeout. index=%lu",
                       static_cast<unsigned long>((*i).index_)));
      indexes.push_back((*i).index_);
      i = trackedRequests_.erase(i);
      eoi = trackedRequests_.end();
    } else {
      ++i;
    }
  }
  return indexes;
}

size_t UTMetadataRequestTracker::avail() const
{
  const size_t MAX_OUTSTANDING_REQUEST = 1;
  if(MAX_OUTSTANDING_REQUEST > count()) {
    return MAX_OUTSTANDING_REQUEST-count();
  } else {
    return 0;
  }
}

std::vector<size_t> UTMetadataRequestTracker::getAllTrackedIndex() const
{
  std::vector<size_t> indexes;
  for(std::vector<RequestEntry>::const_iterator i = trackedRequests_.begin(),
        eoi = trackedRequests_.end(); i != eoi; ++i) {
    indexes.push_back((*i).index_);
  }
  return indexes;
}

} // namespace aria2
