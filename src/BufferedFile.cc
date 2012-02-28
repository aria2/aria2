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
#include <cstdarg>
#include <ostream>

#include "a2io.h"
#include "util.h"

namespace aria2 {

const std::string BufferedFile::READ = "rb";
const std::string BufferedFile::WRITE = "wb";
const std::string BufferedFile::APPEND = "ab";

BufferedFile::BufferedFile(const std::string& filename, const std::string& mode)
{
  fp_ = a2fopen(utf8ToWChar(filename).c_str(), utf8ToWChar(mode).c_str());
  open_ = fp_;
}

BufferedFile::BufferedFile(FILE* fp)
  : fp_(fp), open_(true)
{}

BufferedFile::~BufferedFile()
{
  close();
}

BufferedFile::operator unspecified_bool_type() const
{
  return (!open_ || ferror(fp_)) ? 0 : &BufferedFile::good_state;
}

size_t BufferedFile::read(void* ptr, size_t count)
{
  return fread(ptr, 1, count, fp_);
}

size_t BufferedFile::write(const void* ptr, size_t count)
{
  return fwrite(ptr, 1, count, fp_);
}

size_t BufferedFile::write(const char* str)
{
  return write(str, strlen(str));
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

std::string BufferedFile::getLine()
{
  std::string res;
  if(eof()) {
    return res;
  }
  char buf[4096];
  while(gets(buf, sizeof(buf))) {
    size_t len = strlen(buf);
    bool lineBreak = false;
    if(buf[len-1] == '\n') {
      --len;
      lineBreak = true;
    }
    res.append(buf, len);
    if(lineBreak) {
      break;
    }
  }
  return res;
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

bool BufferedFile::eof()
{
  return open_ && feof(fp_);
}

size_t BufferedFile::transfer(std::ostream& out)
{
  size_t count = 0;
  char buf[4096];
  while(1) {
    size_t r = this->read(buf, sizeof(buf));
    out.write(buf, r);
    count += r;
    if(r < sizeof(buf)) {
      break;
    }
  }
  return count;
}

int BufferedFile::printf(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  int r = vfprintf(fp_, format, ap);
  va_end(ap);
  return r;
}

int BufferedFile::flush()
{
  return fflush(fp_);
}

} // namespace aria2
