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
#ifndef _D_REQUEST_GROUP_MAN_H_
#define _D_REQUEST_GROUP_MAN_H_

#include "common.h"
#include "RequestGroup.h"
#include "LogFactory.h"

class DownloadEngine;

class RequestGroupMan {
private:
  RequestGroups _requestGroups;
  RequestGroups _reservedGroups;
  const Logger* _logger;
  int32_t _maxSimultaneousDownloads;
public:
  RequestGroupMan(const RequestGroups& requestGroups = RequestGroups(), int32_t maxSimultaneousDownloads = 1):
    _requestGroups(requestGroups),
    _logger(LogFactory::getInstance()),
    _maxSimultaneousDownloads(maxSimultaneousDownloads) {}

  bool downloadFinished()
  {
    for(RequestGroups::iterator itr = _requestGroups.begin();
	itr != _requestGroups.end(); ++itr) {
      if((*itr)->numConnection > 0 || !(*itr)->getSegmentMan()->finished()) {
	return false;
      }
    }
    return true;
  }

  void save()
  {
    for(RequestGroups::iterator itr = _requestGroups.begin();
	itr != _requestGroups.end(); ++itr) {
      if(!(*itr)->getSegmentMan()->finished()) {
	(*itr)->getSegmentMan()->save();
      }
    }
  }

  void closeFile()
  {
    for(RequestGroups::iterator itr = _requestGroups.begin();
	itr != _requestGroups.end(); ++itr) {
      (*itr)->getSegmentMan()->diskWriter->closeFile();
    }
  }
  
  int64_t getDownloadLength() const
  {
    int64_t downloadLength = 0;
    for(RequestGroups::const_iterator itr = _requestGroups.begin();
	itr != _requestGroups.end(); ++itr) {
      downloadLength += (*itr)->getSegmentMan()->getDownloadLength();
    }
    return downloadLength;
  }

  int64_t getTotalLength() const
  {
    int64_t totalLength = 0;
    for(RequestGroups::const_iterator itr = _requestGroups.begin();
	itr != _requestGroups.end(); ++itr) {
      totalLength += (*itr)->getSegmentMan()->totalSize;
    }
    return totalLength;
  }

  Commands getInitialCommands(DownloadEngine* e) const;

  void removeStoppedGroup();

  void fillRequestGroupFromReserver(DownloadEngine* e);

  void addRequestGroup(const RequestGroupHandle& group)
  {
    _requestGroups.push_back(group);
  }

  void addReservedGroup(const RequestGroups& groups)
  {
    _reservedGroups.insert(_reservedGroups.end(), groups.begin(), groups.end());
  }

  void addReservedGroup(const RequestGroupHandle& group)
  {
    _reservedGroups.push_back(group);
  }

  int32_t countRequestGroup() const
  {
    return _requestGroups.size();
  }
		  
  RequestGroupHandle getRequestGroup(int32_t index) const
  {
    if(index < (int32_t)_requestGroups.size()) {
      return _requestGroups[index];
    } else {
      return 0;
    }
  }
		  
  void showDownloadResults(ostream& o) const;

  int32_t getErrors() const
  {
    int32_t errors = 0;
    for(RequestGroups::const_iterator itr = _requestGroups.begin();
	itr != _requestGroups.end(); ++itr) {
      errors += (*itr)->getSegmentMan()->errors;
    }
    return errors;
  }
};

typedef SharedHandle<RequestGroupMan> RequestGroupManHandle;

#endif // _D_REQUEST_GROUP_MAN_H_
