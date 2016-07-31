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
#ifndef D_OPTION_HANDLER_H
#define D_OPTION_HANDLER_H

#include "common.h"

#include <string>
#include <vector>
#include <iosfwd>
#include <functional>

#include "console.h"
#include "prefs.h"

namespace aria2 {

extern const char NO_DESCRIPTION[];
extern const std::string NO_DEFAULT_VALUE;

extern const std::string PATH_TO_FILE;
extern const std::string PATH_TO_FILE_STDIN;
extern const std::string PATH_TO_FILE_STDOUT;
extern const std::string PATH_TO_DIR;
extern const std::string PATH_TO_COMMAND;

class Option;

class OptionHandler {
public:
  virtual ~OptionHandler() = default;

  virtual void parse(Option& option, const std::string& arg) const = 0;

  virtual std::string createPossibleValuesString() const = 0;

  virtual bool hasTag(uint32_t tag) const = 0;

  virtual void addTag(uint32_t tag) = 0;

  virtual std::string toTagString() const = 0;

  virtual const char* getName() const = 0;

  virtual const char* getDescription() const = 0;

  virtual const std::string& getDefaultValue() const = 0;

  virtual bool isHidden() const = 0;

  virtual void hide() = 0;

  virtual PrefPtr getPref() const = 0;

  enum ARG_TYPE { REQ_ARG, OPT_ARG, NO_ARG };

  virtual ARG_TYPE getArgType() const = 0;

  virtual char getShortName() const = 0;

  // Returns true if option value should be erased from argv to
  // prevent it from appearing in the output of ps.
  virtual bool getEraseAfterParse() const = 0;

  virtual void setEraseAfterParse(bool eraseAfterParse) = 0;

  virtual bool getInitialOption() const = 0;
  virtual void setInitialOption(bool f) = 0;
  virtual bool getChangeOption() const = 0;
  virtual void setChangeOption(bool f) = 0;
  virtual bool getChangeOptionForReserved() const = 0;
  virtual void setChangeOptionForReserved(bool f) = 0;
  virtual bool getChangeGlobalOption() const = 0;
  virtual void setChangeGlobalOption(bool f) = 0;
  virtual bool getCumulative() const = 0;
  virtual void setCumulative(bool f) = 0;
};

std::ostream& operator<<(std::ostream& o, const OptionHandler& optionHandler);

void write(const Console& out, const OptionHandler& optionHandler);

} // namespace aria2

#endif // D_OPTION_HANDLER_H
