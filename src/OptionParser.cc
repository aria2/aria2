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
#include "OptionParser.h"

#include <unistd.h>
#include <getopt.h>

#include <cstring>
#include <istream>
#include <utility>

#include "Util.h"
#include "OptionHandlerImpl.h"
#include "Option.h"
#include "A2STR.h"
#include "a2functional.h"
#include "array_fun.h"
#include "OptionHandlerFactory.h"

namespace aria2 {

OptionParser::OptionParser():_idCounter(0) {}

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
      (*longOpts).name = (*first)->getName().c_str();
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
(std::ostream& out, std::deque<std::string>& nonopts, int argc, char* const argv[])
{
  size_t numPublicOption = countPublicOption(_optionHandlers.begin(),
					     _optionHandlers.end());
  int lopt;
  array_ptr<struct option> longOpts(new struct option[numPublicOption+1]);
  putOptions(longOpts, &lopt,_optionHandlers.begin(),_optionHandlers.end());
  std::string optstring = createOptstring(_optionHandlers.begin(),
					  _optionHandlers.end());
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
    if(Util::startsWith(line, A2STR::SHARP_C)) {
      continue;
    }
    std::pair<std::string, std::string> nv = Util::split(line, A2STR::EQUAL_C);
    OptionHandlerHandle handler = getOptionHandlerByName(nv.first);
    handler->parse(option, nv.second);
  }
}

OptionHandlerHandle OptionParser::getOptionHandlerByName(const std::string& optName)
{
  for(OptionHandlers::iterator itr = _optionHandlers.begin();
      itr != _optionHandlers.end(); ++itr) {
    if((*itr)->canHandle(optName)) {
      return *itr;
    }
  }
  return SharedHandle<OptionHandler>(new NullOptionHandler());
}

void OptionParser::setOptionHandlers(const std::deque<SharedHandle<OptionHandler> >& optionHandlers)
{
  _optionHandlers = optionHandlers;
  for(std::deque<SharedHandle<OptionHandler> >::iterator i =
	_optionHandlers.begin(); i != _optionHandlers.end(); ++i) {
    (*i)->setOptionID(++_idCounter);
  }
}

void OptionParser::addOptionHandler
(const SharedHandle<OptionHandler>& optionHandler)
{
  optionHandler->setOptionID(++_idCounter);
  _optionHandlers.push_back(optionHandler);
}

void OptionParser::parseDefaultValues(Option& option) const
{
  for(std::deque<SharedHandle<OptionHandler> >::const_iterator i =
	_optionHandlers.begin(); i != _optionHandlers.end(); ++i) {
    if(!(*i)->getDefaultValue().empty()) {
      (*i)->parse(option, (*i)->getDefaultValue());
    }
  }
}

class FindByTag :
  public std::unary_function<SharedHandle<OptionHandler>, bool> {
private:
  std::string _tag;
public:
  FindByTag(const std::string& tag):_tag(tag) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() && optionHandler->hasTag(_tag);
  }
};

std::deque<SharedHandle<OptionHandler> >
OptionParser::findByTag(const std::string& tag) const
{
  std::deque<SharedHandle<OptionHandler> > result;
  std::remove_copy_if(_optionHandlers.begin(), _optionHandlers.end(),
		      std::back_inserter(result),
		      std::not1(FindByTag(tag)));
  return result;
}

class FindByNameSubstring :
  public std::unary_function<SharedHandle<OptionHandler> , bool> {
private:
  std::string _substring;
public:
  FindByNameSubstring(const std::string& substring):_substring(substring) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() &&
      optionHandler->getName().find(_substring) != std::string::npos;
  }
};

std::deque<SharedHandle<OptionHandler> >
OptionParser::findByNameSubstring(const std::string& substring) const
{
  std::deque<SharedHandle<OptionHandler> > result;
  std::remove_copy_if(_optionHandlers.begin(), _optionHandlers.end(),
		      std::back_inserter(result),
		      std::not1(FindByNameSubstring(substring)));
  return result;  
}

std::deque<SharedHandle<OptionHandler> > OptionParser::findAll() const
{
  std::deque<SharedHandle<OptionHandler> > result;
  std::remove_copy_if(_optionHandlers.begin(), _optionHandlers.end(),
		      std::back_inserter(result),
		      mem_fun_sh(&OptionHandler::isHidden));
  return result;
}

template<typename InputIterator, typename Predicate>
static SharedHandle<OptionHandler> findOptionHandler
(InputIterator first, InputIterator last, Predicate pred)
{
  InputIterator i = std::find_if(first, last, pred);
  if(i == last) {
    return SharedHandle<OptionHandler>();
  } else {
    return *i;
  }
}

class FindByName :
  public std::unary_function<SharedHandle<OptionHandler> , bool> {
private:
  std::string _name;
public:
  FindByName(const std::string& name):_name(name) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() && optionHandler->getName() == _name;
  }
};

SharedHandle<OptionHandler>
OptionParser::findByName(const std::string& name) const
{
  return findOptionHandler(_optionHandlers.begin(), _optionHandlers.end(),
			   FindByName(name));
}

class FindByID:public std::unary_function<SharedHandle<OptionHandler>, bool> {
private:
  int _id;
public:
  FindByID(int id):_id(id) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() && optionHandler->getOptionID() == _id;
  }
};

SharedHandle<OptionHandler> OptionParser::findByID(int id) const
{
  return findOptionHandler(_optionHandlers.begin(), _optionHandlers.end(),
			   FindByID(id));
}

class FindByShortName:
    public std::unary_function<SharedHandle<OptionHandler>, bool> {
private:
  char _shortName;
public:
  FindByShortName(char shortName):_shortName(shortName) {}

  bool operator()(const SharedHandle<OptionHandler>& optionHandler) const
  {
    return !optionHandler->isHidden() &&
      optionHandler->getShortName() == _shortName;
  }
};

SharedHandle<OptionHandler> OptionParser::findByShortName(char shortName) const
{
  return findOptionHandler(_optionHandlers.begin(), _optionHandlers.end(),
			   FindByShortName(shortName));
}


const std::deque<SharedHandle<OptionHandler> >&
OptionParser::getOptionHandlers() const
{
  return _optionHandlers;
}

SharedHandle<OptionParser> OptionParser::_optionParser;

SharedHandle<OptionParser> OptionParser::getInstance()
{
  if(_optionParser.isNull()) {
    _optionParser.reset(new OptionParser());
    _optionParser->setOptionHandlers(OptionHandlerFactory::createOptionHandlers());
  }
  return _optionParser;
}

} // namespace aria2
