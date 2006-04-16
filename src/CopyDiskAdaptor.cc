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
#include "CopyDiskAdaptor.h"
#include "Util.h"

CopyDiskAdaptor::CopyDiskAdaptor(DiskWriter* diskWriter):DiskAdaptor(diskWriter) {}

CopyDiskAdaptor::~CopyDiskAdaptor() {}

void CopyDiskAdaptor::onDownloadComplete() {
  closeFile();
  fixFilename();
  openFile();
}

void CopyDiskAdaptor::fixFilename() {
  if(topDir != NULL) {
    topDir->createDir(storeDir, true);
  }
  long long int offset = 0;
  for(FileEntries::iterator itr = fileEntries.begin();
      itr != fileEntries.end(); itr++) {
    if(!itr->extracted && itr->requested) {
      string dest = storeDir+"/"+itr->path;
      //logger->info("writing file %s", dest.c_str());
      Util::rangedFileCopy(dest, getFilePath(), offset, itr->length);
      itr->extracted = true;
    }
    offset += itr->length;
  }
}

string CopyDiskAdaptor::getFilePath() const {
  return storeDir+"/"+tempFilename;
}
