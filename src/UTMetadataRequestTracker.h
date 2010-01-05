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
#ifndef _UT_METADATA_REQUEST_TRACKER_H_
#define _UT_METADATA_REQUEST_TRACKER_H_

#include "common.h"

#include <vector>

#include "TimeA2.h"

namespace aria2 {

class Logger;

class UTMetadataRequestTracker {
private:
  struct RequestEntry {
    size_t _index;
    Time _dispatchedTime;

    RequestEntry(size_t index):_index(index) {}

    bool elapsed(time_t t) const
    {
      return _dispatchedTime.elapsed(t);
    }

    bool operator==(const RequestEntry& e) const
    {
      return _index == e._index;
    }
  };

  std::vector<RequestEntry> _trackedRequests;

  Logger* _logger;
public:
  UTMetadataRequestTracker();

  // Add request index to tracking list.
  void add(size_t index);

  // Returns true if request index is tracked.
  bool tracks(size_t index);

  // Remove index from tracking list.
  void remove(size_t index);

  // Returns all tracking indexes.
  std::vector<size_t> getAllTrackedIndex() const;

  // Removes request index which is timed out and returns their indexes.
  std::vector<size_t> removeTimeoutEntry();

  // Returns the number of tracking list.
  size_t count() const
  {
    return _trackedRequests.size();
  }

  // Returns the number of additional index this tracker can track.
  size_t avail() const;
};

} // namespace aria2

#endif // _UT_METADATA_REQUEST_TRACKER_H_
