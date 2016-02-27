/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2014 Tatsuhiro Tsujikawa
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
#ifndef D_OPENED_FILE_COUNTER_H
#define D_OPENED_FILE_COUNTER_H

#include "common.h"

namespace aria2 {

class RequestGroupMan;

class OpenedFileCounter {
public:
  OpenedFileCounter(RequestGroupMan* rgman, size_t maxOpenFiles);

  // Keeps the number of open files under the global limit specified
  // in the option.  The caller requests that |numNewFiles| files are
  // going to be opened.  This function requires that |numNewFiles| is
  // less than or equal to the limit.
  //
  // Currently the only download using MultiDiskAdaptor is affected by
  // the global limit.
  void ensureMaxOpenFileLimit(size_t numNewFiles);

  // Reduces the number of open files managed by this object.
  void reduceNumOfOpenedFile(size_t numCloseFiles);

  void setMaxOpenFiles(size_t maxOpenFiles) { maxOpenFiles_ = maxOpenFiles; }

  // Deactivates this object.
  void deactivate();

private:
  RequestGroupMan* rgman_;
  size_t maxOpenFiles_;
  size_t numOpenFiles_;
};

} // namespace aria2

#endif // D_OPENED_FILE_COUNTER_H
