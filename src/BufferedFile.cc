/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#include "BufferedFile.h"

#include <cstring>

#include "a2io.h"
#include "util.h"

namespace aria2 {

const std::string BufferedFile::READ = "rb";
const std::string BufferedFile::WRITE = "wb";

BufferedFile::BufferedFile(const std::string& filename, const std::string& mode)
{
  fp_ = a2fopen(utf8ToWChar(filename).c_str(), utf8ToWChar(mode).c_str());
  open_ = fp_;
}

BufferedFile::~BufferedFile()
{
  close();
}

BufferedFile::operator unspecified_bool_type() const
{
  return (!open_ || ferror(fp_) || feof(fp_)) ? 0 : &BufferedFile::good_state;
}

size_t BufferedFile::read(void* ptr, size_t count)
{
  return fread(ptr, 1, count, fp_);
}

size_t BufferedFile::write(const void* ptr, size_t count)
{
  return fwrite(ptr, 1, count, fp_);
}

char* BufferedFile::gets(char* s, int size)
{
  return fgets(s, size, fp_);
}

char* BufferedFile::getsn(char* s, int size)
{
  char* ptr = fgets(s, size, fp_);
  if(ptr) {
    int len = strlen(ptr);
    if(ptr[len-1] == '\n') {
      ptr[len-1] = '\0';
    }
  }
  return ptr;
}

int BufferedFile::close()
{
  if(open_) {
    open_ = false;
    return fclose(fp_);
  } else {
    return 0;
  }
}

} // namespace aria2
