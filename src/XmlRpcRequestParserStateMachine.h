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
#ifndef D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H
#define D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H

#include "ParserStateMachine.h"

#include <string>
#include <stack>
#include <memory>

namespace aria2 {

class ValueBase;

namespace rpc {

class XmlRpcRequestParserController;
class XmlRpcRequestParserState;

class XmlRpcRequestParserStateMachine : public ParserStateMachine {
public:
  XmlRpcRequestParserStateMachine();
  virtual ~XmlRpcRequestParserStateMachine();

  virtual bool needsCharactersBuffering() const CXX11_OVERRIDE;
  virtual bool finished() const CXX11_OVERRIDE;

  virtual void beginElement(const char* localname, const char* prefix,
                            const char* nsUri,
                            const std::vector<XmlAttr>& attrs) CXX11_OVERRIDE;

  virtual void endElement(const char* localname, const char* prefix,
                          const char* nsUri,
                          std::string characters) CXX11_OVERRIDE;

  virtual void reset() CXX11_OVERRIDE;

  void setMethodName(std::string methodName);
  const std::string& getMethodName() const;
  void popArrayFrame();
  void popStructFrame();
  void pushFrame();
  void setCurrentFrameValue(std::unique_ptr<ValueBase> value);
  const std::unique_ptr<ValueBase>& getCurrentFrameValue() const;
  std::unique_ptr<ValueBase> popCurrentFrameValue();
  void setCurrentFrameName(std::string name);

  void pushUnknownElementState();
  void pushMethodCallState();
  void pushMethodNameState();
  void pushParamsState();
  void pushParamState();
  void pushValueState();
  void pushIntState();
  void pushStringState();
  void pushBase64State();
  void pushStructState();
  void pushMemberState();
  void pushNameState();
  void pushArrayState();
  void pushDataState();
  void pushArrayValueState();

private:
  std::stack<XmlRpcRequestParserState*> stateStack_;
  XmlRpcRequestParserController* controller_;
};

} // namespace rpc

} // namespace aria2

#endif // D_XML_RPC_REQUEST_PARSER_STATE_MACHINE_H
