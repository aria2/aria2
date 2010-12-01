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
#ifndef D_DOWNLOAD_FAILURE_EXCEPTION_H
#define D_DOWNLOAD_FAILURE_EXCEPTION_H
#include "RecoverableException.h"

namespace aria2 {

/**
 * Throw this exception when a RequestGroup should aborted.
 * FYI, DlAbortEx is the exception to abort 1 Request.
 */
class DownloadFailureException:public RecoverableException {
protected:
  virtual SharedHandle<Exception> copy() const;
public:
  DownloadFailureException(const char* file, int line, const std::string& msg);

  DownloadFailureException(const char* file, int line, const std::string& msg,
                           const Exception& cause);

  DownloadFailureException(const char* file, int line,
                           const std::string& msg,
                           error_code::Value code);

  DownloadFailureException(const char* file, int line,
                           int errNum,
                           const std::string& msg,
                           error_code::Value code);
};

#define DOWNLOAD_FAILURE_EXCEPTION(arg)                 \
  DownloadFailureException(__FILE__, __LINE__, arg)
#define DOWNLOAD_FAILURE_EXCEPTION2(arg1, arg2)                 \
  DownloadFailureException(__FILE__, __LINE__, arg1, arg2)
#define DOWNLOAD_FAILURE_EXCEPTION3(arg1, arg2, arg3)                   \
  DownloadFailureException(__FILE__, __LINE__, arg1, arg2, arg3)

} // namespace aria2

#endif // D_DOWNLOAD_FAILURE_EXCEPTION_H
