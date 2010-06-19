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
#ifndef _D_XML_RPC_REQUEST_PARSER_CONTROLLER_H_
#define _D_XML_RPC_REQUEST_PARSER_CONTROLLER_H_

#include "common.h"

#include <stack>
#include <string>

#include "ValueBase.h"

namespace aria2 {

namespace xmlrpc {

class XmlRpcRequestParserController {
private:

  struct StateFrame {
    SharedHandle<ValueBase> _value;
    std::string _name;

    bool validMember() const
    {
      return !_value.isNull() && !_name.empty();
    }
  };

  std::stack<StateFrame> _frameStack;

  StateFrame _currentFrame;

  std::string _methodName;
public:
  void pushFrame();

  // Pops StateFrame p from _frameStack and set p[_currentFrame._name]
  // = _currentFrame._value and _currentFrame = p;
  void popStructFrame();

  // Pops StateFrame p from _frameStack and add _currentFrame._value
  // to p and _currentFrame = p;
  void popArrayFrame();
  
  void setCurrentFrameValue(const SharedHandle<ValueBase>& value);

  void setCurrentFrameName(const std::string& name);

  const SharedHandle<ValueBase>& getCurrentFrameValue() const;

  void setMethodName(const std::string& methodName)
  {
    _methodName = methodName;
  }

  const std::string& getMethodName() const { return _methodName; }
};

} // namespace xmlrpc

} // namespace aria2

#endif // _D_XML_RPC_REQUEST_PARSER_CONTROLLER_H_
