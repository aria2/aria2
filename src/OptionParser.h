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
#ifndef D_OPTION_PARSER_H
#define D_OPTION_PARSER_H

#include "common.h"

#include <string>
#include <vector>
#include <iosfwd>
#include <memory>

#include <aria2/aria2.h>

#include "prefs.h"

namespace aria2 {

class Option;
class OptionHandler;

class OptionParser {
private:
  std::vector<OptionHandler*> handlers_;
  // Index of handler in handlers_ for option who has short option name.
  std::vector<size_t> shortOpts_;
  static std::shared_ptr<OptionParser> optionParser_;

public:
  OptionParser();
  ~OptionParser();

  // Parses options in argv and writes option name and value to out in
  // NAME=VALUE format. Non-option strings are stored in nonopts.
  // Throws Exception when an unrecognized option is found.
  void parseArg(std::ostream& out, std::vector<std::string>& nonopts, int argc,
                char* argv[]) const;

  void parse(Option& option, std::istream& ios) const;

  void parse(Option& option, const KeyVals& options) const;

  void parseDefaultValues(Option& option) const;

  void setOptionHandlers(const std::vector<OptionHandler*>& handlers);

  void addOptionHandler(OptionHandler* handler);

  // Hidden options are not returned.
  std::vector<const OptionHandler*> findByTag(uint32_t tag) const;

  // Hidden options are not returned.
  std::vector<const OptionHandler*>
  findByNameSubstring(const std::string& substring) const;

  // Hidden options are not returned.
  std::vector<const OptionHandler*> findAll() const;

  // Hidden options are not returned.
  const OptionHandler* find(PrefPtr pref) const;

  // Hidden options are not returned.
  const OptionHandler* findById(size_t id) const;

  // Hidden options are not returned.
  const OptionHandler* findByShortName(char shortName) const;

  static const std::shared_ptr<OptionParser>& getInstance();
  // Deletes statically allocated instance. Call this at the end of the
  // program.
  static void deleteInstance();
};

} // namespace aria2

#endif // D_OPTION_PARSER_H
