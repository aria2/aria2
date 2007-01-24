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
#ifndef _D_DISK_ADAPTOR_WRITER_H_
#define _D_DISK_ADAPTOR_WRITER_H_

#include "DiskWriter.h"
#include "DiskAdaptor.h"

class DiskAdaptorWriter : public DiskWriter {
private:
  DiskAdaptorHandle diskAdaptor;
public:
  DiskAdaptorWriter(const DiskAdaptorHandle& diskAdaptor):
    diskAdaptor(diskAdaptor) {}

  virtual ~DiskAdaptorWriter() {}

  virtual void initAndOpenFile(const string& filename, uint64_t totalLength = 0)
  {
    diskAdaptor->initAndOpenFile();
  }
  
  virtual void openFile(const string& filename, uint64_t totalLength = 0)
  {
    diskAdaptor->openFile();
  }

  virtual void closeFile()
  {
    diskAdaptor->closeFile();
  }

  virtual void openExistingFile(const string& filename)
  {
    diskAdaptor->openExistingFile();
  }

  virtual void writeData(const char* data, uint32_t len, int64_t position = 0)
  {
    diskAdaptor->writeData((const unsigned char*)data, len, position);
  }

  virtual int readData(char* data, uint32_t len, int64_t position)
  {
    return diskAdaptor->readData((unsigned char*)data, len, position);
  }

  virtual string messageDigest(int64_t offset, uint64_t length,
			       const MessageDigestContext::DigestAlgo& algo)
  {
    return diskAdaptor->messageDigest(offset, length, algo);
  }
};

#endif // _D_DISK_ADAPTOR_WRITER_H_
