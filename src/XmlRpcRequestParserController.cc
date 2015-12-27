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
#include "XmlRpcRequestParserController.h"

#include <cassert>

namespace aria2 {

namespace rpc {

void XmlRpcRequestParserController::pushFrame()
{
  frameStack_.push(std::move(currentFrame_));
  currentFrame_ = StateFrame();
}

void XmlRpcRequestParserController::popStructFrame()
{
  assert(!frameStack_.empty());

  StateFrame parentFrame = std::move(frameStack_.top());
  Dict* dict = downcast<Dict>(parentFrame.value_);
  assert(dict);
  frameStack_.pop();
  if (currentFrame_.validMember()) {
    dict->put(std::move(currentFrame_.name_), std::move(currentFrame_.value_));
  }
  currentFrame_ = std::move(parentFrame);
}

void XmlRpcRequestParserController::popArrayFrame()
{
  assert(!frameStack_.empty());

  StateFrame parentFrame = std::move(frameStack_.top());
  List* list = downcast<List>(parentFrame.value_);
  assert(list);
  frameStack_.pop();
  if (currentFrame_.value_) {
    list->append(std::move(currentFrame_.value_));
  }
  currentFrame_ = std::move(parentFrame);
}

void XmlRpcRequestParserController::setCurrentFrameValue(
    std::unique_ptr<ValueBase> value)
{
  currentFrame_.value_ = std::move(value);
}

void XmlRpcRequestParserController::setCurrentFrameName(std::string name)
{
  currentFrame_.name_ = std::move(name);
}

const std::unique_ptr<ValueBase>&
XmlRpcRequestParserController::getCurrentFrameValue() const
{
  return currentFrame_.value_;
}

std::unique_ptr<ValueBase> XmlRpcRequestParserController::popCurrentFrameValue()
{
  return std::move(currentFrame_.value_);
}

void XmlRpcRequestParserController::reset()
{
  while (!frameStack_.empty()) {
    frameStack_.pop();
  }
  currentFrame_.reset();
  methodName_.clear();
}

void XmlRpcRequestParserController::setMethodName(std::string methodName)
{
  methodName_ = std::move(methodName);
}

} // namespace rpc

} // namespace aria2
