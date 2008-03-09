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
#include "ResourcesMetalinkParserState.h"
#include "MetalinkParserStateMachine.h"
#include "Util.h"
#include "RecoverableException.h"

namespace aria2 {

void ResourcesMetalinkParserState::beginElement(MetalinkParserStateMachine* stm,
						const std::string& name,
						const std::map<std::string, std::string>& attrs)
{
  if(name == "url") {
    stm->setURLState();
    std::string type;
    {
      std::map<std::string, std::string>::const_iterator itr = attrs.find("type");
      if(itr == attrs.end()) {
	return;
      } else {
	type = (*itr).second;
      }
    }
    std::string location;
    {
      std::map<std::string, std::string>::const_iterator itr = attrs.find("location");
      if(itr != attrs.end()) {
	location = Util::toUpper((*itr).second);
      }
    }
    int preference;
    {
      std::map<std::string, std::string>::const_iterator itr = attrs.find("preference");
      if(itr == attrs.end()) {
	preference = 0;
      } else {
	try {
	  preference = Util::parseInt((*itr).second);
	} catch(RecoverableException* e) {
	  delete e;
	  preference = 0;
	}
      }
    }
    int maxConnections;
    {
      std::map<std::string, std::string>::const_iterator itr = attrs.find("maxconnections");
      if(itr == attrs.end()) {
	maxConnections = -1;
      } else {
	try {
	  maxConnections = Util::parseInt((*itr).second);
	} catch(RecoverableException* e) {
	  delete e;
	  maxConnections = -1;
	}
      }
    }
    stm->newResourceTransaction();
    stm->setTypeOfResource(type);
    stm->setLocationOfResource(location);
    stm->setPreferenceOfResource(preference);
    stm->setMaxConnectionsOfResource(maxConnections);
  } else {
    stm->setSkipTagState(this);
  }
}

void ResourcesMetalinkParserState::endElement(MetalinkParserStateMachine* stm,
					      const std::string& name,
					      const std::string& characters)
{
  stm->setFileState();
}

} // namespace aria2
