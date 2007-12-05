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
#include "FileMetalinkParserState.h"
#include "MetalinkParserStateMachine.h"
#include "Util.h"
#include "RecoverableException.h"

void FileMetalinkParserState::beginElement(MetalinkParserStateMachine* stm,
					   const string& name,
					   const map<string, string>& attrs)
{
  if(name == "size") {
    stm->setSizeState();
  } else if(name == "version") {
    stm->setVersionState();
  } else if(name == "language") {
    stm->setLanguageState();
  } else if(name == "os") {
    stm->setOSState();
#ifdef ENABLE_MESSAGE_DIGEST
  } else if(name == "verification") {
    stm->setVerificationState();
#endif // ENABLE_MESSAGE_DIGEST
  } else if(name == "resources") {
    stm->setResourcesState();
    int32_t maxConnections;
    {
      map<string, string>::const_iterator itr = attrs.find("maxconnections");
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
    stm->setMaxConnectionsOfEntry(maxConnections);
  } else {
    stm->setSkipTagState(this);
  }
}

void FileMetalinkParserState::endElement(MetalinkParserStateMachine* stm,
					  const string& name,
					  const string& characters)
{
  stm->commitEntryTransaction();
  stm->setFilesState();
}
