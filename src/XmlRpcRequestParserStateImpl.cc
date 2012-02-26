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
#include "XmlRpcRequestParserStateImpl.h"

#include <cstring>

#include "XmlRpcRequestParserStateMachine.h"
#include "RecoverableException.h"
#include "util.h"
#include "base64.h"
#include "ValueBase.h"

namespace aria2 {

namespace rpc {

// InitialXmlRpcRequestParserState

void InitialXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "methodCall") == 0) {
    psm->pushMethodCallState();
  } else {
    psm->pushUnknownElementState();
  }  
}
  
void InitialXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{}

// UnknownElementXmlRpcRequestParserState

void UnknownElementXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  psm->pushUnknownElementState();
}

// MethodCallXmlRpcRequestParserState

void MethodCallXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "methodName") == 0) {
    psm->pushMethodNameState();
  } else if(strcmp(name, "params") == 0) {
    psm->setCurrentFrameValue(List::g());
    psm->pushParamsState();
  } else {
    psm->pushUnknownElementState();
  }  
}
  
// MethodNameXmlRpcRequestParserState

void MethodNameXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  psm->pushUnknownElementState();
}
  
void MethodNameXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  psm->setMethodName(characters);
}

// ParamsXmlRpcRequestParserState

void ParamsXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "param") == 0) {
    psm->pushFrame();
    psm->pushParamState();
  } else {
    psm->pushUnknownElementState();
  }
}
  
// ParamXmlRpcRequestParserState

void ParamXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "value") == 0) {
    psm->pushValueState();
  } else {
    psm->pushUnknownElementState();
  }
}

void ParamXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  psm->popArrayFrame();
}
 
// ValueXmlRpcRequestParserState

void ValueXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "i4") == 0 || strcmp(name, "int") == 0) {
    psm->pushIntState();
  } else if(strcmp(name, "struct") == 0) {
    psm->setCurrentFrameValue(Dict::g());
    psm->pushStructState();
  } else if(strcmp(name, "array") == 0) {
    psm->setCurrentFrameValue(List::g());
    psm->pushArrayState();
  } else if(strcmp(name, "string") == 0 || strcmp(name, "double") == 0) {
    psm->pushStringState();
  } else if(strcmp(name, "base64") == 0) {
    psm->pushBase64State();
  } else {
    psm->pushUnknownElementState();
  }
}

void ValueXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  // XML-RPC specification says that if no data type tag is used, the
  // data must be treated as string.  To prevent from overwriting
  // current frame value, we first check it is still null.
  if(!psm->getCurrentFrameValue() && !characters.empty()) {
    psm->setCurrentFrameValue(String::g(characters));
  }
}

// IntXmlRpcRequestParserState

void IntXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  psm->pushUnknownElementState();
}
  
void IntXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  int32_t value;
  if(util::parseIntNoThrow(value, characters)) {
    psm->setCurrentFrameValue(Integer::g(value));
  } else {
    // nothing to do here: We just leave current frame value to null.
  }
}

// StringXmlRpcRequestParserState

void StringXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  psm->pushUnknownElementState();
}
  
void StringXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  psm->setCurrentFrameValue(String::g(characters));
}

// Base64XmlRpcRequestParserState

void Base64XmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  psm->pushUnknownElementState();
}
  
void Base64XmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  psm->setCurrentFrameValue
    (String::g(base64::decode(characters.begin(), characters.end())));
}

// StructXmlRpcRequestParserState

void StructXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "member") == 0) {
    psm->pushFrame();
    psm->pushMemberState();
  } else {
    psm->pushUnknownElementState();
  }
}

// MemberXmlRpcRequestParserState

void MemberXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "name") == 0) {
    psm->pushNameState();
  } else if(strcmp(name, "value") == 0) {
    psm->pushValueState();
  } else {
    psm->pushUnknownElementState();
  }
}

void MemberXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  psm->popStructFrame();
}

// NameXmlRpcRequestParserState

void NameXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  psm->pushUnknownElementState();
}

void NameXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  psm->setCurrentFrameName(characters);
}

// ArrayXmlRpcRequestParserState

void ArrayXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "data") == 0) {
    psm->pushDataState();
  } else {
    psm->pushUnknownElementState();
  }
}

// DataXmlRpcRequestParserState

void DataXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::vector<XmlAttr>& attrs)
{
  if(strcmp(name, "value") == 0) {
    psm->pushFrame();
    psm->pushArrayValueState();
  } else {
    psm->pushUnknownElementState();
  }
}

// ArrayValueXmlRpcRequestParserState

void ArrayValueXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* psm,
 const char* name,
 const std::string& characters)
{
  ValueXmlRpcRequestParserState::endElement(psm, name, characters);
  psm->popArrayFrame();
}

} // namespace rpc

} // namespace aria2
