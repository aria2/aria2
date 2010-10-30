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
#include <istream>
#include <utility>

#include "util.h"
#include "OptionHandlerImpl.h"
#include "Option.h"
#include "A2STR.h"
#include "a2functional.h"
#include "array_fun.h"
#include "OptionHandlerFactory.h"

namespace aria2 {

OptionParser::OptionParser():
  idCounter_(0)
{}

template<typename InputIterator>
static size_t countPublicOption(InputIterator first, InputIterator last)
{
  size_t count = 0;
  for(; first != last; ++first) {
    if(!(*first)->isHidden()) {
      ++count;
    }
  }
  return count;
}

template<typename InputIterator>
static void putOptions(struct option* longOpts, int* plopt,
                       InputIterator first, InputIterator last)
{
  for(; first != last; ++first) {
    if(!(*first)->isHidden()) {
#ifdef HAVE_OPTION_CONST_NAME
      (*longOpts).name = (*first)->getName().c_str();
#else // !HAVE_OPTION_CONST_NAME
      (*longOpts).name = strdup((*first)->getName().c_str());
#endif // !HAVE_OPTION_CONST_NAME
      switch((*first)->getArgType()) {
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
      if((*first)->getShortName() == 0) {
        (*longOpts).flag = plopt;
        (*longOpts).val = (*first)->getOptionID();
      } else {
        (*longOpts).flag = 0;
        (*longOpts).val = (*first)->getShortName();
      }
      ++longOpts;
    }
  }
  (*longOpts).name = 0;
  (*longOpts).has_arg = 0;
  (*longOpts).flag = 0;
  (*longOpts).val = 0;
}

template<typename InputIterator>
static std::string createOptstring(InputIterator first, InputIterator last)
{
  std::string str = "";
  for(; first != last; ++first) {
    if(!(*first)->isHidden()) {
      if((*first)->getShortName() != 0) {
        str += (*first)->getShortName();
        if((*first)->getArgType() == OptionHandler::REQ_ARG) {
          str += ":";
        } else if((*first)->getArgType() == OptionHandler::OPT_ARG) {
          str += "::";
        }
      }
    }
  }
  return str;
}

void OptionParser::parseArg
(std::ostream& out, std::vector<std::string>& nonopts,
 int argc, char* const argv[])
{
  size_t numPublicOption = countPublicOption(optionHandlers_.begin(),
                                             optionHandlers_.end());
  int lopt;
  array_ptr<struct option> longOpts(new struct option[numPublicOption+1]);
  putOptions(longOpts, &lopt,optionHandlers_.begin(),optionHandlers_.end());
  std::string optstring = createOptstring(optionHandlers_.begin(),
                                          optionHandlers_.end());
  while(1) {
    int c = getopt_long(argc, argv, optstring.c_str(), longOpts, 0);
    if(c == -1) {
      break;
    }
    SharedHandle<OptionHandler> op;
    if(c == 0) {
      op = findByID(lopt);
    } else {
      op = findByShortName(c);
    }
    if(op.isNull()) {
      throw DL_ABORT_EX("Failed to parse command-line options.");
    }
    out << op->getName() << "=";
    if(optarg) {
      out << optarg;
    }
    out << "\n";
  }
  std::copy(argv+optind, argv+argc, std::back_inserter(nonopts));
}

void OptionParser::parse(Option& option, std::istream& is)
{
  std::string line;
  int32_t linenum = 0;
  while(getline(is, line)) {
    ++linenum;
    if(util::startsWith(line, A2STR::SHARP_C)) {
      continue;
    }
    std::pair<std::string, std::string> nv;
    util::divide(nv, line, '=');
    if(nv.first.empty()) {
      continue;
    }
    OptionHandlerHandle handler = getOptionHandlerByName(nv.first);
    handler->parse(option, nv.second);
  }
}

namespace {
class DummyOptionHandler:public NameMatchOptionHandler {
protected:
  virtual void parseArg(Option& option, const std::string& arg) {}
public:
  DummyOptionHandler(const std::string& name):NameMatchOptionHandler(name) {}

  virtual std::string createPossibleValuesString() const
  {
    return A2STR::NIL;
  }
};
} // namespace

OptionHandlerHandle OptionParser::getOptionHandlerByName
(const std::string& optName)
{
  SharedHandle<OptionHandler> handler(new DummyOptionHandler(optName));
  std::vector<SharedHandle<OptionHandler> >::const_iterator i =
    std::lower_bound(optionHandlers_.begin(), optionHandlers_.end(),
                     handler, OptionHandlerNameLesser());
  if(i != optionHandlers_.end() && (*i)->canHandle(optName)) {
    handler = *i;
  } else {
    handler.reset(new NullOptionHandler());
  }
  return handler;
}

void OptionParser::setOptionHandlers
(const std::vector<SharedHandle<OptionHandler> >& optionHandlers)
{
  optionHandlers_ = optionHandlers;
  for(std::vector<SharedHandle<OptionHandler> >::const_iterator i =
        optionHandlers_.begin(), eoi = optionHandlers_.end();
      i != eoi; ++i) {
    (*i)->setOptionID(++idCounter_);
  }
  std::sort(optionHandlers_.begin(), optionHandlers_.end(),
            OptionHandlerNameLesser());
}

void OptionParser::addOptionHandler
(const SharedHandle<OptionHandler>& optionHandler)
{
  optionHandler->setOptionID(++idCounter_);
  std::vector<SharedHandle<OptionHandler> >::iterator i =
    std::lower_bound(optionHandlers_.begin(), optionHandlers_.end(),
                     optionHandler, OptionHandlerNameLesser());
  optionHandlers_.insert(i, optionHandler);
}

void OptionParser::parseDefaultValues(Option& option) const
{
  for(std::vector<SharedHandle<OptionHandler> >::const_iterator i =
        optionHandlers_.begin(), eoi = optionHandlers_.end();
      i != eoi; ++i) {
    if(!(*i)->getDefaultValue().empty()) {
      (*i)->parse(option, (*i)->getDefaultValue());
    }
  }
}

namespace {
class FindOptionHandlerByTag :
    public std::unary_function<SharedHandle<OptionHandler>, bool> {
private:
  std::string tag_;
public:
  FindOptionHandlerByTag(const std::string& tag):tag_(tag) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() && optionHandler->hasTag(tag_);
  }
};
} // namespace

std::vector<SharedHandle<OptionHandler> >
OptionParser::findByTag(const std::string& tag) const
{
  std::vector<SharedHandle<OptionHandler> > result;
  std::remove_copy_if(optionHandlers_.begin(), optionHandlers_.end(),
                      std::back_inserter(result),
                      std::not1(FindOptionHandlerByTag(tag)));
  std::sort(result.begin(), result.end(), OptionHandlerIDLesser());
  return result;
}

namespace {
class FindOptionHandlerByNameSubstring :
    public std::unary_function<SharedHandle<OptionHandler> , bool> {
private:
  std::string substring_;
public:
  FindOptionHandlerByNameSubstring
  (const std::string& substring):substring_(substring) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() &&
      optionHandler->getName().find(substring_) != std::string::npos;
  }
};
} // namespace

