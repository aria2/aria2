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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "NameMatchOptionHandler.h"
#include "Util.h"
#include "FatalException.h"
#include "prefs.h"
#include "Option.h"
#include "StringFormat.h"
#include "A2STR.h"
#include <utility>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iterator>

namespace aria2 {

class NullOptionHandler : public OptionHandler {
public:
  virtual ~NullOptionHandler() {}

  virtual bool canHandle(const std::string& optName) { return true; }

  virtual void parse(Option* option, const std::string& arg) {}

  virtual bool hasTag(const std::string& tag) const { return false; }

  virtual void addTag(const std::string& tag) {}

  virtual std::string toTagString() const { return A2STR::NIL; }

  virtual const std::string& getName() const { return A2STR::NIL; }

  virtual const std::string& getDescription() const { return A2STR::NIL; }

  virtual const std::string& getDefaultValue() const { return A2STR::NIL; }

  virtual std::string createPossibleValuesString() const { return A2STR::NIL; }

  virtual bool isHidden() const { return true; }
};

class BooleanOptionHandler : public NameMatchOptionHandler {
public:
  BooleanOptionHandler(const std::string& optName,
		       const std::string& description = NO_DESCRIPTION,
		       const std::string& defaultValue = NO_DEFAULT_VALUE):
    NameMatchOptionHandler(optName, description, defaultValue) {}

  virtual ~BooleanOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    if(optarg == "true") {
      option->put(_optName, V_TRUE);
    } else if(optarg == "false") {
      option->put(_optName, V_FALSE);
    } else {
      std::string msg = _optName+" "+_("must be either 'true' or 'false'.");
      throw FatalException(msg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    return "true,false";
  }
};

class IntegerRangeOptionHandler : public NameMatchOptionHandler {
private:
  int32_t _min;
  int32_t _max;
public:
  IntegerRangeOptionHandler(const std::string& optName,
			    const std::string& description,
			    const std::string& defaultValue,
			    int32_t min, int32_t max):
    NameMatchOptionHandler(optName, description, defaultValue),
    _min(min), _max(max) {}

  virtual ~IntegerRangeOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    IntSequence seq = Util::parseIntRange(optarg);
    while(seq.hasNext()) {
      int32_t v = seq.next();
      if(v < _min || _max < v) {
	std::string msg = _optName+" "+_("must be between %s and %s.");
	throw FatalException
	  (StringFormat(msg.c_str(), Util::itos(_min).c_str(),
			Util::itos(_max).c_str()).str());
      }
      option->put(_optName, optarg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    return Util::itos(_min)+"-"+Util::itos(_max);
  }
};

class NumberOptionHandler : public NameMatchOptionHandler {
private:
  int64_t _min;
  int64_t _max;
public:
  NumberOptionHandler(const std::string& optName,
		      const std::string& description = NO_DESCRIPTION,
		      const std::string& defaultValue = NO_DEFAULT_VALUE,
		      int64_t min = -1,
		      int64_t max = -1,
		      bool hidden = false):
    NameMatchOptionHandler(optName, description, defaultValue, hidden),
    _min(min), _max(max) {}

  virtual ~NumberOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    int64_t num = Util::parseLLInt(optarg);
    parseArg(option, num);
  }

