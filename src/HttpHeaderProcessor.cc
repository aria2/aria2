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
#include "HttpHeaderProcessor.h"
#include "message.h"
#include "Util.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"

void HttpHeaderProcessor::update(const char* data, int32_t length)
{
  checkHeaderLimit(length);
  strm.write(data, length);
}

void HttpHeaderProcessor::update(const string& data)
{
  checkHeaderLimit(data.size());
  strm << data;
}

void HttpHeaderProcessor::checkHeaderLimit(int32_t incomingLength)
{
  strm.seekg(0, ios::end);
  if((int32_t)strm.tellg()+incomingLength > _limit) {
    throw new DlAbortEx("Too large http header");
  }
}

bool HttpHeaderProcessor::eoh() const
{
  string str = strm.str();
  if(str.find("\r\n\r\n") == string::npos && str.find("\n\n") == string::npos) {
    return false;
  } else {
    return true;
  }
}

int32_t HttpHeaderProcessor::getPutBackDataLength() const
{
  string str = strm.str();
  string::size_type delimpos = string::npos;
  if((delimpos = str.find("\r\n\r\n")) != string::npos) {
    return str.size()-(delimpos+4);
  } else if((delimpos = str.find("\n\n")) != string::npos) {
    return str.size()-(delimpos+2);
  } else {
    return 0;
  }
}

void HttpHeaderProcessor::clear()
{
  strm.str("");
}

pair<string, HttpHeaderHandle> HttpHeaderProcessor::getHttpStatusHeader()
{
  strm.seekg(0, ios::beg);
  string line;
  getline(strm, line);  
  // check HTTP status value
  if(line.size() <= 12) {
    throw new DlRetryEx(EX_NO_STATUS_HEADER);
  }
  string status = line.substr(9, 3);
  HttpHeaderHandle httpHeader = new HttpHeader();
  while(getline(strm, line)) {
    line = Util::trim(line);
    if(line.empty()) {
      break;
    }
    pair<string, string> hp;
    Util::split(hp, line, ':');
    httpHeader->put(hp.first, hp.second);
  }
  
  return pair<string, HttpHeaderHandle>(status, httpHeader);
}

string HttpHeaderProcessor::getHeaderString() const
{
  string str = strm.str();
  string::size_type delimpos = string::npos;
  if((delimpos = str.find("\r\n\r\n")) != string::npos ||
     (delimpos = str.find("\n\n")) != string::npos) {
    return str.substr(0, delimpos);
  } else {
    return str;
  }
}

