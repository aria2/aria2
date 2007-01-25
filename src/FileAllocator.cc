
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
#include "FileAllocator.h"
#include "DlAbortEx.h"
#include "TimeA2.h"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

void FileAllocator::allocate(int fd, int64_t totalLength)
{
  if(0 != lseek(fd, 0, SEEK_SET)) {
    throw new DlAbortEx("Seek failed: %s", strerror(errno));
  }
  int32_t bufSize = 4096;
  char buf[4096];
  memset(buf, 0, bufSize);
  int64_t x = (totalLength+bufSize-1)/bufSize;
  fileAllocationMonitor->setMinValue(0);
  fileAllocationMonitor->setMaxValue(totalLength);
  fileAllocationMonitor->setCurrentValue(0);
  fileAllocationMonitor->showProgress();
  Time cp;
  for(int64_t i = 0; i < x; ++i) {
    if(write(fd, buf, bufSize) < 0) {
      throw new DlAbortEx("Allocation failed: %s", strerror(errno));
    }
    if(cp.elapsedInMillis(500)) {
      fileAllocationMonitor->setCurrentValue(i*bufSize);
      fileAllocationMonitor->showProgress();
      cp.reset();
    }
  }
  fileAllocationMonitor->setCurrentValue(totalLength);
  fileAllocationMonitor->showProgress();
  ftruncate(fd, totalLength);
}
