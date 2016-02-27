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
#ifndef D_A2_ALGO_H
#define D_A2_ALGO_H

#include "common.h"

#include <algorithm>

namespace aria2 {

template <typename InputIterator, typename OutputIterator>
OutputIterator ncopy(InputIterator first, InputIterator last, size_t count,
                     OutputIterator destination)
{
  OutputIterator x = destination;
  while (count--) {
    x = std::copy(first, last, destination);
  }
  return x;
}

// Find the longest incremental sequence(such as 5,6,7,8,9) in the
// range [first, last) and returns the pair of the iterator points the
// first element of the sequence and the length of the sequence.  The
// incremental sequence is such that value[i]+1 == value[j] for i,j
// that satisfy i+1=j in a range.
//
// For example, the longest incremental sequence in {
// 1,2,3,4,7,10,11,12,13,14,15,100,112,113,114} is
// {10,11,12,13,14,15}.  Therefore, returns the iterator points to 10
// and the length 6.
template <typename InputIterator>
std::pair<InputIterator, size_t> max_sequence(InputIterator first,
                                              InputIterator last)
{
  InputIterator maxfirst = last;
  size_t maxlen = 0;
  while (first != last) {
    InputIterator seqfirst = first;
    size_t len = 1;
    InputIterator prev = seqfirst;
    ++first;
    while (first != last && *prev + 1 == *first) {
      ++len;
      prev = first;
      ++first;
    }
    if (maxlen < len) {
      maxlen = len;
      maxfirst = seqfirst;
    }
  }
  return std::pair<InputIterator, size_t>(maxfirst, maxlen);
}

template <typename InputIterator, typename T>
InputIterator findSecond(InputIterator first, InputIterator last, const T& t)
{
  for (; first != last; ++first) {
    if ((*first).second == t) {
      return first;
    }
  }
  return last;
}

template <class InputIterator, class Predicate>
InputIterator find_wrap_if(InputIterator first, InputIterator last,
                           InputIterator current, Predicate pred)
{
  InputIterator itr = std::find_if(current, last, pred);
  if (itr == last) {
    itr = std::find_if(first, current, pred);
  }
  return itr;
}

} // namespace aria2

#endif // D_A2_ALGO_H
