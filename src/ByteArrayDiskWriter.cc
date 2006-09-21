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
#include "ByteArrayDiskWriter.h"
#include "Util.h"

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

void ByteArrayDiskWriter::initAndOpenFile(const string& filename) {
  openFile(filename);
}

void ByteArrayDiskWriter::openFile(const string& filename) {
  clear();
  init();
}

void ByteArrayDiskWriter::closeFile() {
  clear();
}

void ByteArrayDiskWriter::openExistingFile(const string& filename) {
  openFile(filename);
}

void ByteArrayDiskWriter::writeData(const char* data, int dataLength, long long int position) {
  if(bufLength+dataLength >= maxBufLength) {
    maxBufLength = Util::expandBuffer(&buf, bufLength, bufLength+dataLength);
  }
  memcpy(buf+bufLength, data, dataLength);
  bufLength += dataLength;
}

int ByteArrayDiskWriter::readData(char* data, int len, long long int position) {
  if(position >= bufLength) {
    return 0;
  }
  int readLength;
  if(position+len <= bufLength) {
    readLength = len;
  } else {
    readLength = bufLength-position;
  }
  memcpy(data, buf+position, readLength);
  return readLength;
}

