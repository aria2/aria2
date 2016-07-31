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

AnnounceList::AnnounceList() : currentTrackerInitialized_(false) {}

AnnounceList::AnnounceList(
    const std::vector<std::vector<std::string>>& announceList)
    : currentTrackerInitialized_(false)
{
  reconfigure(announceList);
}

AnnounceList::AnnounceList(
    const std::deque<std::shared_ptr<AnnounceTier>>& announceTiers)
    : tiers_(announceTiers), currentTrackerInitialized_(false)
{
  resetIterator();
}

AnnounceList::~AnnounceList() = default;

void AnnounceList::reconfigure(
    const std::vector<std::vector<std::string>>& announceList)
{
  for (const auto& vec : announceList) {
    if (vec.empty()) {
      continue;
    }

    std::deque<std::string> uris(std::begin(vec), std::end(vec));
    auto tier = std::make_shared<AnnounceTier>(std::move(uris));
    tiers_.push_back(std::move(tier));
  }
  resetIterator();
}

void AnnounceList::reconfigure(const std::string& url)
{
  std::deque<std::string> urls{url};
  tiers_.push_back(std::make_shared<AnnounceTier>(std::move(urls)));
  resetIterator();
}

void AnnounceList::resetIterator()
{
  currentTier_ = std::begin(tiers_);
  if (currentTier_ != std::end(tiers_) && (*currentTier_)->urls.size()) {
    currentTracker_ = std::begin((*currentTier_)->urls);
    currentTrackerInitialized_ = true;
  }
  else {
    currentTrackerInitialized_ = false;
  }
}

std::string AnnounceList::getAnnounce() const
{
  if (currentTrackerInitialized_) {
    return *currentTracker_;
  }
  else {
    return A2STR::NIL;
  }
}

void AnnounceList::announceSuccess()
{
  if (currentTrackerInitialized_) {
    (*currentTier_)->nextEvent();
    auto url = *currentTracker_;
    (*currentTier_)->urls.erase(currentTracker_);
    (*currentTier_)->urls.push_front(std::move(url));
    currentTier_ = std::begin(tiers_);
    currentTracker_ = std::begin((*currentTier_)->urls);
  }
}

void AnnounceList::announceFailure()
{
  if (currentTrackerInitialized_) {
    ++currentTracker_;
    if (currentTracker_ == std::end((*currentTier_)->urls)) {
      // force next event
      (*currentTier_)->nextEventIfAfterStarted();
      ++currentTier_;
      if (currentTier_ == std::end(tiers_)) {
        currentTrackerInitialized_ = false;
      }
      else {
        currentTracker_ = std::begin((*currentTier_)->urls);
      }
    }
  }
}

AnnounceTier::AnnounceEvent AnnounceList::getEvent() const
{
  if (currentTrackerInitialized_) {
    return (*currentTier_)->event;
  }
  else {
    return AnnounceTier::STARTED;
  }
}

void AnnounceList::setEvent(AnnounceTier::AnnounceEvent event)
{
  if (currentTrackerInitialized_) {
    (*currentTier_)->event = event;
  }
}

const char* AnnounceList::getEventString() const
{
  if (currentTrackerInitialized_) {
    switch ((*currentTier_)->event) {
    case AnnounceTier::STARTED:
    case AnnounceTier::STARTED_AFTER_COMPLETION:
      return "started";
    case AnnounceTier::STOPPED:
      return "stopped";
    case AnnounceTier::COMPLETED:
      return "completed";
    default:
      return "";
    }
  }
  else {
    return "";
  }
}

namespace {
class FindStoppedAllowedTier {
public:
  bool operator()(const std::shared_ptr<AnnounceTier>& tier) const
  {
    switch (tier->event) {
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
  bool operator()(const std::shared_ptr<AnnounceTier>& tier) const
  {
    switch (tier->event) {
    case AnnounceTier::DOWNLOADING:
    case AnnounceTier::COMPLETED:
      return true;
    default:
      return false;
    }
  }
};
} // namespace

size_t AnnounceList::countStoppedAllowedTier() const
{
  return count_if(std::begin(tiers_), std::end(tiers_),
                  FindStoppedAllowedTier());
}

size_t AnnounceList::countCompletedAllowedTier() const
{
  return count_if(std::begin(tiers_), std::end(tiers_),
                  FindCompletedAllowedTier());
}

void AnnounceList::setCurrentTier(
    std::deque<std::shared_ptr<AnnounceTier>>::iterator itr)
{
  if (itr != std::end(tiers_)) {
    currentTier_ = std::move(itr);
    currentTracker_ = std::begin((*currentTier_)->urls);
  }
}

void AnnounceList::moveToStoppedAllowedTier()
{
  auto itr = find_wrap_if(std::begin(tiers_), std::end(tiers_), currentTier_,
                          FindStoppedAllowedTier());
  setCurrentTier(std::move(itr));
}

void AnnounceList::moveToCompletedAllowedTier()
{
  auto itr = find_wrap_if(std::begin(tiers_), std::end(tiers_), currentTier_,
                          FindCompletedAllowedTier());
  setCurrentTier(std::move(itr));
}

void AnnounceList::shuffle()
{
  for (const auto& tier : tiers_) {
    auto& urls = tier->urls;
    std::shuffle(std::begin(urls), std::end(urls),
                 *SimpleRandomizer::getInstance());
  }
}

bool AnnounceList::allTiersFailed() const
{
  return currentTier_ == std::end(tiers_);
}

void AnnounceList::resetTier() { resetIterator(); }

bool AnnounceList::currentTierAcceptsStoppedEvent() const
{
  if (currentTrackerInitialized_) {
    return FindStoppedAllowedTier()(*currentTier_);
  }

  return false;
}

bool AnnounceList::currentTierAcceptsCompletedEvent() const
{
  if (currentTrackerInitialized_) {
    return FindCompletedAllowedTier()(*currentTier_);
  }

  return false;
}

size_t AnnounceList::countTier() const { return tiers_.size(); }

} // namespace aria2
