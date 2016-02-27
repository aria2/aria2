/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
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
#include "OptionHandler.h"
#include <ostream>

#define DEFAULT_MSG _("                              Default: ")
#define TAGS_MSG _("                              Tags: ")
#define POSSIBLE_MSG _("                              Possible Values: ")

namespace aria2 {

const char NO_DESCRIPTION[] = "";
const std::string NO_DEFAULT_VALUE("");

const std::string PATH_TO_FILE("/path/to/file");
const std::string PATH_TO_FILE_STDIN("/path/to/file, -");
const std::string PATH_TO_FILE_STDOUT("/path/to/file, -");
const std::string PATH_TO_DIR("/path/to/directory");
const std::string PATH_TO_COMMAND("/path/to/command");

std::ostream& operator<<(std::ostream& o, const OptionHandler& optionHandler)
{
  o << optionHandler.getDescription() << "\n\n";
  std::string possibleValues = optionHandler.createPossibleValuesString();
  if (!possibleValues.empty()) {
    o << POSSIBLE_MSG << possibleValues << "\n";
  }
  if (!optionHandler.getDefaultValue().empty()) {
    o << DEFAULT_MSG << optionHandler.getDefaultValue() << "\n";
  }
  o << TAGS_MSG << optionHandler.toTagString();
  return o;
}

void write(const Console& out, const OptionHandler& optionHandler)
{
  out->printf("%s\n\n", optionHandler.getDescription());
  std::string possibleValues = optionHandler.createPossibleValuesString();
  if (!possibleValues.empty()) {
    out->printf("%s%s\n", POSSIBLE_MSG, possibleValues.c_str());
  }
  if (!optionHandler.getDefaultValue().empty()) {
    out->printf("%s%s\n", DEFAULT_MSG, optionHandler.getDefaultValue().c_str());
  }
  out->printf("%s%s\n", TAGS_MSG, optionHandler.toTagString().c_str());
}

} // namespace aria2
