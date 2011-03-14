/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "ExpatXmlRpcRequestProcessor.h"

#include <stack>

#include <expat.h>

#include "XmlRpcRequestParserStateMachine.h"
#include "util.h"
#include "DlAbortEx.h"
#include "message.h"

namespace aria2 {

namespace rpc {

namespace {
struct SessionData {
  XmlRpcRequestParserStateMachine* stm_;

  std::stack<std::string> charactersStack_;

  SessionData(XmlRpcRequestParserStateMachine* stm):stm_(stm) {}
};
} // namespace

namespace {
void mlStartElement(void* userData, const char* name, const char** attrs)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);

  std::map<std::string, std::string> attrmap;
  if(attrs) {
    const char** p = attrs;
    while(*p != 0) {
      std::string name = *p++;
      if(*p == 0) {
        break;
      }
      std::string value = util::strip(*p++);
      attrmap[name] = value;
    }
  }
  sd->stm_->beginElement(name, attrmap);
  if(sd->stm_->needsCharactersBuffering()) {
    sd->charactersStack_.push(std::string());
  }
}
} // namespace

namespace {
void mlEndElement(void* userData, const char* name)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::string characters;
  if(sd->stm_->needsCharactersBuffering()) {
    characters = util::strip(sd->charactersStack_.top());
    sd->charactersStack_.pop();
  }
  sd->stm_->endElement(name, characters);
}
} // namespace

namespace {
void mlCharacters(void* userData, const char* ch, int len)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  if(sd->stm_->needsCharactersBuffering()) {
    sd->charactersStack_.top() += std::string(&ch[0], &ch[len]);
  }
}
} // namespace

RpcRequest
XmlRpcRequestProcessor::parseMemory(const std::string& xml)
{
  stm_.reset(new XmlRpcRequestParserStateMachine());
  SharedHandle<SessionData> sessionData(new SessionData(stm_.get()));

  XML_Parser parser = XML_ParserCreate(0);

  XML_SetUserData(parser, sessionData.get());
  XML_SetElementHandler(parser, &mlStartElement, &mlEndElement);
  XML_SetCharacterDataHandler(parser, &mlCharacters);

  int r = XML_Parse(parser, xml.data(), xml.size(), 1);
  XML_ParserFree(parser);

  if(r == XML_STATUS_ERROR) {
    throw DL_ABORT_EX(MSG_CANNOT_PARSE_XML_RPC_REQUEST);
  }
  if(!asList(stm_->getCurrentFrameValue())) {
    throw DL_ABORT_EX("Bad XML-RPC parameter list");
  }
  return RpcRequest(stm_->getMethodName(),
                    static_pointer_cast<List>(stm_->getCurrentFrameValue()));
}

} // namespace rpc

} // namespace aria2
