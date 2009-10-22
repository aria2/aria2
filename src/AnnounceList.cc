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
#include "AnnounceList.h"

#include <algorithm>

#include "A2STR.h"
#include "SimpleRandomizer.h"
#include "bencode.h"
#include "Util.h"

namespace aria2 {

const std::string AnnounceList::STARTED("started");

const std::string AnnounceList::STOPPED("stopped");

const std::string AnnounceList::COMPLETED("completed");

AnnounceList::AnnounceList(const BDE& announceList):
  currentTrackerInitialized(false) {
  reconfigure(announceList);
}

AnnounceList::AnnounceList(const AnnounceTiers& announceTiers):
  tiers(announceTiers), currentTrackerInitialized(false)  {
  resetIterator();
}

void AnnounceList::reconfigure(const BDE& announceList)
{
  if(announceList.isList()) {
    for(BDE::List::const_iterator itr = announceList.listBegin();
	itr != announceList.listEnd(); ++itr) {
      const BDE& elemList = *itr;
      if(!elemList.isList()) {
	continue;
      }
      std::deque<std::string> urls;
      for(BDE::List::const_iterator elemItr = elemList.listBegin();
	  elemItr != elemList.listEnd(); ++elemItr) {
	const BDE& data = *elemItr;
	if(data.isString()) {
	  urls.push_back(util::trim(data.s()));
	}
      }
      if(!urls.empty()) {
	AnnounceTierHandle tier(new AnnounceTier(urls));
	tiers.push_back(tier);
      }
    }
    resetIterator();
  }
}

void AnnounceList::reconfigure(const std::string& url) {
  std::deque<std::string> urls;
  urls.push_back(url);
  tiers.push_back(AnnounceTierHandle(new AnnounceTier(urls)));
  resetIterator();
}

void AnnounceList::resetIterator() {
  currentTier = tiers.begin();
  if(currentTier != tiers.end() && (*currentTier)->urls.size()) {
    currentTracker = (*currentTier)->urls.begin();
    currentTrackerInitialized = true;
  } else {
    currentTrackerInitialized = false;
  }
}

std::string AnnounceList::getAnnounce() const {
  if(currentTrackerInitialized) {
    return *currentTracker;
  } else {
    return A2STR::NIL;
  }
}

void AnnounceList::announceSuccess() {
  if(currentTrackerInitialized) {
    (*currentTier)->nextEvent();
    std::string url = *currentTracker;
    (*currentTier)->urls.erase(currentTracker);
    (*currentTier)->urls.push_front(url);
    currentTier = tiers.begin();
    currentTracker = (*currentTier)->urls.begin();
  }
}

void AnnounceList::announceFailure() {
  if(currentTrackerInitialized) {
    ++currentTracker;
    if(currentTracker == (*currentTier)->urls.end()) {
      // force next event
      (*currentTier)->nextEventIfAfterStarted();
      ++currentTier;
      if(currentTier == tiers.end()) {
	currentTrackerInitialized = false;
      } else {
	currentTracker = (*currentTier)->urls.begin();
      }
    }
  }
}

AnnounceTier::AnnounceEvent AnnounceList::getEvent() const {
  if(currentTrackerInitialized) {
    return (*currentTier)->event;
  } else {
    return AnnounceTier::STARTED;
  }
}

void AnnounceList::setEvent(AnnounceTier::AnnounceEvent event) {
  if(currentTrackerInitialized) {
    (*currentTier)->event = event;
  }
}

std::string AnnounceList::getEventString() const {
  if(currentTrackerInitialized) {
    switch((*currentTier)->event) {
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

class FindStoppedAllowedTier {
public:
  bool operator()(const AnnounceTierHandle& tier) const {
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

class FindCompletedAllowedTier {
public:
  bool operator()(const AnnounceTierHandle& tier) const {
    switch(tier->event) {
    case AnnounceTier::DOWNLOADING:
    case AnnounceTier::COMPLETED:
      return true;
    default:
      return false;
    }
  }
};

size_t AnnounceList::countStoppedAllowedTier() const {
  return count_if(tiers.begin(), tiers.end(), FindStoppedAllowedTier());
}

size_t AnnounceList::countCompletedAllowedTier() const {
  return count_if(tiers.begin(), tiers.end(), FindCompletedAllowedTier());
}

void AnnounceList::setCurrentTier(const AnnounceTiers::iterator& itr) {
  if(itr != tiers.end()) {
    currentTier = itr;
    currentTracker = (*currentTier)->urls.begin();
  }
}

template<class InputIterator, class Predicate>
InputIterator
find_wrap_if(InputIterator first, InputIterator last,
	     InputIterator current, Predicate pred) {
  InputIterator itr = std::find_if(current, last, pred);
  if(itr == last) {
    itr = std::find_if(first, current, pred);
  }
  return itr;
}

void AnnounceList::moveToStoppedAllowedTier() {
  AnnounceTiers::iterator itr = find_wrap_if(tiers.begin(), tiers.end(),
					     currentTier,
					     FindStoppedAllowedTier());
  setCurrentTier(itr);
}

void AnnounceList::moveToCompletedAllowedTier() {
  AnnounceTiers::iterator itr = find_wrap_if(tiers.begin(), tiers.end(),
					     currentTier,
					     FindCompletedAllowedTier());
  setCurrentTier(itr);
}

void AnnounceList::shuffle() {
  for(AnnounceTiers::iterator itr = tiers.begin();
      itr != tiers.end(); ++itr) {
    std::deque<std::string>& urls = (*itr)->urls;
    std::random_shuffle(urls.begin(), urls.end(),
			*(SimpleRandomizer::getInstance().get()));
  }
}

bool AnnounceList::allTiersFailed() const
{
  return currentTier == tiers.end();
}

void AnnounceList::resetTier()
{
  resetIterator();
}

bool AnnounceList::currentTierAcceptsStoppedEvent() const
{
  if(currentTrackerInitialized) {
    return FindStoppedAllowedTier()(*currentTier);
  } else {
    return false;
  }
}

bool AnnounceList::currentTierAcceptsCompletedEvent() const
{
  if(currentTrackerInitialized) {
    return FindCompletedAllowedTier()(*currentTier);
  } else {
    return false;
  }
}

} // namespace aria2
