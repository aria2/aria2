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
#include "AbstractOptionHandler.h"

#include <algorithm>

#include "OptionHandlerException.h"
#include "a2functional.h"
#include "Option.h"
#include "prefs.h"
#include "help_tags.h"

namespace aria2 {

AbstractOptionHandler::AbstractOptionHandler(PrefPtr pref,
                                             const char* description,
                                             const std::string& defaultValue,
                                             ARG_TYPE argType, char shortName)
    : pref_(pref),
      description_(description),
      defaultValue_(defaultValue),
      argType_(argType),
      shortName_(shortName),
      tags_(0),
      flags_(0)
{
}

AbstractOptionHandler::~AbstractOptionHandler() = default;

void AbstractOptionHandler::parse(Option& option, const std::string& arg) const
{
  try {
    parseArg(option, arg);
  }
  catch (Exception& e) {
    throw OPTION_HANDLER_EXCEPTION2(pref_, e);
  }
}

bool AbstractOptionHandler::hasTag(uint32_t tag) const
{
  return (tags_ & (1 << tag));
}

void AbstractOptionHandler::addTag(uint32_t tag) { tags_ |= (1 << tag); }

std::string AbstractOptionHandler::toTagString() const
{
  std::string s;
  for (int i = 0; i < MAX_HELP_TAG; ++i) {
    if (tags_ & (1 << i)) {
      s += strHelpTag(i);
      s += ", ";
    }
  }
  if (!s.empty()) {
    s.resize(s.size() - 2);
  }
  return s;
}

const char* AbstractOptionHandler::getName() const { return pref_->k; }

void AbstractOptionHandler::updateFlags(int flag, bool val)
{
  if (val) {
    flags_ |= flag;
  }
  else {
    flags_ &= ~flag;
  }
}

bool AbstractOptionHandler::isHidden() const { return flags_ & FLAG_HIDDEN; }

void AbstractOptionHandler::hide() { updateFlags(FLAG_HIDDEN, true); }

bool AbstractOptionHandler::getEraseAfterParse() const
{
  return flags_ & FLAG_ERASE_AFTER_PARSE;
}

void AbstractOptionHandler::setEraseAfterParse(bool f)
{
  updateFlags(FLAG_ERASE_AFTER_PARSE, f);
}

bool AbstractOptionHandler::getInitialOption() const
{
  return flags_ & FLAG_INITIAL_OPTION;
}

void AbstractOptionHandler::setInitialOption(bool f)
{
  updateFlags(FLAG_INITIAL_OPTION, f);
}

bool AbstractOptionHandler::getChangeOption() const
{
  return flags_ & FLAG_CHANGE_OPTION;
}

void AbstractOptionHandler::setChangeOption(bool f)
{
  updateFlags(FLAG_CHANGE_OPTION, f);
}

bool AbstractOptionHandler::getChangeOptionForReserved() const
{
  return flags_ & FLAG_CHANGE_OPTION_FOR_RESERVED;
}

void AbstractOptionHandler::setChangeOptionForReserved(bool f)
{
  updateFlags(FLAG_CHANGE_OPTION_FOR_RESERVED, f);
}

bool AbstractOptionHandler::getChangeGlobalOption() const
{
  return flags_ & FLAG_CHANGE_GLOBAL_OPTION;
}

void AbstractOptionHandler::setChangeGlobalOption(bool f)
{
  updateFlags(FLAG_CHANGE_GLOBAL_OPTION, f);
}

bool AbstractOptionHandler::getCumulative() const
{
  return flags_ & FLAG_CUMULATIVE;
}

void AbstractOptionHandler::setCumulative(bool f)
{
  updateFlags(FLAG_CUMULATIVE, f);
}

} // namespace aria2
