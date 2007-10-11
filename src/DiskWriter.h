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

#include "BinaryStream.h"
#include "DlAbortEx.h"

/**
 * Interface for writing to a binary stream of bytes.
 *
 */
class DiskWriter:public BinaryStream {
public:
  virtual ~DiskWriter() {}
  /**
   * Creates a file output stream to write to the file with the specified name.
   * If the file exists, then it is truncated to 0 length.
   * @param filename the file name to be opened.
   */
  virtual void initAndOpenFile(const string& filename, int64_t totalLength = 0)  = 0;
  
  virtual void openFile(const string& filename, int64_t totalLength = 0) = 0;

  /**
   * Closes this output stream.
   */
  // TODO we have to examine the return value of close()
  virtual void closeFile() = 0;

  /**
   * Opens a file output stream to write to the file with the specified name.
   * If the file doesnot exists, an exception may be throwed.
   *
   * @param filename the file name to be opened.
   */
  virtual void openExistingFile(const string& filename, int64_t totalLength = 0) = 0;

  virtual void truncate(int64_t length) = 0;

  // Returns file length
  virtual int64_t size() const = 0;
};

typedef SharedHandle<DiskWriter> DiskWriterHandle;

#endif // _D_DISK_WRITER_H_
