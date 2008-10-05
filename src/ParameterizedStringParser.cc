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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "FatalException.h"
#include "Util.h"
#include "PStringSegment.h"
#include "PStringSelect.h"
#include "PStringNumLoop.h"
#include "NumberDecorator.h"
#include "FixedWidthNumberDecorator.h"
#include "AlphaNumberDecorator.h"
#include <utility>

namespace aria2 {

PStringDatumHandle ParameterizedStringParser::parse(const std::string& src)
{
  int offset = 0;
  return diggPString(src, offset);
}

PStringDatumHandle ParameterizedStringParser::diggPString(const std::string& src,
							  int& offset)
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

PStringDatumHandle ParameterizedStringParser::createSegment(const std::string& src,
							    int& offset)
{
  std::string::size_type nextDelimiterIndex = src.find_first_of("[{", offset);
  if(nextDelimiterIndex == std::string::npos) {
    nextDelimiterIndex = src.size();
  }
  std::string value = src.substr(offset, nextDelimiterIndex-offset);
  offset = nextDelimiterIndex;
  PStringDatumHandle next = diggPString(src, offset);
  return SharedHandle<PStringDatum>(new PStringSegment(value, next));
}

PStringDatumHandle ParameterizedStringParser::createSelect(const std::string& src,
							   int& offset)
{
  ++offset;
  std::string::size_type rightParenIndex = src.find("}", offset);
  if(rightParenIndex == std::string::npos) {
    throw FatalException("Missing '}' in the parameterized string.");
  }
  std::deque<std::string> values;
  Util::slice(values, src.substr(offset, rightParenIndex-offset), ',', true);
  if(values.empty()) {
    throw FatalException("Empty {} is not allowed.");
  }
  offset = rightParenIndex+1;
  PStringDatumHandle next = diggPString(src, offset);
  return SharedHandle<PStringDatum>(new PStringSelect(values, next));
}

PStringDatumHandle ParameterizedStringParser::createLoop(const std::string& src,
							 int& offset)
{
  ++offset;
  std::string::size_type rightParenIndex = src.find("]", offset);
  if(rightParenIndex == std::string::npos) {
    throw FatalException("Missing ']' in the parameterized string.");
  }
  std::string loopStr = src.substr(offset, rightParenIndex-offset);
  offset = rightParenIndex+1;

  unsigned int step = 1;
  std::string::size_type colonIndex = loopStr.find(":");
  if(colonIndex != std::string::npos) {
    std::string stepStr = loopStr.substr(colonIndex+1);
    if(Util::isNumber(stepStr)) {
      step = Util::parseUInt(stepStr);
    } else {
      throw FatalException("A step count must be a positive number.");
    }
    loopStr.erase(colonIndex);
  }
  std::pair<std::string, std::string> range = Util::split(loopStr, "-");
  if(range.first.empty() || range.second.empty()) {
    throw FatalException("Loop range missing.");
  }
  NumberDecoratorHandle nd;
  unsigned int start;
  unsigned int end;
  if(Util::isNumber(range.first) && Util::isNumber(range.second)) {
    nd.reset(new FixedWidthNumberDecorator(range.first.size()));
    start = Util::parseUInt(range.first);
    end = Util::parseUInt(range.second);
  } else if(Util::isLowercase(range.first) && Util::isLowercase(range.second)) {
    nd.reset(new AlphaNumberDecorator(range.first.size()));
    start = Util::alphaToNum(range.first);
    end = Util::alphaToNum(range.second);
  } else if(Util::isUppercase(range.first) && Util::isUppercase(range.second)) {
    nd.reset(new AlphaNumberDecorator(range.first.size(), true));
    start = Util::alphaToNum(range.first);
    end = Util::alphaToNum(range.second);
  } else {
    throw FatalException("Invalid loop range.");
  }

  PStringDatumHandle next(diggPString(src, offset));
  return SharedHandle<PStringDatum>(new PStringNumLoop(start, end, step, nd, next));
}

} // namespace aria2
