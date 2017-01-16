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
#include <stdexcept>

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
#include "help_tags.h"
#include "MessageDigest.h"

namespace aria2 {

BooleanOptionHandler::BooleanOptionHandler(PrefPtr pref,
                                           const char* description,
                                           const std::string& defaultValue,
                                           OptionHandler::ARG_TYPE argType,
                                           char shortName)
    : AbstractOptionHandler(pref, description, defaultValue, argType, shortName)
{
}

BooleanOptionHandler::~BooleanOptionHandler() = default;

void BooleanOptionHandler::parseArg(Option& option,
                                    const std::string& optarg) const
{
  if (optarg == "true" || ((argType_ == OptionHandler::OPT_ARG ||
                            argType_ == OptionHandler::NO_ARG) &&
                           optarg.empty())) {
    option.put(pref_, A2_V_TRUE);
  }
  else if (optarg == "false") {
    option.put(pref_, A2_V_FALSE);
  }
  else {
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

IntegerRangeOptionHandler::IntegerRangeOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    int32_t min, int32_t max, char shortName)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::REQ_ARG, shortName),
      min_(min),
      max_(max)
{
}

IntegerRangeOptionHandler::~IntegerRangeOptionHandler() = default;

void IntegerRangeOptionHandler::parseArg(Option& option,
                                         const std::string& optarg) const
{
  auto sgl = util::parseIntSegments(optarg);
  sgl.normalize();
  while (sgl.hasNext()) {
    int v = sgl.next();
    if (v < min_ || max_ < v) {
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

NumberOptionHandler::NumberOptionHandler(PrefPtr pref, const char* description,
                                         const std::string& defaultValue,
                                         int64_t min, int64_t max,
                                         char shortName)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::REQ_ARG, shortName),
      min_(min),
      max_(max)
{
}

NumberOptionHandler::~NumberOptionHandler() = default;

void NumberOptionHandler::parseArg(Option& option,
                                   const std::string& optarg) const
{
  int64_t number;
  if (util::parseLLIntNoThrow(number, optarg)) {
    parseArg(option, number);
  }
  else {
    throw DL_ABORT_EX(fmt("Bad number %s", optarg.c_str()));
  }
}

void NumberOptionHandler::parseArg(Option& option, int64_t number) const
{
  if ((min_ == -1 || min_ <= number) && (max_ == -1 || number <= max_)) {
    option.put(pref_, util::itos(number));
    return;
  }

  std::string msg = pref_->k;
  msg += " ";
  if (min_ == -1 && max_ != -1) {
    msg += fmt(_("must be smaller than or equal to %" PRId64 "."), max_);
  }
  else if (min_ != -1 && max_ != -1) {
    msg += fmt(_("must be between %" PRId64 " and %" PRId64 "."), min_, max_);
  }
  else if (min_ != -1 && max_ == -1) {
    msg += fmt(_("must be greater than or equal to %" PRId64 "."), min_);
  }
  else {
    msg += _("must be a number.");
  }
  throw DL_ABORT_EX(msg);
}

std::string NumberOptionHandler::createPossibleValuesString() const
{
  std::string values;
  if (min_ == -1) {
    values += "*";
  }
  else {
    values += util::itos(min_);
  }
  values += "-";
  if (max_ == -1) {
    values += "*";
  }
  else {
    values += util::itos(max_);
  }
  return values;
}

UnitNumberOptionHandler::UnitNumberOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    int64_t min, int64_t max, char shortName)
    : NumberOptionHandler(pref, description, defaultValue, min, max, shortName)
{
}

UnitNumberOptionHandler::~UnitNumberOptionHandler() = default;

void UnitNumberOptionHandler::parseArg(Option& option,
                                       const std::string& optarg) const
{
  int64_t num = util::getRealSize(optarg);
  NumberOptionHandler::parseArg(option, num);
}

FloatNumberOptionHandler::FloatNumberOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    double min, double max, char shortName)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::REQ_ARG, shortName),
      min_(min),
      max_(max)
{
}

FloatNumberOptionHandler::~FloatNumberOptionHandler() = default;

