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
#ifndef D_XML_RPC_DISK_WRITER_H
#define D_XML_RPC_DISK_WRITER_H

#include "DiskWriter.h"
#include "RpcRequest.h"
#include "XmlRpcRequestParserStateMachine.h"
#include "XmlParser.h"

namespace aria2 {

namespace rpc {

// DiskWriter backed with XML-RPC XmlParser. The written bytes are
// consumed by XML-RPC XmlParser. It is only capable of sequential
// write so offset argument in write() will be ignored. It also does
// not offer read().
class XmlRpcDiskWriter : public DiskWriter {
public:
  XmlRpcDiskWriter();

  virtual ~XmlRpcDiskWriter();

  virtual void initAndOpenFile(int64_t totalLength = 0);
  
  virtual void openFile(int64_t totalLength = 0)
  {
    initAndOpenFile(totalLength);
  }

  virtual void closeFile() {}

  virtual void openExistingFile(int64_t totalLength = 0)
  {
    initAndOpenFile(totalLength);
  }

  virtual int64_t size()
  {
    return 0;
  }

  virtual void writeData(const unsigned char* data, size_t len, int64_t offset);

  virtual ssize_t readData(unsigned char* data, size_t len, int64_t offset)
  {
    return 0;
  }

  int finalize();
  RpcRequest getResult() const;
  int reset();
private:
  XmlRpcRequestParserStateMachine psm_;
  xml::XmlParser parser_;
};

} // namespace rpc

} // namespace aria2

#endif // D_XML_RPC_DISK_WRITER_H
