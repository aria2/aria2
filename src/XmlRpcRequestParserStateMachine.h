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
#ifndef D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H
#define D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H

#include "common.h"

#include <string>
#include <map>
#include <stack>

#include "XmlRpcRequestParserController.h"
#include "XmlRpcRequestParserStateImpl.h"
#include "ValueBase.h"

namespace aria2 {

namespace xmlrpc {

class XmlRpcRequestParserStateMachine {
private:
  XmlRpcRequestParserController* controller_;

  std::stack<XmlRpcRequestParserState*> stateStack_;

  static InitialXmlRpcRequestParserState* initialState_;
  static MethodCallXmlRpcRequestParserState* methodCallState_;
  static MethodNameXmlRpcRequestParserState* methodNameState_;
  static ParamsXmlRpcRequestParserState* paramsState_;
  static ParamXmlRpcRequestParserState* paramState_;
  static ValueXmlRpcRequestParserState* valueState_;
  static IntXmlRpcRequestParserState* intState_;
  static StringXmlRpcRequestParserState* stringState_;
  static Base64XmlRpcRequestParserState* base64State_;
  static StructXmlRpcRequestParserState* structState_;
  static MemberXmlRpcRequestParserState* memberState_;
  static NameXmlRpcRequestParserState* nameState_;
  static ArrayXmlRpcRequestParserState* arrayState_;
  static DataXmlRpcRequestParserState* dataState_;
  static ArrayValueXmlRpcRequestParserState* arrayValueState_;
  
  static UnknownElementXmlRpcRequestParserState* unknownElementState_;
public:
  XmlRpcRequestParserStateMachine();

  ~XmlRpcRequestParserStateMachine();

  void beginElement(const std::string& name,
                    const std::map<std::string, std::string>& attrs)
  {
    stateStack_.top()->beginElement(this, name, attrs);
  }
  
  void endElement(const std::string& name, const std::string& characters)
  {
    stateStack_.top()->endElement(this, name, characters);
    stateStack_.pop();
  }

  void setMethodName(const std::string& methodName)
  {
    controller_->setMethodName(methodName);
  }

  const std::string& getMethodName() const
  {
    return controller_->getMethodName();
  }

  void popArrayFrame()
  {
    controller_->popArrayFrame();
  }

  void popStructFrame()
  {
    controller_->popStructFrame();
  }

  void pushFrame()
  {
    controller_->pushFrame();
  }

  void setCurrentFrameValue(const SharedHandle<ValueBase>& value)
  {
    controller_->setCurrentFrameValue(value);
  }

  const SharedHandle<ValueBase>& getCurrentFrameValue() const
  {
    return controller_->getCurrentFrameValue();
  }

  void setCurrentFrameName(const std::string& name)
  {
    controller_->setCurrentFrameName(name);
  }

  bool needsCharactersBuffering() const
  {
    return stateStack_.top()->needsCharactersBuffering();
  }

  void pushUnknownElementState() { stateStack_.push(unknownElementState_); }

  void pushMethodCallState() { stateStack_.push(methodCallState_); }

  void pushMethodNameState() { stateStack_.push(methodNameState_); }

  void pushParamsState() { stateStack_.push(paramsState_); }

  void pushParamState() { stateStack_.push(paramState_); }

  void pushValueState() { stateStack_.push(valueState_); }

  void pushIntState() { stateStack_.push(intState_); }

  void pushStringState() { stateStack_.push(stringState_); }

  void pushBase64State() { stateStack_.push(base64State_); }

  void pushStructState() { stateStack_.push(structState_); }

  void pushMemberState() { stateStack_.push(memberState_); }

  void pushNameState() { stateStack_.push(nameState_); }

  void pushArrayState() { stateStack_.push(arrayState_); }

  void pushDataState() { stateStack_.push(dataState_); }

  void pushArrayValueState() { stateStack_.push(arrayValueState_); }
};

} // namespace xmlrpc

} // namespace aria2

#endif // D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H
