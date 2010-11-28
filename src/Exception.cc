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
#include "Exception.h"

#include <sstream>

namespace aria2 {

Exception::Exception
(const char* file,
 int line,
 const std::string& msg)
  : file_(file),
    line_(line),
    errNum_(0),
    msg_(msg)
{}

Exception::Exception
(const char* file,
 int line,
 const std::string& msg,
 const Exception& cause)
  : file_(file),
    line_(line),
    errNum_(0),
    msg_(msg),
    cause_(cause.copy())
{}

Exception::Exception
(const char* file,
 int line,
 const Exception& e)
  : file_(file),
    line_(line),
    errNum_(0),
    msg_(e.msg_),
    cause_(e.cause_)
{}

Exception::Exception
(const char* file,
 int line,
 int errNum,
 const std::string& msg)
  : file_(file),
    line_(line),
    errNum_(errNum),
    msg_(msg)
{}

Exception::~Exception() throw() {}

std::string Exception::stackTrace() const
{
  std::stringstream s;
  s << "Exception: " << "[" << file_ << ":" << line_ << "] ";
  if(errNum_) {
    s << "errno=" << errNum_ << " ";
  }
  s  << what() << "\n";
  SharedHandle<Exception> e = cause_;
  while(e) {
    s << "  -> " << "[" << e->file_ << ":" << e->line_ << "] "
      << e->what() << "\n";
    e = e->cause_;
  }
  return s.str();
}

const char* Exception::what() const throw()
{
  return msg_.c_str();
}

} // namespace aria2
