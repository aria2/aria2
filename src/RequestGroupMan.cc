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
#include "DownloadEngine.h"
#include <iomanip>

void RequestGroupMan::removeStoppedGroup()
{
  int32_t count = 0;
  RequestGroups temp;
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr)->numConnection > 0) {
      temp.push_back(*itr);
    } else {
      (*itr)->closeFile();
      if((*itr)->downloadFinished()) {
	_logger->notice("Download complete: %s", (*itr)->getFilePath().c_str());
	(*itr)->remove();
      } else {
	(*itr)->save();
      }
      ++count;
    }
  }
  _requestGroups = temp;
  if(count > 0) {
    _logger->debug("%d RequestGroup(s) deleted.", count);
  }
}

void RequestGroupMan::fillRequestGroupFromReserver(DownloadEngine* e)
{
  removeStoppedGroup();

  int32_t count = 0;
  for(int32_t num = _maxSimultaneousDownloads-_requestGroups.size();
      num > 0 && _reservedGroups.size() > 0; --num) {

    RequestGroupHandle groupToAdd = _reservedGroups.front();
    _reservedGroups.pop_front();
    
    _requestGroups.push_back(groupToAdd);
    groupToAdd->initSegmentMan();
    groupToAdd->setGID(++_gidCounter);
    Commands commands = groupToAdd->createNextCommand(e, 1);
    count += commands.size();
    e->addCommand(commands);
  }
  if(count > 0) {
    _logger->debug("%d RequestGroup(s) added.", count);
  }
}

Commands RequestGroupMan::getInitialCommands(DownloadEngine* e)
{
  Commands commands;
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
	itr != _requestGroups.end(); ++itr) {
    (*itr)->initSegmentMan();
    (*itr)->setGID(++_gidCounter);
    Commands nextCommands = (*itr)->createNextCommand(e, 1);
    if(!nextCommands.empty()) {
      commands.push_back(nextCommands.front());
    }
  }
  return commands;
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
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    o << setw(3) << (*itr)->getGID() << "|";
    if((*itr)->downloadFinished()) {
      o << "OK  ";
    } else {
      o << "ERR ";
    }
    o << "|";
    if((*itr)->downloadFinished()) {
      o << (*itr)->getFilePath();
    } else {
      Strings uris = (*itr)->getUris();
      if(uris.empty()) {
	o << "n/a";
      } else {
	o << uris.front();
	if(uris.size() > 1) {
	  o << " (" << uris.size()-1 << "more)";
	}
      }
    }
    o << "\n";
  }
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
