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
#include "ValueBaseStructParserStateMachine.h"

#include <cstring>

#include "XmlRpcRequestParserController.h"
#include "ValueBaseStructParserStateImpl.h"
#include "ValueBase.h"

namespace aria2 {

namespace {
ValueValueBaseStructParserState* valueState =
  new ValueValueBaseStructParserState();
DictValueBaseStructParserState* dictState =
  new DictValueBaseStructParserState();
DictKeyValueBaseStructParserState* dictKeyState =
  new DictKeyValueBaseStructParserState();
DictDataValueBaseStructParserState* dictDataState =
  new DictDataValueBaseStructParserState();
ArrayValueBaseStructParserState* arrayState =
  new ArrayValueBaseStructParserState();
ArrayDataValueBaseStructParserState* arrayDataState =
  new ArrayDataValueBaseStructParserState();
StringValueBaseStructParserState* stringState =
  new StringValueBaseStructParserState();
NumberValueBaseStructParserState* numberState =
  new NumberValueBaseStructParserState();
BoolValueBaseStructParserState* boolState =
  new BoolValueBaseStructParserState();
NullValueBaseStructParserState* nullState =
  new NullValueBaseStructParserState();
} // namespace

const SharedHandle<ValueBase>&
ValueBaseStructParserStateMachine::noResult()
{
  return ValueBase::none;
}

ValueBaseStructParserStateMachine::ValueBaseStructParserStateMachine()
  : ctrl_(new rpc::XmlRpcRequestParserController())
{
  stateStack_.push(valueState);
}

ValueBaseStructParserStateMachine::~ValueBaseStructParserStateMachine()
{
  delete ctrl_;
}

void ValueBaseStructParserStateMachine::reset()
{
  while(!stateStack_.empty()) {
    stateStack_.pop();
  }
  stateStack_.push(valueState);
  ctrl_->reset();
}

void ValueBaseStructParserStateMachine::beginElement(int elementType)
{
  stateStack_.top()->beginElement(this, elementType);
}

void ValueBaseStructParserStateMachine::endElement(int elementType)
{
  stateStack_.top()->endElement(this, elementType);
  stateStack_.pop();
}

SharedHandle<ValueBase>
ValueBaseStructParserStateMachine::getResult() const
{
  return getCurrentFrameValue();
}

void ValueBaseStructParserStateMachine::charactersCallback
(const char* data, size_t len)
{
  sessionData_.str.append(data, len);
}

void ValueBaseStructParserStateMachine::numberCallback
(int64_t number, int frac, int exp)
{
  sessionData_.number.number = number;
  sessionData_.number.frac = frac;
  sessionData_.number.exp = exp;
}

void ValueBaseStructParserStateMachine::boolCallback(bool bval)
{
  sessionData_.bval = bval;
}

const std::string& ValueBaseStructParserStateMachine::getCharacters() const
{
  return sessionData_.str;
}

const ValueBaseStructParserStateMachine::NumberData&
ValueBaseStructParserStateMachine::getNumber() const
{
  return sessionData_.number;
}

bool ValueBaseStructParserStateMachine::getBool() const
{
  return sessionData_.bval;
}

void ValueBaseStructParserStateMachine::popArrayFrame()
{
  ctrl_->popArrayFrame();
}

void ValueBaseStructParserStateMachine::popDictFrame()
{
  ctrl_->popStructFrame();
}

void ValueBaseStructParserStateMachine::pushFrame()
{
  ctrl_->pushFrame();
}

void ValueBaseStructParserStateMachine::setCurrentFrameValue
(const SharedHandle<ValueBase>& value)
{
  ctrl_->setCurrentFrameValue(value);
}

const SharedHandle<ValueBase>&
ValueBaseStructParserStateMachine::getCurrentFrameValue() const
{
  return ctrl_->getCurrentFrameValue();
}

void ValueBaseStructParserStateMachine::setCurrentFrameName
(const std::string& name)
{
  ctrl_->setCurrentFrameName(name);
}

void ValueBaseStructParserStateMachine::pushDictState()
{
  stateStack_.push(dictState);
}

void ValueBaseStructParserStateMachine::pushDictKeyState()
{
  sessionData_.str.clear();
  stateStack_.push(dictKeyState);
}

void ValueBaseStructParserStateMachine::pushDictDataState()
{
  stateStack_.push(dictDataState);
}

void ValueBaseStructParserStateMachine::pushArrayState()
{
  stateStack_.push(arrayState);
}

void ValueBaseStructParserStateMachine::pushArrayDataState()
{
  stateStack_.push(arrayDataState);
}

void ValueBaseStructParserStateMachine::pushStringState()
{
  sessionData_.str.clear();
  stateStack_.push(stringState);
}

void ValueBaseStructParserStateMachine::pushNumberState()
{
  memset(&sessionData_.number, 0, sizeof(sessionData_.number));
  stateStack_.push(numberState);
}

void ValueBaseStructParserStateMachine::pushBoolState()
{
  sessionData_.bval = false;
  stateStack_.push(boolState);
}

void ValueBaseStructParserStateMachine::pushNullState()
{
  stateStack_.push(nullState);
}

} // namespace aria2
