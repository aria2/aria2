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
#include "BencodeParser.h"
#include "StructParserStateMachine.h"
#include "util.h"

namespace aria2 {

namespace bittorrent {

namespace {
enum {
  BENCODE_FINISH,
  BENCODE_ERROR,
  BENCODE_INITIAL,
  BENCODE_VALUE,
  BENCODE_DICT_KEY,
  BENCODE_DICT_VAL,
  BENCODE_LIST,
  BENCODE_STRING_LEN,
  BENCODE_STRING,
  BENCODE_NUMBER_SIGN,
  BENCODE_NUMBER
};
} // namespace

BencodeParser::BencodeParser(StructParserStateMachine* psm)
  : psm_(psm),
    currentState_(BENCODE_INITIAL),
    numberSign_(1),
    number_(0),
    numConsumed_(0),
    lastError_(0)
{
  stateStack_.push(BENCODE_FINISH);
}

BencodeParser::~BencodeParser()
{}

ssize_t BencodeParser::parseUpdate(const char* data, size_t size)
{
  size_t i;
  if(currentState_ == BENCODE_FINISH) {
    return 0;
  } else if(currentState_ == BENCODE_ERROR) {
    return lastError_;
  }
  for(i = 0; i < size && currentState_ != BENCODE_FINISH; ++i) {
    char c = data[i];
    switch(currentState_) {
    case BENCODE_LIST:
      if(c == 'e') {
        onListEnd();
        break;
      } else {
        int rv = pushState(currentState_);
        if(rv < 0) {
          return rv;
        }
        currentState_ = BENCODE_VALUE;
        runBeginCallback(STRUCT_ARRAY_DATA_T);
      }
      // Fall through
    case BENCODE_INITIAL:
    case BENCODE_VALUE:
      switch(c) {
      case 'd': {
        currentState_ = BENCODE_DICT_KEY;
        runBeginCallback(STRUCT_DICT_T);
        break;
      }
      case'l':
        currentState_ = BENCODE_LIST;
        runBeginCallback(STRUCT_ARRAY_T);
        break;
      case 'i':
        number_ = 0;
        numberSign_ = 1;
        numConsumed_ = 0;
        currentState_ = BENCODE_NUMBER_SIGN;
        runBeginCallback(STRUCT_NUMBER_T);
        break;
      default:
        if(util::isDigit(c)) {
          strLength_ = c - '0';
          numConsumed_ = 1;
          currentState_ = BENCODE_STRING_LEN;
          runBeginCallback(STRUCT_STRING_T);
          break;
        } else {
          currentState_ = BENCODE_ERROR;
          return lastError_ = ERR_UNEXPECTED_CHAR_BEFORE_VAL;
        }
      }
      break;
    case BENCODE_DICT_KEY: {
      if(c == 'e') {
        onDictEnd();
        break;
      }
      int rv = pushState(currentState_);
      if(rv < 0) {
        return rv;
      }
      strLength_ = 0;
      numConsumed_ = 0;
      runBeginCallback(STRUCT_DICT_KEY_T);
      currentState_ = BENCODE_STRING_LEN;
      // Fall through
    }
    case BENCODE_STRING_LEN: {
      size_t j;
      for(j = i; j < size && in(data[j], '0', '9'); ++j) {
        if((INT64_MAX - (data[j] - '0'))/ 10 < strLength_) {
          currentState_ = BENCODE_ERROR;
          return lastError_ = ERR_STRING_LENGTH_OUT_OF_RANGE;
        }
        strLength_ *= 10;
        strLength_ += data[j] - '0';
      }
      numConsumed_ += j - i;
      if(j != size) {
        if(data[j] != ':' || numConsumed_ == 0) {
          currentState_ = BENCODE_ERROR;
          return lastError_ = ERR_INVALID_STRING_LENGTH;
        }
        i = j;
        currentState_ = BENCODE_STRING;
        if(strLength_ == 0) {
          runCharactersCallback(0, 0);
          onStringEnd();
        }
      } else {
        i = j - 1;
      }
      break;
    }
    case BENCODE_STRING: {
      size_t nread = std::min(static_cast<int64_t>(size - i), strLength_);
      runCharactersCallback(&data[i], nread);
      strLength_ -= nread;
      i += nread - 1;
      if(strLength_ == 0) {
        onStringEnd();
      }
      break;
    }
    case BENCODE_NUMBER_SIGN: {
      switch(c) {
      case '+':
        numberSign_ = 1;
        currentState_ = BENCODE_NUMBER;
        break;
      case '-':
        numberSign_ = -1;
        currentState_ = BENCODE_NUMBER;
        break;
      default:
        if(util::isDigit(c)) {
          number_ = c - '0';
          numConsumed_ = 1;
          currentState_ = BENCODE_NUMBER;
        }
      }
      break;
    }
    case BENCODE_NUMBER: {
      size_t j;
      for(j = i; j < size && in(data[j], '0', '9'); ++j) {
        if((INT64_MAX - (data[j] - '0'))/ 10 < number_) {
          currentState_ = BENCODE_ERROR;
          return lastError_ = ERR_NUMBER_OUT_OF_RANGE;
        }
        number_ *= 10;
        number_ += data[j] - '0';
      }
      numConsumed_ += j - i;
      if(j != size) {
        if(data[j] != 'e' || numConsumed_ == 0) {
          currentState_ = BENCODE_ERROR;
          return lastError_ = ERR_INVALID_NUMBER;
        }
        i = j;
        onNumberEnd();
      } else {
        i = j - 1;
      }
      break;
    }
    }
  }
  return i;
}

ssize_t BencodeParser::parseFinal(const char* data, size_t len)
{
  ssize_t rv;
  rv = parseUpdate(data, len);
  if(rv >= 0) {
    if(currentState_ != BENCODE_FINISH &&
       currentState_ != BENCODE_INITIAL) {
      rv = ERR_PREMATURE_DATA;
    }
  }
  return rv;
}

void BencodeParser::reset()
{
  psm_->reset();
  currentState_ = BENCODE_INITIAL;
  lastError_ = 0;
  while(!stateStack_.empty()) {
    stateStack_.pop();
  }
  stateStack_.push(BENCODE_FINISH);
}

void BencodeParser::onStringEnd()
{
  runEndCallback(stateTop() == BENCODE_DICT_KEY ?
                 STRUCT_DICT_KEY_T : STRUCT_STRING_T);
  onValueEnd();
}

void BencodeParser::onNumberEnd()
{
  runNumberCallback(numberSign_ * number_);
  runEndCallback(STRUCT_NUMBER_T);
  onValueEnd();
}

void BencodeParser::onDictEnd()
{
  runEndCallback(STRUCT_DICT_T);
  onValueEnd();
}

void BencodeParser::onListEnd()
{
  runEndCallback(STRUCT_ARRAY_T);
  onValueEnd();
}

void BencodeParser::onValueEnd()
{
  switch(stateTop()) {
  case BENCODE_DICT_KEY:
    popState();
    pushState(BENCODE_DICT_VAL);
    currentState_ = BENCODE_VALUE;
    runBeginCallback(STRUCT_DICT_DATA_T);
    break;
  case BENCODE_DICT_VAL:
    runEndCallback(STRUCT_DICT_DATA_T);
    popState();
    currentState_ = BENCODE_DICT_KEY;
    break;
  case BENCODE_LIST:
    runEndCallback(STRUCT_ARRAY_DATA_T);
    popState();
    currentState_ = BENCODE_LIST;
    break;
  default:
    assert(stateTop() == BENCODE_FINISH);
    currentState_ = stateTop();
    break;
  }
}

int BencodeParser::pushState(int state)
{
  if(stateStack_.size() >= 50) {
    return ERR_STRUCTURE_TOO_DEEP;
  } else {
    stateStack_.push(state);
    return 0;
  }
}

int BencodeParser::stateTop() const
{
  return stateStack_.top();
}

int BencodeParser::popState()
{
  int state = stateStack_.top();
  stateStack_.pop();
  return state;
}

void BencodeParser::runBeginCallback(int elementType)
{
  // switch(elementType) {
  // case STRUCT_DICT_T:
  //   std::cout << "object start" << std::endl;
  //   break;
  // case STRUCT_DICT_KEY_T:
  //   std::cout << "object key start" << std::endl;
  //   break;
  // case STRUCT_DICT_DATA_T:
  //   std::cout << "object data start" << std::endl;
  //   break;
  // case STRUCT_ARRAY_T:
  //   std::cout << "array start" << std::endl;
  //   break;
  // case STRUCT_ARRAY_DATA_T:
  //   std::cout << "array data start" << std::endl;
  //   break;
  // case STRUCT_STRING_T:
  //   std::cout << "string start" << std::endl;
  //   break;
  // case STRUCT_NUMBER_T:
  //   std::cout << "number start" << std::endl;
  //   break;
  // case STRUCT_BOOL_T:
  //   std::cout << "bool start" << std::endl;
  //   break;
  // case STRUCT_NULL_T:
  //   std::cout << "null start" << std::endl;
  //   break;
  // default:
  //   break;
  // };
  psm_->beginElement(elementType);
}

void BencodeParser::runEndCallback(int elementType)
{
  // switch(elementType) {
  // case STRUCT_DICT_T:
  //   std::cout << "object end" << std::endl;
  //   break;
  // case STRUCT_DICT_KEY_T:
  //   std::cout << "object key end" << std::endl;
  //   break;
  // case STRUCT_DICT_DATA_T:
  //   std::cout << "object data end" << std::endl;
  //   break;
  // case STRUCT_ARRAY_T:
  //   std::cout << "array end" << std::endl;
  //   break;
  // case STRUCT_ARRAY_DATA_T:
  //   std::cout << "array data end" << std::endl;
  //   break;
  // case STRUCT_STRING_T:
  //   std::cout << "string end" << std::endl;
  //   break;
  // case STRUCT_NUMBER_T:
  //   std::cout << "number end" << std::endl;
  //   break;
  // case STRUCT_BOOL_T:
  //   std::cout << "bool end" << std::endl;
  //   break;
  // case STRUCT_NULL_T:
  //   std::cout << "null end" << std::endl;
  //   break;
  // default:
  //   break;
  // };
  psm_->endElement(elementType);
}

void BencodeParser::runCharactersCallback(const char* data, size_t len)
{
  psm_->charactersCallback(data, len);
}

void BencodeParser::runNumberCallback(int64_t number)
{
  psm_->numberCallback(number, 0, 0);
}

} // namespace bittorrent

} // namespace aria2
