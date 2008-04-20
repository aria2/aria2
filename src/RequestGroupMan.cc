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
#include "RequestGroupMan.h"
#include "BtProgressInfoFile.h"
#include "RecoverableException.h"
#include "RequestGroup.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DownloadEngine.h"
#include "message.h"
#include "a2functional.h"
#include "DownloadResult.h"
#include <iomanip>
#include <sstream>
#include <numeric>
#include <algorithm>

namespace aria2 {

RequestGroupMan::RequestGroupMan(const RequestGroups& requestGroups,
				 unsigned int maxSimultaneousDownloads):
  _requestGroups(requestGroups),
  _logger(LogFactory::getInstance()),
  _maxSimultaneousDownloads(maxSimultaneousDownloads),
  _gidCounter(0) {}

bool RequestGroupMan::downloadFinished()
{
  if(!_reservedGroups.empty()) {
    return false;
  }
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr)->getNumCommand() > 0 || !(*itr)->downloadFinished()) {
      return false;
    }
  }
  return true;
}

void RequestGroupMan::addRequestGroup(const RequestGroupHandle& group)
{
  _requestGroups.push_back(group);
}

void RequestGroupMan::addReservedGroup(const RequestGroups& groups)
{
  _reservedGroups.insert(_reservedGroups.end(), groups.begin(), groups.end());
}

void RequestGroupMan::addReservedGroup(const RequestGroupHandle& group)
{
  _reservedGroups.push_back(group);
}

size_t RequestGroupMan::countRequestGroup() const
{
  return _requestGroups.size();
}

RequestGroupHandle RequestGroupMan::getRequestGroup(size_t index) const
{
  if(index < _requestGroups.size()) {
    return _requestGroups[index];
  } else {
    return SharedHandle<RequestGroup>();
  }
}

void RequestGroupMan::removeStoppedGroup()
{
  unsigned int count = 0;
  RequestGroups temp;
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr)->getNumCommand() > 0) {
      temp.push_back(*itr);
    } else {
      try {
	(*itr)->closeFile();      
	if((*itr)->downloadFinished()) {
	  (*itr)->reportDownloadFinished();
	  if((*itr)->allDownloadFinished()) {
	    (*itr)->getProgressInfoFile()->removeFile();
	  } else {
	    (*itr)->getProgressInfoFile()->save();
	  }
	  RequestGroups nextGroups = (*itr)->postDownloadProcessing();
	  if(nextGroups.size() > 0) {
	    _logger->debug("Adding %u RequestGroups as a result of PostDownloadHandler.", nextGroups.size());
	    std::copy(nextGroups.rbegin(), nextGroups.rend(), std::front_inserter(_reservedGroups));
	  }
	} else {
	  (*itr)->getProgressInfoFile()->save();
	}
      } catch(RecoverableException* ex) {
	_logger->error(EX_EXCEPTION_CAUGHT, ex);
	delete ex;
      }
      (*itr)->releaseRuntimeResource();
      ++count;
      _downloadResults.push_back((*itr)->createDownloadResult());
    }
  }
  _requestGroups = temp;
  if(count > 0) {
    _logger->debug("%u RequestGroup(s) deleted.", count);
  }
}

void RequestGroupMan::fillRequestGroupFromReserver(DownloadEngine* e)
{
  RequestGroups temp;
  removeStoppedGroup();
  unsigned int count = 0;
  for(int num = _maxSimultaneousDownloads-_requestGroups.size();
      num > 0 && _reservedGroups.size() > 0; --num) {
    RequestGroupHandle groupToAdd = _reservedGroups.front();
    _reservedGroups.pop_front();
    try {
      if(!groupToAdd->isDependencyResolved()) {
	temp.push_front(groupToAdd);
	continue;
      }
      Commands commands = groupToAdd->createInitialCommand(e);
      _requestGroups.push_back(groupToAdd);
      ++count;
      e->addCommand(commands);
    } catch(RecoverableException* ex) {
      _logger->error(EX_EXCEPTION_CAUGHT, ex);
      delete ex;
      _downloadResults.push_back(groupToAdd->createDownloadResult());
    }
  }
  std::copy(temp.begin(), temp.end(), std::front_inserter(_reservedGroups));
  if(count > 0) {
    e->setNoWait(true);
    _logger->debug("%d RequestGroup(s) added.", count);
  }
}

