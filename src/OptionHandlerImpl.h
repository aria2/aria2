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
#ifndef _D_OPTION_HANDLER_IMPL_H_
#define _D_OPTION_HANDLER_IMPL_H_

#include "OptionHandler.h"

#include <cassert>
#include <cstdio>
#include <utility>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iterator>
#include <vector>

#include "NameMatchOptionHandler.h"
#include "util.h"
#include "DlAbortEx.h"
#include "prefs.h"
#include "Option.h"
#include "StringFormat.h"
#include "A2STR.h"
#include "Request.h"
#include "a2functional.h"
#include "message.h"
#include "File.h"
#include "FileEntry.h"
#include "a2io.h"

namespace aria2 {

class NullOptionHandler : public OptionHandler {
private:
  int id_;
public:
  virtual ~NullOptionHandler() {}

  virtual bool canHandle(const std::string& optName) { return true; }

  virtual void parse(Option& option, const std::string& arg) {}

  virtual bool hasTag(const std::string& tag) const { return false; }

  virtual void addTag(const std::string& tag) {}

  virtual std::string toTagString() const { return A2STR::NIL; }

  virtual const std::string& getName() const { return A2STR::NIL; }

  virtual const std::string& getDescription() const { return A2STR::NIL; }

  virtual const std::string& getDefaultValue() const { return A2STR::NIL; }

  virtual std::string createPossibleValuesString() const { return A2STR::NIL; }

  virtual bool isHidden() const { return true; }

  virtual void hide() {}

  virtual OptionHandler::ARG_TYPE getArgType() const
  {
    return OptionHandler::NO_ARG;
  }

  virtual int getOptionID() const
  {
    return id_;
  }

  virtual void setOptionID(int id)
  {
    id_ = id;
  }

  virtual char getShortName() const
  {
    return 0;
  }
};

class BooleanOptionHandler : public NameMatchOptionHandler {
public:
  BooleanOptionHandler(const std::string& optName,
                       const std::string& description = NO_DESCRIPTION,
                       const std::string& defaultValue = NO_DEFAULT_VALUE,
                       OptionHandler::ARG_TYPE argType = OptionHandler::REQ_ARG,
                       char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           argType, shortName) {}

  virtual ~BooleanOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    if(optarg == "true" ||
       ((argType_ == OptionHandler::OPT_ARG ||
         argType_ == OptionHandler::NO_ARG)
        && optarg.empty())) {
      option.put(optName_, V_TRUE);
    } else if(optarg == "false") {
      option.put(optName_, V_FALSE);
    } else {
      std::string msg = optName_;
      strappend(msg, " ", _("must be either 'true' or 'false'."));
      throw DL_ABORT_EX(msg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    return "true,false";
  }
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
                            char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName),
    min_(min), max_(max) {}

  virtual ~IntegerRangeOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    IntSequence seq = util::parseIntRange(optarg);
    while(seq.hasNext()) {
      int32_t v = seq.next();
      if(v < min_ || max_ < v) {
        std::string msg = optName_;
        strappend(msg, " ", _("must be between %s and %s."));
        throw DL_ABORT_EX
          (StringFormat(msg.c_str(), util::itos(min_).c_str(),
                        util::itos(max_).c_str()).str());
      }
      option.put(optName_, optarg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    return util::itos(min_)+"-"+util::itos(max_);
  }
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
                      char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName),
    min_(min), max_(max) {}

  virtual ~NumberOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    int64_t num = util::parseLLInt(optarg);
    parseArg(option, num);
  }

