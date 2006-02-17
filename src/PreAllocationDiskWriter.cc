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
#include "PreAllocationDiskWriter.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "DlAbortEx.h"

PreAllocationDiskWriter::PreAllocationDiskWriter(unsigned long int size):AbstractDiskWriter(),size(size) {}

PreAllocationDiskWriter::~PreAllocationDiskWriter() {}

void PreAllocationDiskWriter::initAndOpenFile(string filename) {
  createFile(filename, O_DIRECT);
  
  int pageSize = getpagesize();
  char* buf;
  int size = pageSize*4;
  int rt = posix_memalign((void**)&buf, pageSize, size);
  if(rt != 0) {
    throw new DlAbortEx(strerror(rt));
  }
  memset(buf, 0, size);
  int x = size/4096;
  int r = size%4096;
  for(int i = 0; i < x; i++) {
    if(write(fd, buf, size) < 0) {
      free(buf);
      throw new DlAbortEx(strerror(errno));
    }
  }
  free(buf);
  closeFile();
  openExistingFile(filename);
  char cbuf[4096];
  memset(cbuf, 0, sizeof(cbuf));
  if(write(fd, cbuf, r) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
}

void PreAllocationDiskWriter::writeData(const char* data, int len, unsigned long int offset) {
  int x = lseek(fd, offset, SEEK_SET);
  // TODO check the return value of write
  writeDataInternal(data, len);
}

