/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#ifndef D_PARAMED_STRING_H
#define D_PARAMED_STRING_H

#include "common.h"

#include <string>
#include <vector>
#include <algorithm>

#include "util.h"
#include "DlAbortEx.h"
#include "fmt.h"

namespace aria2 {

namespace paramed_string {

template<typename InputIterator>
InputIterator expandChoice
(std::vector<std::string>& res, InputIterator first, InputIterator last)
{
  ++first;
  InputIterator i = std::find(first, last, '}');
  if(i == last) {
    throw DL_ABORT_EX("Missing '}' in the parameterized string.");
  }
  std::vector<Scip> choices;
  util::splitIter(first, i, std::back_inserter(choices), ',', true, false);
  std::vector<std::string> res2;
  res2.reserve(res.size()*choices.size());
  for(std::vector<std::string>::const_iterator i = res.begin(),
        eoi = res.end(); i != eoi; ++i) {
    for(std::vector<Scip>::const_iterator j = choices.begin(),
          eoj = choices.end(); j != eoj; ++j) {
      res2.push_back(*i);
      res2.back().append((*j).first, (*j).second);
    }
  }
  res.swap(res2);
  return i+1;
}

template<typename InputIterator>
int32_t fromBase26(InputIterator first, InputIterator last, char zero)
{
  int32_t res = 0;
  for(; first != last; ++first) {
    res *= 26;
    res += *first-zero;
    if(res > static_cast<int32_t>(UINT16_MAX)) {
      throw DL_ABORT_EX("Loop range overflow.");
    }
  }
  return res;
}

std::string toBase26(int32_t n, char zero, size_t width);

template<typename InputIterator>
InputIterator expandLoop
(std::vector<std::string>& res, InputIterator first, InputIterator last)
{
  ++first;
  InputIterator i = std::find(first, last, ']');
  if(i == last) {
    throw DL_ABORT_EX("Missing ']' in the parameterized string.");
  }
  InputIterator colon = std::find(first, i, ':');
  uint32_t step;
  if(colon == i) {
    step = 1;
  } else {
    if(!util::parseUIntNoThrow(step, std::string(colon+1, i))) {
      throw DL_ABORT_EX("A step count must be a positive number.");
    }
    if(step > UINT16_MAX) {
      throw DL_ABORT_EX("Loop step overflow.");
    }
  }
  InputIterator minus = std::find(first, colon, '-');
  if(minus == colon) {
    throw DL_ABORT_EX("Loop range missing.");
  }
  if(util::isNumber(first, minus) && util::isNumber(minus+1, colon)) {
    uint32_t start, end;
    if(!util::parseUIntNoThrow(start, std::string(first, minus)) ||
       !util::parseUIntNoThrow(end, std::string(minus+1, colon))) {
      throw DL_ABORT_EX("Loop range missing.");
    }
    if(start > UINT16_MAX || end > UINT16_MAX) {
      throw DL_ABORT_EX("Loop range overflow.");
    }
    if(start <= end) {
      std::string format;
      if(minus-first == colon-minus-1) {
        format = fmt("%%0%lud", static_cast<unsigned long>(minus-first));
      } else {
        format = "%d";
      }
      std::vector<std::string> res2;
      res2.reserve(res.size()*((end+1-start)/step));
      for(std::vector<std::string>::const_iterator i = res.begin(),
            eoi = res.end(); i != eoi; ++i) {
        for(uint32_t j = start; j <= end; j += step) {
          res2.push_back(*i);
          res2.back() += fmt(format.c_str(), j);
        }
      }
      res.swap(res2);
    }
  } else if((util::isLowercase(first, minus) &&
             util::isLowercase(minus+1, colon)) ||
            (util::isUppercase(first, minus) &&
             util::isUppercase(minus+1, colon))) {
    char zero = ('a' <= *first && *first <= 'z' ? 'a' : 'A');
    int32_t start, end;
    start = fromBase26(first, minus, zero);
    end = fromBase26(minus+1, colon, zero);
    if(start <= end) {
      size_t width;
      if(minus-first == colon-minus-1) {
        width = minus-first;
      } else {
        width = 0;
      }
      std::vector<std::string> res2;
      res2.reserve(res.size()*((end+1-start)/step));
      for(std::vector<std::string>::const_iterator i = res.begin(),
            eoi = res.end(); i != eoi; ++i) {
        for(int32_t j = start; j <= end; j += step) {
          res2.push_back(*i);
          res2.back() += toBase26(j, zero, width);
        }
      }
      res.swap(res2);
    }
  } else {
    throw DL_ABORT_EX("Invalid loop range.");
  }
  return i+1;
}

// Expand parameterized string.
// The available parameters are loop [] and choice {}.
//
// Loop: [START-END:STEP]
//
// A is arbitrary string. START and END must satisfy one of following
// condition:
//
// * both are decimal digits and START <= END.
//
// * both are composed of 'a' to 'z' letter and START <= END
//   lexicographically.
//
// * both are composed of 'A' to 'Z' letter and START <= END
//   lexicographically.
//
// Leading zeros in START and END are kep preserved if the length of
// START and END in string representation is equal.
//
// When loop through START to END, we include both START and END.
//
// STEP is dicimal number and it is used as loop step.  STEP can be
// omitted. If omitted, preceding ':' also must be omitted.
//
// START, END and STEP must be less than or equal to 65535 in decimal.
//
// Examples:
// "alpha:[1-2]:bravo" -> ["alpha:1:bravo", "alpha:2:bravo"]
// "alpha:[05-10:5]" -> ["alpha:05:bravo", "alpha:10:bravo"]
//
// Choice: {C1,C2,...,Cn}
//
// C1 to Cn are arbitrary string but they cannot contain ','.
//
// Examples:
// "alpha:[foo,bar]:bravo" -> ["alpha:foo:bravo", "alpha:bar:bravo"]
template<typename InputIterator, typename OutputIterator>
void expand
(InputIterator first,
 InputIterator last,
 OutputIterator out)
{
  std::vector<std::string> res;
  res.push_back("");
  while(first != last) {
    InputIterator i = first;
    for(; i != last && *i != '{' && *i != '['; ++i);
    for(std::vector<std::string>::iterator j = res.begin(), eoj = res.end();
        j != eoj; ++j) {
      (*j).append(first, i);
    }
    first = i;
    if(first == last) {
      break;
    }
    if(*first == '{') {
      first = expandChoice(res, first, last);
    } else if(*first == '[') {
      first = expandLoop(res, first, last);
    }
  }
  if(res.size() != 1 || !res[0].empty()) {
    std::copy(res.begin(), res.end(), out);
  }
}

} // namespace paramed_string

} // namespace aria2

#endif // D_PARAMED_STRING_H
