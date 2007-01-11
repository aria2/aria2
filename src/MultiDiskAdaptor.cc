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
#include "MultiDiskAdaptor.h"
#include "DefaultDiskWriter.h"
#include "DlAbortEx.h"
#include "message.h"
#include "Util.h"
#include <errno.h>

void MultiDiskAdaptor::resetDiskWriterEntries() {
  diskWriterEntries.clear();
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    DiskWriterEntryHandle entry = new DiskWriterEntry(*itr);
    if((*itr)->isRequested()) {
      entry->setDiskWriter(DefaultDiskWriter::createNewDiskWriter(option));
    } else {
      entry->setDiskWriter(new DefaultDiskWriter());
    }
    diskWriterEntries.push_back(entry);
  }
}

string MultiDiskAdaptor::getTopDirPath() const {
  return storeDir+"/"+topDir;
}

void MultiDiskAdaptor::mkdir() const {
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    (*itr)->setupDir(getTopDirPath());
  }
}

void MultiDiskAdaptor::openFile() {
  mkdir();
  resetDiskWriterEntries();
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    (*itr)->openFile(getTopDirPath());
  }
}

void MultiDiskAdaptor::initAndOpenFile() {
  mkdir();
  resetDiskWriterEntries();
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    (*itr)->initAndOpenFile(getTopDirPath());
  }
}

void MultiDiskAdaptor::openExistingFile() {
  resetDiskWriterEntries();
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    (*itr)->openExistingFile(getTopDirPath());
  }
}

void MultiDiskAdaptor::closeFile() {
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    (*itr)->closeFile();
  }
}

void MultiDiskAdaptor::onDownloadComplete() {
  closeFile();
  openFile();
}

void MultiDiskAdaptor::writeData(const unsigned char* data, uint32_t len,
				 int64_t offset)
{
  int64_t fileOffset = offset;
  bool writing = false;
  uint32_t rem = len;
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end() && rem != 0; itr++) {
    if(isInRange(*itr, offset) || writing) {
      uint32_t writeLength = calculateLength(*itr, fileOffset, rem);
      (*itr)->getDiskWriter()->writeData(data+(len-rem), writeLength, fileOffset);
      rem -= writeLength;
      writing = true;
      fileOffset = 0;
    } else {
      fileOffset -= (*itr)->getFileEntry()->getLength();
    }
  }
  if(!writing) {
    throw new DlAbortEx(EX_FILE_OFFSET_OUT_OF_RANGE, offset);
  }
}

bool MultiDiskAdaptor::isInRange(const DiskWriterEntryHandle entry,
				 int64_t offset) const
{
  return entry->getFileEntry()->getOffset() <= offset &&
    offset < entry->getFileEntry()->getOffset()+entry->getFileEntry()->getLength();
}

uint32_t MultiDiskAdaptor::calculateLength(const DiskWriterEntryHandle entry,
					   int64_t fileOffset,
					   uint32_t rem) const
{
  uint32_t length;
  if(entry->getFileEntry()->getLength() < fileOffset+rem) {
    length = entry->getFileEntry()->getLength()-fileOffset;
  } else {
    length = rem;
  }
  return length;
}

int MultiDiskAdaptor::readData(unsigned char* data, uint32_t len, int64_t offset)
{
  int64_t fileOffset = offset;
  bool reading = false;
  uint32_t rem = len;
  uint32_t totalReadLength = 0;
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end() && rem != 0; itr++) {
    if(isInRange(*itr, offset) || reading) {
      uint32_t readLength = calculateLength((*itr), fileOffset, rem);
      totalReadLength += (*itr)->getDiskWriter()->readData(data+(len-rem), readLength, fileOffset);
      rem -= readLength;
      reading = true;
      fileOffset = 0;
    } else {
      fileOffset -= (*itr)->getFileEntry()->getLength();
    }
  }
  if(!reading) {
    throw new DlAbortEx(EX_FILE_OFFSET_OUT_OF_RANGE, offset);
  }
  return totalReadLength;
}

void MultiDiskAdaptor::hashUpdate(const DiskWriterEntryHandle entry,
				  int64_t offset, uint64_t length)
{
  uint32_t BUFSIZE = 16*1024;
  unsigned char buf[BUFSIZE];
  for(uint64_t i = 0; i < length/BUFSIZE; i++) {
    if((int32_t)BUFSIZE != entry->getDiskWriter()->readData(buf, BUFSIZE, offset)) {
      throw new DlAbortEx(EX_FILE_SHA1SUM, "", strerror(errno));
    }
    ctx.digestUpdate(buf, BUFSIZE);
    offset += BUFSIZE;
  }
  uint32_t r = length%BUFSIZE;
  if(r > 0) {
    if((int32_t)r != entry->getDiskWriter()->readData(buf, r, offset)) {
      throw new DlAbortEx(EX_FILE_SHA1SUM, "", strerror(errno));
    }
    ctx.digestUpdate(buf, r);
  }
}

string MultiDiskAdaptor::sha1Sum(int64_t offset, uint64_t length) {
  int64_t fileOffset = offset;
  bool reading = false;
  uint32_t rem = length;
  ctx.digestReset();

  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end() && rem != 0; itr++) {
    if(isInRange(*itr, offset) || reading) {
      uint32_t readLength = calculateLength((*itr), fileOffset, rem);
      hashUpdate(*itr, fileOffset, readLength);
      rem -= readLength;
      reading = true;
      fileOffset = 0;
    } else {
      fileOffset -= (*itr)->getFileEntry()->getLength();
    }
  }
  if(!reading) {
    throw new DlAbortEx(EX_FILE_OFFSET_OUT_OF_RANGE, offset);
  }
  unsigned char hashValue[20];
  ctx.digestFinal(hashValue);
  return Util::toHex(hashValue, 20);
}

bool MultiDiskAdaptor::fileExists()
{
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    if((*itr)->fileExists(getTopDirPath())) {
      return true;
    }				   
  }
  return false;
}