  void parseArg(Option* option, int64_t number)
  {
    if((_min == -1 || _min <= number) && (_max ==  -1 || number <= _max)) {
      option->put(_optName, Util::itos(number));
    } else {
      std::string msg = _optName+" ";
      if(_min == -1 && _max != -1) {
	msg += StringFormat(_("must be smaller than or equal to %s."),
			    Util::itos(_max).c_str()).str();
      } else if(_min != -1 && _max != -1) {
	msg += StringFormat(_("must be between %s and %s."),
			    Util::itos(_min).c_str(), Util::itos(_max).c_str()).str();
      } else if(_min != -1 && _max == -1) {
	msg += StringFormat(_("must be greater than or equal to %s."),
			    Util::itos(_min).c_str()).str();
      } else {
	msg += _("must be a number.");
      }
      throw FatalException(msg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    return (_min == -1 ? "*" : Util::itos(_min))+"-"+
      (_max == -1 ? "*" : Util::itos(_max));
  }
};

class UnitNumberOptionHandler : public NumberOptionHandler {
public:
  UnitNumberOptionHandler(const std::string& optName,
			  const std::string& description = NO_DESCRIPTION,
			  const std::string& defaultValue = NO_DEFAULT_VALUE,
			  int64_t min = -1,
			  int64_t max = -1,
			  bool hidden = false):
    NumberOptionHandler(optName, description, defaultValue, min, max, hidden) {}

  virtual ~UnitNumberOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    int64_t num = Util::getRealSize(optarg);
    NumberOptionHandler::parseArg(option, num);
  }
};

class FloatNumberOptionHandler : public NameMatchOptionHandler {
private:
  double _min;
  double _max;
public:
  FloatNumberOptionHandler(const std::string& optName,
			   const std::string& description = NO_DESCRIPTION,
			   const std::string& defaultValue = NO_DEFAULT_VALUE,
			   double min = -1, double max = -1):
    NameMatchOptionHandler(optName, description, defaultValue),
    _min(min), _max(max) {}

  virtual ~FloatNumberOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    double number = strtod(optarg.c_str(), 0);
    if((_min < 0 || _min <= number) && (_max < 0 || number <= _max)) {
      option->put(_optName, optarg);
    } else {
      std::string msg = _optName+" ";
      if(_min < 0 && _max >= 0) {
	msg += StringFormat(_("must be smaller than or equal to %.1f."),
			    _max).str();
      } else if(_min >= 0 && _max >= 0) {
	msg += StringFormat(_("must be between %.1f and %.1f."),
			    _min, _max).str();
      } else if(_min >= 0 && _max < 0) {
	msg += StringFormat(_("must be greater than or equal to %.1f."),
			    _min).str();
      } else {
	msg += _("must be a number.");
      }
      throw FatalException(msg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    std::string valuesString;
    if(_min < 0) {
      valuesString += "*";
    } else {
      char buf[11];
      snprintf(buf, sizeof(buf), "%.1f", _min);
      valuesString += buf;
    }
    valuesString += "-";
    if(_max < 0) {
      valuesString += "*";
    } else {
      char buf[11];
      snprintf(buf, sizeof(buf), "%.1f", _max);
      valuesString += buf;
    }
    return valuesString;
  }
};

class DefaultOptionHandler : public NameMatchOptionHandler {
private:
  std::string _possibleValuesString;
public:
  DefaultOptionHandler(const std::string& optName,
		       const std::string& description = NO_DESCRIPTION,
		       const std::string& defaultValue = NO_DEFAULT_VALUE,
		       const std::string& possibleValuesString = A2STR::NIL,
		       bool hidden = false):
    NameMatchOptionHandler(optName, description, defaultValue, hidden),
    _possibleValuesString(possibleValuesString) {}

  virtual ~DefaultOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    option->put(_optName, optarg);
  }

  virtual std::string createPossibleValuesString() const
  {
    return _possibleValuesString;
  }
};

class CumulativeOptionHandler : public NameMatchOptionHandler {
private:
  std::string _delim;

  std::string _possibleValuesString;
public:
  CumulativeOptionHandler(const std::string& optName,
			  const std::string& description,
			  const std::string& defaultValue,
			  const std::string& delim,
			  const std::string& possibleValuesString = A2STR::NIL):
    NameMatchOptionHandler(optName, description, defaultValue),
    _delim(delim),
    _possibleValuesString(possibleValuesString) {}

  virtual ~CumulativeOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    std::string value = option->get(_optName);
    value += optarg+_delim;
    option->put(_optName, value);
  }

  virtual std::string createPossibleValuesString() const
  {
    return _possibleValuesString;
  }
};

class ParameterOptionHandler : public NameMatchOptionHandler {
private:
  std::deque<std::string> _validParamValues;
public:
  ParameterOptionHandler(const std::string& optName,
			 const std::string& description,
			 const std::string& defaultValue,
			 const std::deque<std::string>& validParamValues):
    NameMatchOptionHandler(optName, description, defaultValue),
    _validParamValues(validParamValues) {}

