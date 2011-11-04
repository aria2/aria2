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
#include "ParameterizedStringParser.h"

#include <utility>

#include "DlAbortEx.h"
#include "util.h"
#include "PStringSegment.h"
#include "PStringSelect.h"
#include "PStringNumLoop.h"
#include "NumberDecorator.h"
#include "FixedWidthNumberDecorator.h"
#include "AlphaNumberDecorator.h"

namespace aria2 {

SharedHandle<PStringDatum>
ParameterizedStringParser::parse(const std::string& src)
{
  int offset = 0;
  return diggPString(src, offset);
}

SharedHandle<PStringDatum>
ParameterizedStringParser::diggPString(const std::string& src, int& offset)
{
  if(src.size() == (size_t)offset) {
    return SharedHandle<PStringDatum>();
  }
  switch(src[offset]) {
  case '[':
    return createLoop(src, offset);
  case '{':
    return createSelect(src, offset);
  default:
    return createSegment(src, offset);
  } 
}

SharedHandle<PStringDatum>
ParameterizedStringParser::createSegment(const std::string& src, int& offset)
{
  std::string::size_type nextDelimiterIndex = src.find_first_of("[{", offset);
  if(nextDelimiterIndex == std::string::npos) {
    nextDelimiterIndex = src.size();
  }
  std::string value = src.substr(offset, nextDelimiterIndex-offset);
  offset = nextDelimiterIndex;
  SharedHandle<PStringDatum> next = diggPString(src, offset);
  return SharedHandle<PStringDatum>(new PStringSegment(value, next));
}

SharedHandle<PStringDatum>
ParameterizedStringParser::createSelect(const std::string& src, int& offset)
{
  ++offset;
  std::string::size_type rightParenIndex = src.find("}", offset);
  if(rightParenIndex == std::string::npos) {
    throw DL_ABORT_EX("Missing '}' in the parameterized string.");
  }
  std::vector<std::string> values;
  util::split(src.begin()+offset, src.begin()+rightParenIndex,
              std::back_inserter(values), ',', true);
  if(values.empty()) {
    throw DL_ABORT_EX("Empty {} is not allowed.");
  }
  offset = rightParenIndex+1;
  SharedHandle<PStringDatum> next = diggPString(src, offset);
  return SharedHandle<PStringDatum>(new PStringSelect(values, next));
}

SharedHandle<PStringDatum>
ParameterizedStringParser::createLoop(const std::string& src, int& offset)
{
  ++offset;
  std::string::size_type rightParenIndex = src.find("]", offset);
  if(rightParenIndex == std::string::npos) {
    throw DL_ABORT_EX("Missing ']' in the parameterized string.");
  }
  std::string loopStr = src.substr(offset, rightParenIndex-offset);
  offset = rightParenIndex+1;

  unsigned int step = 1;
  std::string::size_type colonIndex = loopStr.find(":");
  if(colonIndex != std::string::npos) {
    std::string stepStr = loopStr.substr(colonIndex+1);
    if(util::isNumber(stepStr)) {
      step = util::parseUInt(loopStr.begin()+colonIndex+1, loopStr.end());
    } else {
      throw DL_ABORT_EX("A step count must be a positive number.");
    }
    loopStr.erase(colonIndex);
  }
  std::pair<Scip, Scip> range;
  util::divide(range, loopStr.begin(), loopStr.end(), '-');
  if(range.first.first == range.first.second ||
     range.second.first == range.second.second) {
    throw DL_ABORT_EX("Loop range missing.");
  }
  SharedHandle<NumberDecorator> nd;
  unsigned int start;
  unsigned int end;
  std::string rstart(range.first.first, range.first.second);
  std::string rend(range.second.first, range.second.second);
  if(util::isNumber(rstart) && util::isNumber(rend)) {
    nd.reset(new FixedWidthNumberDecorator(rstart.size()));
    start = util::parseUInt(rstart.begin(), rstart.end());
    end = util::parseUInt(rend.begin(), rend.end());
  } else if(util::isLowercase(rstart) && util::isLowercase(rend)) {
    nd.reset(new AlphaNumberDecorator(rstart.size()));
    start = util::alphaToNum(rstart);
    end = util::alphaToNum(rend);
  } else if(util::isUppercase(rstart) && util::isUppercase(rend)) {
    nd.reset(new AlphaNumberDecorator(rstart.size(), true));
    start = util::alphaToNum(rstart);
    end = util::alphaToNum(rend);
  } else {
    throw DL_ABORT_EX("Invalid loop range.");
  }

  SharedHandle<PStringDatum> next(diggPString(src, offset));
  return SharedHandle<PStringDatum>(new PStringNumLoop(start, end, step, nd, next));
}

} // namespace aria2
