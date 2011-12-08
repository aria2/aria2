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

#include <cstring>

#include "MetalinkParserStateMachine.h"
#include "RecoverableException.h"
#include "MetalinkResource.h"
#include "util.h"
#include "XmlAttr.h"

namespace aria2 {

const char METALINK3_NAMESPACE_URI[] = "http://www.metalinker.org/";

namespace {
bool checkNsUri(const char* nsUri)
{
  return nsUri && strcmp(nsUri, METALINK3_NAMESPACE_URI) == 0;
}
} // namespace

void MetalinkMetalinkParserState::beginElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(checkNsUri(nsUri) && strcmp(localname, "files") == 0) {
    psm->setFilesState();
  } else {
    psm->setSkipTagState();
  }
}

void FilesMetalinkParserState::beginElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(checkNsUri(nsUri) && strcmp(localname, "file") == 0) {
    psm->setFileState();
    std::vector<XmlAttr>::const_iterator itr =
      findAttr(attrs, "name", METALINK3_NAMESPACE_URI);
    if(itr != attrs.end()) {
      std::string name((*itr).value, (*itr).valueLength);
      if(name.empty() || util::detectDirTraversal(name)) {
        return;
      }
      psm->newEntryTransaction();
      psm->setFileNameOfEntry(name);
    }
  } else {
    psm->setSkipTagState();
  }
}

void FileMetalinkParserState::beginElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(!checkNsUri(nsUri)) {
    psm->setSkipTagState();
  } else if(strcmp(localname, "size") == 0) {
    psm->setSizeState();
  } else if(strcmp(localname, "version") == 0) {
    psm->setVersionState();
  } else if(strcmp(localname, "language") == 0) {
    psm->setLanguageState();
  } else if(strcmp(localname, "os") == 0) {
    psm->setOSState();
  } else if(strcmp(localname, "verification") == 0) {
    psm->setVerificationState();
  } else if(strcmp(localname, "resources") == 0) {
    psm->setResourcesState();
    int maxConnections;
    std::vector<XmlAttr>::const_iterator itr =
      findAttr(attrs, "maxconnections", METALINK3_NAMESPACE_URI);
    if(itr == attrs.end()) {
      maxConnections = -1;
    } else {
      if(!util::parseIntNoThrow
         (maxConnections, std::string((*itr).value, (*itr).valueLength)) ||
         maxConnections <= 0) {
        maxConnections = -1;
      }
    }
    psm->setMaxConnectionsOfEntry(maxConnections);
  } else {
    psm->setSkipTagState();
  }
}

void FileMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->commitEntryTransaction();
}

void SizeMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  // current metalink specification doesn't require size element.
  int64_t size;
  if(util::parseLLIntNoThrow(size, characters) && size >= 0 &&
     size <= std::numeric_limits<off_t>::max()) {
    psm->setFileLengthOfEntry(size);
  }
}

void VersionMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->setVersionOfEntry(characters);
}

void LanguageMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->setLanguageOfEntry(characters);
}

void OSMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->setOSOfEntry(characters);
}

void VerificationMetalinkParserState::beginElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(!checkNsUri(nsUri)) {
    psm->setSkipTagState();
  } else
#ifdef ENABLE_MESSAGE_DIGEST
    if(strcmp(localname, "hash") == 0) {
      psm->setHashState();
      std::vector<XmlAttr>::const_iterator itr =
        findAttr(attrs, "type", METALINK3_NAMESPACE_URI);
      if(itr == attrs.end()) {
        return;
      } else {
        psm->newChecksumTransaction();
        psm->setTypeOfChecksum(std::string((*itr).value, (*itr).valueLength));
      }
    } else if(strcmp(localname, "pieces") == 0) {
      psm->setPiecesState();
      uint32_t length;
      {
        std::vector<XmlAttr>::const_iterator itr =
          findAttr(attrs, "length", METALINK3_NAMESPACE_URI);
        if(itr == attrs.end()) {
          return;
        } else {
          if(!util::parseUIntNoThrow
             (length, std::string((*itr).value, (*itr).valueLength))) {
            return;
          }
        }
      }
      std::string type;
      {
        std::vector<XmlAttr>::const_iterator itr =
          findAttr(attrs, "type", METALINK3_NAMESPACE_URI);
        if(itr == attrs.end()) {
          return;
        } else {
          type.assign((*itr).value, (*itr).valueLength);
        }
      }
      psm->newChunkChecksumTransaction();
      psm->setLengthOfChunkChecksum(length);
      psm->setTypeOfChunkChecksum(type);
    } else
