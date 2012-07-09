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
#include "ValueBaseStructParserStateImpl.h"
#include "ValueBaseStructParserStateMachine.h"
#include "ValueBase.h"

namespace aria2 {

void ValueValueBaseStructParserState::beginElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  switch(elementType) {
  case STRUCT_DICT_T:
    psm->setCurrentFrameValue(Dict::g());
    psm->pushDictState();
    break;
  case STRUCT_ARRAY_T:
    psm->setCurrentFrameValue(List::g());
    psm->pushArrayState();
    break;
  case STRUCT_STRING_T:
    psm->pushStringState();
    break;
  case STRUCT_NUMBER_T:
    psm->pushNumberState();
    break;
  case STRUCT_BOOL_T:
    psm->pushBoolState();
    break;
  case STRUCT_NULL_T:
    psm->pushNullState();
    break;
  default:
    // Not reachable
    assert(0);
  }
}

void DictValueBaseStructParserState::beginElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  switch(elementType) {
  case STRUCT_DICT_KEY_T:
    psm->pushFrame();
    psm->pushDictKeyState();
    break;
  case STRUCT_DICT_DATA_T:
    psm->pushDictDataState();
    break;
  default:
    // Not reachable
    assert(0);
  }
}

void DictKeyValueBaseStructParserState::endElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  psm->setCurrentFrameName(psm->getCharacters());
}

void DictDataValueBaseStructParserState::endElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  psm->popDictFrame();
}

void ArrayValueBaseStructParserState::beginElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  assert(elementType == STRUCT_ARRAY_DATA_T);
  psm->pushFrame();
  psm->pushArrayDataState();
}

void ArrayDataValueBaseStructParserState::endElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  psm->popArrayFrame();
}

void StringValueBaseStructParserState::endElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  psm->setCurrentFrameValue(String::g(psm->getCharacters()));
}

void NumberValueBaseStructParserState::endElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  // TODO Ignore frac and exp
  psm->setCurrentFrameValue(Integer::g(psm->getNumber().number));
}

void BoolValueBaseStructParserState::endElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  psm->setCurrentFrameValue(psm->getBool() ? Bool::gTrue() : Bool::gFalse());
}

void NullValueBaseStructParserState::endElement
(ValueBaseStructParserStateMachine* psm, int elementType)
{
  psm->setCurrentFrameValue(Null::g());
}

} // namespace aria2
