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
#ifndef _D_REQUEST_GROUP_ENTRY_H_
#define _D_REQUEST_GROUP_ENTRY_H_

#include "ProgressAwareEntry.h"
#include "Request.h"
#include "RequestGroup.h"

class DownloadCommand;

class RequestGroupEntry : public ProgressAwareEntry {
protected:
  int _cuid;
  RequestHandle _currentRequest;
  RequestGroup* _requestGroup;
  DownloadCommand* _nextDownloadCommand;
  bool _shouldAddNumConnection;
public:
  RequestGroupEntry(int cuid,
		    const RequestHandle& currentRequest,
		    RequestGroup* requestGroup,
		    DownloadCommand* nextDownloadCommand = 0):
    _cuid(cuid),
    _currentRequest(currentRequest),
    _requestGroup(requestGroup),
    _nextDownloadCommand(nextDownloadCommand)
  {
    if(nextDownloadCommand) {
      _shouldAddNumConnection = false;
    } else {
      _shouldAddNumConnection = true;
      ++_requestGroup->numConnection;
    }
  }

  virtual ~RequestGroupEntry();

  virtual int64_t getTotalLength() const
  {
    return _requestGroup->getTotalLength();
  }

  int getCUID() const
  {
    return _cuid;
  }

  RequestHandle getCurrentRequest() const
  {
    return _currentRequest;
  }

  RequestGroup* getRequestGroup() const
  {
    return _requestGroup;
  }
  /*
  void setNextDownloadCommand(DownloadCommand* command)
  {
    _nextDownloadCommand = command;
  }
  */
  DownloadCommand* getNextDownloadCommand() const
  {
    return _nextDownloadCommand;
  }

  DownloadCommand* popNextDownloadCommand()
  {
    DownloadCommand* temp = _nextDownloadCommand;
    _nextDownloadCommand = 0;
    return temp;
  }
  
  bool operator==(const RequestGroupEntry& entry) const
  {
    return this == &entry;
  }
};

typedef SharedHandle<RequestGroupEntry> RequestGroupEntryHandle;
#endif // _D_REQUEST_GROUP_ENTRY_H_
