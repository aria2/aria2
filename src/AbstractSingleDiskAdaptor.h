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
#ifndef D_ABSTRACT_SINGLE_DISK_ADAPTOR_H
#define D_ABSTRACT_SINGLE_DISK_ADAPTOR_H

#include "DiskAdaptor.h"

namespace aria2 {

class DiskWriter;
class FileAllocationIterator;

class AbstractSingleDiskAdaptor : public DiskAdaptor {
private:
  SharedHandle<DiskWriter> diskWriter_;
  int64_t totalLength_;
  bool readOnly_;
public:
  AbstractSingleDiskAdaptor();

  virtual ~AbstractSingleDiskAdaptor();

  virtual void initAndOpenFile();

  virtual void openFile();

  virtual void closeFile();

  virtual void openExistingFile();

  virtual void writeData(const unsigned char* data, size_t len,
                         int64_t offset);

  virtual ssize_t readData(unsigned char* data, size_t len, int64_t offset);

  virtual bool fileExists();

  virtual int64_t size();

  virtual void truncate(int64_t length);
  
  virtual SharedHandle<FileAllocationIterator> fileAllocationIterator();

  // Make sure that DiskWriter is set before calling this function.
  virtual void enableReadOnly();

  // Make sure that DiskWriter is set before calling this function.
  virtual void disableReadOnly();
    
  virtual bool isReadOnlyEnabled() const { return readOnly_; }

  virtual void enableMmap();
  
  virtual void cutTrailingGarbage();

  virtual const std::string& getFilePath() = 0;

  void setDiskWriter(const SharedHandle<DiskWriter>& diskWriter);

  const SharedHandle<DiskWriter>& getDiskWriter() const
  {
    return diskWriter_;
  }

  void setTotalLength(int64_t totalLength);

  int64_t getTotalLength() const
  {
    return totalLength_;
  }
};

} // namespace aria2

#endif // D_ABSTRACT_SINGLE_DISK_ADAPTOR_H
