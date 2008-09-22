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
#include "Util.h"
#include "OptionHandlerImpl.h"
#include "Option.h"
#include "A2STR.h"
#include "a2functional.h"
#include <istream>
#include <utility>

namespace aria2 {

void OptionParser::parse(Option* option, std::istream& is)
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
}

void OptionParser::addOptionHandler
(const SharedHandle<OptionHandler>& optionHandler)
{
  _optionHandlers.push_back(optionHandler);
}

void OptionParser::parseDefaultValues(Option* option) const
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

std::deque<SharedHandle<OptionHandler> > OptionParser::findAll() const
{
  std::deque<SharedHandle<OptionHandler> > result;
  std::remove_copy_if(_optionHandlers.begin(), _optionHandlers.end(),
		      std::back_inserter(result),
		      mem_fun_sh(&OptionHandler::isHidden));
  return result;
}

SharedHandle<OptionHandler>
OptionParser::findByName(const std::string& name) const
{
  std::deque<SharedHandle<OptionHandler> >::const_iterator i =
    std::find_if(_optionHandlers.begin(), _optionHandlers.end(),
		 FindByName(name));
  if(i == _optionHandlers.end()) {
    return SharedHandle<OptionHandler>();
  } else {
    return *i;
  }
}

const std::deque<SharedHandle<OptionHandler> >&
OptionParser::getOptionHandlers() const
{
  return _optionHandlers;
}

} // namespace aria2