void FloatNumberOptionHandler::parseArg(Option& option,
                                        const std::string& optarg) const
{
  double number = strtod(optarg.c_str(), nullptr);
  if ((min_ < 0 || min_ <= number) && (max_ < 0 || number <= max_)) {
    option.put(pref_, optarg);
  }
  else {
    std::string msg = pref_->k;
    msg += " ";
    if (min_ < 0 && max_ >= 0) {
      msg += fmt(_("must be smaller than or equal to %.1f."), max_);
    }
    else if (min_ >= 0 && max_ >= 0) {
      msg += fmt(_("must be between %.1f and %.1f."), min_, max_);
    }
    else if (min_ >= 0 && max_ < 0) {
      msg += fmt(_("must be greater than or equal to %.1f."), min_);
    }
    else {
      msg += _("must be a number.");
    }
    throw DL_ABORT_EX(msg);
  }
}

std::string FloatNumberOptionHandler::createPossibleValuesString() const
{
  std::string valuesString;
  if (min_ < 0) {
    valuesString += "*";
  }
  else {
    valuesString += fmt("%.1f", min_);
  }
  valuesString += "-";
  if (max_ < 0) {
    valuesString += "*";
  }
  else {
    valuesString += fmt("%.1f", max_);
  }
  return valuesString;
}

DefaultOptionHandler::DefaultOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    const std::string& possibleValuesString, OptionHandler::ARG_TYPE argType,
    char shortName)
    : AbstractOptionHandler(pref, description, defaultValue, argType,
                            shortName),
      possibleValuesString_(possibleValuesString),
      allowEmpty_(true)
{
}

DefaultOptionHandler::~DefaultOptionHandler() = default;

void DefaultOptionHandler::parseArg(Option& option,
                                    const std::string& optarg) const
{
  if (!allowEmpty_ && optarg.empty()) {
    throw DL_ABORT_EX("Empty string is not allowed");
  }
  option.put(pref_, optarg);
}

std::string DefaultOptionHandler::createPossibleValuesString() const
{
  return possibleValuesString_;
}

void DefaultOptionHandler::setAllowEmpty(bool allow) { allowEmpty_ = allow; }

CumulativeOptionHandler::CumulativeOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    const std::string& delim, const std::string& possibleValuesString,
    OptionHandler::ARG_TYPE argType, char shortName)
    : AbstractOptionHandler(pref, description, defaultValue, argType,
                            shortName),
      delim_(delim),
      possibleValuesString_(possibleValuesString)
{
}

CumulativeOptionHandler::~CumulativeOptionHandler() = default;

void CumulativeOptionHandler::parseArg(Option& option,
                                       const std::string& optarg) const
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

IndexOutOptionHandler::IndexOutOptionHandler(PrefPtr pref,
                                             const char* description,
                                             char shortName)
    : AbstractOptionHandler(pref, description, NO_DEFAULT_VALUE,
                            OptionHandler::REQ_ARG, shortName)
{
}

IndexOutOptionHandler::~IndexOutOptionHandler() = default;

void IndexOutOptionHandler::parseArg(Option& option,
                                     const std::string& optarg) const
{
  // See optarg is in the format of "INDEX=PATH"
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

ChecksumOptionHandler::ChecksumOptionHandler(PrefPtr pref,
                                             const char* description,
                                             char shortName)
    : AbstractOptionHandler(pref, description, NO_DEFAULT_VALUE,
                            OptionHandler::REQ_ARG, shortName)
{
}

ChecksumOptionHandler::ChecksumOptionHandler(
    PrefPtr pref, const char* description,
    std::vector<std::string> acceptableTypes, char shortName)
    : AbstractOptionHandler(pref, description, NO_DEFAULT_VALUE,
                            OptionHandler::REQ_ARG, shortName),
      acceptableTypes_(std::move(acceptableTypes))
{
}

ChecksumOptionHandler::~ChecksumOptionHandler() = default;

void ChecksumOptionHandler::parseArg(Option& option,
                                     const std::string& optarg) const
{
  auto p = util::divide(std::begin(optarg), std::end(optarg), '=');
  std::string hashType(p.first.first, p.first.second);
  if (!acceptableTypes_.empty() &&
      std::find(std::begin(acceptableTypes_), std::end(acceptableTypes_),
                hashType) == std::end(acceptableTypes_)) {
    throw DL_ABORT_EX(
        fmt("Checksum type %s is not acceptable", hashType.c_str()));
  }
  std::string hexDigest(p.second.first, p.second.second);
  util::lowercase(hashType);
  util::lowercase(hexDigest);
  if (!MessageDigest::isValidHash(hashType, hexDigest)) {
    throw DL_ABORT_EX(_("Unrecognized checksum"));
  }
  option.put(pref_, optarg);
}

std::string ChecksumOptionHandler::createPossibleValuesString() const
{
  return "HASH_TYPE=HEX_DIGEST";
}

ParameterOptionHandler::ParameterOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    std::vector<std::string> validParamValues, char shortName)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::REQ_ARG, shortName),
      validParamValues_(std::move(validParamValues))
{
}

