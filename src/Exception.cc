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

Exception::Exception(const char* file, int line, const std::string& msg)
    : file_(file),
      line_(line),
      errNum_(0),
      msg_(msg),
      errorCode_(error_code::UNKNOWN_ERROR)
{
}

Exception::Exception(const char* file, int line, const std::string& msg,
                     error_code::Value errorCode, const Exception& cause)
    : file_(file),
      line_(line),
      errNum_(0),
      msg_(msg),
      errorCode_(errorCode),
      cause_(cause.copy())
{
}

Exception::Exception(const char* file, int line, const std::string& msg,
                     const Exception& cause)
    : file_(file),
      line_(line),
      errNum_(0),
      msg_(msg),
      errorCode_(cause.errorCode_),
      cause_(cause.copy())
{
}

Exception::Exception(const char* file, int line, const std::string& msg,
                     error_code::Value errorCode)
    : file_(file), line_(line), errNum_(0), msg_(msg), errorCode_(errorCode)
{
}

Exception::Exception(const char* file, int line, int errNum,
                     const std::string& msg)
    : file_(file),
      line_(line),
      errNum_(errNum),
      msg_(msg),
      errorCode_(error_code::UNKNOWN_ERROR)
{
}

Exception::Exception(const char* file, int line, int errNum,
                     const std::string& msg, error_code::Value errorCode)
    : file_(file),
      line_(line),
      errNum_(errNum),
      msg_(msg),
      errorCode_(errorCode)
{
}

Exception::~Exception() throw() = default;

std::string Exception::stackTrace() const
{
  std::stringstream s;
  s << "Exception: "
    << "[" << file_ << ":" << line_ << "] ";
  if (errNum_) {
    s << "errNum=" << errNum_ << " ";
  }
  s << "errorCode=" << errorCode_ << " ";
  s << what() << "\n";
  std::shared_ptr<Exception> e = cause_;
  while (e) {
    s << "  -> "
      << "[" << e->file_ << ":" << e->line_ << "] ";
    if (e->getErrNum()) {
      s << "errNum=" << e->getErrNum() << " ";
    }
    s << "errorCode=" << e->getErrorCode() << " " << e->what() << "\n";
    e = e->cause_;
  }
  return s.str();
}

const char* Exception::what() const throw() { return msg_.c_str(); }

} // namespace aria2
