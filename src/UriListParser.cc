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
#include "UriListParser.h"

#include <cstring>
#include <sstream>

#include "a2io.h"
#include "util.h"
#include "Option.h"
#include "OptionHandlerFactory.h"
#include "OptionHandler.h"
#include "A2STR.h"
#include "BufferedFile.h"
#include "OptionParser.h"

#if HAVE_ZLIB
#  include "GZipFile.h"
#endif

#if !defined(_WIN32) && defined(HAVE_POLL)
#  include <fcntl.h>
#  include <unistd.h>
#  include <poll.h>
#  define A2_PIPE_INPUT_SUPPORTED 1
#endif

namespace aria2 {

namespace {
// How long parseNext waits for option lines that might still belong to
// the current entry when reading from a pipe. Producers typically write
// an entry (URL line plus option lines) in one or few writes; a short
// grace period binds options written slightly later than the URL line
// without stalling the event loop.
const int URI_OPTION_GRACE_MS = 100;

// Returns the fd to poll/read for stdin pipe input, or -1 when the
// input is not stdin or not a pipe/terminal.
int detectStdinPipeFd(const std::string& filename)
{
#ifdef A2_PIPE_INPUT_SUPPORTED
  if (filename != "-" && filename != DEV_STDIN) {
    return -1;
  }
  struct stat st {};
  if (::fstat(STDIN_FILENO, &st) == 0 &&
      (S_ISFIFO(st.st_mode) || S_ISCHR(st.st_mode))) {
    return STDIN_FILENO;
  }
#endif // A2_PIPE_INPUT_SUPPORTED
  return -1;
}
} // namespace

UriListParser::UriListParser(const std::string& filename)
    : pollFd_(detectStdinPipeFd(filename)),
      // For stdin pipes no IOFile is created at all: a GZipFile would
      // dup(fileno(stdin)) and fclose(stdin), moving the pipe to another
      // fd and closing fd 0. Reading the raw fd avoids that entirely.
      fp_(pollFd_ != -1
              ? nullptr
              :
#if HAVE_ZLIB
              make_unique<GZipFile>(filename.c_str(), IOFile::READ)
#else
              make_unique<BufferedFile>(filename.c_str(), IOFile::READ)
#endif
                  ),
      stdinPipe_(pollFd_ != -1)
{
#ifdef A2_PIPE_INPUT_SUPPORTED
  if (stdinPipe_) {
    // Read the pipe ourselves in non-blocking mode; the stdio based
    // IOFile would pre-fetch bytes into its internal buffer, hiding
    // them from poll() and blocking inside fgets mid-entry.
    int flags = ::fcntl(pollFd_, F_GETFL, 0);
    if (flags != -1) {
      ::fcntl(pollFd_, F_SETFL, flags | O_NONBLOCK);
    }
  }
#endif // A2_PIPE_INPUT_SUPPORTED
}

UriListParser::~UriListParser() = default;

void UriListParser::pumpInput()
{
#ifdef A2_PIPE_INPUT_SUPPORTED
  if (pipeEof_) {
    return;
  }
  char buf[4096];
  for (;;) {
    ssize_t r = ::read(pollFd_, buf, sizeof(buf));
    if (r > 0) {
      pendingBuf_.append(buf, static_cast<size_t>(r));
      continue;
    }
    if (r == 0) {
      pipeEof_ = true;
    }
    break;
  }
#endif // A2_PIPE_INPUT_SUPPORTED
}

bool UriListParser::fetchPipeLine()
{
  pumpInput();
  size_t pos = pendingBuf_.find('\n');
  if (pos != std::string::npos) {
    line_.assign(pendingBuf_, 0, pos);
    pendingBuf_.erase(0, pos + 1);
    return true;
  }
  if (pipeEof_ && !pendingBuf_.empty()) {
    line_ = pendingBuf_;
    pendingBuf_.clear();
    return true;
  }
  line_.clear();
  return false;
}

void UriListParser::parseNext(std::vector<std::string>& uris, Option& op)
{
  const std::shared_ptr<OptionParser>& optparser = OptionParser::getInstance();
  while (1) {
    if (!line_.empty() && line_[0] != '#') {
      util::split(line_.begin(), line_.end(), std::back_inserter(uris), '\t',
                  true);
      // line_ has been consumed as this entry's URIs; the option loop
      // below refills it with a lookahead line. Without clearing, a
      // suspended lookahead would re-emit this entry next call.
      line_.clear();
      // Read options
      std::stringstream ss;
      while (1) {
        if (stdinPipe_) {
          // On a pipe a dry read marks the entry boundary; a blank
          // line also explicitly terminates an entry.
          if (!inputReady(URI_OPTION_GRACE_MS)) {
            break;
          }
          if (!fetchPipeLine()) {
            break;
          }
          if (line_.empty()) {
            break;
          }
        }
        else {
          line_ = fp_->getLine();
          if (line_.empty()) {
            if (fp_->eof()) {
              break;
            }
            else if (!*fp_) {
              throw DL_ABORT_EX("UriListParser:I/O error.");
            }
            else {
              continue;
            }
          }
        }
        if (line_[0] == ' ' || line_[0] == '\t') {
          ss << line_ << "\n";
        }
        else if (line_[0] == '#') {
          continue;
        }
        else {
          break;
        }
      }
      optparser->parse(op, ss);
      return;
    }
    if (stdinPipe_) {
      if (!fetchPipeLine()) {
        // Suspended or exhausted; either way there is no entry to emit.
        return;
      }
    }
    else {
      line_ = fp_->getLine();
      if (line_.empty()) {
        if (fp_->eof()) {
          return;
        }
        else if (!*fp_) {
          throw DL_ABORT_EX("UriListParser:I/O error.");
        }
      }
    }
  }
}

bool UriListParser::hasNext()
{
  bool rv;
  if (stdinPipe_) {
    // Drain available input first so a pending EOF is discovered here
    // instead of reporting readability forever. Only a complete line
    // (or a final partial line at EOF) counts as input; a buffered
    // fragment must suspend instead of spinning parseNext.
    pumpInput();
    rv = !line_.empty() || pendingBuf_.find('\n') != std::string::npos ||
         (pipeEof_ ? !pendingBuf_.empty() : inputReady());
  }
  else {
    rv = !line_.empty() || (fp_ && *fp_ && !fp_->eof());
  }
  if (!rv && exhausted() && fp_) {
    fp_->close();
  }
  return rv;
}

bool UriListParser::exhausted() const
{
  if (stdinPipe_) {
    return line_.empty() && pendingBuf_.empty() && pipeEof_;
  }
  return line_.empty() && (!fp_ || !*fp_ || fp_->eof());
}

bool UriListParser::inputReady(int timeoutMs) const
{
  if (!stdinPipe_) {
    return true;
  }
#ifdef A2_PIPE_INPUT_SUPPORTED
  if (pipeEof_ || pendingBuf_.find('\n') != std::string::npos) {
    return true;
  }
  struct pollfd pfd {pollFd_, POLLIN, 0};
  return ::poll(&pfd, 1, timeoutMs) > 0;
#else
  // Without poll we cannot check non-blocking; assume ready.
  return true;
#endif // A2_PIPE_INPUT_SUPPORTED
}

void UriListParser::waitForInput()
{
#ifdef A2_PIPE_INPUT_SUPPORTED
  if (!stdinPipe_ || pipeEof_ ||
      pendingBuf_.find('\n') != std::string::npos) {
    return;
  }
  struct pollfd pfd {pollFd_, POLLIN, 0};
  ::poll(&pfd, 1, -1);
#endif // A2_PIPE_INPUT_SUPPORTED
}

} // namespace aria2
