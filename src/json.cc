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
#include "a2functional.h"
#include "util.h"
#include "base64.h"

namespace aria2 {

namespace json {

std::string jsonEscape(const std::string& s)
{
  std::string t;
  for (std::string::const_iterator i = s.begin(), eoi = s.end(); i != eoi;
       ++i) {
    if (*i == '"' || *i == '\\' || *i == '/') {
      t += "\\";
      t += *i;
    }
    else if (*i == '\b') {
      t += "\\b";
    }
    else if (*i == '\f') {
      t += "\\f";
    }
    else if (*i == '\n') {
      t += "\\n";
    }
    else if (*i == '\r') {
      t += "\\r";
    }
    else if (*i == '\t') {
      t += "\\t";
    }
    else if (in(static_cast<unsigned char>(*i), 0x00u, 0x1Fu)) {
      t += "\\u00";
      char temp[3];
      temp[2] = '\0';
      temp[0] = (*i >> 4);
      temp[1] = (*i) & 0x0Fu;
      for (int j = 0; j < 2; ++j) {
        if (temp[j] < 10) {
          temp[j] += '0';
        }
        else {
          temp[j] += 'A' - 10;
        }
      }
      t += temp;
    }
    else {
      t.append(i, i + 1);
    }
  }
  return t;
}

// Serializes JSON object or array.
std::string encode(const ValueBase* json)
{
  std::ostringstream out;
  return encode(out, json).str();
}

JsonGetParam::JsonGetParam(const std::string& request,
                           const std::string& callback)
    : request(request), callback(callback)
{
}

JsonGetParam decodeGetParams(const std::string& query)
{
  std::string jsonRequest;
  std::string callback;
  if (query.empty() || query[0] != '?') {
    return JsonGetParam(jsonRequest, callback);
  }

  Scip method = std::make_pair(query.end(), query.end());
  Scip id = std::make_pair(query.end(), query.end());
  Scip params = std::make_pair(query.end(), query.end());
  std::vector<Scip> getParams;
  util::splitIter(query.begin() + 1, query.end(), std::back_inserter(getParams),
                  '&');
  for (const auto& i : getParams) {
    if (util::startsWith(i.first, i.second, "method=")) {
      method.first = i.first + 7;
      method.second = i.second;
    }
    else if (util::startsWith(i.first, i.second, "id=")) {
      id.first = i.first + 3;
      id.second = i.second;
    }
    else if (util::startsWith(i.first, i.second, "params=")) {
      params.first = i.first + 7;
      params.second = i.second;
    }
    else if (util::startsWith(i.first, i.second, "jsoncallback=")) {
      callback.assign(i.first + 13, i.second);
    }
  }
  std::string decparam = util::percentDecode(params.first, params.second);
  std::string jsonParam = base64::decode(decparam.begin(), decparam.end());
  if (method.first == method.second && id.first == id.second) {
    // Assume batch call.
    jsonRequest = jsonParam;
  }
  else {
    jsonRequest = "{";
    if (method.first != method.second) {
      jsonRequest += "\"method\":\"";
      jsonRequest.append(method.first, method.second);
      jsonRequest += "\"";
    }
    if (id.first != id.second) {
      jsonRequest += ",\"id\":\"";
      jsonRequest.append(id.first, id.second);
      jsonRequest += "\"";
    }
    if (params.first != params.second) {
      jsonRequest += ",\"params\":";
      jsonRequest += jsonParam;
    }
    jsonRequest += "}";
  }
  return JsonGetParam(jsonRequest, callback);
}

} // namespace json

} // namespace aria2
