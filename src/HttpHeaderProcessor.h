/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#ifndef D_HTTP_HEADER_PROCESSOR_H
#define D_HTTP_HEADER_PROCESSOR_H

#include "common.h"

#include <utility>
#include <string>
#include <memory>

namespace aria2 {

class HttpHeader;

class HttpHeaderProcessor {
public:
  enum ParserMode { CLIENT_PARSER, SERVER_PARSER };

  HttpHeaderProcessor(ParserMode mode);

  ~HttpHeaderProcessor();
  /**
   * Parses incoming data. Returns true if end of header is reached.
   * This function stops processing data when end of header is
   * reached.
   */
  bool parse(const unsigned char* data, size_t length);
  bool parse(const std::string& data);

  /**
   * Retruns the number of bytes processed in the last invocation of
   * parse().
   */
  size_t getLastBytesProcessed() const;

  /**
   * Processes the received header as a http response header and
   * returns HttpHeader object. This method transfers the ownership of
   * resulting HttpHeader to the caller.
   */
  std::unique_ptr<HttpHeader> getResult();

  std::string getHeaderString() const;

  /**
   * Resets internal status and ready for next header processing.
   */
  void clear();

private:
  ParserMode mode_;
  int state_;
  size_t lastBytesProcessed_;
  std::string buf_;
  std::string lastFieldName_;
  int lastFieldHdKey_;
  std::unique_ptr<HttpHeader> result_;
  std::string headers_;
};

} // namespace aria2

#endif // D_HTTP_HEADER_PROCESSOR_H
