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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#ifndef _D_ANNOUNCE_LIST_H_
#define _D_ANNOUNCE_LIST_H_

#include "common.h"
#include "MetaEntry.h"
#include "AnnounceTier.h"

class AnnounceList {
public:
private:
  AnnounceTiers tiers;
  AnnounceTiers::iterator currentTier;
  Strings::iterator currentTracker;
  bool currentTrackerInitialized;

  void resetIterator();
  void setCurrentTier(const AnnounceTiers::iterator& itr);
public:
  AnnounceList():currentTrackerInitialized(false) {}
  AnnounceList(const MetaEntry* announceListEntry);
  AnnounceList(const AnnounceTiers& tiers);

  void reconfigure(const MetaEntry* announceListEntry);
  void reconfigure(const string& url);

  int countTier() const {
    return tiers.size();
  }

  /**
   * Shuffles all the URLs in each group.
   */
  void shuffle();

  /**
   * Returns announce URL.
   */
  string getAnnounce() const;

  /**
   * Returns announce event, such as started, stopped, completed, etc.
   */
  string getEventString() const;

  AnnounceTier::AnnounceEvent getEvent() const;

  void setEvent(AnnounceTier::AnnounceEvent event);

  /**
   * Removes current announce URL from its group and inserts it before the
   * first element of the group.
   * The internal announce group pointer points to the first element of the
   * first group after this call.
   */
  void announceSuccess();

  /**
   * The internal announce URL pointer points to next URL.
   * If the current URL is the last element of its group, then the first
   * element of the next group is pointed.
   */
  void announceFailure();

  /**
   * Counts the number of tiers to which the "stopped" event can be sent.
   */
  int countStoppedAllowedTier() const;

  /**
   * Counts the number of tiers to which the "completed" event can be sent.
   */
  int countCompletedAllowedTier() const;

  /**
   * Moves current tier pointer to the tier to which the "stopped" event can
   * be sent.
   * 2-stage search operation is made.
   * The first search operation is performed from current pointer position
   * to the end. If no such tier is found, the second search is performed from
   * the first to the current pointer position.
   */
  void moveToStoppedAllowedTier();
  void moveToCompletedAllowedTier();
};

#endif // _D_ANNOUNCE_LIST_H_
