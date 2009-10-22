/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "MetalinkParserStateImpl.h"
#include "MetalinkParserStateMachine.h"
#include "RecoverableException.h"
#include "util.h"

namespace aria2 {

namespace {

const std::string FILE("file");
const std::string FILES("files");
const std::string HASH("hash");
const std::string LANGUAGE("language");
const std::string LENGTH("length");
const std::string LOCATION("location");
const std::string MAXCONNECTIONS("maxconnections");
const std::string METALINK("metalink");
// Can't use name VERSION because it is used as a macro.
const std::string METALINK_VERSION("version");
const std::string METAURL("metaurl");
const std::string NAME("name");
const std::string OS("os");
const std::string PIECE("piece");
const std::string PIECES("pieces");
const std::string PREFERENCE("preference");
const std::string RESOURCES("resources");
const std::string SIGNATURE("signature");
const std::string SIZE("size");
const std::string TYPE("type");
const std::string URL("url");
const std::string VERIFICATION("verification");
}

void InitialMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == METALINK) {
    stm->setMetalinkState();
  } else {
    stm->setSkipTagState();
  }
}

void MetalinkMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == FILES) {
    stm->setFilesState();
  } else {
    stm->setSkipTagState();
  }
}

void FilesMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == FILE) {
    stm->setFileState();
    std::map<std::string, std::string>::const_iterator itr = attrs.find(NAME);
    if(itr != attrs.end()) {
      stm->newEntryTransaction();
      stm->setFileNameOfEntry((*itr).second);
    }
  } else {
    stm->setSkipTagState();
  }
}

void FileMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == SIZE) {
    stm->setSizeState();
  } else if(name == METALINK_VERSION) {
    stm->setVersionState();
  } else if(name == LANGUAGE) {
    stm->setLanguageState();
  } else if(name == OS) {
    stm->setOSState();
  } else if(name == VERIFICATION) {
    stm->setVerificationState();
  } else if(name == RESOURCES) {
    stm->setResourcesState();
    int maxConnections;
    {
      std::map<std::string, std::string>::const_iterator itr =
	attrs.find(MAXCONNECTIONS);
      if(itr == attrs.end()) {
	maxConnections = -1;
      } else {
	try {
	  maxConnections = util::parseInt((*itr).second);
	} catch(RecoverableException& e) {
	  maxConnections = -1;
	}
      }
    }
    stm->setMaxConnectionsOfEntry(maxConnections);
  } else {
    stm->setSkipTagState();
  }
}

void FileMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->commitEntryTransaction();
}

void SizeMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

void SizeMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  try {
    stm->setFileLengthOfEntry(util::parseLLInt(characters));
  } catch(RecoverableException& e) {
    // current metalink specification doesn't require size element.
  }
}

void VersionMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

void VersionMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setVersionOfEntry(characters);
}

void LanguageMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

void LanguageMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setLanguageOfEntry(characters);
}

void OSMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

void OSMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setOSOfEntry(characters);
}

void VerificationMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(name == HASH) {
    stm->setHashState();
    std::map<std::string, std::string>::const_iterator itr = attrs.find(TYPE);
    if(itr == attrs.end()) {
      return;
    } else {
      std::string type = (*itr).second;
      stm->newChecksumTransaction();
      stm->setTypeOfChecksum(type);
    }
  } else if(name == PIECES) {
    stm->setPiecesState();
    try {
      size_t length;
      {
	std::map<std::string, std::string>::const_iterator itr =
	  attrs.find(LENGTH);
	if(itr == attrs.end()) {
	  return;
	} else {
	  length = util::parseInt((*itr).second);
	}
      }
      std::string type;
      {
	std::map<std::string, std::string>::const_iterator itr =
	  attrs.find(TYPE);
	if(itr == attrs.end()) {
	  return;
	} else {
	  type = (*itr).second;
	}
      }
      stm->newChunkChecksumTransaction();
      stm->setLengthOfChunkChecksum(length);
      stm->setTypeOfChunkChecksum(type);
    } catch(RecoverableException& e) {
      stm->cancelChunkChecksumTransaction();
    }
  } else
#endif // ENABLE_MESSAGE_DIGEST
  if(name == SIGNATURE) {
    stm->setSignatureState();
    std::map<std::string, std::string>::const_iterator itr = attrs.find(TYPE);
    if(itr == attrs.end()) {
      return;
    } else {
      stm->newSignatureTransaction();
      stm->setTypeOfSignature((*itr).second);

      std::map<std::string, std::string>::const_iterator itr = attrs.find(FILE);
      if(itr != attrs.end()) {
	stm->setFileOfSignature((*itr).second);
      }
    }
  } else {
    stm->setSkipTagState();
  }
}

void HashMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

void HashMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setHashOfChecksum(characters);
  stm->commitChecksumTransaction();
}

void PiecesMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == HASH) {
    stm->setPieceHashState();
    std::map<std::string, std::string>::const_iterator itr = attrs.find(PIECE);
    if(itr == attrs.end()) {
      stm->cancelChunkChecksumTransaction();
    } else {
      try {
	stm->createNewHashOfChunkChecksum(util::parseInt((*itr).second));
      } catch(RecoverableException& e) {
	stm->cancelChunkChecksumTransaction();
      }
    }
  } else {
    stm->setSkipTagState();
  }
}

void PiecesMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->commitChunkChecksumTransaction();
}

void PieceHashMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

void PieceHashMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setMessageDigestOfChunkChecksum(characters);
  stm->addHashOfChunkChecksum();
}

void SignatureMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

void SignatureMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setBodyOfSignature(characters);
  stm->commitSignatureTransaction();
}

void ResourcesMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  if(name == URL) {
    stm->setURLState();
    std::string type;
    {
      std::map<std::string, std::string>::const_iterator itr = attrs.find(TYPE);
      if(itr == attrs.end()) {
	return;
      } else {
	type = (*itr).second;
      }
    }
    std::string location;
    {
      std::map<std::string, std::string>::const_iterator itr =
	attrs.find(LOCATION);
      if(itr != attrs.end()) {
	location = util::toUpper((*itr).second);
      }
    }
    int preference;
    {
      std::map<std::string, std::string>::const_iterator itr =
	attrs.find(PREFERENCE);
      if(itr == attrs.end()) {
	preference = 0;
      } else {
	try {
	  preference = util::parseInt((*itr).second);
	} catch(RecoverableException& e) {
	  preference = 0;
	}
      }
    }
    int maxConnections;
    {
      std::map<std::string, std::string>::const_iterator itr =
	attrs.find(MAXCONNECTIONS);
      if(itr == attrs.end()) {
	maxConnections = -1;
      } else {
	try {
	  maxConnections = util::parseInt((*itr).second);
	} catch(RecoverableException& e) {
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
    stm->setSkipTagState();
  }
}

void URLMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

void URLMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::string& characters)
{
  stm->setURLOfResource(characters);
  stm->commitResourceTransaction();
}

void SkipTagMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& name,
 const std::map<std::string, std::string>& attrs)
{
  stm->setSkipTagState();
}

} // namespace aria2
