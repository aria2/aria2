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
#ifndef D_URI_LIST_PARSER_H
#define D_URI_LIST_PARSER_H

#include "common.h"

#include <string>
#include <deque>
#include <iosfwd>
#include <memory>

#include "Option.h"
#include "IOFile.h"

namespace aria2 {

class UriListParser {
private:
  // Detected stdin pipe fd (-1 when input is not a pipe/terminal).
  // Declared first: fp_ construction depends on it because a
  // GZipFile wrapper would dup(0)+close(0) and steal the fd.
  int pollFd_{-1};

  std::unique_ptr<IOFile> fp_;

  std::string line_;

  // State for non-blocking reads when the input is a pipe or terminal.
  // For these inputs we bypass the stdio based IOFile and read the fd
  // directly, accumulating bytes in pendingBuf_.
  bool stdinPipe_{false};

  std::string pendingBuf_;

  bool pipeEof_{false};

  // Drain all currently available bytes from the pipe into
  // pendingBuf_. Sets pipeEof_ when the writer closed the pipe.
  void pumpInput();

  // Fetch the next complete line into line_. Returns false when no
  // complete line is available (suspended pipe) or the input is
  // exhausted; pipeEof_ distinguishes the two cases.
  bool fetchPipeLine();

public:
  UriListParser(const std::string& filename);

  ~UriListParser();

  void parseNext(std::vector<std::string>& uris, Option& op);

  bool hasNext();

  // Returns true if a line is ready to be read without blocking.
  // Always true for regular files; for pipes/terminals uses poll()
  // with the given timeout in milliseconds (0 = pure readiness test).
  bool inputReady(int timeoutMs = 0) const;

  // Blocks until pipe input becomes available or the writer closes
  // the pipe. Used by the non-deferred input path which must consume
  // the whole list before downloads start.
  void waitForInput();

  // True when the input reached its real end (EOF); a suspended pipe
  // is not exhausted.
  bool exhausted() const;

  bool isPipeInput() const { return stdinPipe_; }

  int inputFd() const { return pollFd_; }
};

} // namespace aria2

#endif // D_URI_LIST_PARSER_H
