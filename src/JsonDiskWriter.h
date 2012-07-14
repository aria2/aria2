/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#ifndef D_JSON_DISK_WRITER_H
#define D_JSON_DISK_WRITER_H

#include "DiskWriter.h"
#include "ValueBaseStructParserStateMachine.h"
#include "JsonParser.h"

namespace aria2 {

namespace json {

// DiskWriter backed with ValueBaseJsonParser. The written bytes are
// consumed by ValueBaseJsonParser. It is only capable of sequential
// write so offset argument in write() will be ignored. It also does
// not offer read().
class JsonDiskWriter : public DiskWriter {
public:
  JsonDiskWriter();

  virtual ~JsonDiskWriter();

  virtual void initAndOpenFile(off_t totalLength = 0);
  
  virtual void openFile(off_t totalLength = 0)
  {
    initAndOpenFile(totalLength);
  }

  virtual void closeFile() {}

  virtual void openExistingFile(off_t totalLength = 0)
  {
    initAndOpenFile(totalLength);
  }

  virtual off_t size()
  {
    return 0;
  }

  virtual void writeData(const unsigned char* data, size_t len, off_t offset);

  virtual ssize_t readData(unsigned char* data, size_t len, off_t offset)
  {
    return 0;
  }

  int finalize();
  SharedHandle<ValueBase> getResult() const;
  void reset();
private:
  ValueBaseStructParserStateMachine psm_;
  JsonParser parser_;
};

} // namespace json

} // namespace aria2

#endif // D_JSON_DISK_WRITER_H