#endif // ENABLE_MESSAGE_DIGEST
      if(strcmp(localname, "signature") == 0) {
        psm->setSignatureState();
        std::vector<XmlAttr>::const_iterator itr =
          findAttr(attrs, "type", METALINK3_NAMESPACE_URI);
        if(itr == attrs.end()) {
          return;
        } else {
          psm->newSignatureTransaction();
          psm->setTypeOfSignature
            (std::string((*itr).value, (*itr).valueLength));
          std::vector<XmlAttr>::const_iterator itr =
            findAttr(attrs, "file", METALINK3_NAMESPACE_URI);
          if(itr != attrs.end()) {
            std::string file((*itr).value, (*itr).valueLength);
            if(!util::detectDirTraversal(file)) {
              psm->setFileOfSignature(file);
            }
          }
        }
      } else {
        psm->setSkipTagState();
      }
}

void HashMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->setHashOfChecksum(characters);
  psm->commitChecksumTransaction();
}

void PiecesMetalinkParserState::beginElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(checkNsUri(nsUri) && strcmp(localname, "hash") == 0) {
    psm->setPieceHashState();
    std::vector<XmlAttr>::const_iterator itr =
      findAttr(attrs, "piece", METALINK3_NAMESPACE_URI);
    if(itr == attrs.end()) {
      psm->cancelChunkChecksumTransaction();
    } else {
      uint32_t idx;
      if(util::parseUIntNoThrow
         (idx, std::string((*itr).value, (*itr).valueLength))) {
        psm->createNewHashOfChunkChecksum(idx);
      } else {
        psm->cancelChunkChecksumTransaction();
      }
    }
  } else {
    psm->setSkipTagState();
  }
}

void PiecesMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->commitChunkChecksumTransaction();
}

void PieceHashMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->setMessageDigestOfChunkChecksum(characters);
  psm->addHashOfChunkChecksum();
}

void SignatureMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->setBodyOfSignature(characters);
  psm->commitSignatureTransaction();
}

void ResourcesMetalinkParserState::beginElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(!checkNsUri(nsUri)) {
    psm->setSkipTagState();
  } else if(strcmp(localname, "url") == 0) {
    psm->setURLState();
    std::string type;
    {
      std::vector<XmlAttr>::const_iterator itr =
        findAttr(attrs, "type", METALINK3_NAMESPACE_URI);
      if(itr == attrs.end()) {
        return;
      } else {
        type.assign((*itr).value, (*itr).valueLength);
      }
    }
    std::string location;
    {
      std::vector<XmlAttr>::const_iterator itr =
        findAttr(attrs, "location", METALINK3_NAMESPACE_URI);
      if(itr != attrs.end()) {
        location.assign((*itr).value, (*itr).valueLength);
      }
    }
    int preference;
    {
      std::vector<XmlAttr>::const_iterator itr =
        findAttr(attrs, "preference", METALINK3_NAMESPACE_URI);
      if(itr == attrs.end()) {
        preference = MetalinkResource::getLowestPriority();
      } else {
        if(util::parseIntNoThrow
           (preference, std::string((*itr).value, (*itr).valueLength)) &&
           preference >= 0) {
          // In Metalink3Spec, highest prefernce value is 100.  We
          // use Metalink4Spec priority unit system in which 1 is
          // higest.
          preference = 101-preference;
        } else {
          preference = MetalinkResource::getLowestPriority();
        }
      }
    }
    int maxConnections;
    {
      std::vector<XmlAttr>::const_iterator itr =
        findAttr(attrs, "maxconnections", METALINK3_NAMESPACE_URI);
      if(itr == attrs.end()) {
        maxConnections = -1;
      } else {
        if(!util::parseIntNoThrow
           (maxConnections, std::string((*itr).value, (*itr).valueLength)) ||
           maxConnections <= 0) {
          maxConnections = -1;
        }
      }
    }
    psm->newResourceTransaction();
    psm->setTypeOfResource(type);
    psm->setLocationOfResource(location);
    psm->setPriorityOfResource(preference);
    psm->setMaxConnectionsOfResource(maxConnections);
  } else {
    psm->setSkipTagState();
  }
}

void URLMetalinkParserState::endElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::string& characters)
{
  psm->setURLOfResource(characters);
  psm->commitResourceTransaction();
}

} // namespace aria2
