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

PStringDatumHandle ParameterizedStringParser::parse(const string& src)
{
  int32_t offset = 0;
  return diggPString(src, offset);
}

PStringDatumHandle ParameterizedStringParser::diggPString(const string& src,
							  int32_t& offset)
{
  if(src.size() == (size_t)offset) {
    return 0;
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

PStringDatumHandle ParameterizedStringParser::createSegment(const string& src,
							    int32_t& offset)
{
  string::size_type nextDelimiterIndex = src.find_first_of("[{", offset);
  if(nextDelimiterIndex == string::npos) {
    nextDelimiterIndex = src.size();
  }
  string value = src.substr(offset, nextDelimiterIndex-offset);
  offset = nextDelimiterIndex;
  PStringDatumHandle next = diggPString(src, offset);
  return new PStringSegment(value, next);
}

PStringDatumHandle ParameterizedStringParser::createSelect(const string& src,
							   int32_t& offset)
{
  ++offset;
  string::size_type rightParenIndex = src.find("}", offset);
  if(rightParenIndex == string::npos) {
    throw new FatalException("Missing '}' in the parameterized string.");
  }
  Strings values;
  Util::slice(values, src.substr(offset, rightParenIndex-offset), ',', true);
  if(values.empty()) {
    throw new FatalException("Empty {} is not allowed.");
  }
  offset = rightParenIndex+1;
  PStringDatumHandle next = diggPString(src, offset);
  return new PStringSelect(values, next);
}

PStringDatumHandle ParameterizedStringParser::createLoop(const string& src,
							 int32_t& offset)
{
  ++offset;
  string::size_type rightParenIndex = src.find("]", offset);
  if(rightParenIndex == string::npos) {
    throw new FatalException("Missing ']' in the parameterized string.");
  }
  string loopStr = src.substr(offset, rightParenIndex-offset);
  offset = rightParenIndex+1;

  int32_t step = 1;
  string::size_type colonIndex = loopStr.find(":");
  if(colonIndex != string::npos) {
    string stepStr = loopStr.substr(colonIndex+1);
    if(Util::isNumber(stepStr)) {
      step = strtol(stepStr.c_str(), 0, 10);
    } else {
      throw new FatalException("A step count must be a number.");
    }
    loopStr.erase(colonIndex);
  }
  pair<string, string> range = Util::split(loopStr, "-");
  if(range.first == "" || range.second == "") {
    throw new FatalException("Loop range missing.");
  }
  NumberDecoratorHandle nd = 0;
  int32_t start;
  int32_t end;
  if(Util::isNumber(range.first) && Util::isNumber(range.second)) {
    nd = new FixedWidthNumberDecorator(range.first.size());
    start = strtol(range.first.c_str(), 0, 10);
    end = strtol(range.second.c_str(), 0, 10);
  } else if(Util::isLowercase(range.first) && Util::isLowercase(range.second)) {
    nd = new AlphaNumberDecorator(range.first.size());
    start = Util::alphaToNum(range.first);
    end = Util::alphaToNum(range.second);
  } else if(Util::isUppercase(range.first) && Util::isUppercase(range.second)) {
    nd = new AlphaNumberDecorator(range.first.size(), true);
    start = Util::alphaToNum(range.first);
    end = Util::alphaToNum(range.second);
  } else {
    throw new FatalException("Invalid loop range.");
  }

  PStringDatumHandle next = diggPString(src, offset);
  return new PStringNumLoop(start, end, step, nd, next);
}
