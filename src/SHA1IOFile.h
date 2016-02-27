/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2015 Tatsuhiro Tsujikawa
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
#ifndef D_SHA1_IO_FILE_H
#define D_SHA1_IO_FILE_H

#include "IOFile.h"
#include "MessageDigest.h"

#include <memory>

namespace aria2 {

// Class to calculate SHA1 hash value for data written into this
// object.  No file I/O is done in this class.
class SHA1IOFile : public IOFile {
public:
  SHA1IOFile();

  std::string digest();

protected:
  // Not implemented
  virtual size_t onRead(void* ptr, size_t count) CXX11_OVERRIDE;
  virtual size_t onWrite(const void* ptr, size_t count) CXX11_OVERRIDE;
  // Not implemented
  virtual char* onGets(char* s, int size) CXX11_OVERRIDE;
  virtual int onVprintf(const char* format, va_list va) CXX11_OVERRIDE;
  virtual int onFlush() CXX11_OVERRIDE;
  virtual int onClose() CXX11_OVERRIDE;
  virtual bool onSupportsColor() CXX11_OVERRIDE;
  virtual bool isError() const CXX11_OVERRIDE;
  virtual bool isEOF() const CXX11_OVERRIDE;
  virtual bool isOpen() const CXX11_OVERRIDE;

private:
  std::unique_ptr<MessageDigest> sha1_;
};
} // namespace aria2

#endif // D_SHA1_IO_FILE_H