  ParameterOptionHandler(const std::string& optName,
			 const std::string& description,
			 const std::string& defaultValue,
			 const std::string& validParamValue):
    NameMatchOptionHandler(optName, description, defaultValue)
  {
    _validParamValues.push_back(validParamValue);
  }

  ParameterOptionHandler(const std::string& optName,
			 const std::string& description,
			 const std::string& defaultValue,
			 const std::string& validParamValue1,
			 const std::string& validParamValue2):
    NameMatchOptionHandler(optName, description, defaultValue)
  {
    _validParamValues.push_back(validParamValue1);
    _validParamValues.push_back(validParamValue2);
  }

  ParameterOptionHandler(const std::string& optName,
			 const std::string& description,
			 const std::string& defaultValue,
			 const std::string& validParamValue1,
			 const std::string& validParamValue2,
			 const std::string& validParamValue3):
    NameMatchOptionHandler(optName, description, defaultValue)
  {
    _validParamValues.push_back(validParamValue1);
    _validParamValues.push_back(validParamValue2);
    _validParamValues.push_back(validParamValue3);
  }
   
  virtual ~ParameterOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    std::deque<std::string>::const_iterator itr =
      std::find(_validParamValues.begin(), _validParamValues.end(), optarg);
    if(itr == _validParamValues.end()) {
      std::string msg = _optName+" "+_("must be one of the following:");
      if(_validParamValues.size() == 0) {
	msg += "''";
      } else {
	for(std::deque<std::string>::const_iterator itr = _validParamValues.begin();
	    itr != _validParamValues.end(); ++itr) {
	  msg += "'"+*itr+"' ";
	}
      }
      throw FatalException(msg);
    } else {
      option->put(_optName, optarg);
    }
  }

  virtual std::string createPossibleValuesString() const
  {
    std::stringstream s;
    std::copy(_validParamValues.begin(), _validParamValues.end(),
	      std::ostream_iterator<std::string>(s, ","));
    return Util::trim(s.str(), ", ");
  }
};

class HostPortOptionHandler : public NameMatchOptionHandler {
private:
  std::string _hostOptionName;
  
  std::string _portOptionName;
public:
  HostPortOptionHandler(const std::string& optName,
			const std::string& description,
			const std::string& defaultValue,
			const std::string& hostOptionName,
			const std::string& portOptionName):
    NameMatchOptionHandler(optName, description, defaultValue),
    _hostOptionName(hostOptionName),
    _portOptionName(portOptionName) {}

  virtual ~HostPortOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    std::pair<std::string, std::string> proxy = Util::split(optarg, ":");
    int32_t port = Util::parseInt(proxy.second);
    if(proxy.first.empty() || proxy.second.empty() ||
       port <= 0 || 65535 < port) {
      throw FatalException(_("unrecognized proxy format"));
    }
    option->put(_optName, optarg);
    setHostAndPort(option, proxy.first, port);
  }

  void setHostAndPort(Option* option, const std::string& hostname, uint16_t port)
  {
    option->put(_hostOptionName, hostname);
    option->put(_portOptionName, Util::uitos(port));
  }

  virtual std::string createPossibleValuesString() const
  {
    return "HOST:PORT";
  }
};

class HttpProxyOptionHandler : public HostPortOptionHandler {
public:
  HttpProxyOptionHandler(const std::string& optName,
			 const std::string& description,
			 const std::string& defaultValue,
			 const std::string& hostOptionName,
			 const std::string& portOptionName):
    HostPortOptionHandler(optName, description, defaultValue,
			  hostOptionName, portOptionName)
  {}

  virtual ~HttpProxyOptionHandler() {}

  virtual void parseArg(Option* option, const std::string& optarg)
  {
    HostPortOptionHandler::parseArg(option, optarg);
    option->put(PREF_HTTP_PROXY_ENABLED, V_TRUE);
  }
};

} // namespace aria2

#endif // _D_OPTION_HANDLER_IMPL_H_
