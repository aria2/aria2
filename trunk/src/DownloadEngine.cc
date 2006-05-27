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

void DownloadEngine::run() {
  initStatistics();
  struct timeval cp;
  cp.tv_sec = 0;
  cp.tv_usec = 0;
  Sockets activeSockets;
  while(!commands.empty()) {
    struct timeval now;
    gettimeofday(&now, NULL);
    if(Util::difftvsec(now, cp) >= 1) {
      cp = now;
      int max = commands.size();
      for(int i = 0; i < max; i++) {
	Command* com = commands.front();
	commands.pop_front();
	if(com->execute()) {
	  delete(com);
	}
      }
    } else {
      for(Sockets::iterator itr = activeSockets.begin();
	  itr != activeSockets.end(); itr++) {
	Socket* socket = *itr;
	SockCmdMap::iterator mapItr = sockCmdMap.find(socket);
	if(mapItr != sockCmdMap.end()) {
	  Command* com = (*mapItr).second;
	  commands.erase(remove(commands.begin(), commands.end(), com));
	  if(com->execute()) {
	    delete(com);
	  }
	}
      }
    }
    afterEachIteration();
    activeSockets.clear();
    if(!noWait && !commands.empty()) {
      waitData(activeSockets);
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

void DownloadEngine::waitData(Sockets& activeSockets) {
  fd_set rfds;
  fd_set wfds;
  int retval = 0;
  while(1) {
    struct timeval tv;
    
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    int max = 0;
    for(Sockets::iterator itr = rsockets.begin(); itr != rsockets.end(); itr++) {
      FD_SET((*itr)->getSockfd(), &rfds);
      if(max < (*itr)->getSockfd()) {
	max = (*itr)->getSockfd();
      }
    }
    for(Sockets::iterator itr = wsockets.begin(); itr != wsockets.end(); itr++) {
      FD_SET((*itr)->getSockfd(), &wfds);
      if(max < (*itr)->getSockfd()) {
	max = (*itr)->getSockfd();
      }
    }
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    retval = select(max+1, &rfds, &wfds, NULL, &tv);
    if(retval != -1 || errno != EINTR) {
      break;
    }
  }
  if(retval > 0) {
    for(Sockets::iterator itr = rsockets.begin(); itr != rsockets.end(); itr++) {
      if(FD_ISSET((*itr)->getSockfd(), &rfds)) {
	activeSockets.push_back(*itr);
      }
    }
    for(Sockets::iterator itr = wsockets.begin(); itr != wsockets.end(); itr++) {
      if(FD_ISSET((*itr)->getSockfd(), &wfds)) {
	activeSockets.push_back(*itr);
      }
    }
    sort(activeSockets.begin(), activeSockets.end());
    activeSockets.erase(unique(activeSockets.begin(), activeSockets.end()), activeSockets.end());
  }
}

bool DownloadEngine::addSocket(Sockets& sockets, Socket* socket, Command* command) {
  Sockets::iterator itr = find(sockets.begin(),
			       sockets.end(),
			       socket);
  if(itr == sockets.end()) {
    sockets.push_back(socket);
    SockCmdMap::value_type vt(socket, command);
    sockCmdMap.insert(vt);
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::deleteSocket(Sockets& sockets, Socket* socket) {
  Sockets::iterator itr = find(sockets.begin(),
			       sockets.end(),
			       socket);
  if(itr != sockets.end()) {
    sockets.erase(itr);
    SockCmdMap::iterator mapItr = sockCmdMap.find(socket);
    if(mapItr != sockCmdMap.end()) {
      sockCmdMap.erase(mapItr);
    }
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::addSocketForReadCheck(Socket* socket, Command* command) {
  return addSocket(rsockets, socket, command);
}

bool DownloadEngine::deleteSocketForReadCheck(Socket* socket) {
  return deleteSocket(rsockets , socket);
}

bool DownloadEngine::addSocketForWriteCheck(Socket* socket, Command* command) {
  return addSocket(wsockets, socket, command);
}

bool DownloadEngine::deleteSocketForWriteCheck(Socket* socket) {
  return deleteSocket(wsockets, socket);
}

