/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "DownloadEngine.h"
#include "Util.h"
#include "LogFactory.h"
#include "Time.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <algorithm>

using namespace std;

DownloadEngine::DownloadEngine():noWait(false), segmentMan(NULL) {
  logger = LogFactory::getInstance();
}

DownloadEngine::~DownloadEngine() {
  if(segmentMan != NULL) {
    delete segmentMan;
  }
}

void DownloadEngine::cleanQueue() {
  for_each(commands.begin(), commands.end(), Deleter());
  commands.clear();
}


class FindCommand {
private:
  CommandUuid uuid;
public:
  FindCommand(const CommandUuid& uuid):uuid(uuid) {}

  bool operator()(const Command* command) {
    if(command->getUuid() == uuid) {
      return true;
    } else {
      return false;
    }
  }
};

void DownloadEngine::run() {
  initStatistics();
  Time cp;
  CommandUuids activeCommandUuids;
  while(!commands.empty()) {
    if(cp.elapsed(1)) {
      cp.reset();
      int max = commands.size();
      for(int i = 0; i < max; i++) {
	Command* com = commands.front();
	commands.pop_front();
	if(com->execute()) {
	  delete com;
	}
      }
    } else {
      for(CommandUuids::iterator itr = activeCommandUuids.begin();
	  itr != activeCommandUuids.end(); itr++) {
	Commands::iterator comItr = find_if(commands.begin(), commands.end(),
					    FindCommand(*itr));
	assert(comItr != commands.end());
	Command* com = *comItr;
	commands.erase(comItr);
	if(com->execute()) {
	  delete com;
	}
      }
    }
    afterEachIteration();
    activeCommandUuids.clear();
    if(!noWait && !commands.empty()) {
      waitData(activeCommandUuids);
    }
    noWait = false;
    calculateStatistics();
  }
  onEndOfRun();
}

void DownloadEngine::shortSleep() const {
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000;
  fd_set rfds;
  FD_ZERO(&rfds);
  select(0, &rfds, NULL, NULL, &tv);
}

class SetDescriptor {
private:
  fd_set* fds_ptr;
  int* max_ptr;
public:
  SetDescriptor(int* max_ptr, fd_set* fds_ptr)
    :fds_ptr(fds_ptr), max_ptr(max_ptr) {}

  void operator()(const pair<SocketHandle, CommandUuid>& pa) {
    int fd = pa.first->getSockfd();
    FD_SET(fd, fds_ptr);
    if(*max_ptr < fd) {
      *max_ptr = fd;
    }
  }
};

class AccumulateActiveCommandUuid {
private:
  CommandUuids* activeCommandUuids_ptr;
  fd_set* fds_ptr;
public:
  AccumulateActiveCommandUuid(CommandUuids* activeCommandUuids_ptr,
			      fd_set* fds_ptr)
    :activeCommandUuids_ptr(activeCommandUuids_ptr), fds_ptr(fds_ptr) {}

  void operator()(const pair<SocketHandle, CommandUuid>& pa) {
    if(FD_ISSET(pa.first->getSockfd(), fds_ptr)) {
      activeCommandUuids_ptr->push_back(pa.second);
    }
  }
};

void DownloadEngine::waitData(CommandUuids& activeCommandUuids) {
  fd_set rfds;
  fd_set wfds;
  int retval = 0;
  while(1) {
    struct timeval tv;
    
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    int max = 0;
    for_each(rsockmap.begin(), rsockmap.end(), SetDescriptor(&max, &rfds));
    for_each(wsockmap.begin(), wsockmap.end(), SetDescriptor(&max, &wfds));

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    retval = select(max+1, &rfds, &wfds, NULL, &tv);
    if(retval != -1 || errno != EINTR) {
      break;
    }
  }
  if(retval > 0) {
    for_each(rsockmap.begin(), rsockmap.end(),
	     AccumulateActiveCommandUuid(&activeCommandUuids, &rfds));
    for_each(wsockmap.begin(), wsockmap.end(),
	     AccumulateActiveCommandUuid(&activeCommandUuids, &wfds));
    sort(activeCommandUuids.begin(), activeCommandUuids.end());
    activeCommandUuids.erase(unique(activeCommandUuids.begin(),
				activeCommandUuids.end()),
			 activeCommandUuids.end());
  }
}

bool DownloadEngine::addSocket(SockCmdMap& sockmap,
			       const SocketHandle& socket,
			       const CommandUuid& commandUuid) {
  SockCmdMap::iterator itr = find_if(sockmap.begin(), sockmap.end(),
				     PairFind<SocketHandle, CommandUuid>(socket, commandUuid));
  if(itr == sockmap.end()) {
    SockCmdMap::value_type vt(socket, commandUuid);
    sockmap.insert(vt);
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::deleteSocket(SockCmdMap& sockmap,
				  const SocketHandle& socket,
				  const CommandUuid& commandUuid) {
  SockCmdMap::iterator itr = find_if(sockmap.begin(), sockmap.end(),
				     PairFind<SocketHandle, CommandUuid>(socket, commandUuid));
  if(itr == sockmap.end()) {
    return false;
  } else {
    sockmap.erase(itr);
    return true;
  }
}

bool DownloadEngine::addSocketForReadCheck(const SocketHandle& socket,
					   const CommandUuid& commandUuid) {
  return addSocket(rsockmap, socket, commandUuid);
}

bool DownloadEngine::deleteSocketForReadCheck(const SocketHandle& socket,
					      const CommandUuid& commandUuid) {
  return deleteSocket(rsockmap, socket, commandUuid);
}

bool DownloadEngine::addSocketForWriteCheck(const SocketHandle& socket,
					    const CommandUuid& commandUuid) {
  return addSocket(wsockmap, socket, commandUuid);
}

bool DownloadEngine::deleteSocketForWriteCheck(const SocketHandle& socket,
					       const CommandUuid& commandUuid) {
  return deleteSocket(wsockmap, socket, commandUuid);
}
