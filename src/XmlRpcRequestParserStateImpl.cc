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
#include "XmlRpcRequestParserStateMachine.h"
#include "XmlRpcElements.h"
#include "RecoverableException.h"
#include "util.h"
#include "Base64.h"

namespace aria2 {

namespace rpc {

// InitialXmlRpcRequestParserState

void InitialXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::METHOD_CALL) {
    stm->pushMethodCallState();
  } else {
    stm->pushUnknownElementState();
  }  
}
  
void InitialXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{}

// UnknownElementXmlRpcRequestParserState

void UnknownElementXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->pushUnknownElementState();
}

// MethodCallXmlRpcRequestParserState

void MethodCallXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::METHOD_NAME) {
    stm->pushMethodNameState();
  } else if(name == elements::A2_PARAMS) {
    stm->setCurrentFrameValue(List::g());
    stm->pushParamsState();
  } else {
    stm->pushUnknownElementState();
  }  
}
  
// MethodNameXmlRpcRequestParserState

void MethodNameXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->pushUnknownElementState();
}
  
void MethodNameXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setMethodName(characters);
}

// ParamsXmlRpcRequestParserState

void ParamsXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::PARAM) {
    stm->pushFrame();
    stm->pushParamState();
  } else {
    stm->pushUnknownElementState();
  }
}
  
// ParamXmlRpcRequestParserState

void ParamXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::VALUE) {
    stm->pushValueState();
  } else {
    stm->pushUnknownElementState();
  }
}

void ParamXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->popArrayFrame();
}
 
// ValueXmlRpcRequestParserState

void ValueXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::I4 || name == elements::INT) {
    stm->pushIntState();
  } else if(name == elements::STRUCT) {
    stm->setCurrentFrameValue(Dict::g());
    stm->pushStructState();
  } else if(name == elements::ARRAY) {
    stm->setCurrentFrameValue(List::g());
    stm->pushArrayState();
  } else if(name == elements::STRING || name == elements::DOUBLE) {
    stm->pushStringState();
  } else if(name == elements::BASE64) {
    stm->pushBase64State();
  } else {
    stm->pushUnknownElementState();
  }
}

void ValueXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  // XML-RPC specification says that if no data type tag is used, the
  // data must be treated as string.  To prevent from overwriting
  // current frame value, we first check it is still null.
  if(!stm->getCurrentFrameValue() && !characters.empty()) {
    stm->setCurrentFrameValue(String::g(characters));
  }
}

// IntXmlRpcRequestParserState

void IntXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->pushUnknownElementState();
}
  
void IntXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  try {
    int64_t value = util::parseLLInt(characters.begin(), characters.end());
    stm->setCurrentFrameValue(Integer::g(value));
  } catch(RecoverableException& e) {
    // nothing to do here: We just leave current frame value to null.
  }
}

// StringXmlRpcRequestParserState

void StringXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->pushUnknownElementState();
}
  
void StringXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setCurrentFrameValue(String::g(characters));
}

// Base64XmlRpcRequestParserState

void Base64XmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->pushUnknownElementState();
}
  
void Base64XmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setCurrentFrameValue(String::g(Base64::decode(characters)));
}

// StructXmlRpcRequestParserState

void StructXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::MEMBER) {
    stm->pushFrame();
    stm->pushMemberState();
  } else {
    stm->pushUnknownElementState();
  }
}

// MemberXmlRpcRequestParserState

void MemberXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::NAME) {
    stm->pushNameState();
  } else if(name == elements::VALUE) {
    stm->pushValueState();
  } else {
    stm->pushUnknownElementState();
  }
}

void MemberXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->popStructFrame();
}

// NameXmlRpcRequestParserState

void NameXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->pushUnknownElementState();
}

void NameXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setCurrentFrameName(characters);
}

// ArrayXmlRpcRequestParserState

void ArrayXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::DATA) {
    stm->pushDataState();
  } else {
    stm->pushUnknownElementState();
  }
}

// DataXmlRpcRequestParserState

void DataXmlRpcRequestParserState::beginElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == elements::VALUE) {
    stm->pushFrame();
    stm->pushArrayValueState();
  } else {
    stm->pushUnknownElementState();
  }
}

// ArrayValueXmlRpcRequestParserState

void ArrayValueXmlRpcRequestParserState::endElement
(XmlRpcRequestParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  ValueXmlRpcRequestParserState::endElement(stm, name, characters);
  stm->popArrayFrame();
}

} // namespace rpc

} // namespace aria2
