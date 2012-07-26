/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#ifndef D_VALUE_BASE_STRUCT_PARSER_STATE_MACHINE_H
#define D_VALUE_BASE_STRUCT_PARSER_STATE_MACHINE_H

#include "StructParserStateMachine.h"

#include <string>
#include <stack>

#include "SharedHandle.h"

namespace aria2 {

class ValueBase;

namespace rpc {
class XmlRpcRequestParserController;
} // namespace rpc;

class ValueBaseStructParserState;

// Implementation of StructParserStateMachine, using ValueBase as
// value holder.
class ValueBaseStructParserStateMachine : public StructParserStateMachine {
public:
  typedef SharedHandle<ValueBase> ResultType;
  static const SharedHandle<ValueBase>& noResult();

  struct NumberData {
    int64_t number;
    int frac;
    int exp;
  };

  struct SessionData {
    std::string str;
    NumberData number;
    bool bval;
  };

  ValueBaseStructParserStateMachine();
  virtual ~ValueBaseStructParserStateMachine();

  virtual void beginElement(int elementType);
  virtual void endElement(int elementType);

  virtual void charactersCallback(const char* data, size_t len);
  virtual void numberCallback(int64_t number, int frac, int exp);
  virtual void boolCallback(bool bval);

  SharedHandle<ValueBase> getResult() const;

  virtual void reset();

  const std::string& getCharacters() const;
  const NumberData& getNumber() const;
  bool getBool() const;

  void popArrayFrame();
  void popDictFrame();
  void pushFrame();
  void setCurrentFrameValue(const SharedHandle<ValueBase>& value);
  const SharedHandle<ValueBase>& getCurrentFrameValue() const;
  void setCurrentFrameName(const std::string& name);

  void pushDictState();
  void pushDictKeyState();
  void pushDictDataState();
  void pushArrayState();
  void pushArrayDataState();
  void pushStringState();
  void pushNumberState();
  void pushBoolState();
  void pushNullState();
private:
  rpc::XmlRpcRequestParserController* ctrl_;
  std::stack<ValueBaseStructParserState*> stateStack_;
  SessionData sessionData_;
};

} // namespace aria2

#endif // D_VALUE_BASE_STRUCT_PARSER_STATE_MACHINE_H
