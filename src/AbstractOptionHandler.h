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

#include <vector>

#include "A2STR.h"

namespace aria2 {

class Option;
struct Pref;

class AbstractOptionHandler : public OptionHandler {
protected:
  const Pref* pref_;

  const char* description_;

  std::string defaultValue_;

  std::vector<std::string> tags_;

  OptionHandler::ARG_TYPE argType_;

  char shortName_;

  bool hidden_;

  bool eraseAfterParse_;

  bool initialOption_;
  bool changeOption_;
  bool changeOptionForReserved_;
  bool globalChangeOption_;
  bool cumulative_;

  virtual void parseArg(Option& option, const std::string& arg) = 0;
public:
  AbstractOptionHandler(const Pref* pref,
                         const char* description = NO_DESCRIPTION,
                         const std::string& defaultValue = NO_DEFAULT_VALUE,
                         ARG_TYPE argType = REQ_ARG,
                         char shortName = 0);

  virtual ~AbstractOptionHandler();
  
  virtual void parse(Option& option, const std::string& arg);

  virtual bool hasTag(const std::string& tag) const;

  virtual void addTag(const std::string& tag);

  virtual std::string toTagString() const;

  virtual const char* getName() const;

  virtual const char* getDescription() const
  {
    return description_;
  }

  virtual const std::string& getDefaultValue() const
  {
    return defaultValue_;
  }

  virtual bool isHidden() const
  {
    return hidden_;
  }

  virtual void hide()
  {
    hidden_ = true;
  }

  virtual const Pref* getPref() const
  {
    return pref_;
  }

  virtual char getShortName() const
  {
    return shortName_;
  }

  virtual OptionHandler::ARG_TYPE getArgType() const
  {
    return argType_;
  }

  virtual bool getEraseAfterParse() const
  {
    return eraseAfterParse_;
  }

  virtual void setEraseAfterParse(bool eraseAfterParse)
  {
    eraseAfterParse_ = eraseAfterParse;
  }

  virtual bool getInitialOption() const
  {
    return initialOption_;
  }

  virtual void setInitialOption(bool f)
  {
    initialOption_ = f;
  }

  virtual bool getChangeOption() const
  {
    return changeOption_;
  }

  virtual void setChangeOption(bool f)
  {
    changeOption_ = f;
  }

  virtual bool getChangeOptionForReserved() const
  {
    return changeOptionForReserved_;
  }

  virtual void setChangeOptionForReserved(bool f)
  {
    changeOptionForReserved_ = f;
  }

  virtual bool getChangeGlobalOption() const
  {
    return globalChangeOption_;
  }

  virtual void setChangeGlobalOption(bool f)
  {
    globalChangeOption_ = f;
  }

  virtual bool getCumulative() const
  {
    return cumulative_;
  }

  virtual void setCumulative(bool f)
  {
    cumulative_ = f;
  }
};

} // namespace aria2

#endif // D_ABSTRACT_OPTION_HANDLER_H
