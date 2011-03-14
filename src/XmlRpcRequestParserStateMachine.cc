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
#include "XmlRpcRequestParserStateMachine.h"

namespace aria2 {

namespace rpc {

InitialXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::initialState_ =
  new InitialXmlRpcRequestParserState();

UnknownElementXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::unknownElementState_ =
  new UnknownElementXmlRpcRequestParserState();

MethodCallXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::methodCallState_ =
  new MethodCallXmlRpcRequestParserState();

MethodNameXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::methodNameState_ =
  new MethodNameXmlRpcRequestParserState();

ParamsXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::paramsState_ =
  new ParamsXmlRpcRequestParserState();

ParamXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::paramState_ =
  new ParamXmlRpcRequestParserState();

ValueXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::valueState_ =
  new ValueXmlRpcRequestParserState();

IntXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::intState_ =
  new IntXmlRpcRequestParserState();

StringXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::stringState_ =
  new StringXmlRpcRequestParserState();

Base64XmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::base64State_ =
  new Base64XmlRpcRequestParserState();

StructXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::structState_ =
  new StructXmlRpcRequestParserState();

MemberXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::memberState_ =
  new MemberXmlRpcRequestParserState();

NameXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::nameState_ =
  new NameXmlRpcRequestParserState();

ArrayXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::arrayState_ = 
  new ArrayXmlRpcRequestParserState();

DataXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::dataState_ =
  new DataXmlRpcRequestParserState();

ArrayValueXmlRpcRequestParserState*
XmlRpcRequestParserStateMachine::arrayValueState_ =
  new ArrayValueXmlRpcRequestParserState();

XmlRpcRequestParserStateMachine::XmlRpcRequestParserStateMachine():
  controller_(new XmlRpcRequestParserController())
{
  stateStack_.push(initialState_);
}

XmlRpcRequestParserStateMachine::~XmlRpcRequestParserStateMachine()
{
  delete controller_;
}

} // namespace rpc

} // namespace aria2
