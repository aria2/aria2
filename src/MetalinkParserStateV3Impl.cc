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
#include "MetalinkParserStateV3Impl.h"
#include "MetalinkParserStateMachine.h"
#include "RecoverableException.h"
#include "MetalinkResource.h"
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
// Can't use name VERSION because it is used as a macro.
const std::string METALINK_VERSION("version");
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
} // namespace

const std::string METALINK3_NAMESPACE_URI("http://www.metalinker.org/");

namespace {
class FindAttr {
private:
  const std::string& localname_;
public:
  FindAttr(const std::string& localname):localname_(localname) {}

  bool operator()(const XmlAttr& attr) const
  {
    return attr.localname == localname_ &&
      (attr.nsUri.empty() || attr.nsUri == METALINK3_NAMESPACE_URI);
  }
};
} // namespace

namespace {
template<typename Container>
typename Container::const_iterator findAttr
(const Container& attrs, const std::string& localname)
{
  return std::find_if(attrs.begin(), attrs.end(), FindAttr(localname));
}
} // namespace

void MetalinkMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri == METALINK3_NAMESPACE_URI && localname == FILES) {
    stm->setFilesState();
  } else {
    stm->setSkipTagState();
  }
}

void FilesMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri == METALINK3_NAMESPACE_URI && localname == FILE) {
    stm->setFileState();
    std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, NAME);
    if(itr != attrs.end()) {
      std::string name = util::strip((*itr).value);
      if(name.empty() || util::detectDirTraversal(name)) {
        return;
      }
      stm->newEntryTransaction();
      stm->setFileNameOfEntry(name);
    }
  } else {
    stm->setSkipTagState();
  }
}

void FileMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri != METALINK3_NAMESPACE_URI) {
    stm->setSkipTagState();
  } else if(localname == SIZE) {
    stm->setSizeState();
  } else if(localname == METALINK_VERSION) {
    stm->setVersionState();
  } else if(localname == LANGUAGE) {
    stm->setLanguageState();
  } else if(localname == OS) {
    stm->setOSState();
  } else if(localname == VERIFICATION) {
    stm->setVerificationState();
  } else if(localname == RESOURCES) {
    stm->setResourcesState();
    int maxConnections;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs,MAXCONNECTIONS);
      if(itr == attrs.end()) {
        maxConnections = -1;
      } else {
        try {
          maxConnections = util::parseInt((*itr).value);
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
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->commitEntryTransaction();
}

void SizeMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  try {
    stm->setFileLengthOfEntry(util::parseULLInt(characters));
  } catch(RecoverableException& e) {
    // current metalink specification doesn't require size element.
  }
}

void VersionMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setVersionOfEntry(util::strip(characters));
}

void LanguageMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setLanguageOfEntry(util::strip(characters));
}

void OSMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setOSOfEntry(util::strip(characters));
}

void VerificationMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri != METALINK3_NAMESPACE_URI) {
    stm->setSkipTagState();
  } else
#ifdef ENABLE_MESSAGE_DIGEST
  if(localname == HASH) {
    stm->setHashState();
    std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, TYPE);
    if(itr == attrs.end()) {
      return;
    } else {
      std::string type = util::strip((*itr).value);
      stm->newChecksumTransaction();
      stm->setTypeOfChecksum(type);
    }
  } else if(localname == PIECES) {
    stm->setPiecesState();
    try {
      size_t length;
      {
        std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, LENGTH);
        if(itr == attrs.end()) {
          return;
        } else {
          length = util::parseInt((*itr).value);
        }
      }
      std::string type;
      {
        std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, TYPE);
        if(itr == attrs.end()) {
          return;
        } else {
          type = util::strip((*itr).value);
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
    if(localname == SIGNATURE) {
      stm->setSignatureState();
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, TYPE);
      if(itr == attrs.end()) {
        return;
      } else {
        stm->newSignatureTransaction();
        stm->setTypeOfSignature(util::strip((*itr).value));
        std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, FILE);
        if(itr != attrs.end()) {
          std::string file = util::strip((*itr).value);
          if(!util::detectDirTraversal(file)) {
            stm->setFileOfSignature(file);
          }
        }
      }
    } else {
      stm->setSkipTagState();
    }
}

void HashMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setHashOfChecksum(util::strip(characters));
  stm->commitChecksumTransaction();
}

void PiecesMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri == METALINK3_NAMESPACE_URI && localname == HASH) {
    stm->setPieceHashState();
    std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, PIECE);
    if(itr == attrs.end()) {
      stm->cancelChunkChecksumTransaction();
    } else {
      try {
        stm->createNewHashOfChunkChecksum(util::parseInt((*itr).value));
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
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->commitChunkChecksumTransaction();
}

void PieceHashMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setMessageDigestOfChunkChecksum(util::strip(characters));
  stm->addHashOfChunkChecksum();
}

void SignatureMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setBodyOfSignature(util::strip(characters));
  stm->commitSignatureTransaction();
}

void ResourcesMetalinkParserState::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri != METALINK3_NAMESPACE_URI) {
    stm->setSkipTagState();
  } else if(localname == URL) {
    stm->setURLState();
    std::string type;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, TYPE);
      if(itr == attrs.end()) {
        return;
      } else {
        type = util::strip((*itr).value);
      }
    }
    std::string location;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, LOCATION);
      if(itr != attrs.end()) {
        location = util::strip((*itr).value);
      }
    }
    int preference;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, PREFERENCE);
      if(itr == attrs.end()) {
        preference = MetalinkResource::getLowestPriority();
      } else {
        try {
          // In Metalink3Spec, highest prefernce value is 100.  We
          // uses Metalink4Spec priority unit system in which 1 is
          // higest.
          preference = 101-util::parseInt((*itr).value);
        } catch(RecoverableException& e) {
          preference = MetalinkResource::getLowestPriority();
        }
      }
    }
    int maxConnections;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs,MAXCONNECTIONS);
      if(itr == attrs.end()) {
        maxConnections = -1;
      } else {
        try {
          maxConnections = util::parseInt((*itr).value);
        } catch(RecoverableException& e) {
          maxConnections = -1;
        }
      }
    }
    stm->newResourceTransaction();
    stm->setTypeOfResource(type);
    stm->setLocationOfResource(location);
    stm->setPriorityOfResource(preference);
    stm->setMaxConnectionsOfResource(maxConnections);
  } else {
    stm->setSkipTagState();
  }
}

void URLMetalinkParserState::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setURLOfResource(util::strip(characters));
  stm->commitResourceTransaction();
}

} // namespace aria2
