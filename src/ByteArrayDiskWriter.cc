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
#include "ByteArrayDiskWriter.h"
#include "util.h"
#include "A2STR.h"

namespace aria2 {

ByteArrayDiskWriter::ByteArrayDiskWriter() {}

ByteArrayDiskWriter::~ByteArrayDiskWriter() {}

void ByteArrayDiskWriter::clear()
{
  buf.str(A2STR::NIL);
}

void ByteArrayDiskWriter::initAndOpenFile(uint64_t totalLength)
{
  clear();
}

void ByteArrayDiskWriter::openFile(uint64_t totalLength) {}

void ByteArrayDiskWriter::closeFile() {}

void ByteArrayDiskWriter::openExistingFile(uint64_t totalLength)
{
  openFile();
}

void ByteArrayDiskWriter::writeData(const unsigned char* data, size_t dataLength, off_t position)
{
  uint64_t length = size();
  if(length < (uint64_t)position) {
    buf.seekp(length, std::ios::beg);
    for(uint64_t i = length; i < (uint64_t)position; ++i) {
      buf.put('\0');
    }
  } else {
    buf.seekp(position, std::ios::beg);
  }
  buf.write(reinterpret_cast<const char*>(data), dataLength);
}

ssize_t ByteArrayDiskWriter::readData(unsigned char* data, size_t len, off_t position)
{
  buf.seekg(position, std::ios::beg);
  buf.read(reinterpret_cast<char*>(data), len);
  buf.clear();
  return buf.gcount();
}

uint64_t ByteArrayDiskWriter::size()
{
  buf.seekg(0, std::ios::end);
  buf.clear();
  return buf.tellg();
}

} // namespace aria2
