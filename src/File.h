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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#ifndef D_FILE_H
#define D_FILE_H

#include "common.h"

#include <string>

#include "a2io.h"
#include "TimeA2.h"

namespace aria2 {

/**
 * Represents file and directory
 */
class File {
private:
  std::string name_;
  
  /**
   * Returns the return value of stat(...)
   */
  int fillStat(a2_struct_stat& fstat);
public:
  File(const std::string& name);

  File(const File& c);

  ~File();

  File& operator=(const File& c);

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

  /**
   * Creates the directory denoted by name.
   * This method creates complete directory structure.
   * Returns true if the directory is created successfully, otherwise returns
   * false.
   * If the directory already exists, then returns false.
   */
  bool mkdirs();

  int64_t size();

  mode_t mode();

  std::string getBasename() const;

  std::string getDirname() const;

  const std::string& getPath() const
  {
    return name_;
  }

  static bool isDir(const std::string& filename);

  bool renameTo(const std::string& dest);

  bool utime(const Time& actime, const Time& modtime) const;

  Time getModifiedTime();

  // Returns the current working directory.  If the current working
  // directory cannot be retrieved or its length is larger than 2048,
  // returns ".".
  static std::string getCurrentDir();
  // Returns possible path separators for the underlining platform.
  static const std::string& getPathSeparators();
};

} // namespace aria2

#endif // D_FILE_H
