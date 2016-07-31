/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 * Copyright (C) 2008 Aurelien Lefebvre, Mandriva
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
#include "AdaptiveURISelector.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "DownloadCommand.h"
#include "DownloadContext.h"
#include "ServerStatMan.h"
#include "ServerStat.h"
#include "RequestGroup.h"
#include "Logger.h"
#include "LogFactory.h"
#include "A2STR.h"
#include "prefs.h"
#include "Option.h"
#include "SimpleRandomizer.h"
#include "SocketCore.h"
#include "FileEntry.h"
#include "uri.h"
#include "fmt.h"
#include "SocketRecvBuffer.h"

namespace aria2 {

/* In that URI Selector, select method returns one of the bests
 * mirrors for first and reserved connections. For supplementary
 * ones, it returns mirrors which has not been tested yet, and
 * if each of them already tested, returns mirrors which has to
 * be tested again. Otherwise, it doesn't return anymore mirrors.
 */

AdaptiveURISelector::AdaptiveURISelector(
    std::shared_ptr<ServerStatMan> serverStatMan, RequestGroup* requestGroup)
    : serverStatMan_(std::move(serverStatMan)), requestGroup_(requestGroup)
{
  resetCounters();
}

AdaptiveURISelector::~AdaptiveURISelector() = default;

std::string AdaptiveURISelector::select(
    FileEntry* fileEntry,
    const std::vector<std::pair<size_t, std::string>>& usedHosts)
{
  A2_LOG_DEBUG(
      fmt("AdaptiveURISelector: called %d", requestGroup_->getNumConnection()));
  std::deque<std::string>& uris = fileEntry->getRemainingUris();
  if (uris.empty() && requestGroup_->getNumConnection() <= 1) {
    // here we know the download will fail, trying to find previously
    // failed uris that may succeed with more permissive values
    mayRetryWithIncreasedTimeout(fileEntry);
  }

  std::string selected = selectOne(uris);

  if (selected != A2STR::NIL) {
    uris.erase(std::find(std::begin(uris), std::end(uris), selected));
  }
  return selected;
}

namespace {
constexpr auto MAX_TIMEOUT = 60_s;
} // namespace

void AdaptiveURISelector::mayRetryWithIncreasedTimeout(FileEntry* fileEntry)
{
  if (requestGroup_->getTimeout() * 2 >= MAX_TIMEOUT)
    return;
  requestGroup_->setTimeout(requestGroup_->getTimeout() * 2);

  std::deque<std::string>& uris = fileEntry->getRemainingUris();
  // looking for retries
  std::deque<URIResult> timeouts;
  fileEntry->extractURIResult(timeouts, error_code::TIME_OUT);
  std::transform(std::begin(timeouts), std::end(timeouts),
                 std::back_inserter(uris), std::mem_fn(&URIResult::getURI));

  if (A2_LOG_DEBUG_ENABLED) {
    for (const auto& uri : uris) {
      A2_LOG_DEBUG(
          fmt("AdaptiveURISelector: will retry server with increased"
              " timeout (%ld s): %s",
              static_cast<long int>(requestGroup_->getTimeout().count()),
              uri.c_str()));
    }
  }
}

std::string AdaptiveURISelector::selectOne(const std::deque<std::string>& uris)
{

  if (uris.empty()) {
    return A2STR::NIL;
  }
  else {
    const size_t numPieces =
        requestGroup_->getDownloadContext()->getNumPieces();

    bool reservedContext =
        numPieces > 0 &&
        static_cast<size_t>(nbConnections_) >
            std::min(numPieces, static_cast<size_t>(
                                    requestGroup_->getNumConcurrentCommand()));
    bool selectBest = numPieces == 0 || reservedContext;

    if (numPieces > 0)
      ++nbConnections_;

    /* At least, 3 mirrors must be tested */
    if (getNbTestedServers(uris) < 3) {
      std::string notTested = getFirstNotTestedUri(uris);
      if (notTested != A2STR::NIL) {
        A2_LOG_DEBUG(fmt("AdaptiveURISelector: choosing the first non tested"
                         " mirror: %s",
                         notTested.c_str()));
        --nbServerToEvaluate_;
        return notTested;
      }
    }

    if (!selectBest && nbConnections_ > 1 && nbServerToEvaluate_ > 0) {
      nbServerToEvaluate_--;
      std::string notTested = getFirstNotTestedUri(uris);
      if (notTested != A2STR::NIL) {
        /* Here we return the first untested mirror */
        A2_LOG_DEBUG(fmt("AdaptiveURISelector: choosing non tested mirror %s"
                         " for connection #%d",
                         notTested.c_str(), nbConnections_));
        return notTested;
      }
      else {
        /* Here we return a mirror which need to be tested again */
        std::string toReTest = getFirstToTestUri(uris);
        if (toReTest != A2STR::NIL) {
          A2_LOG_DEBUG(fmt("AdaptiveURISelector: choosing mirror %s which has"
                           " not been tested recently for connection #%d",
                           toReTest.c_str(), nbConnections_));
          return toReTest;
        }
        else {
          return getBestMirror(uris);
        }
      }
    }
    else {
      return getBestMirror(uris);
    }
  }
}

std::string
AdaptiveURISelector::getBestMirror(const std::deque<std::string>& uris) const
{
  /* Here we return one of the bests mirrors */
  int max = getMaxDownloadSpeed(uris);
  int min = max - (int)(max * 0.25);
  std::deque<std::string> bests = getUrisBySpeed(uris, min);

  if (bests.size() < 2) {
    std::string uri = getMaxDownloadSpeedUri(uris);
    A2_LOG_DEBUG(fmt("AdaptiveURISelector: choosing the best mirror :"
                     " %.2fKB/s %s (other mirrors are at least 25%% slower)",
                     (float)max / 1024, uri.c_str()));
    return uri;
  }
  else {
    std::string uri = selectRandomUri(bests);
    A2_LOG_DEBUG(fmt("AdaptiveURISelector: choosing randomly one of the best"
                     " mirrors (range [%.2fKB/s, %.2fKB/s]): %s",
                     (float)min / 1024, (float)max / 1024, uri.c_str()));
    return uri;
  }
}

void AdaptiveURISelector::resetCounters()
{
  nbConnections_ = 1;
  nbServerToEvaluate_ = requestGroup_->getOption()->getAsInt(PREF_SPLIT) - 1;
}

void AdaptiveURISelector::tuneDownloadCommand(
    const std::deque<std::string>& uris, DownloadCommand* command)
{
  adjustLowestSpeedLimit(uris, command);
}

void AdaptiveURISelector::adjustLowestSpeedLimit(
    const std::deque<std::string>& uris, DownloadCommand* command) const
{
  int lowest = requestGroup_->getOption()->getAsInt(PREF_LOWEST_SPEED_LIMIT);
  if (lowest > 0) {
    int low_lowest = 4_k;
    int max = getMaxDownloadSpeed(uris);
    if (max > 0 && lowest > max / 4) {
      A2_LOG_NOTICE(fmt(_("Lowering lowest-speed-limit since known max speed is"
                          " too near (new:%d was:%d max:%d)"),
                        max / 4, lowest, max));
      command->setLowestDownloadSpeedLimit(max / 4);
    }
    else if (max == 0 && lowest > low_lowest) {
      A2_LOG_NOTICE(fmt(_("Lowering lowest-speed-limit since we have no clue"
                          " about available speed (now:%d was:%d)"),
                        low_lowest, lowest));
      command->setLowestDownloadSpeedLimit(low_lowest);
    }
  }
}

namespace {
int getUriMaxSpeed(std::shared_ptr<ServerStat> ss)
{
  return std::max(ss->getSingleConnectionAvgSpeed(),
                  ss->getMultiConnectionAvgSpeed());
}
} // namespace

int AdaptiveURISelector::getMaxDownloadSpeed(
    const std::deque<std::string>& uris) const
{
  std::string uri = getMaxDownloadSpeedUri(uris);
  if (uri == A2STR::NIL)
    return 0;
  return getUriMaxSpeed(getServerStats(uri));
}

std::string AdaptiveURISelector::getMaxDownloadSpeedUri(
    const std::deque<std::string>& uris) const
{
  int max = -1;
  std::string uri = A2STR::NIL;
  for (auto& u : uris) {
    std::shared_ptr<ServerStat> ss = getServerStats(u);
    if (!ss)
      continue;

    if ((int)ss->getSingleConnectionAvgSpeed() > max) {
      max = ss->getSingleConnectionAvgSpeed();
      uri = u;
    }
    if ((int)ss->getMultiConnectionAvgSpeed() > max) {
      max = ss->getMultiConnectionAvgSpeed();
      uri = u;
    }
  }
  return uri;
}

std::deque<std::string>
AdaptiveURISelector::getUrisBySpeed(const std::deque<std::string>& uris,
                                    int min) const
{
  std::deque<std::string> bests;
  for (auto& uri : uris) {
    std::shared_ptr<ServerStat> ss = getServerStats(uri);
    if (!ss)
      continue;
    if (ss->getSingleConnectionAvgSpeed() > min ||
        ss->getMultiConnectionAvgSpeed() > min) {
      bests.push_back(uri);
    }
  }
  return bests;
}

std::string
AdaptiveURISelector::selectRandomUri(const std::deque<std::string>& uris) const
{
  int pos = SimpleRandomizer::getInstance()->getRandomNumber(uris.size());
  auto i = std::begin(uris);
  i = i + pos;
  return *i;
}

std::string AdaptiveURISelector::getFirstNotTestedUri(
    const std::deque<std::string>& uris) const
{
  for (const auto& i : uris) {
    std::shared_ptr<ServerStat> ss = getServerStats(i);
    if (!ss)
      return i;
  }
  return A2STR::NIL;
}

std::string AdaptiveURISelector::getFirstToTestUri(
    const std::deque<std::string>& uris) const
{
  int counter;
  int power;
  for (const auto& u : uris) {
    std::shared_ptr<ServerStat> ss = getServerStats(u);
    if (!ss)
      continue;
    counter = ss->getCounter();
    if (counter > 8)
      continue;
    power = (int)pow(2.0, (float)counter);
    /* We test the mirror another time if it has not been
     * tested since 2^counter days */
    if (ss->getLastUpdated().difference() > std::chrono::hours(power * 24)) {
      return u;
    }
  }
  return A2STR::NIL;
}

std::shared_ptr<ServerStat>
AdaptiveURISelector::getServerStats(const std::string& uri) const
{
  uri_split_result us;
  if (uri_split(&us, uri.c_str()) == 0) {
    std::string host = uri::getFieldString(us, USR_HOST, uri.c_str());
    std::string protocol = uri::getFieldString(us, USR_SCHEME, uri.c_str());
    return serverStatMan_->find(host, protocol);
  }
  else {
    return nullptr;
  }
}

int AdaptiveURISelector::getNbTestedServers(
    const std::deque<std::string>& uris) const
{
  int counter = 0;
  for (const auto& u : uris) {
    std::shared_ptr<ServerStat> ss = getServerStats(u);
    if (!ss)
      ++counter;
  }
  return uris.size() - counter;
}

} // namespace aria2
