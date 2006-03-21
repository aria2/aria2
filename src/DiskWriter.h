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
#ifndef _D_DISK_WRITER_H_
#define _D_DISK_WRITER_H_

#include <string>
#include "common.h"

using namespace std;

/**
 * Interface for writing to a binary stream of bytes.
 *
 */
class DiskWriter {
public:
  virtual ~DiskWriter() {}
  /**
   * Creates a file output stream to write to the file with the specified name.
   * If the file exists, then it is truncated to 0 length.
   * @param filename the file name to be opened.
   */
  virtual void initAndOpenFile(string filename) = 0;

  /**
   * Closes this output stream.
   */
  virtual void closeFile() = 0;

  /**
   * Opens a file output stream to write to the file with the specified name.
   * If the file doesnot exists, an exception may be throwed.
   *
   * @param filename the file name to be opened.
   */
  virtual void openExistingFile(string filename) = 0;

  /*
   * Writes len bytes from data to this binary stream at offset position.
   * In case where offset position is not concerned(just write data
   * sequencially, for example), those subclasses can ignore the offset value.
   *
   * @param data the data
   * @param len the number of bytes to write
   * @param position the offset of this binary stream
   */
  virtual void writeData(const char* data, int len, long long int position = 0) = 0;

  virtual int readData(char* data, int len, long long int position) = 0;

  virtual string sha1Sum(long long int offset, long long int length) = 0;

  virtual void seek(long long int offset) = 0;
};

#endif // _D_DISK_WRITER_H_
