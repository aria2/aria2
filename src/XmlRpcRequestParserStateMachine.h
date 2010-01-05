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
#ifndef _D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H_
#define _D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H_

#include "common.h"

#include <string>
#include <map>
#include <stack>

#include "BDE.h"
#include "XmlRpcRequestParserController.h"
#include "XmlRpcRequestParserStateImpl.h"

namespace aria2 {

class BDE;

namespace xmlrpc {

class XmlRpcRequestParserStateMachine {
private:
  XmlRpcRequestParserController* _controller;

  std::stack<XmlRpcRequestParserState*> _stateStack;

  static InitialXmlRpcRequestParserState* _initialState;
  static MethodCallXmlRpcRequestParserState* _methodCallState;
  static MethodNameXmlRpcRequestParserState* _methodNameState;
  static ParamsXmlRpcRequestParserState* _paramsState;
  static ParamXmlRpcRequestParserState* _paramState;
  static ValueXmlRpcRequestParserState* _valueState;
  static IntXmlRpcRequestParserState* _intState;
  static StringXmlRpcRequestParserState* _stringState;
  static Base64XmlRpcRequestParserState* _base64State;
  static StructXmlRpcRequestParserState* _structState;
  static MemberXmlRpcRequestParserState* _memberState;
  static NameXmlRpcRequestParserState* _nameState;
  static ArrayXmlRpcRequestParserState* _arrayState;
  static DataXmlRpcRequestParserState* _dataState;
  static ArrayValueXmlRpcRequestParserState* _arrayValueState;
  
  static UnknownElementXmlRpcRequestParserState* _unknownElementState;
public:
  XmlRpcRequestParserStateMachine();

  ~XmlRpcRequestParserStateMachine();

  void beginElement(const std::string& name,
                    const std::map<std::string, std::string>& attrs)
  {
    _stateStack.top()->beginElement(this, name, attrs);
  }
  
  void endElement(const std::string& name, const std::string& characters)
  {
    _stateStack.top()->endElement(this, name, characters);
    _stateStack.pop();
  }

  void setMethodName(const std::string& methodName)
  {
    _controller->setMethodName(methodName);
  }

  const std::string& getMethodName() const
  {
    return _controller->getMethodName();
  }

  void popArrayFrame()
  {
    _controller->popArrayFrame();
  }

  void popStructFrame()
  {
    _controller->popStructFrame();
  }

  void pushFrame()
  {
    _controller->pushFrame();
  }

  void setCurrentFrameValue(const BDE& value)
  {
    _controller->setCurrentFrameValue(value);
  }

  const BDE& getCurrentFrameValue() const
  {
    return _controller->getCurrentFrameValue();
  }

  void setCurrentFrameName(const std::string& name)
  {
    _controller->setCurrentFrameName(name);
  }

  bool needsCharactersBuffering() const
  {
    return _stateStack.top()->needsCharactersBuffering();
  }

  void pushUnknownElementState() { _stateStack.push(_unknownElementState); }

  void pushMethodCallState() { _stateStack.push(_methodCallState); }

  void pushMethodNameState() { _stateStack.push(_methodNameState); }

  void pushParamsState() { _stateStack.push(_paramsState); }

  void pushParamState() { _stateStack.push(_paramState); }

  void pushValueState() { _stateStack.push(_valueState); }

  void pushIntState() { _stateStack.push(_intState); }

  void pushStringState() { _stateStack.push(_stringState); }

  void pushBase64State() { _stateStack.push(_base64State); }

  void pushStructState() { _stateStack.push(_structState); }

  void pushMemberState() { _stateStack.push(_memberState); }

  void pushNameState() { _stateStack.push(_nameState); }

  void pushArrayState() { _stateStack.push(_arrayState); }

  void pushDataState() { _stateStack.push(_dataState); }

  void pushArrayValueState() { _stateStack.push(_arrayValueState); }
};

} // namespace xmlrpc

} // namespace aria2

#endif // _D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H_
