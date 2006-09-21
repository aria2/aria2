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
#include "DiskAdaptor.h"
#include "DlAbortEx.h"
#include "LogFactory.h"

DiskAdaptor::DiskAdaptor(DiskWriter* diskWriter):diskWriter(diskWriter), topDir(NULL) {
  logger = LogFactory::getInstance();
}

DiskAdaptor::~DiskAdaptor() {
  delete diskWriter;
  if(topDir != NULL) {
    delete topDir;
  }
}

void DiskAdaptor::openFile() {
  diskWriter->openFile(getFilePath());
}

void DiskAdaptor::closeFile() {
  diskWriter->closeFile();
}

void DiskAdaptor::openExistingFile() {
  diskWriter->openExistingFile(getFilePath());
}

void DiskAdaptor::initAndOpenFile() {
  diskWriter->initAndOpenFile(getFilePath());
}

void DiskAdaptor::writeData(const char* data, int len, long long int position) {
  diskWriter->writeData(data, len, position);
}

int DiskAdaptor::readData(char* data, int len, long long int position) {
  return diskWriter->readData(data, len, position);
}

string DiskAdaptor::sha1Sum(long long int offset, long long int length) {
  return diskWriter->sha1Sum(offset, length);
}

FileEntry DiskAdaptor::getFileEntryFromPath(const string& fileEntryPath) const {
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    if(itr->path == fileEntryPath) {
      return *itr;
    }
  }
  throw new DlAbortEx("no such file entry <%s>", fileEntryPath.c_str());
}

bool DiskAdaptor::addDownloadEntry(const string& fileEntryPath) {
  for(FileEntries::iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    if(itr->path == fileEntryPath) {
      itr->requested = true;
      return true;
    }
  }
  return false;
}

bool DiskAdaptor::addDownloadEntry(int index) {
  if(fileEntries.size() <= (unsigned int)index) {
    return false;
  }
  fileEntries.at(index).requested = true;
  return true;
}

void DiskAdaptor::addAllDownloadEntry() {
  for(FileEntries::iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    itr->requested = true;
  }
}

void DiskAdaptor::removeAllDownloadEntry() {
  for(FileEntries::iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    itr->requested = false;
  }
}
