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

#include <istream>
#include <sstream>

#include "util.h"
#include "Option.h"
#include "OptionHandlerFactory.h"
#include "OptionHandler.h"
#include "A2STR.h"

namespace aria2 {

UriListParser::UriListParser(std::istream& in):in_(in)
{
  optparser_.setOptionHandlers(OptionHandlerFactory::createOptionHandlers());
}

UriListParser::~UriListParser() {}

void UriListParser::getOptions(Option& op)
{
  std::stringstream ss;
  while(getline(in_, line_)) {
    if(util::startsWith(line_, " ")) {
      ss << line_ << "\n";
    } else if(util::startsWith(line_, A2STR::SHARP_C)) {
      continue;
    } else {
      break;
    }
  }
  optparser_.parse(op, ss);
}

void UriListParser::parseNext(std::vector<std::string>& uris, Option& op)
{
  if(line_.empty()) {
    getline(in_, line_);
  }
  if(!in_) {
    return;
  }
  do {
    if(!util::startsWith(line_, A2STR::SHARP_C) && !util::trim(line_).empty()) {
      util::split(line_, std::back_inserter(uris), "\t", true);
      getOptions(op);
      return;
    }
  } while(getline(in_, line_));
}

bool UriListParser::hasNext() const
{
  return in_;
}

} // namespace aria2
