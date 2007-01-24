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
#ifndef _D_DISK_WRITER_H_
#define _D_DISK_WRITER_H_

#include <string>
#include "common.h"
#ifdef ENABLE_MESSAGE_DIGEST
#include "messageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST

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
  virtual void initAndOpenFile(const string& filename, uint64_t totalLength = 0) = 0;
  
  virtual void openFile(const string& filename, uint64_t totalLength = 0) = 0;

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
  virtual void openExistingFile(const string& filename) = 0;

  /*
   * Writes len bytes from data to this binary stream at offset position.
   * In case where offset position is not concerned(just write data
   * sequencially, for example), those subclasses can ignore the offset value.
   *
   * @param data the data
   * @param len the number of bytes to write
   * @param position the offset of this binary stream
   */
  virtual void writeData(const char* data, uint32_t len, int64_t position = 0) = 0;
  virtual void writeData(const unsigned char* data, uint32_t len, int64_t position = 0)
  {
    writeData((const char*)data, len, position);
  }

  virtual int readData(char* data, uint32_t len, int64_t position) = 0;
  virtual int readData(unsigned char* data, uint32_t len, int64_t position) {
    return readData((char*)data, len, position);
  }

  virtual string messageDigest(int64_t offset, uint64_t length,
			       const MessageDigestContext::DigestAlgo& algo) = 0;
};

typedef SharedHandle<DiskWriter> DiskWriterHandle;

#endif // _D_DISK_WRITER_H_
