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
#ifndef D_VALUE_BASE_STRUCT_PARSER_STATE_IMPL_H
#define D_VALUE_BASE_STRUCT_PARSER_STATE_IMPL_H

#include "ValueBaseStructParserState.h"

namespace aria2 {

class ValueValueBaseStructParserState : public ValueBaseStructParserState {
public:
  virtual ~ValueValueBaseStructParserState() = default;

  virtual void beginElement(ValueBaseStructParserStateMachine* psm,
                            int elementType) CXX11_OVERRIDE;

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE
  {
  }
};

class DictValueBaseStructParserState : public ValueBaseStructParserState {
public:
  virtual ~DictValueBaseStructParserState() = default;

  virtual void beginElement(ValueBaseStructParserStateMachine* psm,
                            int elementType) CXX11_OVERRIDE;

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE
  {
  }
};

class DictKeyValueBaseStructParserState : public ValueBaseStructParserState {
public:
  virtual ~DictKeyValueBaseStructParserState() = default;

  virtual void beginElement(ValueBaseStructParserStateMachine* psm,
                            int elementType) CXX11_OVERRIDE
  {
  }

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE;
};

class DictDataValueBaseStructParserState
    : public ValueValueBaseStructParserState {
public:
  virtual ~DictDataValueBaseStructParserState() = default;

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE;
};

class ArrayValueBaseStructParserState : public ValueBaseStructParserState {
public:
  virtual ~ArrayValueBaseStructParserState() = default;

  virtual void beginElement(ValueBaseStructParserStateMachine* psm,
                            int elementType) CXX11_OVERRIDE;

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE
  {
  }
};

class ArrayDataValueBaseStructParserState
    : public ValueValueBaseStructParserState {
public:
  virtual ~ArrayDataValueBaseStructParserState() = default;

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE;
};

class StringValueBaseStructParserState : public ValueBaseStructParserState {
public:
  virtual ~StringValueBaseStructParserState() = default;

  virtual void beginElement(ValueBaseStructParserStateMachine* psm,
                            int elementType) CXX11_OVERRIDE
  {
  }

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE;
};

class NumberValueBaseStructParserState : public ValueBaseStructParserState {
public:
  virtual ~NumberValueBaseStructParserState() = default;

  virtual void beginElement(ValueBaseStructParserStateMachine* psm,
                            int elementType) CXX11_OVERRIDE
  {
  }

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE;
};

class BoolValueBaseStructParserState : public ValueBaseStructParserState {
public:
  virtual ~BoolValueBaseStructParserState() = default;

  virtual void beginElement(ValueBaseStructParserStateMachine* psm,
                            int elementType) CXX11_OVERRIDE
  {
  }

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE;
};

class NullValueBaseStructParserState : public ValueBaseStructParserState {
public:
  virtual ~NullValueBaseStructParserState() = default;

  virtual void beginElement(ValueBaseStructParserStateMachine* psm,
                            int elementType) CXX11_OVERRIDE
  {
  }

  virtual void endElement(ValueBaseStructParserStateMachine* psm,
                          int elementType) CXX11_OVERRIDE;
};

} // namespace aria2

#endif // D_VALUE_BASE_STRUCT_PARSER_STATE_IMPL_H
