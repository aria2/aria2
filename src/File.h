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
#ifndef _D_FILE_H_
#define _D_FILE_H_

#include <string>

using namespace std;

/**
 * Represents file and directory
 */
class File {
private:
  string name;
  int fillStat(struct stat& fstat);
public:
  File(string name);
  ~File();

  /**
   * Tests whether the file or directory denoted by name exists.
   */
  bool exists();

  /**
   * Tests whether the file denoted by name is a regular file.
   */
  bool isFile();

  /**
   * Tests whether the file denoted by name is a directory.
   */
  bool isDir();

  /**
   * Deletes the file or directory denoted by name.
   * If name denotes a directory, it must be empty in order to delete.
   */
  bool remove();
};

#endif // _D_FILE_H_
