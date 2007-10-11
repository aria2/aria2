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
#include "DownloadEngine.h"
#include "Socket.h"
#include "NameResolver.h"
#include "StatCalc.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "FileAllocationMan.h"
#ifdef ENABLE_MESSAGE_DIGEST
#include "CheckIntegrityMan.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "Util.h"
#include "LogFactory.h"
#include "TimeA2.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <algorithm>

volatile sig_atomic_t globalHaltRequested;

SocketEntry::SocketEntry(const SocketHandle& socket,
			 Command* command,
			 TYPE type):
  socket(socket), command(command), type(type) {}

bool SocketEntry::operator==(const SocketEntry& entry)
{
  return socket == entry.socket &&
    command == entry.command &&
    type == entry.type;
}

#ifdef ENABLE_ASYNC_DNS
NameResolverEntry::NameResolverEntry(const NameResolverHandle& nameResolver,
					     Command* command):
  nameResolver(nameResolver), command(command) {}

bool NameResolverEntry::operator==(const NameResolverEntry& entry)
{
  return nameResolver == entry.nameResolver &&
    command == entry.command;
}
#endif // ENABLE_ASYNC_DNS

DownloadEngine::DownloadEngine():logger(LogFactory::getInstance()),
				 _statCalc(0),
				 _haltRequested(false),
				 noWait(false),
				 _requestGroupMan(0),
				 _fileAllocationMan(0)
#ifdef ENABLE_MESSAGE_DIGEST
				,
				 _checkIntegrityMan(0)
#endif // ENABLE_MESSAGE_DIGEST
{}

DownloadEngine::~DownloadEngine() {
  cleanQueue();
}

void DownloadEngine::cleanQueue() {
  for_each(commands.begin(), commands.end(), Deleter());
  commands.clear();
}

void DownloadEngine::executeCommand(Command::STATUS statusFilter)
{
  int32_t max = commands.size();
  for(int32_t i = 0; i < max; i++) {
    Command* com = commands.front();
    commands.pop_front();
    if(com->statusMatch(statusFilter)) {
      if(com->execute()) {
	delete com;
      } else {
	com->transitStatus();
      }
    } else {
      commands.push_back(com);
    }
  }
}

void DownloadEngine::run() {
  Time cp;
  cp.setTimeInSec(0);
  Commands activeCommands;
  while(!commands.empty()) {
    if(cp.elapsed(1)) {
      cp.reset();
      executeCommand(Command::STATUS_ALL);
    } else {
      executeCommand(Command::STATUS_ACTIVE);
    }
    afterEachIteration();
    if(!commands.empty()) {
      waitData();
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

void DownloadEngine::waitData() {
  fd_set rfds;
  fd_set wfds;
  int32_t retval = 0;
  struct timeval tv;
  
  memcpy(&rfds, &rfdset, sizeof(fd_set));
  memcpy(&wfds, &wfdset, sizeof(fd_set));
  
  tv.tv_sec = noWait ? 0 : 1;
  tv.tv_usec = 0;
  retval = select(fdmax+1, &rfds, &wfds, NULL, &tv);
  if(retval > 0) {
    for(SocketEntries::iterator itr = socketEntries.begin();
	itr != socketEntries.end(); ++itr) {
      SocketEntry& entry = *itr;
      if(FD_ISSET(entry.socket->getSockfd(), &rfds) ||
	 FD_ISSET(entry.socket->getSockfd(), &wfds)) {
	entry.command->setStatusActive();
      }
    }
#ifdef ENABLE_ASYNC_DNS
    for(NameResolverEntries::iterator itr = nameResolverEntries.begin();
	itr != nameResolverEntries.end(); ++itr) {
      NameResolverEntry& entry = *itr;
      entry.nameResolver->process(&rfds, &wfds);
      switch(entry.nameResolver->getStatus()) {
      case NameResolver::STATUS_SUCCESS:
      case NameResolver::STATUS_ERROR:
	entry.command->setStatusActive();
	break;
      default:
	break;
      }
    }
#endif // ENABLE_ASYNC_DNS
  }
}

void DownloadEngine::updateFdSet() {
  fdmax = 0;
  FD_ZERO(&rfdset);
  FD_ZERO(&wfdset);
#ifdef ENABLE_ASYNC_DNS
  for(NameResolverEntries::iterator itr = nameResolverEntries.begin();
      itr != nameResolverEntries.end(); ++itr) {
    NameResolverEntry& entry = *itr;
    int32_t fd = entry.nameResolver->getFds(&rfdset, &wfdset);
    if(fdmax < fd) {
      fdmax = fd;
    }
  }
#endif // ENABLE_ASYNC_DNS
  for(SocketEntries::iterator itr = socketEntries.begin();
      itr != socketEntries.end(); ++itr) {
    SocketEntry& entry = *itr;
    int32_t fd = entry.socket->getSockfd();
    switch(entry.type) {
    case SocketEntry::TYPE_RD:
      FD_SET(fd, &rfdset);
      break;
    case SocketEntry::TYPE_WR:
      FD_SET(fd, &wfdset);
      break;
    }
    if(fdmax < fd) {
      fdmax = fd;
    }
  }
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

void DownloadEngine::calculateStatistics()
{
  if(!_statCalc.isNull()) {
    _statCalc->calculateStat(_requestGroupMan, _fileAllocationMan, _checkIntegrityMan);
  }
}

void DownloadEngine::onEndOfRun()
{
  _requestGroupMan->closeFile();
  _requestGroupMan->save();
}

void DownloadEngine::afterEachIteration()
{
  if(globalHaltRequested) {
    globalHaltRequested = false;
    _haltRequested = true;
    _requestGroupMan->halt();
  }
}

void DownloadEngine::fillCommand()
{
  addCommand(_requestGroupMan->getInitialCommands(this));
}

void DownloadEngine::setStatCalc(const StatCalcHandle& statCalc)
{
  _statCalc = statCalc;
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

void DownloadEngine::addCommand(const Commands& commands)
{
  this->commands.insert(this->commands.end(), commands.begin(), commands.end());
}

#endif // ENABLE_ASYNC_DNS
