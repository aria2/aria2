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

namespace aria2 {

UriListParser::UriListParser(const std::string& filename)
#if HAVE_ZLIB
    : fp_(make_unique<GZipFile>(filename.c_str(), IOFile::READ))
#else
    : fp_(make_unique<BufferedFile>(filename.c_str(), IOFile::READ))
#endif
{
}

UriListParser::~UriListParser() = default;

void UriListParser::parseNext(std::vector<std::string>& uris, Option& op)
{
  const std::shared_ptr<OptionParser>& optparser = OptionParser::getInstance();
  while (1) {
    if (!line_.empty() && line_[0] != '#') {
      util::split(line_.begin(), line_.end(), std::back_inserter(uris), '\t',
                  true);
      // Read options
      std::stringstream ss;
      while (1) {
        line_ = fp_->getLine();
        if (line_.empty()) {
          if (fp_->eof()) {
            break;
          }
          else if (!fp_) {
            throw DL_ABORT_EX("UriListParser:I/O error.");
          }
          else {
            continue;
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
    line_ = fp_->getLine();
    if (line_.empty()) {
      if (fp_->eof()) {
        return;
      }
      else if (!fp_) {
        throw DL_ABORT_EX("UriListParser:I/O error.");
      }
    }
  }
}

bool UriListParser::hasNext()
{
  bool rv = !line_.empty() || (fp_ && *fp_ && !fp_->eof());
  if (!rv) {
    fp_->close();
  }
  return rv;
}

} // namespace aria2
