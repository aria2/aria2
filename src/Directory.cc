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
#include "Directory.h"
#include "File.h"
#include "DlAbortEx.h"
#include "message.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

Directory::Directory(const string& name):name(name) {}

Directory::~Directory() {
  for(Files::iterator itr = files.begin(); itr != files.end(); itr++) {
    delete *itr;
  }
}

void Directory::createDir(const string& parentDir, bool recursive) const {
  string path = parentDir+"/"+name;
  File f(path);
  if(f.exists()) {
    if(!f.isDir()) {
      throw new DlAbortEx(EX_NOT_DIRECTORY, path.c_str());
    }
  } else {
    if(mkdir(path.c_str(), S_IRUSR|S_IWUSR|S_IXUSR) == -1) {
      throw new DlAbortEx(EX_MAKE_DIR, path.c_str(), strerror(errno));
    }
  }
  if(recursive) {
    for(Files::const_iterator itr = files.begin(); itr != files.end(); itr++) {
      (*itr)->createDir(path, true);
    }
  }
}

void Directory::addFile(Directory* directory) {
  files.push_back(directory);
}
