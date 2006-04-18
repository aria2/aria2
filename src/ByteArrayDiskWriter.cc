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
#include "ByteArrayDiskWriter.h"

ByteArrayDiskWriter::ByteArrayDiskWriter():buf(NULL) {
}

ByteArrayDiskWriter::~ByteArrayDiskWriter() {
  closeFile();
}

void ByteArrayDiskWriter::clear() {
  if(buf != NULL) {
    delete [] buf;
    buf = NULL;
  }
}

void ByteArrayDiskWriter::init() {
  maxBufLength = 256;
  buf = new char[maxBufLength];
  bufLength = 0;
}

void ByteArrayDiskWriter::reset() {
  clear();
  init();
}

void ByteArrayDiskWriter::initAndOpenFile(string filename) {
  openFile(filename);
}

void ByteArrayDiskWriter::openFile(const string& filename) {
  clear();
  init();
}

void ByteArrayDiskWriter::closeFile() {
  clear();
}

void ByteArrayDiskWriter::openExistingFile(string filename) {
  openFile(filename);
}

void ByteArrayDiskWriter::writeData(const char* data, int dataLength, long long int position) {
  if(bufLength+dataLength >= maxBufLength) {
    expandBuffer(bufLength+dataLength);
  }
  memcpy(buf+bufLength, data, dataLength);
  bufLength += dataLength;
}

void ByteArrayDiskWriter::expandBuffer(int newSize) {
  char* newbuf = new char[newSize];
  memcpy(newbuf, buf, bufLength);
  delete [] buf;
  buf = newbuf;
  maxBufLength = newSize;
}
