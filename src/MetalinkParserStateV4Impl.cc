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
#include "MetalinkParserStateV4Impl.h"
#include "MetalinkParserStateMachine.h"
#include "RecoverableException.h"
#include "MetalinkResource.h"
#include "util.h"

namespace aria2 {

namespace {

const std::string FILE("file");
const std::string HASH("hash");
const std::string LANGUAGE("language");
const std::string LENGTH("length");
const std::string LOCATION("location");
const std::string METALINK("metalink");
// Can't use name VERSION because it is used as a macro.
const std::string METALINK_VERSION("version");
const std::string METAURL("metaurl");
const std::string NAME("name");
const std::string OS("os");
const std::string PIECE("piece");
const std::string PIECES("pieces");
const std::string PRIORITY("priority");
const std::string SIGNATURE("signature");
const std::string SIZE("size");
const std::string TYPE("type");
const std::string MEDIATYPE("mediatype");
const std::string URL("url");
} // namespace

const std::string METALINK4_NAMESPACE_URI("urn:ietf:params:xml:ns:metalink");

namespace {
class FindAttr {
private:
  const std::string& localname_;
public:
  FindAttr(const std::string& localname):localname_(localname) {}

  bool operator()(const XmlAttr& attr) const
  {
    return attr.localname == localname_ &&
      (attr.nsUri.empty() || attr.nsUri == METALINK4_NAMESPACE_URI);
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

void MetalinkMetalinkParserStateV4::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri == METALINK4_NAMESPACE_URI && localname == FILE) {
    stm->setFileStateV4();
    std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, NAME);
    if(itr == attrs.end() || (*itr).value.empty()) {
      stm->logError("Missing file@name");
      return;
    } else if(util::detectDirTraversal((*itr).value)) {
      stm->logError("Bad file@name");
      return;
    }
    stm->newEntryTransaction();
    stm->setFileNameOfEntry((*itr).value);
  } else {
    stm->setSkipTagState();
  }
}

void FileMetalinkParserStateV4::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri != METALINK4_NAMESPACE_URI) {
    stm->setSkipTagState();
  } else if(localname == SIZE) {
    stm->setSizeStateV4();
  } else if(localname == METALINK_VERSION) {
    stm->setVersionStateV4();
  } else if(localname == LANGUAGE) {
    stm->setLanguageStateV4();
  } else if(localname == OS) {
    stm->setOSStateV4();
  } else if(localname == METAURL) {
    stm->setMetaurlStateV4();
    std::string name;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, NAME);
      if(itr != attrs.end()) {
        if((*itr).value.empty() || util::detectDirTraversal((*itr).value)) {
          stm->logError("Bad metaurl@name");
          return;
        } else {
          name = (*itr).value;
        }
      }
    }
    int priority;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, PRIORITY);
      if(itr == attrs.end()) {
        priority = MetalinkResource::getLowestPriority();
      } else {
        try {
          priority = util::parseInt((*itr).value);
          if(priority < 1 || MetalinkResource::getLowestPriority() < priority) {
            stm->logError("metaurl@priority is out of range");
            return;
          }
        } catch(RecoverableException& e) {
          stm->logError("Bad metaurl@priority");
          return;
        }
      }
    }
    std::string mediatype;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, MEDIATYPE);
      if(itr == attrs.end() || (*itr).value.empty()) {
        stm->logError("Missing metaurl@mediatype");
        return;
      } else {
        mediatype = (*itr).value;
      }
    }
    stm->newMetaurlTransaction();
    stm->setPriorityOfMetaurl(priority);
    stm->setMediatypeOfMetaurl(mediatype);
    stm->setNameOfMetaurl(name);
  } else if(localname == URL) {
    stm->setURLStateV4();
    std::string location;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, LOCATION);
      if(itr != attrs.end()) {
        location = (*itr).value;
      }
    }
    int priority;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, PRIORITY);
      if(itr == attrs.end()) {
        priority = MetalinkResource::getLowestPriority();
      } else {
        try {
          priority = util::parseInt((*itr).value);
          if(priority < 1 || MetalinkResource::getLowestPriority() < priority) {
            stm->logError("url@priority is out of range");
            return;
          }
        } catch(RecoverableException& e) {
          stm->logError("Bad url@priority");
          return;
        }
      }
    }
    stm->newResourceTransaction();
    stm->setLocationOfResource(location);
    stm->setPriorityOfResource(priority);
  }
#ifdef ENABLE_MESSAGE_DIGEST
  else if(localname == HASH) {
    stm->setHashStateV4();
    std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, TYPE);
    if(itr == attrs.end() || (*itr).value.empty()) {
      stm->logError("Missing hash@type");
      return;
    } else {
      std::string type = (*itr).value;
      stm->newChecksumTransaction();
      stm->setTypeOfChecksum(type);
    }
  } else if(localname == PIECES) {
    stm->setPiecesStateV4();
    size_t length;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, LENGTH);
      if(itr == attrs.end() || (*itr).value.empty()) {
        stm->logError("Missing pieces@length");
        return;
      } else {
        try {
          length = util::parseInt((*itr).value);
        } catch(RecoverableException& e) {
          stm->logError("Bad pieces@length");
          return;
        }
      }
    }
    std::string type;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, TYPE);
      if(itr == attrs.end() || (*itr).value.empty()) {
        stm->logError("Missing pieces@type");
        return;
      } else {
        type = (*itr).value;
      }
    }
    stm->newChunkChecksumTransactionV4();
    stm->setLengthOfChunkChecksumV4(length);
    stm->setTypeOfChunkChecksumV4(type);
  }
#endif // ENABLE_MESSAGE_DIGEST
  else if(localname == SIGNATURE) {
    stm->setSignatureStateV4();
    std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, MEDIATYPE);
    if(itr == attrs.end() || (*itr).value.empty()) {
      stm->logError("Missing signature@mediatype");
      return;
    }
    stm->newSignatureTransaction();
    stm->setTypeOfSignature((*itr).value);
  } else {
    stm->setSkipTagState();
  }
}

void FileMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->commitEntryTransaction();
}

void SizeMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  try {
    stm->setFileLengthOfEntry(util::parseULLInt(characters));
  } catch(RecoverableException& e) {
    stm->cancelEntryTransaction();
    stm->logError("Bad size");
  }
}

void VersionMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setVersionOfEntry(characters);
}

void LanguageMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setLanguageOfEntry(characters);
}

void OSMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setOSOfEntry(characters);
}

void HashMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setHashOfChecksum(characters);
  stm->commitChecksumTransaction();
}

void PiecesMetalinkParserStateV4::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri == METALINK4_NAMESPACE_URI && localname == HASH) {
    stm->setPieceHashStateV4();
  } else {
    stm->setSkipTagState();
  }
}

void PiecesMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->commitChunkChecksumTransactionV4();
}

void PieceHashMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->addHashOfChunkChecksumV4(characters);
}

void SignatureMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setBodyOfSignature(characters);
  stm->commitSignatureTransaction();
}

void URLMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setURLOfResource(characters);
  stm->commitResourceTransaction();
}

void MetaurlMetalinkParserStateV4::endElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  stm->setURLOfMetaurl(characters);
  stm->commitMetaurlTransaction();
}

} // namespace aria2
