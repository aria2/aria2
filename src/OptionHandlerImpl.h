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
#ifndef D_OPTION_HANDLER_IMPL_H
#define D_OPTION_HANDLER_IMPL_H

#include "OptionHandler.h"

#include <vector>

#include "NameMatchOptionHandler.h"

namespace aria2 {

class Option;

class NullOptionHandler : public OptionHandler {
private:
  int id_;
public:
  virtual ~NullOptionHandler();
  virtual bool canHandle(const std::string& optName);
  virtual void parse(Option& option, const std::string& arg);
  virtual bool hasTag(const std::string& tag) const;
  virtual void addTag(const std::string& tag);
  virtual std::string toTagString() const;
  virtual const std::string& getName() const;
  virtual const std::string& getDescription() const;
  virtual const std::string& getDefaultValue() const;
  virtual std::string createPossibleValuesString() const;
  virtual bool isHidden() const;
  virtual void hide();
  virtual OptionHandler::ARG_TYPE getArgType() const;
  virtual int getOptionID() const;
  virtual void setOptionID(int id);
  virtual char getShortName() const;
  virtual bool getEraseAfterParse() const;
  virtual void setEraseAfterParse(bool eraseAfterParse);
};

class BooleanOptionHandler : public NameMatchOptionHandler {
public:
  BooleanOptionHandler(const std::string& optName,
                       const std::string& description = NO_DESCRIPTION,
                       const std::string& defaultValue = NO_DEFAULT_VALUE,
                       OptionHandler::ARG_TYPE argType = OptionHandler::REQ_ARG,
                       char shortName = 0);
  virtual ~BooleanOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class IntegerRangeOptionHandler : public NameMatchOptionHandler {
private:
  int32_t min_;
  int32_t max_;
public:
  IntegerRangeOptionHandler(const std::string& optName,
                            const std::string& description,
                            const std::string& defaultValue,
                            int32_t min, int32_t max,
                            char shortName = 0);
  virtual ~IntegerRangeOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class NumberOptionHandler : public NameMatchOptionHandler {
private:
  int64_t min_;
  int64_t max_;
public:
  NumberOptionHandler(const std::string& optName,
                      const std::string& description = NO_DESCRIPTION,
                      const std::string& defaultValue = NO_DEFAULT_VALUE,
                      int64_t min = -1,
                      int64_t max = -1,
                      char shortName = 0);
  virtual ~NumberOptionHandler();

  virtual void parseArg(Option& option, const std::string& optarg);
  void parseArg(Option& option, int64_t number);
  virtual std::string createPossibleValuesString() const;
};

class UnitNumberOptionHandler : public NumberOptionHandler {
public:
  UnitNumberOptionHandler(const std::string& optName,
                          const std::string& description = NO_DESCRIPTION,
                          const std::string& defaultValue = NO_DEFAULT_VALUE,
                          int64_t min = -1,
                          int64_t max = -1,
                          char shortName = 0);
  virtual ~UnitNumberOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
};

class FloatNumberOptionHandler : public NameMatchOptionHandler {
private:
  double min_;
  double max_;
public:
  FloatNumberOptionHandler(const std::string& optName,
                           const std::string& description = NO_DESCRIPTION,
                           const std::string& defaultValue = NO_DEFAULT_VALUE,
                           double min = -1, double max = -1,
                           char shortName = 0);
  virtual ~FloatNumberOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class DefaultOptionHandler : public NameMatchOptionHandler {
private:
  std::string possibleValuesString_;
public:
  DefaultOptionHandler(const std::string& optName,
                       const std::string& description = NO_DESCRIPTION,
                       const std::string& defaultValue = NO_DEFAULT_VALUE,
                       const std::string& possibleValuesString = A2STR::NIL,
                       OptionHandler::ARG_TYPE argType = OptionHandler::REQ_ARG,
                       char shortName = 0);
  virtual ~DefaultOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class CumulativeOptionHandler : public NameMatchOptionHandler {
private:
  std::string delim_;
  std::string possibleValuesString_;
public:
  CumulativeOptionHandler(const std::string& optName,
                          const std::string& description,
                          const std::string& defaultValue,
                          const std::string& delim,
                          const std::string& possibleValuesString = A2STR::NIL,
                          OptionHandler::ARG_TYPE argType =
                          OptionHandler::REQ_ARG,
                          char shortName = 0);
  virtual ~CumulativeOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class IndexOutOptionHandler : public NameMatchOptionHandler {
private:
public:
  IndexOutOptionHandler(const std::string& optName,
                        const std::string& description,
                        char shortName = 0);
  virtual ~IndexOutOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class ParameterOptionHandler : public NameMatchOptionHandler {
private:
  std::vector<std::string> validParamValues_;
public:
  ParameterOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         const std::vector<std::string>& validParamValues,
                         char shortName = 0);
  ParameterOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         const std::string& validParamValue,
                         char shortName = 0);
  ParameterOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         const std::string& validParamValue1,
                         const std::string& validParamValue2,
                         char shortName = 0);
  ParameterOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         const std::string& validParamValue1,
                         const std::string& validParamValue2,
                         const std::string& validParamValue3,
                         char shortName = 0);
  virtual ~ParameterOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class HostPortOptionHandler : public NameMatchOptionHandler {
private:
  std::string hostOptionName_;
  std::string portOptionName_;
public:
  HostPortOptionHandler(const std::string& optName,
                        const std::string& description,
                        const std::string& defaultValue,
                        const std::string& hostOptionName,
                        const std::string& portOptionName,
                        char shortName = 0);
  virtual ~HostPortOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  void setHostAndPort
  (Option& option, const std::string& hostname, uint16_t port);
  virtual std::string createPossibleValuesString() const;
};

class HttpProxyUserOptionHandler:public NameMatchOptionHandler {
public:
  HttpProxyUserOptionHandler(const std::string& optName,
                             const std::string& description,
                             const std::string& defaultValue,
                             char shortName = 0);
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class HttpProxyPasswdOptionHandler:public NameMatchOptionHandler {
public:
  HttpProxyPasswdOptionHandler(const std::string& optName,
                               const std::string& description,
                               const std::string& defaultValue,
                               char shortName = 0);
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class HttpProxyOptionHandler : public NameMatchOptionHandler {
private:
  std::string proxyUserPref_;
  std::string proxyPasswdPref_;
public:
  HttpProxyOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         char shortName = 0);
  virtual ~HttpProxyOptionHandler();
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class LocalFilePathOptionHandler : public NameMatchOptionHandler {
private:
  bool acceptStdin_;
public:
  LocalFilePathOptionHandler
  (const std::string& optName,
   const std::string& description = NO_DESCRIPTION,
   const std::string& defaultValue = NO_DEFAULT_VALUE,
   bool acceptStdin = false,
   char shortName = 0);
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

class PrioritizePieceOptionHandler:public NameMatchOptionHandler {
public:
  PrioritizePieceOptionHandler
  (const std::string& optName,
   const std::string& description = NO_DESCRIPTION,
   const std::string& defaultValue = NO_DEFAULT_VALUE,
   char shortName = 0);
  virtual void parseArg(Option& option, const std::string& optarg);
  virtual std::string createPossibleValuesString() const;
};

// This class is used to deprecate option and optionally handle its
// option value using replacing option.
class DeprecatedOptionHandler:public OptionHandler {
private:
  SharedHandle<OptionHandler> depOptHandler_;
  SharedHandle<OptionHandler> repOptHandler_;
public:
  // depOptHandler is deprecated option and repOptHandler is replacing
  // new option. If there is no replacing option, omit second
  // argument.
  DeprecatedOptionHandler
  (const SharedHandle<OptionHandler>& depOptHandler,
   const SharedHandle<OptionHandler>& repOptHandler =
   SharedHandle<OptionHandler>());
  virtual bool canHandle(const std::string& optName);
  virtual void parse(Option& option, const std::string& arg);
  virtual std::string createPossibleValuesString() const;
  virtual bool hasTag(const std::string& tag) const;
  virtual void addTag(const std::string& tag);
  virtual std::string toTagString() const;
  virtual const std::string& getName() const;
  virtual const std::string& getDescription() const;
  virtual const std::string& getDefaultValue() const;
  virtual bool isHidden() const;
  virtual void hide();
  virtual ARG_TYPE getArgType() const;
  virtual char getShortName() const;
  virtual int getOptionID() const;
  virtual void setOptionID(int id);
  virtual bool getEraseAfterParse() const;
  virtual void setEraseAfterParse(bool eraseAfterParse);
};

} // namespace aria2

#endif // D_OPTION_HANDLER_IMPL_H
