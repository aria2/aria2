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
#ifndef D_ABSTRACT_OPTION_HANDLER_H
#define D_ABSTRACT_OPTION_HANDLER_H

#include "OptionHandler.h"

namespace aria2 {

class Option;
struct Pref;

class AbstractOptionHandler : public OptionHandler {
protected:
  PrefPtr pref_;

  const char* description_;

  std::string defaultValue_;

  OptionHandler::ARG_TYPE argType_;

  char shortName_;

  virtual void parseArg(Option& option, const std::string& arg) const = 0;

public:
  AbstractOptionHandler(PrefPtr pref, const char* description = NO_DESCRIPTION,
                        const std::string& defaultValue = NO_DEFAULT_VALUE,
                        ARG_TYPE argType = REQ_ARG, char shortName = 0);

  virtual ~AbstractOptionHandler();

  virtual void parse(Option& option,
                     const std::string& arg) const CXX11_OVERRIDE;

  virtual bool hasTag(uint32_t tag) const CXX11_OVERRIDE;

  virtual void addTag(uint32_t tag) CXX11_OVERRIDE;

  virtual std::string toTagString() const CXX11_OVERRIDE;

  virtual const char* getName() const CXX11_OVERRIDE;

  virtual const char* getDescription() const CXX11_OVERRIDE
  {
    return description_;
  }

  virtual const std::string& getDefaultValue() const CXX11_OVERRIDE
  {
    return defaultValue_;
  }

  virtual PrefPtr getPref() const CXX11_OVERRIDE { return pref_; }

  virtual char getShortName() const CXX11_OVERRIDE { return shortName_; }

  virtual OptionHandler::ARG_TYPE getArgType() const CXX11_OVERRIDE
  {
    return argType_;
  }

  virtual bool isHidden() const CXX11_OVERRIDE;

  virtual void hide() CXX11_OVERRIDE;

  virtual bool getEraseAfterParse() const CXX11_OVERRIDE;

  virtual void setEraseAfterParse(bool f) CXX11_OVERRIDE;

  virtual bool getInitialOption() const CXX11_OVERRIDE;

  virtual void setInitialOption(bool f) CXX11_OVERRIDE;

  virtual bool getChangeOption() const CXX11_OVERRIDE;

  virtual void setChangeOption(bool f) CXX11_OVERRIDE;

  virtual bool getChangeOptionForReserved() const CXX11_OVERRIDE;

  virtual void setChangeOptionForReserved(bool f) CXX11_OVERRIDE;

  virtual bool getChangeGlobalOption() const CXX11_OVERRIDE;

  virtual void setChangeGlobalOption(bool f) CXX11_OVERRIDE;

  virtual bool getCumulative() const CXX11_OVERRIDE;

  virtual void setCumulative(bool f) CXX11_OVERRIDE;

  enum Flag {
    FLAG_HIDDEN = 1,
    FLAG_ERASE_AFTER_PARSE = 1 << 1,
    FLAG_INITIAL_OPTION = 1 << 2,
    FLAG_CHANGE_OPTION = 1 << 3,
    FLAG_CHANGE_OPTION_FOR_RESERVED = 1 << 4,
    FLAG_CHANGE_GLOBAL_OPTION = 1 << 5,
    FLAG_CUMULATIVE = 1 << 6
  };

private:
  // bitwise OR of (1 << HelpTag value) defined in help_tags.h.
  uint32_t tags_;

  // bitwise OR of Flag values.
  uint8_t flags_;

  void updateFlags(int flag, bool val);
};

} // namespace aria2

#endif // D_ABSTRACT_OPTION_HANDLER_H
