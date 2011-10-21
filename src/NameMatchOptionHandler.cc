/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "NameMatchOptionHandler.h"

#include <algorithm>

#include "OptionHandlerException.h"
#include "a2functional.h"
#include "Option.h"
#include "prefs.h"

namespace aria2 {

NameMatchOptionHandler::NameMatchOptionHandler
(const Pref* pref,
 const std::string& description,
 const std::string& defaultValue,
 ARG_TYPE argType,
 char shortName)
  : pref_(pref),
    description_(description),
    defaultValue_(defaultValue),
    id_(0),
    argType_(argType),
    shortName_(shortName),
    hidden_(false),
    eraseAfterParse_(false)
{}

NameMatchOptionHandler::~NameMatchOptionHandler() {}
  
bool NameMatchOptionHandler::canHandle(const std::string& optName)
{
  return pref_->k == optName;
}

void NameMatchOptionHandler::parse(Option& option, const std::string& arg)
{
  try {
    parseArg(option, arg);
  } catch(Exception& e) {
    throw OPTION_HANDLER_EXCEPTION2(pref_, e);
  }
}

bool NameMatchOptionHandler::hasTag(const std::string& tag) const
{
  return std::find(tags_.begin(), tags_.end(), tag) != tags_.end();
}

void NameMatchOptionHandler::addTag(const std::string& tag)
{
  tags_.push_back(tag);
}

std::string NameMatchOptionHandler::toTagString() const
{
  return strjoin(tags_.begin(), tags_.end(), ", ");
}

const std::string& NameMatchOptionHandler::getName() const
{
  return pref_->k;
}

} // namespace aria2