std::vector<SharedHandle<OptionHandler> >
OptionParser::findByNameSubstring(const std::string& substring) const
{
  std::vector<SharedHandle<OptionHandler> > result;
  std::remove_copy_if(optionHandlers_.begin(), optionHandlers_.end(),
                      std::back_inserter(result),
                      std::not1(FindOptionHandlerByNameSubstring(substring)));
  std::sort(result.begin(), result.end(), OptionHandlerIDLesser());
  return result;  
}

std::vector<SharedHandle<OptionHandler> > OptionParser::findAll() const
{
  std::vector<SharedHandle<OptionHandler> > result;
  std::remove_copy_if(optionHandlers_.begin(), optionHandlers_.end(),
                      std::back_inserter(result),
                      mem_fun_sh(&OptionHandler::isHidden));
  std::sort(result.begin(), result.end(), OptionHandlerIDLesser());
  return result;
}

template<typename InputIterator, typename Predicate>
static SharedHandle<OptionHandler> findOptionHandler
(InputIterator first, InputIterator last, Predicate pred)
{
  SharedHandle<OptionHandler> handler;
  InputIterator i = std::find_if(first, last, pred);
  if(i != last) {
    handler = *i;
  }
  return handler;
}

SharedHandle<OptionHandler>
OptionParser::findByName(const std::string& name) const
{
  SharedHandle<OptionHandler> handler(new DummyOptionHandler(name));
  std::vector<SharedHandle<OptionHandler> >::const_iterator i =
    std::lower_bound(optionHandlers_.begin(), optionHandlers_.end(),
                     handler, OptionHandlerNameLesser());
  if(i != optionHandlers_.end() && (*i)->getName() == name &&
     !(*i)->isHidden()) {
    handler = *i;
  } else {
    handler.reset();
  }
  return handler;
}

namespace {
class FindOptionHandlerByID:public std::unary_function
<SharedHandle<OptionHandler>, bool> {
private:
  int id_;
public:
  FindOptionHandlerByID(int id):id_(id) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() && optionHandler->getOptionID() == id_;
  }
};
} // namespace

SharedHandle<OptionHandler> OptionParser::findByID(int id) const
{
  return findOptionHandler(optionHandlers_.begin(), optionHandlers_.end(),
                           FindOptionHandlerByID(id));
}

namespace {
class FindOptionHandlerByShortName:
    public std::unary_function<SharedHandle<OptionHandler>, bool> {
private:
  char shortName_;
public:
  FindOptionHandlerByShortName(char shortName):shortName_(shortName) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() &&
      optionHandler->getShortName() == shortName_;
  }
};
} // namespace

SharedHandle<OptionHandler> OptionParser::findByShortName(char shortName) const
{
  return findOptionHandler(optionHandlers_.begin(), optionHandlers_.end(),
                           FindOptionHandlerByShortName(shortName));
}


SharedHandle<OptionParser> OptionParser::optionParser_;

const SharedHandle<OptionParser>& OptionParser::getInstance()
{
  if(optionParser_.isNull()) {
    optionParser_.reset(new OptionParser());
    optionParser_->setOptionHandlers(OptionHandlerFactory::createOptionHandlers());
  }
  return optionParser_;
}

} // namespace aria2