  void parseArg(Option& option, int64_t number)
  {
    if((min_ == -1 || min_ <= number) && (max_ ==  -1 || number <= max_)) {
      option.put(optName_, util::itos(number));
    } else {
      std::string msg = optName_;
      msg += " ";
      if(min_ == -1 && max_ != -1) {
        msg += StringFormat(_("must be smaller than or equal to %s."),
                            util::itos(max_).c_str()).str();
      } else if(min_ != -1 && max_ != -1) {
        msg += StringFormat(_("must be between %s and %s."),
                            util::itos(min_).c_str(), util::itos(max_).c_str()).str();
      } else if(min_ != -1 && max_ == -1) {
        msg += StringFormat(_("must be greater than or equal to %s."),
                            util::itos(min_).c_str()).str();
      } else {
        msg += _("must be a number.");
      }
      throw DL_ABORT_EX(msg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    std::string values;
    if(min_ == -1) {
      values += "*";
    } else {
      values += util::itos(min_);
    }
    values += "-";
    if(max_ == -1) {
      values += "*";
    } else {
      values += util::itos(max_);
    }
    return values;
  }
};

class UnitNumberOptionHandler : public NumberOptionHandler {
public:
  UnitNumberOptionHandler(const std::string& optName,
                          const std::string& description = NO_DESCRIPTION,
                          const std::string& defaultValue = NO_DEFAULT_VALUE,
                          int64_t min = -1,
                          int64_t max = -1,
                          char shortName = 0):
    NumberOptionHandler(optName, description, defaultValue, min, max,
                        shortName) {}

  virtual ~UnitNumberOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    int64_t num = util::getRealSize(optarg);
    NumberOptionHandler::parseArg(option, num);
  }
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
                           char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName),
    min_(min), max_(max) {}

  virtual ~FloatNumberOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    double number = strtod(optarg.c_str(), 0);
    if((min_ < 0 || min_ <= number) && (max_ < 0 || number <= max_)) {
      option.put(optName_, optarg);
    } else {
      std::string msg = optName_;
      msg += " ";
      if(min_ < 0 && max_ >= 0) {
        msg += StringFormat(_("must be smaller than or equal to %.1f."),
                            max_).str();
      } else if(min_ >= 0 && max_ >= 0) {
        msg += StringFormat(_("must be between %.1f and %.1f."),
                            min_, max_).str();
      } else if(min_ >= 0 && max_ < 0) {
        msg += StringFormat(_("must be greater than or equal to %.1f."),
                            min_).str();
      } else {
        msg += _("must be a number.");
      }
      throw DL_ABORT_EX(msg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    std::string valuesString;
    if(min_ < 0) {
      valuesString += "*";
    } else {
      char buf[11];
      snprintf(buf, sizeof(buf), "%.1f", min_);
      valuesString += buf;
    }
    valuesString += "-";
    if(max_ < 0) {
      valuesString += "*";
    } else {
      char buf[11];
      snprintf(buf, sizeof(buf), "%.1f", max_);
      valuesString += buf;
    }
    return valuesString;
  }
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
                       char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue, argType,
                           shortName),
    possibleValuesString_(possibleValuesString) {}

  virtual ~DefaultOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    option.put(optName_, optarg);
  }

  virtual std::string createPossibleValuesString() const
  {
    return possibleValuesString_;
  }
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
                          char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue, argType,
                           shortName),
    delim_(delim),
    possibleValuesString_(possibleValuesString) {}

  virtual ~CumulativeOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    std::string value = option.get(optName_);
    strappend(value, optarg, delim_);
    option.put(optName_, value);
  }

  virtual std::string createPossibleValuesString() const
  {
    return possibleValuesString_;
  }
};

class IndexOutOptionHandler : public NameMatchOptionHandler {
private:
public:
  IndexOutOptionHandler(const std::string& optName,
                        const std::string& description,
                        char shortName = 0):
    NameMatchOptionHandler(optName, description, NO_DEFAULT_VALUE,
                           OptionHandler::REQ_ARG, shortName) {}

  virtual ~IndexOutOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    // See optarg is in the fomrat of "INDEX=PATH"
    util::parseIndexPath(optarg);
    std::string value = option.get(optName_);
    strappend(value, optarg, "\n");
    option.put(optName_, value);
  }

  virtual std::string createPossibleValuesString() const
  {
    return "INDEX=PATH";
  }
};