ParameterOptionHandler::~ParameterOptionHandler() = default;

void ParameterOptionHandler::parseArg(Option& option,
                                      const std::string& optarg) const
{
  auto itr =
      std::find(validParamValues_.begin(), validParamValues_.end(), optarg);
  if (itr == validParamValues_.end()) {
    std::string msg = pref_->k;
    msg += " ";
    msg += _("must be one of the following:");
    if (validParamValues_.size() == 0) {
      msg += "''";
    }
    else {
      for (const auto& p : validParamValues_) {
        msg += "'";
        msg += p;
        msg += "' ";
      }
    }
    throw DL_ABORT_EX(msg);
  }
  else {
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

HostPortOptionHandler::HostPortOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    PrefPtr hostOptionName, PrefPtr portOptionName, char shortName)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::REQ_ARG, shortName),
      hostOptionName_(hostOptionName),
      portOptionName_(portOptionName)
{
}

HostPortOptionHandler::~HostPortOptionHandler() = default;

void HostPortOptionHandler::parseArg(Option& option,
                                     const std::string& optarg) const
{
  std::string uri = "http://";
  uri += optarg;
  Request req;
  if (!req.setUri(uri)) {
    throw DL_ABORT_EX(_("Unrecognized format"));
  }
  option.put(pref_, optarg);
  setHostAndPort(option, req.getHost(), req.getPort());
}

void HostPortOptionHandler::setHostAndPort(Option& option,
                                           const std::string& hostname,
                                           uint16_t port) const
{
  option.put(hostOptionName_, hostname);
  option.put(portOptionName_, util::uitos(port));
}

std::string HostPortOptionHandler::createPossibleValuesString() const
{
  return "HOST:PORT";
}

HttpProxyOptionHandler::HttpProxyOptionHandler(PrefPtr pref,
                                               const char* description,
                                               const std::string& defaultValue,
                                               char shortName)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::REQ_ARG, shortName),
      proxyUserPref_(option::k2p(std::string(pref->k) + "-user")),
      proxyPasswdPref_(option::k2p(std::string(pref->k) + "-passwd"))
{
}

HttpProxyOptionHandler::~HttpProxyOptionHandler() = default;

void HttpProxyOptionHandler::parseArg(Option& option,
                                      const std::string& optarg) const
{
  if (optarg.empty()) {
    option.put(pref_, optarg);
  }
  else {
    std::string uri;
    if (util::startsWith(optarg, "http://") ||
        util::startsWith(optarg, "https://") ||
        util::startsWith(optarg, "ftp://")) {
      uri = optarg;
    }
    else {
      uri = "http://";
      uri += optarg;
    }
    uri::UriStruct us;
    if (!uri::parse(us, uri)) {
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

LocalFilePathOptionHandler::LocalFilePathOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    bool acceptStdin, char shortName, bool mustExist,
    const std::string& possibleValuesString)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::REQ_ARG, shortName),
      possibleValuesString_(possibleValuesString),
      acceptStdin_(acceptStdin),
      mustExist_(mustExist)
{
}

void LocalFilePathOptionHandler::parseArg(Option& option,
                                          const std::string& optarg) const
{
  if (acceptStdin_ && optarg == "-") {
    option.put(pref_, DEV_STDIN);
  }
  else {
    auto path = util::replace(optarg, "${HOME}", util::getHomeDir());
    if (mustExist_) {
      File f(path);
      std::string err;
      if (!f.exists(err)) {
        throw DL_ABORT_EX(err);
      }
      if (f.isDir()) {
        throw DL_ABORT_EX(fmt(MSG_NOT_FILE, optarg.c_str()));
      }
    }
    option.put(pref_, path);
  }
}

std::string LocalFilePathOptionHandler::createPossibleValuesString() const
{
  if (!possibleValuesString_.empty()) {
    return possibleValuesString_;
  }
  if (acceptStdin_) {
    return PATH_TO_FILE_STDIN;
  }
  else {
    return PATH_TO_FILE;
  }
}

PrioritizePieceOptionHandler::PrioritizePieceOptionHandler(
    PrefPtr pref, const char* description, const std::string& defaultValue,
    char shortName)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::REQ_ARG, shortName)
{
}

