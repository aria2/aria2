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
#include "DlAbortEx.h"
#include "FatalException.h"
#include "prefs.h"

class NullOptionHandler : public OptionHandler {
public:
  virtual ~NullOptionHandler() {}

  virtual bool canHandle(const string& optName) { return true; }

  virtual void parse(Option* option, const string& arg) {}
};

class BooleanOptionHandler : public NameMatchOptionHandler {
public:
  BooleanOptionHandler(const string& optName):NameMatchOptionHandler(optName) {}
  virtual ~BooleanOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
  {
    if(optarg == "true") {
      option->put(_optName, V_TRUE);
    } else if(optarg == "false") {
      option->put(_optName, V_FALSE);
    } else {
      string msg = _optName+" "+_("must be either 'true' or 'false'.");
      throw new FatalException(msg.c_str());
    }
  }
};

class IntegerRangeOptionHandler : public NameMatchOptionHandler {
private:
  int32_t _min;
  int32_t _max;
public:
  IntegerRangeOptionHandler(const string& optName, int32_t min, int32_t max):NameMatchOptionHandler(optName), _min(min), _max(max) {}

  virtual ~IntegerRangeOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
  {
    IntSequence seq = Util::parseIntRange(optarg);
    while(seq.hasNext()) {
      int32_t v = seq.next();
      if(v < _min || _max < v) {
	string msg = _optName+" "+_("must be between %s and %s.");
	throw new DlAbortEx(msg.c_str(), Util::llitos(_min).c_str(), Util::llitos(_max).c_str());
      }
      option->put(_optName, optarg);
    }
  }
};

class NumberOptionHandler : public NameMatchOptionHandler {
private:
  int64_t _min;
  int64_t _max;
public:
  NumberOptionHandler(const string& optName, int64_t min = -1, int64_t max = -1):NameMatchOptionHandler(optName), _min(min), _max(max) {}

  virtual ~NumberOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
  {
    int64_t num = Util::parseLLInt(optarg);
    parseArg(option, num);
  }

  void parseArg(Option* option, int64_t number)
  {
    if((_min == -1 || _min <= number) && (_max ==  -1 || number <= _max)) {
      option->put(_optName, Util::llitos(number));
    } else {
      string msg = _optName+" ";
      if(_min == -1 && _max != -1) {
	msg += _("must be smaller than or equal to %s.");
	throw new FatalException(msg.c_str(), Util::llitos(_max).c_str());
      } else if(_min != -1 && _max != -1) {
	msg += _("must be between %s and %s.");
	throw new FatalException(msg.c_str(), Util::llitos(_min).c_str(), Util::llitos(_max).c_str());
      } else if(_min != -1 && _max == -1) {
	msg += _("must be greater than or equal to %s.");
	throw new FatalException(msg.c_str(), Util::llitos(_min).c_str());
      } else {
	msg += _("must be a number.");
	throw new FatalException(msg.c_str());
      }
    }
  }
};

class UnitNumberOptionHandler : public NumberOptionHandler {
public:
  UnitNumberOptionHandler(const string& optName, int64_t min = -1, int64_t max = -1):NumberOptionHandler(optName, min, max) {}

  virtual ~UnitNumberOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
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
  FloatNumberOptionHandler(const string& optName, double min = -1, double max = -1):NameMatchOptionHandler(optName), _min(min), _max(max) {}

  virtual ~FloatNumberOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
  {
    double number = strtod(optarg.c_str(), 0);
    if((_min < 0 || _min <= number) && (_max < 0 || number <= _max)) {
      option->put(_optName, optarg);
    } else {
      string msg = _optName+" ";
      if(_min < 0 && _max >= 0) {
	msg += _("must be smaller than or equal to %.1f.");
	throw new FatalException(msg.c_str(), _max);
      } else if(_min >= 0 && _max >= 0) {
	msg += _("must be between %.1f and %.1f.");
	throw new FatalException(msg.c_str(), _min, _max);
      } else if(_min >= 0 && _max < 0) {
	msg += _("must be greater than or equal to %.1f.");
	throw new FatalException(msg.c_str(), _min);
      } else {
	msg += _("must be a number.");
	throw new FatalException(msg.c_str());
      }
    }
  }
};

class DefaultOptionHandler : public NameMatchOptionHandler {
public:
  DefaultOptionHandler(const string& optName):NameMatchOptionHandler(optName) {}

  virtual ~DefaultOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
  {
    option->put(_optName, optarg);
  }
};

class ParameterOptionHandler : public NameMatchOptionHandler {
private:
  Strings _validParamValues;
public:
  ParameterOptionHandler(const string& optName, const Strings& validParamValues):
    NameMatchOptionHandler(optName), _validParamValues(validParamValues) {}

  ParameterOptionHandler(const string& optName, const string& validParamValue):
    NameMatchOptionHandler(optName)
  {
    _validParamValues.push_back(validParamValue);
  }

  ParameterOptionHandler(const string& optName,
			 const string& validParamValue1,
			 const string& validParamValue2):
    NameMatchOptionHandler(optName)
  {
    _validParamValues.push_back(validParamValue1);
    _validParamValues.push_back(validParamValue2);
  }

  ParameterOptionHandler(const string& optName,
			 const string& validParamValue1,
			 const string& validParamValue2,
			 const string& validParamValue3):
    NameMatchOptionHandler(optName)
  {
    _validParamValues.push_back(validParamValue1);
    _validParamValues.push_back(validParamValue2);
    _validParamValues.push_back(validParamValue3);
  }
   
  virtual ~ParameterOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
  {
    Strings::const_iterator itr = find(_validParamValues.begin(), _validParamValues.end(), optarg);
    if(itr == _validParamValues.end()) {
      string msg = _optName+" "+_("must be one of the following:");
      if(_validParamValues.size() == 0) {
	msg += "''";
      } else {
	for(Strings::const_iterator itr = _validParamValues.begin();
	    itr != _validParamValues.end(); ++itr) {
	  msg += "'"+*itr+"' ";
	}
      }
      throw new FatalException(msg.c_str());
    } else {
      option->put(_optName, optarg);
    }
  }
};

class HttpProxyOptionHandler : public NameMatchOptionHandler {
public:
  HttpProxyOptionHandler(const string& optName):NameMatchOptionHandler(optName) {}

  virtual ~HttpProxyOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
  {
    pair<string, string> proxy = Util::split(optarg, ":");
    int32_t port = Util::parseInt(proxy.second);
    if(proxy.first.empty() || proxy.second.empty() ||
       port <= 0 || 65535 < port) {
      throw new FatalException(_("unrecognized proxy format"));
    }
    option->put(PREF_HTTP_PROXY, optarg);
    option->put(PREF_HTTP_PROXY_HOST, proxy.first);
    option->put(PREF_HTTP_PROXY_PORT, Util::itos(port));
    option->put(PREF_HTTP_PROXY_ENABLED, V_TRUE);
  }
};

class LogOptionHandler : public NameMatchOptionHandler {
public:
  LogOptionHandler(const string& optName):NameMatchOptionHandler(optName) {}

  virtual ~LogOptionHandler() {}

  virtual void parseArg(Option* option, const string& optarg)
  {
    if("-" == optarg) {
      option->put(PREF_STDOUT_LOG, V_TRUE);
    } else {
      option->put(PREF_LOG, optarg);
    }
  }
};

#endif // _D_OPTION_HANDLER_IMPL_H_
