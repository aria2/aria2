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
#include "OptionHandlerImpl.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <utility>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iterator>
#include <vector>

#include "util.h"
#include "DlAbortEx.h"
#include "prefs.h"
#include "Option.h"
#include "fmt.h"
#include "A2STR.h"
#include "Request.h"
#include "a2functional.h"
#include "message.h"
#include "File.h"
#include "FileEntry.h"
#include "a2io.h"
#include "LogFactory.h"
#include "uri.h"
#include "SegList.h"
#include "array_fun.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

BooleanOptionHandler::BooleanOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 OptionHandler::ARG_TYPE argType,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          argType, shortName)
{}

BooleanOptionHandler::~BooleanOptionHandler() {}

void BooleanOptionHandler::parseArg(Option& option, const std::string& optarg)
{
  if(optarg == "true" ||
     ((argType_ == OptionHandler::OPT_ARG ||
       argType_ == OptionHandler::NO_ARG)
      && optarg.empty())) {
    option.put(pref_, A2_V_TRUE);
  } else if(optarg == "false") {
    option.put(pref_, A2_V_FALSE);
  } else {
    std::string msg = pref_->k;
    msg += " ";
    msg += _("must be either 'true' or 'false'.");
    throw DL_ABORT_EX(msg);
  }
}

std::string BooleanOptionHandler::createPossibleValuesString() const
{
  return "true, false";
}

IntegerRangeOptionHandler::IntegerRangeOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 int32_t min, int32_t max,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName),
    min_(min),
    max_(max)
{}

IntegerRangeOptionHandler::~IntegerRangeOptionHandler() {}

void IntegerRangeOptionHandler::parseArg
(Option& option, const std::string& optarg)
{
  SegList<int> sgl;
  util::parseIntSegments(sgl, optarg);
  sgl.normalize();
  while(sgl.hasNext()) {
    int v = sgl.next();
    if(v < min_ || max_ < v) {
      std::string msg = pref_->k;
      msg += " ";
      msg += _("must be between %d and %d.");
      throw DL_ABORT_EX(fmt(msg.c_str(), min_, max_));
    }
    option.put(pref_, optarg);
  }
}

std::string IntegerRangeOptionHandler::createPossibleValuesString() const
{
  return fmt("%d-%d", min_, max_);
}

NumberOptionHandler::NumberOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 int64_t min,
 int64_t max,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName),
    min_(min),
    max_(max)
{}

NumberOptionHandler::~NumberOptionHandler() {}

void NumberOptionHandler::parseArg(Option& option, const std::string& optarg)
{
  parseArg(option, util::parseLLInt(optarg));
}

void NumberOptionHandler::parseArg(Option& option, int64_t number)
{
  if((min_ == -1 || min_ <= number) && (max_ ==  -1 || number <= max_)) {
    option.put(pref_, util::itos(number));
  } else {
    std::string msg = pref_->k;
    msg += " ";
    if(min_ == -1 && max_ != -1) {
      msg += fmt(_("must be smaller than or equal to %" PRId64 "."), max_);
    } else if(min_ != -1 && max_ != -1) {
      msg += fmt(_("must be between %" PRId64 " and %" PRId64 "."),
                 min_, max_);
    } else if(min_ != -1 && max_ == -1) {
      msg += fmt(_("must be greater than or equal to %" PRId64 "."), min_);
    } else {
      msg += _("must be a number.");
    }
    throw DL_ABORT_EX(msg);
  }
}

std::string NumberOptionHandler::createPossibleValuesString() const
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

UnitNumberOptionHandler::UnitNumberOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 int64_t min,
 int64_t max,
 char shortName)
  : NumberOptionHandler(pref, description, defaultValue, min, max,
                        shortName)
{}

UnitNumberOptionHandler::~UnitNumberOptionHandler() {}

void UnitNumberOptionHandler::parseArg
(Option& option, const std::string& optarg)
{
  int64_t num = util::getRealSize(optarg);
  NumberOptionHandler::parseArg(option, num);
}

FloatNumberOptionHandler::FloatNumberOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 double min,
 double max,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName),
    min_(min),
    max_(max)
{}

FloatNumberOptionHandler::~FloatNumberOptionHandler() {}

