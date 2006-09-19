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
#include "TimeA2.h"
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
  cleanQueue();
  delete segmentMan;
}

void DownloadEngine::cleanQueue() {
  for_each(commands.begin(), commands.end(), Deleter());
  commands.clear();
}

void DownloadEngine::run() {
  initStatistics();
  Time cp;
  cp.setTimeInSec(0);
  Commands activeCommands;
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
      for(Commands::iterator itr = activeCommands.begin();
	  itr != activeCommands.end(); itr++) {
	Commands::iterator comItr = find(commands.begin(), commands.end(),
					 *itr);
	assert(comItr != commands.end());
	Command* command = *itr;
	commands.erase(comItr);
	if(command->execute()) {
	  delete command;
	}
      }
    }
    afterEachIteration();
    activeCommands.clear();
    if(!noWait && !commands.empty()) {
      waitData(activeCommands);
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
#ifdef ENABLE_ASYNC_DNS
  void operator()(const NameResolverEntry& entry) {
    int tempFd = entry.nameResolver->getFds(rfds_ptr, wfds_ptr);
    if(*max_ptr < tempFd) {
      *max_ptr = tempFd;
    }
  }
#endif // ENABLE_ASYNC_DNS
};

class AccumulateActiveCommand {
private:
  Commands* activeCommands_ptr;
  fd_set* rfds_ptr;
  fd_set* wfds_ptr;
public:
  AccumulateActiveCommand(Commands* activeCommands_ptr,
		       fd_set* rfds_ptr,
		       fd_set* wfds_ptr):
    activeCommands_ptr(activeCommands_ptr),
    rfds_ptr(rfds_ptr),
    wfds_ptr(wfds_ptr) {}

  void operator()(const SocketEntry& entry) {
    if(FD_ISSET(entry.socket->getSockfd(), rfds_ptr) ||
       FD_ISSET(entry.socket->getSockfd(), wfds_ptr)) {
      activeCommands_ptr->push_back(entry.command);
    }
    /*
    switch(entry.type) {
    case SocketEntry::TYPE_RD:
      if(FD_ISSET(entry.socket->getSockfd(), rfds_ptr)) {
      activeCommands_ptr->push_back(entry.command);
      }
      break;
    case SocketEntry::TYPE_WR:
      if(FD_ISSET(entry.socket->getSockfd(), wfds_ptr)) {
	activeCommands_ptr->push_back(entry.command);
      }
      break;
    }
    */
  }
#ifdef ENABLE_ASYNC_DNS
  void operator()(const NameResolverEntry& entry) {
    entry.nameResolver->process(rfds_ptr, wfds_ptr);
    switch(entry.nameResolver->getStatus()) {
    case NameResolver::STATUS_SUCCESS:
    case NameResolver::STATUS_ERROR:
      activeCommands_ptr->push_back(entry.command);
      break;
    default:
      break;
    }
  }
#endif // ENABLE_ASYNC_DNS
};

void DownloadEngine::waitData(Commands& activeCommands) {
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
	     AccumulateActiveCommand(&activeCommands, &rfds, &wfds));
#ifdef ENABLE_ASYNC_DNS
    for_each(nameResolverEntries.begin(), nameResolverEntries.end(),
	     AccumulateActiveCommand(&activeCommands, &rfds, &wfds));
#endif // ENABLE_ASYNC_DNS
    sort(activeCommands.begin(), activeCommands.end());
    activeCommands.erase(unique(activeCommands.begin(),
				activeCommands.end()),
			 activeCommands.end());
  }
}

void DownloadEngine::updateFdSet() {
  fdmax = 0;
  FD_ZERO(&rfdset);
  FD_ZERO(&wfdset);
#ifdef ENABLE_ASYNC_DNS
  for_each(nameResolverEntries.begin(), nameResolverEntries.end(),
	   SetDescriptor(&fdmax, &rfdset, &wfdset));
#endif // ENABLE_ASYNC_DNS
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
					   Command* command) {
  SocketEntry entry(socket, command, SocketEntry::TYPE_RD);
  return addSocket(entry);
}

bool DownloadEngine::deleteSocketForReadCheck(const SocketHandle& socket,
					      Command* command) {
  SocketEntry entry(socket, command, SocketEntry::TYPE_RD);
  return deleteSocket(entry);
}

bool DownloadEngine::addSocketForWriteCheck(const SocketHandle& socket,
					    Command* command) {
  SocketEntry entry(socket, command, SocketEntry::TYPE_WR);
  return addSocket(entry);
}

bool DownloadEngine::deleteSocketForWriteCheck(const SocketHandle& socket,
					       Command* command) {
  SocketEntry entry(socket, command, SocketEntry::TYPE_WR);
  return deleteSocket(entry);
}

#ifdef ENABLE_ASYNC_DNS
bool DownloadEngine::addNameResolverCheck(const NameResolverHandle& resolver,
					  Command* command) {
  NameResolverEntry entry(resolver, command);
  NameResolverEntries::iterator itr = find(nameResolverEntries.begin(),
					   nameResolverEntries.end(),
					   entry);
  if(itr == nameResolverEntries.end()) {
    nameResolverEntries.push_back(entry);
    updateFdSet();
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::deleteNameResolverCheck(const NameResolverHandle& resolver,
					     Command* command) {
  NameResolverEntry entry(resolver, command);
  NameResolverEntries::iterator itr = find(nameResolverEntries.begin(),
					   nameResolverEntries.end(),
					   entry);
  if(itr == nameResolverEntries.end()) {
    return false;
  } else {
    nameResolverEntries.erase(itr);
    updateFdSet();
    return true;
  }
}
#endif // ENABLE_ASYNC_DNS
