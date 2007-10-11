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
#include "DlAbortEx.h"
#include "RequestGroup.h"
#include "LogFactory.h"
#include "DownloadEngine.h"
#include "message.h"
#include <iomanip>
#include <sstream>

RequestGroupMan::RequestGroupMan(const RequestGroups& requestGroups,
				 int32_t maxSimultaneousDownloads):
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

int32_t RequestGroupMan::countRequestGroup() const
{
  return _requestGroups.size();
}

RequestGroupHandle RequestGroupMan::getRequestGroup(int32_t index) const
{
  if(index < (int32_t)_requestGroups.size()) {
    return _requestGroups[index];
  } else {
    return 0;
  }
}

void RequestGroupMan::removeStoppedGroup()
{
  int32_t count = 0;
  RequestGroups temp;
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr)->getNumCommand() > 0) {
      temp.push_back(*itr);
    } else {
      (*itr)->closeFile();
      RequestGroups nextGroups = (*itr)->postDownloadProcessing();
      
      if((*itr)->downloadFinished()) {
	_logger->notice(MSG_FILE_DOWNLOAD_COMPLETED,
			(*itr)->getFilePath().c_str());
	(*itr)->getProgressInfoFile()->removeFile();
      } else {
	try {
	  (*itr)->getProgressInfoFile()->save();
	} catch(DlAbortEx* ex) {
	  _logger->error(EX_EXCEPTION_CAUGHT, ex);
	  delete ex;
	}
      }
      (*itr)->releaseRuntimeResource();
      ++count;
      _spentGroups.push_back(*itr);
    }
  }
  _requestGroups = temp;
  if(count > 0) {
    _logger->debug("%d RequestGroup(s) deleted.", count);
  }
}

void RequestGroupMan::fillRequestGroupFromReserver(DownloadEngine* e)
{
  RequestGroups temp;
  removeStoppedGroup();
  int32_t count = 0;
  for(int32_t num = _maxSimultaneousDownloads-_requestGroups.size();
      num > 0 && _reservedGroups.size() > 0; --num) {
    RequestGroupHandle groupToAdd = _reservedGroups.front();
    _reservedGroups.pop_front();
    if(!groupToAdd->isDependencyResolved()) {
      temp.push_front(groupToAdd);
      continue;
    }
    try {
      _requestGroups.push_back(groupToAdd);
      Commands commands = groupToAdd->createInitialCommand(e);
      ++count;
      e->addCommand(commands);
    } catch(DlAbortEx* ex) {
      _logger->error(EX_EXCEPTION_CAUGHT, ex);
      delete ex;
    }
  }
  copy(temp.begin(), temp.end(), front_inserter(_reservedGroups));
  if(count > 0) {
    _logger->debug("%d RequestGroup(s) added.", count);
  }
}

Commands RequestGroupMan::getInitialCommands(DownloadEngine* e)
{
  Commands commands;
  for(RequestGroups::iterator itr = _requestGroups.begin();
	itr != _requestGroups.end();) {
    if((*itr)->isDependencyResolved()) {
      try {
	Commands nextCommands = (*itr)->createInitialCommand(e);
	copy(nextCommands.begin(), nextCommands.end(), back_inserter(commands));
	++itr;
      } catch(DlAbortEx* e) {
	_logger->error(EX_EXCEPTION_CAUGHT, e);
	delete e;
      }
    } else {
      _reservedGroups.push_front((*itr));
      itr = _requestGroups.erase(itr);
    }
  }
  return commands;
}

void RequestGroupMan::save()
{
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if(!(*itr)->downloadFinished()) {
      try {
	(*itr)->getProgressInfoFile()->save();
      } catch(DlAbortEx* e) {
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

void RequestGroupMan::showDownloadResults(ostream& o) const
{
  // Download Results:
  // idx|stat|path/length
  // ===+====+=======================================================================
  o << "\n"
    <<_("Download Results:") << "\n"
    << "idx|stat|path/URI" << "\n"
    << "===+====+======================================================================" << "\n";
  /*
  RequestGroups groups(_spentGroups.begin(), _spentGroups.end());
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr)->downloadFinished()) {
      groups.push_back(*itr);
    }
  }
  */
  for(RequestGroups::const_iterator itr = _spentGroups.begin();
      itr != _spentGroups.end(); ++itr) {
    o << formatDownloadResult((*itr)->downloadFinished()?"OK":"ERR", *itr) << "\n";
  }
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    o << formatDownloadResult((*itr)->downloadFinished()?"OK":"INPR", *itr) << "\n";
  }
}

string RequestGroupMan::formatDownloadResult(const string& status, const RequestGroupHandle& requestGroup) const
{
  stringstream o;
  o << setw(3) << requestGroup->getGID() << "|"
    << setw(4) << status << "|";
  if(requestGroup->downloadFinished()) {
    o << requestGroup->getFilePath();
  } else {
    Strings uris = requestGroup->getUris();
    if(uris.empty()) {
      if(requestGroup->getFilePath().empty()) {
	o << "n/a";
      } else {
	o << requestGroup->getFilePath();
      }
    } else {
      o << uris.front();
      if(uris.size() > 1) {
	o << " (" << uris.size()-1 << "more)";
      }
    }
  }
  return o.str();
}

bool RequestGroupMan::isSameFileBeingDownloaded(RequestGroup* requestGroup) const
{
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

TransferStat RequestGroupMan::calculateStat()
{
  TransferStat stat;
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    stat = stat+(*itr)->calculateStat();
  }
  return stat;
}
