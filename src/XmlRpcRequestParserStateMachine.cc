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
#include "XmlRpcRequestParserStateMachine.h"
#include "XmlRpcRequestParserController.h"
#include "XmlRpcRequestParserStateImpl.h"

namespace aria2 {

namespace rpc {

namespace {
auto initialState = new InitialXmlRpcRequestParserState();

auto unknownElementState = new UnknownElementXmlRpcRequestParserState();

auto methodCallState = new MethodCallXmlRpcRequestParserState();

auto methodNameState = new MethodNameXmlRpcRequestParserState();

auto paramsState = new ParamsXmlRpcRequestParserState();

auto paramState = new ParamXmlRpcRequestParserState();

auto valueState = new ValueXmlRpcRequestParserState();

auto intState = new IntXmlRpcRequestParserState();

auto stringState = new StringXmlRpcRequestParserState();

auto base64State = new Base64XmlRpcRequestParserState();

auto structState = new StructXmlRpcRequestParserState();

auto memberState = new MemberXmlRpcRequestParserState();

auto nameState = new NameXmlRpcRequestParserState();

auto arrayState = new ArrayXmlRpcRequestParserState();

auto dataState = new DataXmlRpcRequestParserState();

auto arrayValueState = new ArrayValueXmlRpcRequestParserState();
} // namespace

XmlRpcRequestParserStateMachine::XmlRpcRequestParserStateMachine()
    : controller_(new XmlRpcRequestParserController())
{
  stateStack_.push(initialState);
}

XmlRpcRequestParserStateMachine::~XmlRpcRequestParserStateMachine()
{
  delete controller_;
}

void XmlRpcRequestParserStateMachine::reset()
{
  controller_->reset();
  while (!stateStack_.empty()) {
    stateStack_.pop();
  }
  stateStack_.push(initialState);
}

bool XmlRpcRequestParserStateMachine::needsCharactersBuffering() const
{
  return stateStack_.top()->needsCharactersBuffering();
}

bool XmlRpcRequestParserStateMachine::finished() const
{
  return stateStack_.top() == initialState;
}

void XmlRpcRequestParserStateMachine::beginElement(
    const char* localname, const char* prefix, const char* nsUri,
    const std::vector<XmlAttr>& attrs)
{
  stateStack_.top()->beginElement(this, localname, attrs);
}

void XmlRpcRequestParserStateMachine::endElement(const char* localname,
                                                 const char* prefix,
                                                 const char* nsUri,
                                                 std::string characters)
{
  stateStack_.top()->endElement(this, localname, std::move(characters));
  stateStack_.pop();
}

void XmlRpcRequestParserStateMachine::setMethodName(std::string methodName)
{
  controller_->setMethodName(std::move(methodName));
}

const std::string& XmlRpcRequestParserStateMachine::getMethodName() const
{
  return controller_->getMethodName();
}

void XmlRpcRequestParserStateMachine::popArrayFrame()
{
  controller_->popArrayFrame();
}

void XmlRpcRequestParserStateMachine::popStructFrame()
{
  controller_->popStructFrame();
}

void XmlRpcRequestParserStateMachine::pushFrame() { controller_->pushFrame(); }

void XmlRpcRequestParserStateMachine::setCurrentFrameValue(
    std::unique_ptr<ValueBase> value)
{
  controller_->setCurrentFrameValue(std::move(value));
}

const std::unique_ptr<ValueBase>&
XmlRpcRequestParserStateMachine::getCurrentFrameValue() const
{
  return controller_->getCurrentFrameValue();
}

std::unique_ptr<ValueBase>
XmlRpcRequestParserStateMachine::popCurrentFrameValue()
{
  return controller_->popCurrentFrameValue();
}

void XmlRpcRequestParserStateMachine::setCurrentFrameName(std::string name)
{
  controller_->setCurrentFrameName(std::move(name));
}

void XmlRpcRequestParserStateMachine::pushUnknownElementState()
{
  stateStack_.push(unknownElementState);
}

void XmlRpcRequestParserStateMachine::pushMethodCallState()
{
  stateStack_.push(methodCallState);
}

void XmlRpcRequestParserStateMachine::pushMethodNameState()
{
  stateStack_.push(methodNameState);
}

void XmlRpcRequestParserStateMachine::pushParamsState()
{
  stateStack_.push(paramsState);
}

void XmlRpcRequestParserStateMachine::pushParamState()
{
  stateStack_.push(paramState);
}

void XmlRpcRequestParserStateMachine::pushValueState()
{
  stateStack_.push(valueState);
}

void XmlRpcRequestParserStateMachine::pushIntState()
{
  stateStack_.push(intState);
}

void XmlRpcRequestParserStateMachine::pushStringState()
{
  stateStack_.push(stringState);
}

void XmlRpcRequestParserStateMachine::pushBase64State()
{
  stateStack_.push(base64State);
}

void XmlRpcRequestParserStateMachine::pushStructState()
{
  stateStack_.push(structState);
}

void XmlRpcRequestParserStateMachine::pushMemberState()
{
  stateStack_.push(memberState);
}

void XmlRpcRequestParserStateMachine::pushNameState()
{
  stateStack_.push(nameState);
}

void XmlRpcRequestParserStateMachine::pushArrayState()
{
  stateStack_.push(arrayState);
}

void XmlRpcRequestParserStateMachine::pushDataState()
{
  stateStack_.push(dataState);
}

void XmlRpcRequestParserStateMachine::pushArrayValueState()
{
  stateStack_.push(arrayValueState);
}

} // namespace rpc

} // namespace aria2
