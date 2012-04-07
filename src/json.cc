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
#include "json.h"

#include <sstream>

#include "array_fun.h"
#include "DlAbortEx.h"
#include "error_code.h"
#include "a2functional.h"
#include "util.h"
#include "fmt.h"
#include "base64.h"

namespace aria2 {

namespace json {

// Function prototype declaration
namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decode(InputIterator first, InputIterator last, size_t depth);
} // namespace

namespace {
const char WS[] = { 0x20, 0x09, 0x0a, 0x0d };
const char ESCAPE_CHARS[] = { '"', '\\', '/', '\b', '\f', '\n', '\r', '\t' };
const size_t MAX_STRUCTURE_DEPTH = 100;
} // namespace

namespace {
template<typename InputIterator>
InputIterator skipWs(InputIterator first, InputIterator last)
{
  while(first != last && std::find(vbegin(WS), vend(WS), *first) != vend(WS)) {
    ++first;
  }
  return first;
}
} // namespace

namespace {
template<typename InputIterator>
void checkEof(InputIterator first, InputIterator last)
{
  if(first == last) {
    throw DL_ABORT_EX2("JSON decoding failed: unexpected EOF",
                       error_code::JSON_PARSE_ERROR);
  }
}
} // namespace

namespace {
template<typename InputIterator>
InputIterator
decodeKeyword(InputIterator first, InputIterator last,
              const std::string& keyword)
{
  size_t len = keyword.size();
  for(size_t i = 0; i < len; ++i) {
    checkEof(first, last);
    if(*first != keyword[i]) {
      throw DL_ABORT_EX2(fmt("JSON decoding failed: %s not found.",
                             keyword.c_str()),
                         error_code::JSON_PARSE_ERROR);
    }
    ++first;
  }
  return first;
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeTrue(InputIterator first, InputIterator last)
{
  first = decodeKeyword(first, last, "true");
  return std::make_pair(Bool::gTrue(), first);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeFalse(InputIterator first, InputIterator last)
{
  first = decodeKeyword(first, last, "false");
  return std::make_pair(Bool::gFalse(), first);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeNull(InputIterator first, InputIterator last)
{
  first = decodeKeyword(first, last, "null");
  return std::make_pair(Null::g(), first);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeString(InputIterator first, InputIterator last)
{
  // Consume first char, assuming it is '"'.
  ++first;
  std::string s;
  InputIterator offset = first;
  while(first != last) {
    if(*first == '"') {
      break;
    }
    if(*first == '\\') {
      s.append(offset, first);
      ++first;
      checkEof(first, last);
      if(*first == 'u') {
        ++first;
        InputIterator uchars = first;
        for(int i = 0; i < 4; ++i, ++first) {
          checkEof(first, last);
        }
        checkEof(first, last);
        uint16_t codepoint = util::parseUInt(std::string(uchars, first), 16);
        if(codepoint <= 0x007fu) {
          unsigned char temp[1];
          temp[0] = static_cast<char>(codepoint);
          s.append(&temp[0], &temp[sizeof(temp)]);
        } else if(codepoint <= 0x07ffu) {
          unsigned char temp[2];
          temp[0] = 0xC0u | (codepoint >> 6);
          temp[1] = 0x80u | (codepoint & 0x003fu);
          s.append(&temp[0], &temp[sizeof(temp)]);
        } else if(in(codepoint, 0xD800u, 0xDBFFu)) {
          // surrogate pair
          if(*first != '\\' || first+1 == last ||
             *(first+1) != 'u') {
            throw DL_ABORT_EX2("JSON decoding failed: bad UTF-8 sequence.",
                               error_code::JSON_PARSE_ERROR);
          }
          first += 2;
          InputIterator uchars = first;
          for(int i = 0; i < 4; ++i, ++first) {
            checkEof(first, last);
          }
          checkEof(first, last);
          uint16_t codepoint2 = util::parseUInt(std::string(uchars, first), 16);
          if(!in(codepoint2, 0xDC00u, 0xDFFFu)) {
            throw DL_ABORT_EX2("JSON decoding failed: bad UTF-8 sequence.",
                               error_code::JSON_PARSE_ERROR);
          }
          uint32_t fullcodepoint = 0x010000u;
          fullcodepoint += (codepoint & 0x03FFu) << 10;
          fullcodepoint += (codepoint2 & 0x03FFu);
          unsigned char temp[4];
          temp[0] = 0xf0u | (fullcodepoint >> 18);
          temp[1] = 0x80u | ((fullcodepoint >> 12) & 0x003Fu);
          temp[2] = 0x80u | ((fullcodepoint >> 6) & 0x003Fu);
          temp[3] = 0x80u | (fullcodepoint & 0x003Fu);
          s.append(&temp[0], &temp[sizeof(temp)]);
        } else {
          unsigned char temp[3];
          temp[0] = 0xE0u | (codepoint >> 12);
          temp[1] = 0x80u | ((codepoint >> 6) & 0x003Fu);
          temp[2] = 0x80u | (codepoint & 0x003Fu);
          s.append(&temp[0], &temp[sizeof(temp)]);
        }
        offset = first;
      } else {
        if(*first == 'b') {
          s += "\b";
        } else if(*first == 'f') {
          s += "\f";
        } else if(*first == 'n') {
          s += "\n";
        } else if(*first == 'r') {
          s += "\r";
        } else if(*first == 't') {
          s += "\t";
        } else {
          s.append(first, first+1);
        }
        ++first;
        offset = first;
      }
    } else {
      ++first;
    }
  }
  checkEof(first, last);
  if(std::distance(offset, first) > 0) {
    s.append(offset, first);
  }
  if(!util::isUtf8(s)) {
    throw DL_ABORT_EX2("JSON decoding failed: Non UTF-8 string.",
                       error_code::JSON_PARSE_ERROR);
  }
  ++first;
  return std::make_pair(String::g(s), first);
}
} // namespace

namespace {
template<typename InputIterator>
void checkEmptyDigit(InputIterator first, InputIterator last)
{
  if(std::distance(first, last) == 0) {
    throw DL_ABORT_EX2("JSON decoding failed: zero DIGIT.",
                       error_code::JSON_PARSE_ERROR);
  }
}
} // namespace

namespace {
template<typename InputIterator>
void checkLeadingZero(InputIterator first, InputIterator last)
{
  if(std::distance(first, last) > 2 && *first == '0') {
    throw DL_ABORT_EX2("JSON decoding failed: leading zero.",
                       error_code::JSON_PARSE_ERROR);
  }
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeNumber(InputIterator first, InputIterator last)
{
  std::string s;
  if(*first == '-') {
    s.append(first, first+1);
    ++first;
  }
  InputIterator offset = first;
  while(first != last && in(*first, '0', '9')) {
    ++first;
  }
  checkEof(first, last);
  checkEmptyDigit(offset, first);
  checkLeadingZero(offset, first);
  s.append(offset, first);
  bool fp = false;
  if(*first == '.') {
    fp = true;
    s.append(first, first+1);
    ++first;
    offset = first;
    while(first != last && in(*first, '0', '9')) {
      ++first;
    }
    checkEof(first, last);
    checkEmptyDigit(offset, first);
    s.append(offset, first);
  }
  if(*first == 'e') {
    fp = true;
    s.append(first, first+1);
    ++first;
    checkEof(first, last);
    if(*first == '+' || *first == '-') {
      s.append(first, first+1);
      ++first;
    }
    offset = first;
    while(first != last && in(*first, '0', '9')) {
      ++first;
    }
    checkEof(first, last);
    checkEmptyDigit(offset, first);
    s.append(offset, first);
  }
  if(fp) {
    // Since we don't have floating point coutner part in ValueBase,
    // we just treat it as string.
    return std::make_pair(String::g(s), first);
  } else {
    Integer::ValueType val = util::parseLLInt(s);
    return std::make_pair(Integer::g(val), first);
  }
}
} // namespace

namespace {
void checkDepth(size_t depth)
{
  if(depth >= MAX_STRUCTURE_DEPTH) {
    throw DL_ABORT_EX2("JSON decoding failed: Structure is too deep.",
                       error_code::JSON_PARSE_ERROR);
  }
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeArray(InputIterator first, InputIterator last, size_t depth)
{
  checkDepth(depth);
  SharedHandle<List> list = List::g();
  // Consume first char, assuming it is '['.
  ++first;
  first = skipWs(first, last);
  checkEof(first, last);
  if(*first != ']') {
    while(1) {
      std::pair<SharedHandle<ValueBase>, InputIterator>
        r = decode(first, last, depth);
      list->append(r.first);
      first = r.second;
      first = skipWs(first, last);
      if(first == last || (*first != ',' && *first != ']')) {
        throw DL_ABORT_EX2("JSON decoding failed:"
                           " value-separator ',' or ']' is not found.",
                           error_code::JSON_PARSE_ERROR);
      }
      if(*first == ']') {
        break;
      }
      ++first;
    }
  }
  ++first;
  return std::make_pair(list, first);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decodeObject(InputIterator first, InputIterator last, size_t depth)
{
  checkDepth(depth);
  SharedHandle<Dict> dict = Dict::g();
  // Consume first char, assuming it is '{'
  ++first;
  first = skipWs(first, last);
  checkEof(first, last);
  if(*first != '}') {
    while(1) {
      std::pair<SharedHandle<ValueBase>, InputIterator>
        keyRet = decodeString(first, last);
      first = keyRet.second;
      first = skipWs(first, last);
      if(first == last || *first != ':') {
        throw DL_ABORT_EX2("JSON decoding failed:"
                           " name-separator ':' is not found.",
                           error_code::JSON_PARSE_ERROR);
      }
      ++first;
      std::pair<SharedHandle<ValueBase>, InputIterator>
        valueRet = decode(first, last, depth);
      dict->put(downcast<String>(keyRet.first)->s(), valueRet.first);
      first = valueRet.second;
      first = skipWs(first, last);
      if(first == last || (*first != ',' && *first != '}')) {
        throw DL_ABORT_EX2("JSON decoding failed:"
                           " value-separator ',' or '}' is not found.",
                           error_code::JSON_PARSE_ERROR);
      }
      if(*first == '}') {
        break;
      }
      ++first;
      first = skipWs(first, last);
      checkEof(first, last);
    }
  }
  ++first;
  return std::make_pair(dict, first);
}
} // namespace

namespace {
template<typename InputIterator>
std::pair<SharedHandle<ValueBase>, InputIterator>
decode(InputIterator first, InputIterator last, size_t depth)
{
  first = skipWs(first, last);
  if(first == last) {
    throw DL_ABORT_EX2("JSON decoding failed:"
                       " Unexpected EOF in term context.",
                       error_code::JSON_PARSE_ERROR);
  }
  if(*first == '[') {
    return decodeArray(first, last, depth+1);
  } else if(*first == '{') {
    return decodeObject(first, last, depth+1);
  } else if(*first == '"') {
    return decodeString(first, last);
  } else if(*first == '-' || in(*first, '0', '9')) {
    return decodeNumber(first, last);
  } else if(*first == 't') {
    return decodeTrue(first, last);
  } else if(*first == 'f') {
    return decodeFalse(first, last);
  } else if(*first == 'n') {
    return decodeNull(first, last);
  } else {
    throw DL_ABORT_EX2("JSON decoding failed:"
                       " Unexpected character in term context.",
                       error_code::JSON_PARSE_ERROR);
  }
}
} // namespace

template<typename InputIterator>
SharedHandle<ValueBase> decode(InputIterator first, InputIterator last)
{
  first = skipWs(first, last);
  if(first == last) {
    throw DL_ABORT_EX2("JSON decoding failed:"
                       " Unexpected EOF in term context.",
                       error_code::JSON_PARSE_ERROR);
  }
  std::pair<SharedHandle<ValueBase>, InputIterator> r;
  if(*first == '[') {
    r = decodeArray(first, last, 1);
  } else if(*first == '{') {
    r = decodeObject(first, last, 1);
  } else {
    throw DL_ABORT_EX2("JSON decoding failed:"
                       " Unexpected EOF in term context.",
                       error_code::JSON_PARSE_ERROR);
  }
  return r.first;
}

SharedHandle<ValueBase> decode(const std::string& json)
{
  return decode(json.begin(), json.end());
}

SharedHandle<ValueBase> decode(const unsigned char* json, size_t len)
{
  return decode(json, json+len);
}

std::string jsonEscape(const std::string& s)
{
  std::string t;
  for(std::string::const_iterator i = s.begin(), eoi = s.end(); i != eoi;
      ++i) {
    if(*i == '"' || *i == '\\' || *i == '/') {
      t += "\\";
      t += *i;
    } else if(*i == '\b') {
      t += "\\b";
    } else if(*i == '\f') {
      t += "\\f";
    } else if(*i == '\n') {
      t += "\\n";
    } else if(*i == '\r') {
      t += "\\r";
    } else if(*i == '\t') {
      t += "\\t";
    } else if(in(static_cast<unsigned char>(*i), 0x00u, 0x1Fu)) {
      t += "\\u00";
      char temp[3];
      temp[2] = '\0';
      temp[0] = (*i >> 4);
      temp[1] = (*i)&0x0Fu;
      for(int j = 0; j < 2; ++j) {
        if(temp[j] < 10) {
          temp[j] += '0';
        } else {
          temp[j] += 'A'-10;
        }
      }
      t += temp;
    } else {
      t.append(i, i+1);
    }
  }
  return t;
}

// Serializes JSON object or array.
std::string encode(const SharedHandle<ValueBase>& json)
{
  std::ostringstream out;
  return encode(out, json.get()).str();
}

JsonGetParam::JsonGetParam
(const std::string& request, const std::string& callback)
  : request(request), callback(callback)
{}

JsonGetParam
decodeGetParams(const std::string& query)
{
  std::string jsonRequest;
  std::string callback;
  if(!query.empty() && query[0] == '?') {
    Scip method;
    Scip id;
    Scip params;
    std::vector<Scip> getParams;
    util::splitIter(query.begin()+1, query.end(), std::back_inserter(getParams),
                    '&');
    for(std::vector<Scip>::const_iterator i =
          getParams.begin(), eoi = getParams.end(); i != eoi; ++i) {
      if(util::startsWith((*i).first, (*i).second, "method=")) {
        method.first = (*i).first+7;
        method.second = (*i).second;
      } else if(util::startsWith((*i).first, (*i).second, "id=")) {
        id.first = (*i).first+3;
        id.second = (*i).second;
      } else if(util::startsWith((*i).first, (*i).second, "params=")) {
        params.first = (*i).first+7;
        params.second = (*i).second;
      } else if(util::startsWith((*i).first, (*i).second, "jsoncallback=")) {
        callback.assign((*i).first+13, (*i).second);
      }
    }
    std::string decparam = util::percentDecode(params.first, params.second);
    std::string jsonParam = base64::decode(decparam.begin(), decparam.end());
    if(method.first == method.second && id.first == id.second) {
      // Assume batch call.
      jsonRequest = jsonParam;
    } else {
      jsonRequest = "{";
      if(method.first != method.second) {
        jsonRequest += "\"method\":\"";
        jsonRequest.append(method.first, method.second);
        jsonRequest += "\"";
      }
      if(id.first != id.second) {
        jsonRequest += ",\"id\":\"";
        jsonRequest.append(id.first, id.second);
        jsonRequest += "\"";
      }
      if(params.first != params.second) {
        jsonRequest += ",\"params\":";
        jsonRequest += jsonParam;
      }
      jsonRequest += "}";
    }
  }
  return JsonGetParam(jsonRequest, callback);
}

} // namespace json

} // namespace aria2
