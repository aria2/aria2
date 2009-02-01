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
#include "AdaptiveURISelector.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "DownloadCommand.h"
#include "DownloadContext.h"
#include "ServerStatMan.h"
#include "ServerStat.h"
#include "RequestGroup.h"
#include "LogFactory.h"
#include "A2STR.h"
#include "prefs.h"
#include "Option.h"
#include "SimpleRandomizer.h"
#include "SocketCore.h"

namespace aria2 {

/* In that URI Selector, select method returns one of the bests 
 * mirrors for first and reserved connections. For supplementary 
 * ones, it returns mirrors which has not been tested yet, and 
 * if each of them already tested, returns mirrors which has to 
 * be tested again. Otherwise, it doesn't return anymore mirrors.
 */

AdaptiveURISelector::AdaptiveURISelector
(const SharedHandle<ServerStatMan>& serverStatMan, 
 const SharedHandle<RequestGroup>& requestGroup):
  _serverStatMan(serverStatMan),
  _requestGroup(requestGroup),
  _nbConnections(1),
  _logger(LogFactory::getInstance())
{
  const Option* op = _requestGroup->getOption();
  _nbServerToEvaluate = op->getAsInt(PREF_METALINK_SERVERS) - 1;
}

AdaptiveURISelector::~AdaptiveURISelector() {}

std::string AdaptiveURISelector::select(std::deque<std::string>& uris)
{
  _logger->debug("AdaptiveURISelector: called %d",
		 _requestGroup->getNumConnection());
  if (uris.empty() && _requestGroup->getNumConnection() <= 1) {
    // here we know the download will fail, trying to find previously
    // failed uris that may succeed with more permissive values
    mayRetryWithIncreasedTimeout(uris);
  }
 
  std::string selected = selectOne(uris);

  if(selected != A2STR::NIL)
    uris.erase(std::find(uris.begin(), uris.end(), selected));

  return selected;
}

void AdaptiveURISelector::mayRetryWithIncreasedTimeout
(std::deque<std::string>& uris)
{
  if (_requestGroup->getTimeout()*2 >= MAX_TIMEOUT) return;
  _requestGroup->setTimeout(_requestGroup->getTimeout()*2);

  // looking for retries
  const std::deque<URIResult>& uriResults = _requestGroup->getURIResults();
  for (std::deque<URIResult>::const_iterator i = uriResults.begin();
       i != uriResults.end(); ++i) {
    if ((*i).getResult() == DownloadResult::TIME_OUT) {
      _logger->debug("AdaptiveURISelector: will retry server with increased"
		     " timeout (%d s): %s",
		     _requestGroup->getTimeout(), (*i).getURI().c_str());
      uris.push_back((*i).getURI());
    }
  }
  std::sort(uris.begin(), uris.end());
  uris.erase(std::unique(uris.begin(), uris.end()), uris.end());
}

std::string AdaptiveURISelector::selectOne(const std::deque<std::string>& uris)
{

  if(uris.empty()) {
    return A2STR::NIL;
  } else {
    const unsigned int numPieces =
      _requestGroup->getDownloadContext()->getNumPieces();

    bool reservedContext = numPieces > 0 && 
      _nbConnections > std::min(numPieces,
				_requestGroup->getNumConcurrentCommand());
    bool selectBest = numPieces == 0 || reservedContext;
    
    if(numPieces > 0)
      _nbConnections++;

    /* At least, 3 mirrors must be tested */
    if(getNbTestedServers(uris) < 3) {
      std::string notTested = getFirstNotTestedUri(uris);
      if(notTested != A2STR::NIL) {
	_logger->debug("AdaptiveURISelector: choosing the first non tested"
		       " mirror: %s", notTested.c_str());
        --_nbServerToEvaluate;
        return notTested;
      }
    }
    
    if(!selectBest && _nbConnections > 1 && _nbServerToEvaluate > 0) {
      _nbServerToEvaluate--;
      std::string notTested = getFirstNotTestedUri(uris);
      if(notTested != A2STR::NIL) {
        /* Here we return the first untested mirror */
	_logger->debug("AdaptiveURISelector: choosing non tested mirror %s for"
		       " connection #%d", notTested.c_str(), _nbConnections);
        return notTested;
      } else {
	/* Here we return a mirror which need to be tested again */
	std::string toReTest = getFirstToTestUri(uris);
	_logger->debug("AdaptiveURISelector: choosing mirror %s which has not"
		       " been tested recently for connection #%d",
		       toReTest.c_str(), _nbConnections);
	return toReTest;
      }
    }
    else {
      /* Here we return one of the bests mirrors */
      unsigned int max = getMaxDownloadSpeed(uris);
      unsigned int min = max-(int)(max*0.25);
      std::deque<std::string> bests = getUrisBySpeed(uris, min);
      
      if (bests.size() < 2) {
	std::string uri = getMaxDownloadSpeedUri(uris);
	_logger->debug("AdaptiveURISelector: choosing the best mirror :"
		       " %.2fKB/s %s (other mirrors are at least 25%% slower)",
		       (float) max/1024, uri.c_str());
	return uri;
      } else {
	std::string uri = selectRandomUri(bests);
	_logger->debug("AdaptiveURISelector: choosing randomly one of the best"
		       " mirrors (range [%.2fKB/s, %.2fKB/s]): %s",
		       (float) min/1024, (float) max/1024, uri.c_str());
	return uri;
      }
    }
  }
}

void AdaptiveURISelector::resetCounters()
{
  const Option* op = _requestGroup->getOption();
  _nbConnections = 1;
  _nbServerToEvaluate = op->getAsInt(PREF_METALINK_SERVERS) - 1;
}

void AdaptiveURISelector::tuneDownloadCommand
(const std::deque<std::string>& uris, DownloadCommand* command)
{
  adjustLowestSpeedLimit(uris, command);
}

void AdaptiveURISelector::adjustLowestSpeedLimit
(const std::deque<std::string>& uris, DownloadCommand* command) const
{
  const Option* op = _requestGroup->getOption();
  unsigned int lowest = op->getAsInt(PREF_LOWEST_SPEED_LIMIT);
  if (lowest > 0) {
    unsigned int low_lowest = 4 * 1024;
    unsigned int max = getMaxDownloadSpeed(uris);
    if (max > 0 && lowest > max / 4) {
      _logger->notice("Lowering lowest-speed-limit since known max speed is too"
		      " near (new:%d was:%d max:%d)", max / 4, lowest, max);
      command->setLowestDownloadSpeedLimit(max / 4);
    } else if (max == 0 && lowest > low_lowest) {
      _logger->notice("Lowering lowest-speed-limit since we have no clue about"
		      " available speed (now:%d was:%d)", low_lowest, lowest);
      command->setLowestDownloadSpeedLimit(low_lowest);
    }
  }
}

static unsigned int getUriMaxSpeed(SharedHandle<ServerStat> ss)
{
  return std::max(ss->getSingleConnectionAvgSpeed(),
		  ss->getMultiConnectionAvgSpeed());
}

unsigned int AdaptiveURISelector::getMaxDownloadSpeed
(const std::deque<std::string>& uris) const
{
  std::string uri = getMaxDownloadSpeedUri(uris);
  if(uri == A2STR::NIL)
    return 0;
  return getUriMaxSpeed(getServerStats(uri));
}

std::string AdaptiveURISelector::getMaxDownloadSpeedUri
(const std::deque<std::string>& uris) const
{
  int max = -1;
  std::string uri = A2STR::NIL;
  for(std::deque<std::string>::const_iterator i = uris.begin();
      i != uris.end(); ++i) {
    SharedHandle<ServerStat> ss = getServerStats(*i);
    if(ss.isNull())
      continue;

    if((int)ss->getSingleConnectionAvgSpeed() > max) {
      max = ss->getSingleConnectionAvgSpeed();
      uri = (*i);
    }
    if((int)ss->getMultiConnectionAvgSpeed() > max) {
      max = ss->getMultiConnectionAvgSpeed();
      uri = (*i);
    }
  }
  return uri;
}

std::deque<std::string> AdaptiveURISelector::getUrisBySpeed
(const std::deque<std::string>& uris, unsigned int min) const
{
  std::deque<std::string> bests;
  for(std::deque<std::string>::const_iterator i = uris.begin();
          i != uris.end(); ++i) {
    SharedHandle<ServerStat> ss = getServerStats(*i);
    if(ss.isNull())
      continue;
    if(ss->getSingleConnectionAvgSpeed() > min ||
            ss->getMultiConnectionAvgSpeed() > min) {
      bests.push_back(*i);
    }
  }
  return bests;
}

std::string AdaptiveURISelector::selectRandomUri
(const std::deque<std::string>& uris) const
{
  int pos = SimpleRandomizer::getInstance()->getRandomNumber(uris.size());
  std::deque<std::string>::const_iterator i = uris.begin();
  i = i+pos;
  return *i;
}

std::string AdaptiveURISelector::getFirstNotTestedUri
(const std::deque<std::string>& uris) const
{
  for(std::deque<std::string>::const_iterator i = uris.begin(); 
      i != uris.end(); ++i) {
    SharedHandle<ServerStat> ss = getServerStats(*i);
    if(ss.isNull())
      return *i;
  }
  return A2STR::NIL;
}

std::string AdaptiveURISelector::getFirstToTestUri
(const std::deque<std::string>& uris) const
{
  unsigned int counter;
  int power;
  for(std::deque<std::string>::const_iterator i = uris.begin();
          i != uris.end(); ++i) {
    SharedHandle<ServerStat> ss = getServerStats(*i);
    if(ss.isNull())
      continue;
    counter = ss->getCounter();
    if(counter > 8)
      continue;
    power = (int)pow(2.0, (float)counter);
    /* We test the mirror another time if it has not been
     * tested since 2^counter days */
    if(ss->getLastUpdated().difference() > power*24*60*60) {
      return *i;
    }
  }
  return A2STR::NIL;
}

SharedHandle<ServerStat> AdaptiveURISelector::getServerStats
(const std::string& uri) const
{
  Request r;
  r.setUrl(uri);
  return _serverStatMan->find(r.getHost(), r.getProtocol());
}

unsigned int AdaptiveURISelector::getNbTestedServers
(const std::deque<std::string>& uris) const
{
  unsigned int counter = 0;
  for(std::deque<std::string>::const_iterator i = uris.begin();
      i != uris.end(); ++i) {
    SharedHandle<ServerStat> ss = getServerStats(*i);
    if(ss.isNull())
      counter++;
  }
  return uris.size() - counter;
}

} // namespace aria2