class ParameterOptionHandler : public NameMatchOptionHandler {
private:
  std::vector<std::string> validParamValues_;
public:
  ParameterOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         const std::vector<std::string>& validParamValues,
                         char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName),
    validParamValues_(validParamValues) {}

  ParameterOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         const std::string& validParamValue,
                         char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName)
  {
    validParamValues_.push_back(validParamValue);
  }

  ParameterOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         const std::string& validParamValue1,
                         const std::string& validParamValue2,
                         char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName)
  {
    validParamValues_.push_back(validParamValue1);
    validParamValues_.push_back(validParamValue2);
  }

  ParameterOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         const std::string& validParamValue1,
                         const std::string& validParamValue2,
                         const std::string& validParamValue3,
                         char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName)
  {
    validParamValues_.push_back(validParamValue1);
    validParamValues_.push_back(validParamValue2);
    validParamValues_.push_back(validParamValue3);
  }
   
  virtual ~ParameterOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    std::vector<std::string>::const_iterator itr =
      std::find(validParamValues_.begin(), validParamValues_.end(), optarg);
    if(itr == validParamValues_.end()) {
      std::string msg = optName_;
      strappend(msg, " ", _("must be one of the following:"));
      if(validParamValues_.size() == 0) {
        msg += "''";
      } else {
        for(std::vector<std::string>::const_iterator itr =
              validParamValues_.begin(), eoi = validParamValues_.end();
            itr != eoi; ++itr) {
          strappend(msg, "'", *itr, "' ");
        }
      }
      throw DL_ABORT_EX(msg);
    } else {
      option.put(optName_, optarg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    std::stringstream s;
    std::copy(validParamValues_.begin(), validParamValues_.end(),
              std::ostream_iterator<std::string>(s, ","));
    return util::trim(s.str(), ", ");
  }
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
                        char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName),
    hostOptionName_(hostOptionName),
    portOptionName_(portOptionName) {}

  virtual ~HostPortOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    std::pair<std::string, std::string> proxy = util::split(optarg, ":");
    int32_t port = util::parseInt(proxy.second);
    if(proxy.first.empty() || proxy.second.empty() ||
       port <= 0 || 65535 < port) {
      throw DL_ABORT_EX(_("unrecognized proxy format"));
    }
    option.put(optName_, optarg);
    setHostAndPort(option, proxy.first, port);
  }

  void setHostAndPort(Option& option, const std::string& hostname, uint16_t port)
  {
    option.put(hostOptionName_, hostname);
    option.put(portOptionName_, util::uitos(port));
  }

  virtual std::string createPossibleValuesString() const
  {
    return "HOST:PORT";
  }
};

class HttpProxyUserOptionHandler:public NameMatchOptionHandler {
public:
  HttpProxyUserOptionHandler(const std::string& optName,
                             const std::string& description,
                             const std::string& defaultValue,
                             char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName)
  {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    if(util::endsWith(optName_, "-user")) {
      const std::string proxyPref = optName_.substr(0, optName_.size()-5);
      const std::string& olduri = option.get(proxyPref);
      if(!olduri.empty()) {
        Request req;
        bool b = req.setUri(olduri);
        assert(b);
        std::string uri = "http://";
        if(!optarg.empty()) {
          uri += util::percentEncode(optarg);
        }
        if(req.hasPassword()) {
          uri += A2STR::COLON_C;
          uri += util::percentEncode(req.getPassword());
        }
        if(uri.size() > 7) {
          uri += "@";
        }
        strappend(uri, req.getHost(),A2STR::COLON_C,util::uitos(req.getPort()));
        option.put(proxyPref, uri);
      }
    }
    option.put(optName_, optarg);
  }

  virtual std::string createPossibleValuesString() const
  {
    return "";
  }
};

