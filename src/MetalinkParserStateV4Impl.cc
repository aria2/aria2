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
}

const std::string METALINK4_NAMESPACE_URI("urn:ietf:params:xml:ns:metalink");

namespace {
class FindAttr {
private:
  const std::string& _localname;
public:
  FindAttr(const std::string& localname):_localname(localname) {}

  bool operator()(const XmlAttr& attr) const
  {
    return attr.localname == _localname &&
      (attr.nsUri.empty() || attr.nsUri == METALINK4_NAMESPACE_URI);
  }
};
}

template<typename Container>
static typename Container::const_iterator findAttr
(const Container& attrs, const std::string& localname)
{
  return std::find_if(attrs.begin(), attrs.end(), FindAttr(localname));
}

void MetalinkMetalinkParserStateV4::beginElement
(MetalinkParserStateMachine* stm,
 const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(nsUri == METALINK4_NAMESPACE_URI && localname == FILE) {
    std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, NAME);
    if(itr != attrs.end()) {
      // TODO Windows path separator support.
      if(util::detectDirTraversal((*itr).value)) {
        stm->setSkipTagState();
      } else {
        stm->setFileStateV4();
        stm->newEntryTransaction();
        stm->setFileNameOfEntry((*itr).value);
      }
    }
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
    stm->setURLStateV4();
    // TODO currently NAME is ignored
    int priority;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, PRIORITY);
      if(itr == attrs.end()) {
        priority = MetalinkResource::getLowestPriority();
      } else {
        try {
          priority = util::parseInt((*itr).value);
          if(priority < 1 || MetalinkResource::getLowestPriority() < priority) {
            priority = MetalinkResource::getLowestPriority();
          }
        } catch(RecoverableException& e) {
          priority = MetalinkResource::getLowestPriority();
        }
      }
    }
    std::string mediatype;
    {
      std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, MEDIATYPE);
      if(itr == attrs.end()) {
        return;
      } else {
        mediatype = (*itr).value;
      }
    }
    stm->newResourceTransaction();
    stm->setPriorityOfResource(priority);
    stm->setTypeOfResource(mediatype);
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
            priority = MetalinkResource::getLowestPriority();
          }
        } catch(RecoverableException& e) {
          priority = MetalinkResource::getLowestPriority();
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
    if(itr == attrs.end()) {
      return;
    } else {
      std::string type = (*itr).value;
      stm->newChecksumTransaction();
      stm->setTypeOfChecksum(type);
    }
  } else if(localname == PIECES) {
    stm->setPiecesStateV4();
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
	  type = (*itr).value;
	}
      }
      stm->newChunkChecksumTransactionV4();
      stm->setLengthOfChunkChecksumV4(length);
      stm->setTypeOfChunkChecksumV4(type);
    } catch(RecoverableException& e) {
      stm->cancelChunkChecksumTransactionV4();
    }
  }
#endif // ENABLE_MESSAGE_DIGEST
  else if(localname == SIGNATURE) {
    stm->setSignatureStateV4();
    std::vector<XmlAttr>::const_iterator itr = findAttr(attrs, MEDIATYPE);
    if(itr == attrs.end()) {
      return;
    } else {
      stm->newSignatureTransaction();
      stm->setTypeOfSignature((*itr).value);
    }
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
    stm->setFileLengthOfEntry(util::parseLLInt(characters));
  } catch(RecoverableException& e) {
    // current metalink specification doesn't require size element.
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
  stm->commitChunkChecksumTransaction();
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

} // namespace aria2
