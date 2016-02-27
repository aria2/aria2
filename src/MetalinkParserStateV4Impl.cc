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

#include <cstring>

#include "MetalinkParserStateMachine.h"
#include "RecoverableException.h"
#include "MetalinkResource.h"
#include "util.h"
#include "XmlAttr.h"

namespace aria2 {

const char METALINK4_NAMESPACE_URI[] = "urn:ietf:params:xml:ns:metalink";

namespace {
bool checkNsUri(const char* nsUri)
{
  return nsUri && strcmp(nsUri, METALINK4_NAMESPACE_URI) == 0;
}
} // namespace

void MetalinkMetalinkParserStateV4::beginElement(
    MetalinkParserStateMachine* psm, const char* localname, const char* prefix,
    const char* nsUri, const std::vector<XmlAttr>& attrs)
{
  if (checkNsUri(nsUri) && strcmp(localname, "file") != 0) {
    psm->setSkipTagState();
    return;
  }

  psm->setFileStateV4();
  auto itr = findAttr(attrs, "name", METALINK4_NAMESPACE_URI);
  if (itr == attrs.end() || (*itr).valueLength == 0) {
    psm->logError("Missing file@name");
    return;
  }
  std::string name((*itr).value, (*itr).valueLength);
  if (util::detectDirTraversal(name)) {
    psm->logError("Bad file@name");
    return;
  }
  psm->newEntryTransaction();
  psm->setFileNameOfEntry(name);
}

void FileMetalinkParserStateV4::beginElement(MetalinkParserStateMachine* psm,
                                             const char* localname,
                                             const char* prefix,
                                             const char* nsUri,
                                             const std::vector<XmlAttr>& attrs)
{
  if (!checkNsUri(nsUri)) {
    psm->setSkipTagState();
  }
  else if (strcmp(localname, "size") == 0) {
    psm->setSizeStateV4();
  }
  else if (strcmp(localname, "version") == 0) {
    psm->setVersionStateV4();
  }
  else if (strcmp(localname, "language") == 0) {
    psm->setLanguageStateV4();
  }
  else if (strcmp(localname, "os") == 0) {
    psm->setOSStateV4();
  }
  else if (strcmp(localname, "metaurl") == 0) {
    psm->setMetaurlStateV4();
    std::string name;
    {
      auto itr = findAttr(attrs, "name", METALINK4_NAMESPACE_URI);
      if (itr != attrs.end()) {
        name.assign((*itr).value, (*itr).valueLength);
        if (name.empty() || util::detectDirTraversal(name)) {
          psm->logError("Bad metaurl@name");
          return;
        }
      }
    }
    int priority;
    {
      auto itr = findAttr(attrs, "priority", METALINK4_NAMESPACE_URI);
      if (itr == attrs.end()) {
        priority = MetalinkResource::getLowestPriority();
      }
      else if (util::parseIntNoThrow(
                   priority, std::string((*itr).value, (*itr).valueLength))) {
        if (priority < 1 || MetalinkResource::getLowestPriority() < priority) {
          psm->logError("metaurl@priority is out of range");
          return;
        }
      }
      else {
        psm->logError("Bad metaurl@priority");
        return;
      }
    }
    std::string mediatype;
    {
      auto itr = findAttr(attrs, "mediatype", METALINK4_NAMESPACE_URI);
      if (itr == attrs.end() || (*itr).valueLength == 0) {
        psm->logError("Missing metaurl@mediatype");
        return;
      }
      mediatype.assign((*itr).value, (*itr).valueLength);
    }
    psm->newMetaurlTransaction();
    psm->setPriorityOfMetaurl(priority);
    psm->setMediatypeOfMetaurl(mediatype);
    psm->setNameOfMetaurl(name);
  }
  else if (strcmp(localname, "url") == 0) {
    psm->setURLStateV4();
    std::string location;
    {
      auto itr = findAttr(attrs, "location", METALINK4_NAMESPACE_URI);
      if (itr != attrs.end()) {
        location.assign((*itr).value, (*itr).valueLength);
      }
    }
    int priority;
    {
      auto itr = findAttr(attrs, "priority", METALINK4_NAMESPACE_URI);
      if (itr == attrs.end()) {
        priority = MetalinkResource::getLowestPriority();
      }
      else if (util::parseIntNoThrow(
                   priority, std::string((*itr).value, (*itr).valueLength))) {
        if (priority < 1 || MetalinkResource::getLowestPriority() < priority) {
          psm->logError("url@priority is out of range");
          return;
        }
      }
      else {
        psm->logError("Bad url@priority");
        return;
      }
    }
    psm->newResourceTransaction();
    psm->setLocationOfResource(location);
    psm->setPriorityOfResource(priority);
  }
  else if (strcmp(localname, "hash") == 0) {
    psm->setHashStateV4();
    auto itr = findAttr(attrs, "type", METALINK4_NAMESPACE_URI);
    if (itr == attrs.end() || (*itr).valueLength == 0) {
      psm->logError("Missing hash@type");
      return;
    }
    psm->newChecksumTransaction();
    psm->setTypeOfChecksum(std::string((*itr).value, (*itr).valueLength));
  }
  else if (strcmp(localname, "pieces") == 0) {
    psm->setPiecesStateV4();
    uint32_t length;
    {
      auto itr = findAttr(attrs, "length", METALINK4_NAMESPACE_URI);
      if (itr == attrs.end() || (*itr).valueLength == 0) {
        psm->logError("Missing pieces@length");
        return;
      }
      if (!util::parseUIntNoThrow(
              length, std::string((*itr).value, (*itr).valueLength))) {
        psm->logError("Bad pieces@length");
        return;
      }
    }
    std::string type;
    {
      auto itr = findAttr(attrs, "type", METALINK4_NAMESPACE_URI);
      if (itr == attrs.end() || (*itr).valueLength == 0) {
        psm->logError("Missing pieces@type");
        return;
      }
      type.assign((*itr).value, (*itr).valueLength);
    }
    psm->newChunkChecksumTransactionV4();
    psm->setLengthOfChunkChecksumV4(length);
    psm->setTypeOfChunkChecksumV4(type);
  }
  else if (strcmp(localname, "signature") == 0) {
    psm->setSignatureStateV4();
    auto itr = findAttr(attrs, "mediatype", METALINK4_NAMESPACE_URI);
    if (itr == attrs.end() || (*itr).valueLength == 0) {
      psm->logError("Missing signature@mediatype");
      return;
    }
    psm->newSignatureTransaction();
    psm->setTypeOfSignature(std::string((*itr).value, (*itr).valueLength));
  }
  else {
    psm->setSkipTagState();
  }
}

void FileMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                           const char* localname,
                                           const char* prefix,
                                           const char* nsUri,
                                           std::string characters)
{
  psm->commitEntryTransaction();
}

void SizeMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                           const char* localname,
                                           const char* prefix,
                                           const char* nsUri,
                                           std::string characters)
{
  int64_t size;
  if (util::parseLLIntNoThrow(size, characters) && size >= 0 &&
      size <= std::numeric_limits<a2_off_t>::max()) {
    psm->setFileLengthOfEntry(size);
  }
  else {
    psm->cancelEntryTransaction();
    psm->logError("Bad size");
  }
}

void VersionMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                              const char* localname,
                                              const char* prefix,
                                              const char* nsUri,
                                              std::string characters)
{
  psm->setVersionOfEntry(std::move(characters));
}

void LanguageMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                               const char* localname,
                                               const char* prefix,
                                               const char* nsUri,
                                               std::string characters)
{
  psm->setLanguageOfEntry(std::move(characters));
}

void OSMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                         const char* localname,
                                         const char* prefix, const char* nsUri,
                                         std::string characters)
{
  psm->setOSOfEntry(std::move(characters));
}

void HashMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                           const char* localname,
                                           const char* prefix,
                                           const char* nsUri,
                                           std::string characters)
{
  psm->setHashOfChecksum(std::move(characters));
  psm->commitChecksumTransaction();
}

void PiecesMetalinkParserStateV4::beginElement(
    MetalinkParserStateMachine* psm, const char* localname, const char* prefix,
    const char* nsUri, const std::vector<XmlAttr>& attrs)
{
  if (checkNsUri(nsUri) && strcmp(localname, "hash") == 0) {
    psm->setPieceHashStateV4();
  }
  else {
    psm->setSkipTagState();
  }
}

void PiecesMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                             const char* localname,
                                             const char* prefix,
                                             const char* nsUri,
                                             std::string characters)
{
  psm->commitChunkChecksumTransactionV4();
}

void PieceHashMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                                const char* localname,
                                                const char* prefix,
                                                const char* nsUri,
                                                std::string characters)
{
  psm->addHashOfChunkChecksumV4(std::move(characters));
}

void SignatureMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                                const char* localname,
                                                const char* prefix,
                                                const char* nsUri,
                                                std::string characters)
{
  psm->setBodyOfSignature(std::move(characters));
  psm->commitSignatureTransaction();
}

void URLMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                          const char* localname,
                                          const char* prefix, const char* nsUri,
                                          std::string characters)
{
  psm->setURLOfResource(std::move(characters));
  psm->commitResourceTransaction();
}

void MetaurlMetalinkParserStateV4::endElement(MetalinkParserStateMachine* psm,
                                              const char* localname,
                                              const char* prefix,
                                              const char* nsUri,
                                              std::string characters)
{
  psm->setURLOfMetaurl(std::move(characters));
  psm->commitMetaurlTransaction();
}

} // namespace aria2
