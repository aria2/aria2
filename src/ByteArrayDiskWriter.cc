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
#include "Util.h"

ByteArrayDiskWriter::ByteArrayDiskWriter() {
}

ByteArrayDiskWriter::~ByteArrayDiskWriter() {}

void ByteArrayDiskWriter::clear()
{
  buf.str("");
}

void ByteArrayDiskWriter::initAndOpenFile(const string& filename,
					  int64_t totalLength)
{
  clear();
}

void ByteArrayDiskWriter::openFile(const string& filename,
				   int64_t totalLength)
{
}

void ByteArrayDiskWriter::closeFile()
{}

void ByteArrayDiskWriter::openExistingFile(const string& filename,
					   int64_t totalLength)
{
  openFile(filename);
}

void ByteArrayDiskWriter::writeData(const unsigned char* data, int32_t dataLength, int64_t position)
{
  if(size() < position) {
    buf.seekp(size(), ios::beg);
    for(int64_t i = size(); i < position; ++i) {
      buf.put('\0');
    }
  } else {
    buf.seekp(position, ios::beg);
  }
  buf.write(reinterpret_cast<const char*>(data), dataLength);
}

int32_t ByteArrayDiskWriter::readData(unsigned char* data, int32_t len, int64_t position)
{
  buf.seekg(position, ios::beg);
  buf.read(reinterpret_cast<char*>(data), len);
  buf.clear();
  return buf.gcount();
}