void FloatNumberOptionHandler::parseArg
(Option& option, const std::string& optarg)
{
  double number = strtod(optarg.c_str(), 0);
  if((min_ < 0 || min_ <= number) && (max_ < 0 || number <= max_)) {
    option.put(pref_, optarg);
  } else {
    std::string msg = pref_->k;
    msg += " ";
    if(min_ < 0 && max_ >= 0) {
      msg += fmt(_("must be smaller than or equal to %.1f."), max_);
    } else if(min_ >= 0 && max_ >= 0) {
      msg += fmt(_("must be between %.1f and %.1f."), min_, max_);
    } else if(min_ >= 0 && max_ < 0) {
      msg += fmt(_("must be greater than or equal to %.1f."), min_);
    } else {
      msg += _("must be a number.");
    }
    throw DL_ABORT_EX(msg);
  }
}

std::string FloatNumberOptionHandler::createPossibleValuesString() const
{
  std::string valuesString;
  if(min_ < 0) {
    valuesString += "*";
  } else {
    valuesString += fmt("%.1f", min_);
  }
  valuesString += "-";
  if(max_ < 0) {
    valuesString += "*";
  } else {
    valuesString += fmt("%.1f", max_);
  }
  return valuesString;
}

DefaultOptionHandler::DefaultOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 const std::string& possibleValuesString,
 OptionHandler::ARG_TYPE argType,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue, argType,
                          shortName),
    possibleValuesString_(possibleValuesString)
{}

DefaultOptionHandler::~DefaultOptionHandler() {}

void DefaultOptionHandler::parseArg(Option& option, const std::string& optarg)
{
  option.put(pref_, optarg);
}

std::string DefaultOptionHandler::createPossibleValuesString() const
{
  return possibleValuesString_;
}

CumulativeOptionHandler::CumulativeOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 const std::string& delim,
 const std::string& possibleValuesString,
 OptionHandler::ARG_TYPE argType,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue, argType,
                          shortName),
    delim_(delim),
    possibleValuesString_(possibleValuesString)
{}

CumulativeOptionHandler::~CumulativeOptionHandler() {}

void CumulativeOptionHandler::parseArg
(Option& option, const std::string& optarg)
{
  std::string value = option.get(pref_);
  value += optarg;
  value += delim_;
  option.put(pref_, value);
}

std::string CumulativeOptionHandler::createPossibleValuesString() const
{
  return possibleValuesString_;
}

IndexOutOptionHandler::IndexOutOptionHandler
(const Pref* pref,
 const char* description,
 char shortName)
  : AbstractOptionHandler(pref, description, NO_DEFAULT_VALUE,
                          OptionHandler::REQ_ARG, shortName)
{}

IndexOutOptionHandler::~IndexOutOptionHandler() {}

void IndexOutOptionHandler::parseArg(Option& option, const std::string& optarg)
{
  // See optarg is in the fomrat of "INDEX=PATH"
  util::parseIndexPath(optarg);
  std::string value = option.get(pref_);
  value += optarg;
  value += "\n";
  option.put(pref_, value);
}

std::string IndexOutOptionHandler::createPossibleValuesString() const
{
  return "INDEX=PATH";
}

#ifdef ENABLE_MESSAGE_DIGEST
ChecksumOptionHandler::ChecksumOptionHandler
(const Pref* pref,
 const char* description,
 char shortName)
  : AbstractOptionHandler(pref, description, NO_DEFAULT_VALUE,
                          OptionHandler::REQ_ARG, shortName)
{}

ChecksumOptionHandler::~ChecksumOptionHandler() {}

void ChecksumOptionHandler::parseArg(Option& option, const std::string& optarg)
{
  std::pair<Scip, Scip> p;
  util::divide(p, optarg.begin(), optarg.end(), '=');
  std::string hashType(p.first.first, p.first.second);
  std::string hexDigest(p.second.first, p.second.second);
  util::lowercase(hashType);
  util::lowercase(hexDigest);
  if(!MessageDigest::isValidHash(hashType, hexDigest)) {
    throw DL_ABORT_EX(_("Unrecognized checksum"));
  }
  option.put(pref_, optarg);
}

std::string ChecksumOptionHandler::createPossibleValuesString() const
{
  return "HASH_TYPE=HEX_DIGEST";
}
#endif // ENABLE_MESSAGE_DIGEST

ParameterOptionHandler::ParameterOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 const std::vector<std::string>& validParamValues,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName),
    validParamValues_(validParamValues)
{}

ParameterOptionHandler::ParameterOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 const std::string& validParamValue,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName)
{
  validParamValues_.push_back(validParamValue);
}

ParameterOptionHandler::ParameterOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 const std::string& validParamValue1,
 const std::string& validParamValue2,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName)
{
  validParamValues_.push_back(validParamValue1);
  validParamValues_.push_back(validParamValue2);
}

