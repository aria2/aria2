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
#ifndef D_BENCODE_PARSER_H
#define D_BENCODE_PARSER_H

#include "common.h"

#include <stack>

namespace aria2 {

class StructParserStateMachine;

namespace bittorrent {

enum BencodeError {
  ERR_UNEXPECTED_CHAR_BEFORE_VAL = -1,
  ERR_INVALID_NUMBER = -2,
  ERR_NUMBER_OUT_OF_RANGE = -3,
  ERR_PREMATURE_DATA = -4,
  ERR_STRUCTURE_TOO_DEEP = -5,
  ERR_INVALID_STRING_LENGTH = -6,
  ERR_STRING_LENGTH_OUT_OF_RANGE = -7,
  ERR_INVALID_FLOAT_NUMBER = -8,
};

class BencodeParser {
public:
  BencodeParser(StructParserStateMachine* psm);
  ~BencodeParser();
  // Parses |size| bytes of data |data| and returns the number of
  // bytes processed. On error, one of the negative error codes is
  // returned.
  ssize_t parseUpdate(const char* data, size_t size);
  // Parses |size| bytes of data |data| and returns the number of
  // bytes processed. On error, one of the negative error codes is
  // returned. Call this function to signal the parser that this is
  // the last piece of data. This function does NOT reset the internal
  // state.
  ssize_t parseFinal(const char* data, size_t size);
  // Resets the internal state of the parser and makes it ready for
  // reuse.
  void reset();

private:
  int pushState(int state);
  int stateTop() const;
  int popState();
  void runBeginCallback(int elementType);
  void runEndCallback(int elementType);
  void runCharactersCallback(const char* data, size_t len);
  void runNumberCallback(int64_t number);

  void onStringEnd();
  void onNumberEnd();
  void onDictEnd();
  void onListEnd();
  void onValueEnd();

  StructParserStateMachine* psm_;
  std::stack<int> stateStack_;
  int currentState_;
  int64_t strLength_;
  int numberSign_;
  int64_t number_;
  size_t numConsumed_;
  int lastError_;
};

} // namespace bittorrent

} // namespace aria2

#endif // D_BENCODE_PARSER_H
