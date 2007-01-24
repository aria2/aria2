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
#include "MultiDiskWriter.h"
#include "DlAbortEx.h"
#include "Util.h"
#include "message.h"
#include <errno.h>

MultiDiskWriter::MultiDiskWriter(int pieceLength):
  pieceLength(pieceLength),
  ctx(DIGEST_ALGO_SHA1),
  fileAllocator(0) {
  ctx.digestInit();
}

MultiDiskWriter::~MultiDiskWriter() {
  clearEntries();
}

void MultiDiskWriter::clearEntries() {
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    delete *itr;
  }
  diskWriterEntries.clear();
}

void MultiDiskWriter::setFileEntries(const FileEntries& fileEntries) {
  clearEntries();
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    diskWriterEntries.push_back(new DiskWriterEntry(*itr));
  }
}

void MultiDiskWriter::configureFileAllocator(DiskWriterEntry* entry) {
  if(entry->fileEntry->isRequested()) {
    entry->diskWriter->setFileAllocator(fileAllocator);
  } else {
    entry->diskWriter->setFileAllocator(0);
  }
}

void MultiDiskWriter::openFile(const string& filename) {
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    configureFileAllocator(*itr);
    (*itr)->diskWriter->openFile(filename+"/"+(*itr)->fileEntry->getPath());
  }
}

// filename is a directory which is specified by the user in the option.
void MultiDiskWriter::initAndOpenFile(const string& filename) {
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    configureFileAllocator(*itr);
    (*itr)->diskWriter->initAndOpenFile(filename+"/"+(*itr)->fileEntry->getPath());
  }
}

void MultiDiskWriter::closeFile() {
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    (*itr)->diskWriter->closeFile();
  }
}

void MultiDiskWriter::openExistingFile(const string& filename) {
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end(); itr++) {
    (*itr)->diskWriter->setFileAllocator(0);
    (*itr)->diskWriter->openExistingFile(filename+"/"+(*itr)->fileEntry->getPath());
  }
}

void MultiDiskWriter::writeData(const char* data, int len, long long int offset) {
  long long int fileOffset = offset;
  bool writing = false;
  int rem = len;
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end() && rem != 0; itr++) {
    if(isInRange(*itr, offset) || writing) {
      int writeLength = calculateLength(*itr, fileOffset, rem);
      (*itr)->diskWriter->writeData(data+(len-rem), writeLength, fileOffset);
      rem -= writeLength;
      writing = true;
      fileOffset = 0;
    } else {
      fileOffset -= (*itr)->fileEntry->getLength();
    }
  }
  if(!writing) {
    throw new DlAbortEx(EX_FILE_OFFSET_OUT_OF_RANGE, offset);
  }
}

bool MultiDiskWriter::isInRange(const DiskWriterEntry* entry, long long int offset) const {
  return entry->fileEntry->getOffset() <= offset &&
    offset < entry->fileEntry->getOffset()+entry->fileEntry->getLength();
}

int MultiDiskWriter::calculateLength(const DiskWriterEntry* entry, long long int fileOffset, int rem) const {
  int length;
  if(entry->fileEntry->getLength() < fileOffset+rem) {
    length = entry->fileEntry->getLength()-fileOffset;
  } else {
    length = rem;
  }
  return length;
}

int MultiDiskWriter::readData(char* data, int len, long long int offset) {
  long long int fileOffset = offset;
  bool reading = false;
  int rem = len;
  int totalReadLength = 0;
  for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
      itr != diskWriterEntries.end() && rem != 0; itr++) {
    if(isInRange(*itr, offset) || reading) {
      int readLength = calculateLength((*itr), fileOffset, rem);
      totalReadLength += (*itr)->diskWriter->readData(data+(len-rem), readLength, fileOffset);
      rem -= readLength;
      reading = true;
      fileOffset = 0;
    } else {
      fileOffset -= (*itr)->fileEntry->getLength();
    }
  }
  if(!reading) {
    throw new DlAbortEx(EX_FILE_OFFSET_OUT_OF_RANGE, offset);
  }
  return totalReadLength;
}

void MultiDiskWriter::hashUpdate(DiskWriterEntry* entry, long long int offset, long long int length) {
  int BUFSIZE = 16*1024;
  char buf[BUFSIZE];
  for(int i = 0; i < length/BUFSIZE; i++) {
    if(BUFSIZE != entry->diskWriter->readData(buf, BUFSIZE, offset)) {
      throw string("error");
    }
    ctx.digestUpdate(buf, BUFSIZE);
    offset += BUFSIZE;
  }
  int r = length%BUFSIZE;
  if(r > 0) {
    if(r != entry->diskWriter->readData(buf, r, offset)) {
      throw string("error");
    }
    ctx.digestUpdate(buf, r);
  }
}

string MultiDiskWriter::sha1Sum(long long int offset, long long int length) {
  long long int fileOffset = offset;
  bool reading = false;
  int rem = length;
  ctx.digestReset();
  try {
    for(DiskWriterEntries::iterator itr = diskWriterEntries.begin();
	itr != diskWriterEntries.end() && rem != 0; itr++) {
      if(isInRange(*itr, offset) || reading) {
	int readLength = calculateLength((*itr), fileOffset, rem);
	hashUpdate(*itr, fileOffset, readLength);
	rem -= readLength;
	reading = true;
	fileOffset = 0;
      } else {
	fileOffset -= (*itr)->fileEntry->getLength();
      }
    }
    if(!reading) {
      throw new DlAbortEx(EX_FILE_OFFSET_OUT_OF_RANGE, offset);
    }
    unsigned char hashValue[20];
    ctx.digestFinal(hashValue);
    return Util::toHex(hashValue, 20);
  } catch(string ex) {
    throw new DlAbortEx(EX_FILE_SHA1SUM, "", strerror(errno));
  }
}