ParameterOptionHandler::ParameterOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 const std::string& validParamValue1,
 const std::string& validParamValue2,
 const std::string& validParamValue3,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName)
{
  validParamValues_.push_back(validParamValue1);
  validParamValues_.push_back(validParamValue2);
  validParamValues_.push_back(validParamValue3);
}
   
ParameterOptionHandler::~ParameterOptionHandler() {}

void ParameterOptionHandler::parseArg(Option& option, const std::string& optarg)
{
  std::vector<std::string>::const_iterator itr =
    std::find(validParamValues_.begin(), validParamValues_.end(), optarg);
  if(itr == validParamValues_.end()) {
    std::string msg = pref_->k;
    msg += " ";
    msg += _("must be one of the following:");
    if(validParamValues_.size() == 0) {
      msg += "''";
    } else {
      for(std::vector<std::string>::const_iterator itr =
            validParamValues_.begin(), eoi = validParamValues_.end();
          itr != eoi; ++itr) {
        msg += "'";
        msg += *itr;
        msg += "' ";
      }
    }
    throw DL_ABORT_EX(msg);
  } else {
    option.put(pref_, optarg);
  }
}

std::string ParameterOptionHandler::createPossibleValuesString() const
{
  std::stringstream s;
  std::copy(validParamValues_.begin(), validParamValues_.end(),
            std::ostream_iterator<std::string>(s, ", "));
  return util::strip(s.str(), ", ");
}

HostPortOptionHandler::HostPortOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 const Pref* hostOptionName,
 const Pref* portOptionName,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName),
    hostOptionName_(hostOptionName),
    portOptionName_(portOptionName)
{}

HostPortOptionHandler::~HostPortOptionHandler() {}

void HostPortOptionHandler::parseArg(Option& option, const std::string& optarg)
{
  std::string uri = "http://";
  uri += optarg;
  Request req;
  if(!req.setUri(uri)) {
    throw DL_ABORT_EX(_("Unrecognized format"));
  }
  option.put(pref_, optarg);
  setHostAndPort(option, req.getHost(), req.getPort());
}

void HostPortOptionHandler::setHostAndPort
(Option& option, const std::string& hostname, uint16_t port)
{
  option.put(hostOptionName_, hostname);
  option.put(portOptionName_, util::uitos(port));
}

std::string HostPortOptionHandler::createPossibleValuesString() const
{
  return "HOST:PORT";
}

HttpProxyOptionHandler::HttpProxyOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName),
    proxyUserPref_(option::k2p(std::string(pref->k)+"-user")),
    proxyPasswdPref_(option::k2p(std::string(pref->k)+"-passwd"))
{}

HttpProxyOptionHandler::~HttpProxyOptionHandler() {}

void HttpProxyOptionHandler::parseArg(Option& option, const std::string& optarg)
{
  if(optarg.empty()) {
    option.put(pref_, optarg);
  } else {
    std::string uri;
    if(util::startsWith(optarg, "http://") ||
       util::startsWith(optarg, "https://") ||
       util::startsWith(optarg, "ftp://")) {
      uri = optarg;
    } else {
      uri = "http://";
      uri += optarg;
    }
    uri::UriStruct us;
    if(!uri::parse(us, uri)) {
      throw DL_ABORT_EX(_("unrecognized proxy format"));
    }
    us.protocol = "http";
    option.put(pref_, uri::construct(us));
  }
}

std::string HttpProxyOptionHandler::createPossibleValuesString() const
{
  return "[http://][USER:PASSWORD@]HOST[:PORT]";
}

LocalFilePathOptionHandler::LocalFilePathOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 bool acceptStdin,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName),
    acceptStdin_(acceptStdin)
{}

void LocalFilePathOptionHandler::parseArg
(Option& option, const std::string& optarg)
{
  if(acceptStdin_ && optarg == "-") {
    option.put(pref_, DEV_STDIN);
  } else {
    File f(optarg);
    if(!f.exists() || f.isDir()) {
      throw DL_ABORT_EX(fmt(MSG_NOT_FILE, optarg.c_str()));
    }
    option.put(pref_, optarg);
  }
}
  
std::string LocalFilePathOptionHandler::createPossibleValuesString() const
{
  if(acceptStdin_) {
    return PATH_TO_FILE_STDIN;
  } else {
    return PATH_TO_FILE;
  }
}

