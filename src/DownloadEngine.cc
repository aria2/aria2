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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <algorithm>

using namespace std;

DownloadEngine::DownloadEngine():noWait(false) {}

DownloadEngine::~DownloadEngine() {
  assert(rsockets.empty());
  assert(wsockets.empty());
}

void DownloadEngine::run() {
  struct timeval cp = { 0, 0 };
  int  speed = 0;
  long long int psize = 0;
  while(!commands.empty()) {
    int max = commands.size();
    for(int i = 0; i < max; i++) {
      Command* com = commands.front();
      commands.pop();
      if(com->execute()) {
	delete(com);
      }
    }
    if(!noWait && !commands.empty()) {
      waitData();
    }
    noWait = false;

    long long int dlSize = segmentMan->getDownloadedSize();
    struct timeval now;
    gettimeofday(&now, NULL);
    if(cp.tv_sec == 0 && cp.tv_usec == 0) {
      cp = now;
      psize = dlSize;
    } else {
      long long int elapsed = Util::difftv(now, cp);
      if(elapsed >= 500000) {
	int nspeed = (int)((dlSize-psize)/(elapsed/1000000.0));
	speed = (nspeed+speed)/2;
	cp = now;
	psize = dlSize;
	cout << "\r                                                                            ";
	cout << "\rProgress " << Util::llitos(dlSize, true) << " Bytes/" <<
	  Util::llitos(segmentMan->totalSize, true) << " Bytes " <<
	  (segmentMan->totalSize == 0 ? 0 : (dlSize*100)/segmentMan->totalSize) << "% " <<
	  speed/1000.0 << "KB/s " <<
	  "(" << commands.size() << " connections)" << flush;
      }
    }

  }
  segmentMan->removeIfFinished();
  diskWriter->closeFile();
  if(segmentMan->finished()) {
    cout << "\nThe download was complete. <" << segmentMan->getFilePath() << ">" << endl;
  } else {
    cout << "\nThe download was not complete because of errors. Check the log." << endl;
  }
}

// void DownloadEngine::shortSleep() {
//   int wait = rpm == 0 ? 0 : 4096*1000/rpm;
//   struct timeval tv;
//   int retval;
//   tv.tv_sec = 0;
//   tv.tv_usec = wait*1000;
//   retval = select(0, NULL, NULL, NULL, &tv);
// }

void DownloadEngine::waitData() {
  fd_set rfds;
  fd_set wfds;
  struct timeval tv;
  int retval;

  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  int max = 0;
  for(vector<Socket*>::iterator itr = rsockets.begin(); itr != rsockets.end(); itr++) {
    FD_SET((*itr)->getSockfd(), &rfds);
    if(max < (*itr)->getSockfd()) {
      max = (*itr)->getSockfd();
    }
  }
  for(vector<Socket*>::iterator itr = wsockets.begin(); itr != wsockets.end(); itr++) {

    FD_SET((*itr)->getSockfd(), &wfds);
    if(max < (*itr)->getSockfd()) {
      max = (*itr)->getSockfd();
    }
  }
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  retval = select(max+1, &rfds, /*&wfds*/NULL, NULL, &tv);
}

bool DownloadEngine::addSocket(vector<Socket*>& sockets, Socket* socket) {
  vector<Socket*>::iterator itr = find(sockets.begin(),
				       sockets.end(),
				       socket);
  if(itr == sockets.end()) {
    sockets.push_back(socket);
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::deleteSocket(vector<Socket*>& sockets, Socket* socket) {
  vector<Socket*>::iterator itr = find(sockets.begin(),
				       sockets.end(),
				       socket);
  if(itr != sockets.end()) {
    sockets.erase(itr);
    return true;
  } else {
    return false;
  }
}

bool DownloadEngine::addSocketForReadCheck(Socket* socket) {
  return addSocket(rsockets, socket);
}

bool DownloadEngine::deleteSocketForReadCheck(Socket* socket) {
  return deleteSocket(rsockets , socket);
}

bool DownloadEngine::addSocketForWriteCheck(Socket* socket) {
  return addSocket(wsockets, socket);
}

bool DownloadEngine::deleteSocketForWriteCheck(Socket* socket) {
  return deleteSocket(wsockets, socket);
}

