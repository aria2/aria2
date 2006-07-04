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
#ifndef _D_ABSTRACT_DISK_WRITER_H_
#define _D_ABSTRACT_DISK_WRITER_H_

#include "DiskWriter.h"
#ifdef ENABLE_MESSAGE_DIGEST
#include "messageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST

class AbstractDiskWriter:public DiskWriter {
protected:
  string filename;
  int fd;
#ifdef ENABLE_MESSAGE_DIGEST
  MessageDigestContext ctx;
#endif // ENABLE_MESSAGE_DIGEST

  void createFile(const string& filename, int addFlags = 0);

  void writeDataInternal(const char* data, int len);
  int readDataInternal(char* data, int len);

  void seek(long long int offset);

public:
  AbstractDiskWriter();
  virtual ~AbstractDiskWriter();

  void openFile(const string& filename);

  void closeFile();

  void openExistingFile(const string& filename);

  string sha1Sum(long long int offset, long long int length);

  void writeData(const char* data, int len, long long int offset);

  int readData(char* data, int len, long long int offset);
};

#endif // _D_ABSTRACT_DISK_WRITER_H_