Commands RequestGroupMan::getInitialCommands(DownloadEngine* e)
{
  Commands commands;
  for(RequestGroups::iterator itr = _requestGroups.begin();
	itr != _requestGroups.end();) {
    try {
      if((*itr)->isDependencyResolved()) {
	Commands nextCommands = (*itr)->createInitialCommand(e);
	std::copy(nextCommands.begin(), nextCommands.end(), std::back_inserter(commands));
	++itr;
      } else {
	_reservedGroups.push_front((*itr));
	itr = _requestGroups.erase(itr);
      }
    } catch(RecoverableException* e) {
      _logger->error(EX_EXCEPTION_CAUGHT, e);
      delete e;
      _downloadResults.push_back((*itr)->createDownloadResult());
      itr = _requestGroups.erase(itr);
    }  
  }
  return commands;
}

void RequestGroupMan::save()
{
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr)->allDownloadFinished()) {
      (*itr)->getProgressInfoFile()->removeFile();
    } else {
      try {
	(*itr)->getProgressInfoFile()->save();
      } catch(RecoverableException* e) {
	_logger->error(EX_EXCEPTION_CAUGHT, e);
	delete e;
      }
    }
  }
}

void RequestGroupMan::closeFile()
{
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    (*itr)->closeFile();
  }
}

RequestGroupMan::DownloadStat RequestGroupMan::getDownloadStat() const
{
  DownloadStat stat;
  size_t finished = 0;
  size_t error = 0;
  size_t inprogress = 0;
  for(std::deque<SharedHandle<DownloadResult> >::const_iterator itr = _downloadResults.begin();
      itr != _downloadResults.end(); ++itr) {
    if((*itr)->result == DownloadResult::FINISHED) {
      ++finished;
    } else {
      ++error;
    }
  }
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    DownloadResultHandle result = (*itr)->createDownloadResult();
    if(result->result == DownloadResult::FINISHED) {
      ++finished;
    } else {
      ++inprogress;
    }
  }
  stat.setCompleted(finished);
  stat.setError(error);
  stat.setInProgress(inprogress);
  stat.setWaiting(_reservedGroups.size());
  return stat;
}

void RequestGroupMan::showDownloadResults(std::ostream& o) const
{
  // Download Results:
  // idx|stat|path/length
  // ===+====+=======================================================================
  o << "\n"
    <<_("Download Results:") << "\n"
    << "gid|stat|path/URI" << "\n"
    << "===+====+======================================================================" << "\n";
  for(std::deque<SharedHandle<DownloadResult> >::const_iterator itr = _downloadResults.begin();
      itr != _downloadResults.end(); ++itr) {
    std::string status = (*itr)->result == DownloadResult::FINISHED ? "OK" : "ERR";
    o << formatDownloadResult(status, *itr) << "\n";
  }
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    DownloadResultHandle result = (*itr)->createDownloadResult();
    std::string status = result->result == DownloadResult::FINISHED ? "OK" : "INPR";
    o << formatDownloadResult(status, result) << "\n";
  }
  o << "\n"
    << _("Status Legend:") << "\n"
    << " (OK):download completed.(ERR):error occurred.(INPR):download in-progress." << "\n";
}

std::string RequestGroupMan::formatDownloadResult(const std::string& status, const DownloadResultHandle& downloadResult) const
{
  std::stringstream o;
  o << std::setw(3) << downloadResult->gid << "|"
    << std::setw(4) << status << "|";
  if(downloadResult->result == DownloadResult::FINISHED) {
    o << downloadResult->filePath;
  } else {
    if(downloadResult->numUri == 0) {
      if(downloadResult->filePath.empty()) {
	o << "n/a";
      } else {
	o << downloadResult->filePath;
      }
    } else {
      o << downloadResult->uri;
      if(downloadResult->numUri > 1) {
	o << " (" << downloadResult->numUri-1 << "more)";
      }
    }
  }
  return o.str();
}

bool RequestGroupMan::isSameFileBeingDownloaded(RequestGroup* requestGroup) const
{
  // TODO it may be good to use dedicated method rather than use isPreLocalFileCheckEnabled
  if(!requestGroup->isPreLocalFileCheckEnabled()) {
    return false;
  }
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr).get() != requestGroup &&
       (*itr)->getFilePath() == requestGroup->getFilePath()) {
      return true;
    }
  }
  return false;
}

void RequestGroupMan::halt()
{
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    (*itr)->setHaltRequested(true);
  }
}

void RequestGroupMan::forceHalt()
{
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    (*itr)->setForceHaltRequested(true);
  }
}

TransferStat RequestGroupMan::calculateStat()
{
  return std::accumulate(_requestGroups.begin(), _requestGroups.end(), TransferStat(),
			 adopt2nd(std::plus<TransferStat>(), mem_fun_sh(&RequestGroup::calculateStat)));
}

} // namespace aria2
