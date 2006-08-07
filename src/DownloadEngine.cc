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

DownloadEngine::DownloadEngine():noWait(false), segmentMan(0) {
  logger = LogFactory::getInstance();
}

DownloadEngine::~DownloadEngine() {
  delete segmentMan;
  cleanQueue();
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
  CommandUuids activeUuids;
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
      for(CommandUuids::iterator itr = activeUuids.begin();
	  itr != activeUuids.end(); itr++) {
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
    activeUuids.clear();
    if(!noWait && !commands.empty()) {
      waitData(activeUuids);
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
  int* max_ptr;
  fd_set* rfds_ptr;
  fd_set* wfds_ptr;
public:
  SetDescriptor(int* max_ptr, fd_set* rfds_ptr, fd_set* wfds_ptr):
    max_ptr(max_ptr),
    rfds_ptr(rfds_ptr),
    wfds_ptr(wfds_ptr) {}

  void operator()(const SocketEntry& entry) {
    int fd = entry.socket->getSockfd();
    switch(entry.type) {
    case SocketEntry::TYPE_RD:
      FD_SET(fd, rfds_ptr);
      break;
    case SocketEntry::TYPE_WR:
      FD_SET(fd, wfds_ptr);
      break;
    }
    if(*max_ptr < fd) {
      *max_ptr = fd;
    }
  }
};

class AccumulateActiveUuid {
private:
  CommandUuids* activeUuids_ptr;
  fd_set* rfds_ptr;
  fd_set* wfds_ptr;
public:
  AccumulateActiveUuid(CommandUuids* activeUuids_ptr,
		       fd_set* rfds_ptr,
		       fd_set* wfds_ptr):
    activeUuids_ptr(activeUuids_ptr),
    rfds_ptr(rfds_ptr),
    wfds_ptr(wfds_ptr) {}

  void operator()(const SocketEntry& entry) {
    if(FD_ISSET(entry.socket->getSockfd(), rfds_ptr) ||
       FD_ISSET(entry.socket->getSockfd(), wfds_ptr)) {
      activeUuids_ptr->push_back(entry.commandUuid);
    }
    /*
    switch(entry.type) {
    case SocketEntry::TYPE_RD:
      if(FD_ISSET(entry.socket->getSockfd(), rfds_ptr)) {
	activeUuids_ptr->push_back(entry.commandUuid);
      }
      break;
    case SocketEntry::TYPE_WR:
      if(FD_ISSET(entry.socket->getSockfd(), wfds_ptr)) {
	activeUuids_ptr->push_back(entry.commandUuid);
      }
      break;
    }
    */
  }
};

void DownloadEngine::waitData(CommandUuids& activeUuids) {
  fd_set rfds;
  fd_set wfds;
  int retval = 0;
  struct timeval tv;
  
  memcpy(&rfds, &rfdset, sizeof(fd_set));
  memcpy(&wfds, &wfdset, sizeof(fd_set));
  
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  retval = select(fdmax+1, &rfds, &wfds, NULL, &tv);
  if(retval > 0) {
    for_each(socketEntries.begin(), socketEntries.end(),
	     AccumulateActiveUuid(&activeUuids, &rfds, &wfds));
	  
    sort(activeUuids.begin(), activeUuids.end());
    activeUuids.erase(unique(activeUuids.begin(),
			     activeUuids.end()),
		      activeUuids.end());
  }
}

void DownloadEngine::updateFdSet() {
  fdmax = 0;
  FD_ZERO(&rfdset);
  FD_ZERO(&wfdset);
  for_each(socketEntries.begin(), socketEntries.end(),
	   SetDescriptor(&fdmax, &rfdset, &wfdset));
}

bool DownloadEngine::addSocket(const SocketEntry& entry) {
  SocketEntries::iterator itr =
    find(socketEntries.begin(), socketEntries.end(), entry);
  if(itr == socketEntries.end()) {
    socketEntries.push_back(entry);
    updateFdSet();
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::deleteSocket(const SocketEntry& entry) {
  SocketEntries::iterator itr =
    find(socketEntries.begin(), socketEntries.end(), entry);
  if(itr == socketEntries.end()) {
    return false;
  } else {
    socketEntries.erase(itr);
    updateFdSet();
    return true;
  }
}

bool DownloadEngine::addSocketForReadCheck(const SocketHandle& socket,
					   const CommandUuid& commandUuid) {
  SocketEntry entry(socket, commandUuid, SocketEntry::TYPE_RD);
  return addSocket(entry);
}

bool DownloadEngine::deleteSocketForReadCheck(const SocketHandle& socket,
					      const CommandUuid& commandUuid) {
  SocketEntry entry(socket, commandUuid, SocketEntry::TYPE_RD);
  return deleteSocket(entry);
}

bool DownloadEngine::addSocketForWriteCheck(const SocketHandle& socket,
					    const CommandUuid& commandUuid) {
  SocketEntry entry(socket, commandUuid, SocketEntry::TYPE_WR);
  return addSocket(entry);
}

bool DownloadEngine::deleteSocketForWriteCheck(const SocketHandle& socket,
					       const CommandUuid& commandUuid) {
  SocketEntry entry(socket, commandUuid, SocketEntry::TYPE_WR);
  return deleteSocket(entry);
}
