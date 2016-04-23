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
#include "OptionParser.h"

#include <unistd.h>
#include <getopt.h>

#include <cstring>
#include <cassert>
#include <istream>
#include <utility>

#include "util.h"
#include "OptionHandlerImpl.h"
#include "Option.h"
#include "A2STR.h"
#include "a2functional.h"
#include "array_fun.h"
#include "OptionHandlerFactory.h"
#include "DlAbortEx.h"
#include "error_code.h"
#include "UnknownOptionException.h"
#include "LogFactory.h"
#include "fmt.h"

namespace aria2 {

OptionParser::OptionParser()
    : handlers_(option::countOption(), nullptr), shortOpts_(256)
{
}

OptionParser::~OptionParser()
{
  std::for_each(handlers_.begin(), handlers_.end(), Deleter());
}

namespace {
template <typename InputIterator>
size_t countPublicOption(InputIterator first, InputIterator last)
{
  size_t count = 0;
  for (; first != last; ++first) {
    if (*first && !(*first)->isHidden()) {
      ++count;
    }
  }
  return count;
}
} // namespace

namespace {
template <typename InputIterator>
void putOptions(struct option* longOpts, int* plopt, InputIterator first,
                InputIterator last)
{
  for (; first != last; ++first) {
    if (*first && !(*first)->isHidden()) {
#ifdef HAVE_OPTION_CONST_NAME
      (*longOpts).name = (*first)->getName();
#else  // !HAVE_OPTION_CONST_NAME
      (*longOpts).name = strdup((*first)->getName());
      if ((*longOpts).name == nullptr) {
        auto errNum = errno;
        A2_LOG_ERROR(
            fmt("strdup() failed: %s", util::safeStrerror(errNum).c_str()));
        exit(EXIT_FAILURE);
      }
#endif // !HAVE_OPTION_CONST_NAME
      switch ((*first)->getArgType()) {
      case OptionHandler::REQ_ARG:
        (*longOpts).has_arg = required_argument;
        break;
      case OptionHandler::OPT_ARG:
        (*longOpts).has_arg = optional_argument;
        break;
      case OptionHandler::NO_ARG:
        (*longOpts).has_arg = no_argument;
        break;
      default:
        abort();
      }
      if ((*first)->getShortName() == 0) {
        (*longOpts).flag = plopt;
        (*longOpts).val = (*first)->getPref()->i;
      }
      else {
        (*longOpts).flag = nullptr;
        (*longOpts).val = (*first)->getShortName();
      }
      ++longOpts;
    }
  }
  (*longOpts).name = nullptr;
  (*longOpts).has_arg = 0;
  (*longOpts).flag = nullptr;
  (*longOpts).val = 0;
}
} // namespace

namespace {
template <typename InputIterator>
std::string createOptstring(InputIterator first, InputIterator last)
{
  std::string str = "";
  for (; first != last; ++first) {
    if (*first && !(*first)->isHidden()) {
      if ((*first)->getShortName() != 0) {
        str += (*first)->getShortName();
        if ((*first)->getArgType() == OptionHandler::REQ_ARG) {
          str += ":";
        }
        else if ((*first)->getArgType() == OptionHandler::OPT_ARG) {
          str += "::";
        }
      }
    }
  }
  return str;
}
} // namespace

void OptionParser::parseArg(std::ostream& out,
                            std::vector<std::string>& nonopts, int argc,
                            char* argv[]) const
{
  size_t numPublicOption =
      countPublicOption(handlers_.begin(), handlers_.end());
  int lopt;
  auto longOpts = make_unique<struct option[]>(numPublicOption + 1);
  putOptions(longOpts.get(), &lopt, handlers_.begin(), handlers_.end());
  std::string optstring = createOptstring(handlers_.begin(), handlers_.end());
  while (1) {
    int c = getopt_long(argc, argv, optstring.c_str(), longOpts.get(), nullptr);
    if (c == -1) {
      break;
    }
    const OptionHandler* op = nullptr;
    if (c == 0) {
      op = findById(lopt);
    }
    else if (c != '?') {
      op = findByShortName(c);
    }
    else {
      assert(c == '?');
      if (optind == 1) {
        throw DL_ABORT_EX2("Failed to parse command-line options.",
                           error_code::OPTION_ERROR);
      }
      int optlen = strlen(argv[optind - 1]);
      const char* optstr = argv[optind - 1];
      for (; *optstr == '-'; ++optstr)
        ;
      int optstrlen = strlen(optstr);
      if (optstrlen + 1 >= optlen) {
        // If this is short option form (1 '-' prefix), just throw
        // error here.
        throw DL_ABORT_EX2("Failed to parse command-line options.",
                           error_code::OPTION_ERROR);
      }
      // There are 3 situations: 1) completely unknown option 2)
      // getopt_long() complained because too few arguments.  3)
      // option is ambiguous.
      int ambiguous = 0;
      for (int i = 1, len = option::countOption(); i < len; ++i) {
        PrefPtr pref = option::i2p(i);
        const OptionHandler* h = find(pref);
        if (h && !h->isHidden()) {
          if (strcmp(pref->k, optstr) == 0) {
            // Exact match, this means getopt_long detected error
            // while handling this option.
            throw DL_ABORT_EX2("Failed to parse command-line options.",
                               error_code::OPTION_ERROR);
          }
          else if (util::startsWith(pref->k, pref->k + strlen(pref->k), optstr,
                                    optstr + optstrlen)) {
            ++ambiguous;
          }
        }
      }
      if (ambiguous == 1) {
        // This is successfully abbreviated option. So it must be case
        // 2).
        throw DL_ABORT_EX2("Failed to parse command-line options.",
                           error_code::OPTION_ERROR);
      }
      throw UNKNOWN_OPTION_EXCEPTION(argv[optind - 1]);
    }
    assert(op);
    out << op->getName() << "=";
    if (optarg) {
      out << optarg;
      if (op->getEraseAfterParse()) {
        for (char* p = optarg; *p != '\0'; ++p) {
          *p = '*';
        }
      }
    }
    out << "\n";
  }
  std::copy(argv + optind, argv + argc, std::back_inserter(nonopts));
}

void OptionParser::parse(Option& option, std::istream& is) const
{
  std::string line;
  while (getline(is, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }
    auto nv = util::divide(std::begin(line), std::end(line), '=');
    if (nv.first.first == nv.first.second) {
      continue;
    }
    PrefPtr pref = option::k2p(std::string(nv.first.first, nv.first.second));
    const OptionHandler* handler = find(pref);
    if (handler) {
      handler->parse(option, std::string(nv.second.first, nv.second.second));
    }
    else {
      A2_LOG_WARN(fmt("Unknown option: %s", line.c_str()));
    }
  }
}

void OptionParser::parse(Option& option, const KeyVals& options) const
{
  for (const auto& o : options) {
    auto pref = option::k2p(o.first);
    const OptionHandler* handler = find(pref);
    if (handler) {
      handler->parse(option, o.second);
    }
    else {
      A2_LOG_WARN(fmt("Unknown option: %s", o.first.c_str()));
    }
  }
}

void OptionParser::setOptionHandlers(
    const std::vector<OptionHandler*>& handlers)
{
  for (const auto& h : handlers) {
    addOptionHandler(h);
  }
}

void OptionParser::addOptionHandler(OptionHandler* handler)
{
  size_t optId = handler->getPref()->i;
  assert(optId < handlers_.size());
  handlers_[optId] = handler;
  if (handler->getShortName()) {
    shortOpts_[static_cast<unsigned char>(handler->getShortName())] = optId;
  }
}

void OptionParser::parseDefaultValues(Option& option) const
{
  for (const auto& h : handlers_) {
    if (h && !h->getDefaultValue().empty()) {
      h->parse(option, h->getDefaultValue());
    }
  }
}

std::vector<const OptionHandler*> OptionParser::findByTag(uint32_t tag) const
{
  std::vector<const OptionHandler*> result;
  for (const auto& h : handlers_) {
    if (h && !h->isHidden() && h->hasTag(tag)) {
      result.push_back(h);
    }
  }
  return result;
}

std::vector<const OptionHandler*>
OptionParser::findByNameSubstring(const std::string& substring) const
{
  std::vector<const OptionHandler*> result;
  for (const auto& h : handlers_) {
    if (h && !h->isHidden()) {
      size_t nameLen = strlen(h->getName());
      if (std::search(h->getName(), h->getName() + nameLen, substring.begin(),
                      substring.end()) != h->getName() + nameLen) {
        result.push_back(h);
      }
    }
  }
  return result;
}

std::vector<const OptionHandler*> OptionParser::findAll() const
{
  std::vector<const OptionHandler*> result;
  for (const auto& h : handlers_) {
    if (h && !h->isHidden()) {
      result.push_back(h);
    }
  }
  return result;
}

const OptionHandler* OptionParser::find(PrefPtr pref) const
{
  return findById(pref->i);
}

const OptionHandler* OptionParser::findById(size_t id) const
{
  if (id >= handlers_.size()) {
    return handlers_[0];
  }
  const OptionHandler* h = handlers_[id];
  if (!h || h->isHidden()) {
    return handlers_[0];
  }
  else {
    return h;
  }
}

const OptionHandler* OptionParser::findByShortName(char shortName) const
{
  size_t idx = static_cast<unsigned char>(shortName);
  return findById(shortOpts_[idx]);
}

std::shared_ptr<OptionParser> OptionParser::optionParser_;

const std::shared_ptr<OptionParser>& OptionParser::getInstance()
{
  if (!optionParser_) {
    optionParser_ = std::make_shared<OptionParser>();
    optionParser_->setOptionHandlers(
        OptionHandlerFactory::createOptionHandlers());
  }
  return optionParser_;
}

void OptionParser::deleteInstance() { optionParser_.reset(); }

} // namespace aria2