PrioritizePieceOptionHandler::PrioritizePieceOptionHandler
(const Pref* pref,
 const char* description,
 const std::string& defaultValue,
 char shortName)
  : AbstractOptionHandler(pref, description, defaultValue,
                          OptionHandler::REQ_ARG, shortName)
{}

void PrioritizePieceOptionHandler::parseArg
(Option& option, const std::string& optarg)
{
  // Parse optarg against empty FileEntry list to detect syntax
  // error.
  std::vector<size_t> result;
  util::parsePrioritizePieceRange
    (result, optarg, std::vector<SharedHandle<FileEntry> >(), 1024);
  option.put(pref_, optarg);
}

std::string PrioritizePieceOptionHandler::createPossibleValuesString() const
{
  return "head[=SIZE], tail[=SIZE]";
}

DeprecatedOptionHandler::DeprecatedOptionHandler
(const SharedHandle<OptionHandler>& depOptHandler,
 const SharedHandle<OptionHandler>& repOptHandler)
  : depOptHandler_(depOptHandler), repOptHandler_(repOptHandler)
{}

void DeprecatedOptionHandler::parse(Option& option, const std::string& arg)
{
  if(repOptHandler_) {
    A2_LOG_WARN(fmt(_("--%s option is deprecated. Use --%s option instead."),
                    depOptHandler_->getName(),
                    repOptHandler_->getName()));
    repOptHandler_->parse(option, arg);
  } else {
    A2_LOG_WARN(fmt(_("--%s option is deprecated."),
                    depOptHandler_->getName()));
  }
}

std::string DeprecatedOptionHandler::createPossibleValuesString() const
{
  return depOptHandler_->createPossibleValuesString();
}

bool DeprecatedOptionHandler::hasTag(const std::string& tag) const
{
  return depOptHandler_->hasTag(tag);
}

void DeprecatedOptionHandler::addTag(const std::string& tag)
{
  depOptHandler_->addTag(tag);
}

std::string DeprecatedOptionHandler::toTagString() const
{
  return depOptHandler_->toTagString();
}

const char* DeprecatedOptionHandler::getName() const
{
  return depOptHandler_->getName();
}

const char* DeprecatedOptionHandler::getDescription() const
{
  return depOptHandler_->getDescription();
}

const std::string& DeprecatedOptionHandler::getDefaultValue() const
{
  return depOptHandler_->getDefaultValue();
}

bool DeprecatedOptionHandler::isHidden() const
{
  return depOptHandler_->isHidden();
}

void DeprecatedOptionHandler::hide()
{
  depOptHandler_->hide();
}

const Pref* DeprecatedOptionHandler::getPref() const
{
  return depOptHandler_->getPref();
}

OptionHandler::ARG_TYPE DeprecatedOptionHandler::getArgType() const
{
  return depOptHandler_->getArgType();
}

char DeprecatedOptionHandler::getShortName() const
{
  return depOptHandler_->getShortName();
}

bool DeprecatedOptionHandler::getEraseAfterParse() const
{
  return depOptHandler_->getEraseAfterParse();
}

void DeprecatedOptionHandler::setEraseAfterParse(bool eraseAfterParse)
{
  depOptHandler_->setEraseAfterParse(eraseAfterParse);
}

bool DeprecatedOptionHandler::getInitialOption() const
{
  return depOptHandler_->getInitialOption();
}

void DeprecatedOptionHandler::setInitialOption(bool f)
{
  depOptHandler_->setInitialOption(f);
}

bool DeprecatedOptionHandler::getChangeOption() const
{
  return depOptHandler_->getChangeOption();
}

void DeprecatedOptionHandler::setChangeOption(bool f)
{
  depOptHandler_->setChangeOption(f);
}

bool DeprecatedOptionHandler::getChangeOptionForReserved() const
{
  return depOptHandler_->getChangeOptionForReserved();
}

void DeprecatedOptionHandler::setChangeOptionForReserved(bool f)
{
  depOptHandler_->setChangeOptionForReserved(f);
}

bool DeprecatedOptionHandler::getChangeGlobalOption() const
{
  return depOptHandler_->getChangeGlobalOption();
}

void DeprecatedOptionHandler::setChangeGlobalOption(bool f)
{
  depOptHandler_->setChangeGlobalOption(f);
}

bool DeprecatedOptionHandler::getCumulative() const
{
  return depOptHandler_->getCumulative();
}

void DeprecatedOptionHandler::setCumulative(bool f)
{
  depOptHandler_->setCumulative(f);
}

} // namespace aria2