void PrioritizePieceOptionHandler::parseArg(Option& option,
                                            const std::string& optarg) const
{
  // Parse optarg against empty FileEntry list to detect syntax
  // error.
  std::vector<size_t> result;
  util::parsePrioritizePieceRange(
      result, optarg, std::vector<std::shared_ptr<FileEntry>>(), 1024);
  option.put(pref_, optarg);
}

std::string PrioritizePieceOptionHandler::createPossibleValuesString() const
{
  return "head[=SIZE], tail[=SIZE]";
}

OptimizeConcurrentDownloadsOptionHandler::
    OptimizeConcurrentDownloadsOptionHandler(PrefPtr pref,
                                             const char* description,
                                             const std::string& defaultValue,
                                             char shortName)
    : AbstractOptionHandler(pref, description, defaultValue,
                            OptionHandler::OPT_ARG, shortName)
{
}

void OptimizeConcurrentDownloadsOptionHandler::parseArg(
    Option& option, const std::string& optarg) const
{
  if (optarg == "true" || optarg.empty()) {
    option.put(pref_, A2_V_TRUE);
  }
  else if (optarg == "false") {
    option.put(pref_, A2_V_FALSE);
  }
  else {
    auto p = util::divide(std::begin(optarg), std::end(optarg), ':');

    std::string coeff_b(p.second.first, p.second.second);
    if (coeff_b.empty()) {
      std::string msg = pref_->k;
      msg += " ";
      msg += _("must be either 'true', 'false' or a pair numeric coefficients "
               "A and B under the form 'A:B'.");
      throw DL_ABORT_EX(msg);
    }

    std::string coeff_a(p.first.first, p.first.second);

    PrefPtr pref = PREF_OPTIMIZE_CONCURRENT_DOWNLOADS_COEFFA;
    std::string* sptr = &coeff_a;
    for (;;) {
      char* end;
      errno = 0;
      strtod(sptr->c_str(), &end);
      if (errno != 0 || sptr->c_str() + sptr->size() != end) {
        throw DL_ABORT_EX(fmt("Bad number '%s'", sptr->c_str()));
      }
      option.put(pref, *sptr);

      if (pref == PREF_OPTIMIZE_CONCURRENT_DOWNLOADS_COEFFB) {
        break;
      }
      pref = PREF_OPTIMIZE_CONCURRENT_DOWNLOADS_COEFFB;
      sptr = &coeff_b;
    }
    option.put(pref_, A2_V_TRUE);
  }
}

std::string
OptimizeConcurrentDownloadsOptionHandler::createPossibleValuesString() const
{
  return "true, false, A:B";
}

DeprecatedOptionHandler::DeprecatedOptionHandler(
    OptionHandler* depOptHandler, const OptionHandler* repOptHandler,
    bool stillWork, std::string additionalMessage)
    : depOptHandler_(depOptHandler),
      repOptHandler_(repOptHandler),
      stillWork_(stillWork),
      additionalMessage_(std::move(additionalMessage))
{
  depOptHandler_->addTag(TAG_DEPRECATED);
}

DeprecatedOptionHandler::~DeprecatedOptionHandler()
{
  delete depOptHandler_;
  // We don't delete repOptHandler_.
}

void DeprecatedOptionHandler::parse(Option& option,
                                    const std::string& arg) const
{
  if (repOptHandler_) {
    A2_LOG_WARN(fmt(_("--%s option is deprecated. Use --%s option instead. %s"),
                    depOptHandler_->getName(), repOptHandler_->getName(),
                    additionalMessage_.c_str()));
    repOptHandler_->parse(option, arg);
  }
  else if (stillWork_) {
    A2_LOG_WARN(fmt(_("--%s option will be deprecated in the future release. "
                      "%s"),
                    depOptHandler_->getName(), additionalMessage_.c_str()));
    depOptHandler_->parse(option, arg);
  }
  else {
    A2_LOG_WARN(fmt(_("--%s option is deprecated. %s"),
                    depOptHandler_->getName(), additionalMessage_.c_str()));
  }
}

std::string DeprecatedOptionHandler::createPossibleValuesString() const
{
  return depOptHandler_->createPossibleValuesString();
}

bool DeprecatedOptionHandler::hasTag(uint32_t tag) const
{
  return depOptHandler_->hasTag(tag);
}

void DeprecatedOptionHandler::addTag(uint32_t tag)
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

void DeprecatedOptionHandler::hide() { depOptHandler_->hide(); }

PrefPtr DeprecatedOptionHandler::getPref() const
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