class HttpProxyPasswdOptionHandler:public NameMatchOptionHandler {
public:
  HttpProxyPasswdOptionHandler(const std::string& optName,
                               const std::string& description,
                               const std::string& defaultValue,
                               char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName)
  {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    if(util::endsWith(optName_, "-passwd")) {
      const std::string proxyPref = optName_.substr(0, optName_.size()-7);
      const std::string& olduri = option.get(proxyPref);
      if(!olduri.empty()) {
        Request req;
        bool b = req.setUri(olduri);
        assert(b);
        std::string uri = "http://";
        if(!req.getUsername().empty()) {
          uri += util::percentEncode(req.getUsername());
        }
        uri += A2STR::COLON_C;
        if(!optarg.empty()) {
          uri += util::percentEncode(optarg);
        }
        if(uri.size() > 7) {
          uri += "@";
        }
        strappend(uri, req.getHost(), A2STR::COLON_C,util::itos(req.getPort()));
        option.put(proxyPref, uri);
      }
    }
    option.put(optName_, optarg);
  }

  virtual std::string createPossibleValuesString() const
  {
    return "";
  }
};

class HttpProxyOptionHandler : public NameMatchOptionHandler {
private:
  std::string proxyUserPref_;
  std::string proxyPasswdPref_;
public:
  HttpProxyOptionHandler(const std::string& optName,
                         const std::string& description,
                         const std::string& defaultValue,
                         char shortName = 0)
    :
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName),
    proxyUserPref_(optName_+"-user"),
    proxyPasswdPref_(optName_+"-passwd")
  {}

  virtual ~HttpProxyOptionHandler() {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    if(optarg.empty()) {
      option.put(optName_, optarg);
    } else {
      Request req;
      std::string uri;
      if(util::startsWith(optarg, "http://")) {
        uri = optarg;
      } else {
        uri = "http://"+optarg;
      }
      if(!req.setUri(uri)) {
        throw DL_ABORT_EX(_("unrecognized proxy format"));
      }
      uri = "http://";
      if(req.getUsername().empty()) {
        if(option.defined(proxyUserPref_)) {
          uri += util::percentEncode(option.get(proxyUserPref_));
        }
      } else {
        uri += util::percentEncode(req.getUsername());
      }
      if(!req.hasPassword()) {
        if(option.defined(proxyPasswdPref_)) {
          uri += A2STR::COLON_C;
          uri += util::percentEncode(option.get(proxyPasswdPref_));
        }
      } else {
        uri += A2STR::COLON_C;
        uri += util::percentEncode(req.getPassword());
      }
      if(uri.size() > 7) {
        uri += "@";
      }
      strappend(uri, req.getHost(), A2STR::COLON_C, util::uitos(req.getPort()));
      option.put(optName_, uri);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    return "[http://][USER:PASSWORD@]HOST[:PORT]";
  }
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
   char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG,
                           shortName),
    acceptStdin_(acceptStdin) {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    if(acceptStdin_ && optarg == "-") {
      option.put(optName_, DEV_STDIN);
    } else {
      File f(optarg);
      if(!f.exists() || f.isDir()) {
        throw DL_ABORT_EX
          (StringFormat(MSG_NOT_FILE, optarg.c_str()).str());
      }
      option.put(optName_, optarg);
    }
  }
  
  virtual std::string createPossibleValuesString() const
  {
    return "/path/to/file";
  }
};

class PrioritizePieceOptionHandler:public NameMatchOptionHandler {
public:
  PrioritizePieceOptionHandler
  (const std::string& optName,
   const std::string& description = NO_DESCRIPTION,
   const std::string& defaultValue = NO_DEFAULT_VALUE,
   char shortName = 0):
    NameMatchOptionHandler(optName, description, defaultValue,
                           OptionHandler::REQ_ARG, shortName) {}

  virtual void parseArg(Option& option, const std::string& optarg)
  {
    // Parse optarg against empty FileEntry list to detect syntax
    // error.
    std::vector<size_t> result;
    util::parsePrioritizePieceRange
      (result, optarg, std::vector<SharedHandle<FileEntry> >(), 1024);
    option.put(optName_, optarg);
  }

  virtual std::string createPossibleValuesString() const
  {
    return "head[=SIZE],tail[=SIZE]";
  }
};

} // namespace aria2

#endif // _D_OPTION_HANDLER_IMPL_H_
