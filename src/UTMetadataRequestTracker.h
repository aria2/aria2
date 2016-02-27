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
#ifndef D_UT_METADATA_REQUEST_TRACKER_H
#define D_UT_METADATA_REQUEST_TRACKER_H

#include "common.h"

#include <vector>

#include "TimerA2.h"
#include "wallclock.h"

namespace aria2 {

class UTMetadataRequestTracker {
public:
  // Made public so that unnamed functor can access this
  struct RequestEntry {
    size_t index_;
    Timer dispatchedTime_;

    RequestEntry(size_t index) : index_(index) {}

    bool elapsed(const std::chrono::seconds t) const
    {
      return dispatchedTime_.difference(global::wallclock()) >= t;
    }

    bool operator==(const RequestEntry& e) const { return index_ == e.index_; }
  };

private:
  std::vector<RequestEntry> trackedRequests_;

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
  size_t count() const { return trackedRequests_.size(); }

  // Returns the number of additional index this tracker can track.
  size_t avail() const;
};

} // namespace aria2

#endif // D_UT_METADATA_REQUEST_TRACKER_H
