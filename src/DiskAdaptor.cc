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

DiskAdaptor::DiskAdaptor(DiskWriter* diskWriter):diskWriter(diskWriter) {
  logger = LogFactory::getInstance();
}

DiskAdaptor::~DiskAdaptor() {
  delete diskWriter;
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

void DiskAdaptor::writeData(const unsigned char* data, uint32_t len, int64_t offset) {
  diskWriter->writeData(data, len, offset);
}

int DiskAdaptor::readData(unsigned char* data, uint32_t len, int64_t offset) {
  return diskWriter->readData(data, len, offset);
}

string DiskAdaptor::sha1Sum(int64_t offset, uint64_t length) {
  return diskWriter->sha1Sum(offset, length);
}

FileEntryHandle DiskAdaptor::getFileEntryFromPath(const string& fileEntryPath) const {
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    if((*itr)->getPath() == fileEntryPath) {
      return *itr;
    }
  }
  throw new DlAbortEx("No such file entry <%s>", fileEntryPath.c_str());
}

bool DiskAdaptor::addDownloadEntry(const string& fileEntryPath) {
  for(FileEntries::iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    if((*itr)->getPath() == fileEntryPath) {
      (*itr)->setRequested(true);
      return true;
    }
  }
  return false;
}

bool DiskAdaptor::addDownloadEntry(int index) {
  if(fileEntries.size() <= (unsigned int)index) {
    return false;
  }
  fileEntries.at(index)->setRequested(true);
  return true;
}

void DiskAdaptor::addAllDownloadEntry() {
  for(FileEntries::iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    (*itr)->setRequested(true);
  }
}

void DiskAdaptor::removeAllDownloadEntry() {
  for(FileEntries::iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    (*itr)->setRequested(false);
  }
}
