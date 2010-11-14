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
#include "AnnounceList.h"

#include <algorithm>

#include "A2STR.h"
#include "SimpleRandomizer.h"
#include "a2algo.h"

namespace aria2 {

const std::string AnnounceList::STARTED("started");

const std::string AnnounceList::STOPPED("stopped");

const std::string AnnounceList::COMPLETED("completed");

AnnounceList::AnnounceList():currentTrackerInitialized_(false) {}

AnnounceList::AnnounceList
(const std::vector<std::vector<std::string> >& announceList):
  currentTrackerInitialized_(false) {
  reconfigure(announceList);
}

AnnounceList::AnnounceList
(const std::deque<SharedHandle<AnnounceTier> >& announceTiers):
  tiers_(announceTiers), currentTrackerInitialized_(false)  {
  resetIterator();
}

AnnounceList::~AnnounceList() {}

void AnnounceList::reconfigure
(const std::vector<std::vector<std::string> >& announceList)
{
  for(std::vector<std::vector<std::string> >::const_iterator itr =
        announceList.begin(), eoi = announceList.end(); itr != eoi; ++itr) {
    if((*itr).empty()) {
      continue;
    }
    std::deque<std::string> urls((*itr).begin(), (*itr).end());
    SharedHandle<AnnounceTier> tier(new AnnounceTier(urls));
    tiers_.push_back(tier);
  }
  resetIterator();
}

void AnnounceList::reconfigure(const std::string& url) {
  std::deque<std::string> urls;
  urls.push_back(url);
  SharedHandle<AnnounceTier> tier(new AnnounceTier(urls));
  tiers_.push_back(tier);
  resetIterator();
}

void AnnounceList::resetIterator() {
  currentTier_ = tiers_.begin();
  if(currentTier_ != tiers_.end() && (*currentTier_)->urls.size()) {
    currentTracker_ = (*currentTier_)->urls.begin();
    currentTrackerInitialized_ = true;
  } else {
    currentTrackerInitialized_ = false;
  }
}

std::string AnnounceList::getAnnounce() const {
  if(currentTrackerInitialized_) {
    return *currentTracker_;
  } else {
    return A2STR::NIL;
  }
}

void AnnounceList::announceSuccess() {
  if(currentTrackerInitialized_) {
    (*currentTier_)->nextEvent();
    std::string url = *currentTracker_;
    (*currentTier_)->urls.erase(currentTracker_);
    (*currentTier_)->urls.push_front(url);
    currentTier_ = tiers_.begin();
    currentTracker_ = (*currentTier_)->urls.begin();
  }
}

void AnnounceList::announceFailure() {
  if(currentTrackerInitialized_) {
    ++currentTracker_;
    if(currentTracker_ == (*currentTier_)->urls.end()) {
      // force next event
      (*currentTier_)->nextEventIfAfterStarted();
      ++currentTier_;
      if(currentTier_ == tiers_.end()) {
        currentTrackerInitialized_ = false;
      } else {
        currentTracker_ = (*currentTier_)->urls.begin();
      }
    }
  }
}

AnnounceTier::AnnounceEvent AnnounceList::getEvent() const {
  if(currentTrackerInitialized_) {
    return (*currentTier_)->event;
  } else {
    return AnnounceTier::STARTED;
  }
}

void AnnounceList::setEvent(AnnounceTier::AnnounceEvent event) {
  if(currentTrackerInitialized_) {
    (*currentTier_)->event = event;
  }
}

std::string AnnounceList::getEventString() const {
  if(currentTrackerInitialized_) {
    switch((*currentTier_)->event) {
    case AnnounceTier::STARTED:
    case AnnounceTier::STARTED_AFTER_COMPLETION:
      return STARTED;
    case AnnounceTier::STOPPED:
      return STOPPED;
    case AnnounceTier::COMPLETED:
      return COMPLETED;
    default:
      return A2STR::NIL;
    }
  } else {
    return A2STR::NIL;
  }
}

namespace {
class FindStoppedAllowedTier {
public:
  bool operator()(const SharedHandle<AnnounceTier>& tier) const {
    switch(tier->event) {
    case AnnounceTier::DOWNLOADING:
    case AnnounceTier::STOPPED:
    case AnnounceTier::COMPLETED:
    case AnnounceTier::SEEDING:
      return true;
    default:
      return false;
    }
  }
};
} // namespace

namespace {
class FindCompletedAllowedTier {
public:
  bool operator()(const SharedHandle<AnnounceTier>& tier) const {
    switch(tier->event) {
    case AnnounceTier::DOWNLOADING:
    case AnnounceTier::COMPLETED:
      return true;
    default:
      return false;
    }
  }
};
} // namespace

size_t AnnounceList::countStoppedAllowedTier() const {
  return count_if(tiers_.begin(), tiers_.end(), FindStoppedAllowedTier());
}

size_t AnnounceList::countCompletedAllowedTier() const {
  return count_if(tiers_.begin(), tiers_.end(), FindCompletedAllowedTier());
}

void AnnounceList::setCurrentTier
(const std::deque<SharedHandle<AnnounceTier> >::iterator& itr) {
  if(itr != tiers_.end()) {
    currentTier_ = itr;
    currentTracker_ = (*currentTier_)->urls.begin();
  }
}

void AnnounceList::moveToStoppedAllowedTier() {
  std::deque<SharedHandle<AnnounceTier> >::iterator itr =
    find_wrap_if(tiers_.begin(), tiers_.end(),
                 currentTier_,
                 FindStoppedAllowedTier());
  setCurrentTier(itr);
}

void AnnounceList::moveToCompletedAllowedTier() {
  std::deque<SharedHandle<AnnounceTier> >::iterator itr =
    find_wrap_if(tiers_.begin(), tiers_.end(),
                 currentTier_,
                 FindCompletedAllowedTier());
  setCurrentTier(itr);
}

void AnnounceList::shuffle() {
  for(std::deque<SharedHandle<AnnounceTier> >::const_iterator itr =
        tiers_.begin(), eoi = tiers_.end(); itr != eoi; ++itr) {
    std::deque<std::string>& urls = (*itr)->urls;
    std::random_shuffle(urls.begin(), urls.end(),
                        *(SimpleRandomizer::getInstance().get()));
  }
}

bool AnnounceList::allTiersFailed() const
{
  return currentTier_ == tiers_.end();
}

void AnnounceList::resetTier()
{
  resetIterator();
}

bool AnnounceList::currentTierAcceptsStoppedEvent() const
{
  if(currentTrackerInitialized_) {
    return FindStoppedAllowedTier()(*currentTier_);
  } else {
    return false;
  }
}

bool AnnounceList::currentTierAcceptsCompletedEvent() const
{
  if(currentTrackerInitialized_) {
    return FindCompletedAllowedTier()(*currentTier_);
  } else {
    return false;
  }
}

size_t AnnounceList::countTier() const
{
  return tiers_.size();
}

} // namespace aria2
