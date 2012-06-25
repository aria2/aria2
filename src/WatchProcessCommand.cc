/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#include "WatchProcessCommand.h"
#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "LogFactory.h"
#include "Logger.h"
#include "fmt.h"

#ifdef __APPLE__
#import <sys/types.h>
#import <sys/sysctl.h>
#define MIBSIZE 4
#endif

namespace aria2 {

WatchProcessCommand::WatchProcessCommand
(cuid_t cuid,
 DownloadEngine* e,
 unsigned int pid,
 bool forceHalt)
  : TimeBasedCommand(cuid, e, 1, true),
    pid_(pid),
    forceHalt_(forceHalt)
{}


void WatchProcessCommand::preProcess()
{
  if(getDownloadEngine()->getRequestGroupMan()->downloadFinished() ||
     getDownloadEngine()->isHaltRequested()) {
    enableExit();
  }
}

void WatchProcessCommand::process()
{
  // Check process pid_ is running. If it is not running, shutdown
  // aria2.
  A2_LOG_DEBUG(fmt("Checking proess %u", pid_));
  bool waiting = true;
#ifdef _WIN32
  HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid_);
  DWORD ret = WaitForSingleObject(process, 0);
  CloseHandle(process);
  if (ret != WAIT_TIMEOUT) {
    waiting = false;
  }
#elif __APPLE__
  int mib[MIBSIZE];
  struct kinfo_proc kp;
  size_t len = sizeof(kp);

  mib[0]=CTL_KERN;
  mib[1]=KERN_PROC;
  mib[2]=KERN_PROC_PID;
  mib[3]=pid_;

  int ret = sysctl(mib, MIBSIZE, &kp, &len, NULL, 0);
  if (ret == -1 || len <= 0) {
    waiting = false;
  }
#else
  if (access(fmt("/proc/%u", pid_).c_str(), F_OK) == -1) {
    waiting = false;
  }
#endif
  if(!waiting) {
    A2_LOG_INFO
      (fmt("CUID#%" PRId64 " - Process %u is not running. Commencing shutdown.",
           getCuid(), pid_));
    if(forceHalt_) {
      getDownloadEngine()->requestForceHalt();
    } else {
      getDownloadEngine()->requestHalt();
    }
    enableExit();
  }
}

} // namespace aria2
