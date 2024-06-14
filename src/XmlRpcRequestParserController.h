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
#ifndef D_XML_RPC_REQUEST_PARSER_CONTROLLER_H
#define D_XML_RPC_REQUEST_PARSER_CONTROLLER_H

#include "common.h"

#include <stack>
#include <string>

#include "ValueBase.h"

namespace aria2 {

namespace rpc {

class XmlRpcRequestParserController {
private:
  struct StateFrame {
    std::unique_ptr<ValueBase> value_;
    std::string name_;

    bool validMember(bool allowEmptyMemberName) const
    {
      return value_ && (allowEmptyMemberName || !name_.empty());
    }

    void reset()
    {
      value_.reset();
      name_.clear();
    }
  };

  std::stack<StateFrame> frameStack_;

  StateFrame currentFrame_;

  std::string methodName_;

  bool allowEmptyMemberName_;

public:
  XmlRpcRequestParserController() : allowEmptyMemberName_(false) {}

  void pushFrame();

  // Pops StateFrame p from frameStack_ and set p[currentFrame_.name_]
  // = currentFrame_.value_ and currentFrame_ = p;
  void popStructFrame();

  // Pops StateFrame p from frameStack_ and add currentFrame_.value_
  // to p and currentFrame_ = p;
  void popArrayFrame();

  void setCurrentFrameValue(std::unique_ptr<ValueBase> value);

  void setCurrentFrameName(std::string name);

  const std::unique_ptr<ValueBase>& getCurrentFrameValue() const;

  std::unique_ptr<ValueBase> popCurrentFrameValue();

  void setMethodName(std::string methodName);

  const std::string& getMethodName() const { return methodName_; }

  void setAllowEmptyMemberName(bool b);

  void reset();
};

} // namespace rpc

} // namespace aria2

#endif // D_XML_RPC_REQUEST_PARSER_CONTROLLER_H
