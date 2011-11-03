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
#include "ExpatMetalinkProcessor.h"

#include <cstdio>
#include <cstring>

#include "DefaultDiskWriter.h"
#include "MetalinkParserStateMachine.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "util.h"
#include "message.h"
#include "DlAbortEx.h"
#include "MetalinkParserState.h"
#include "A2STR.h"
#include "error_code.h"
#include "BufferedFile.h"
#include "a2functional.h"

namespace aria2 {

namespace {
class SessionData {
public:
  SharedHandle<MetalinkParserStateMachine> stm_;

  std::deque<std::string> charactersStack_;

  SessionData(const SharedHandle<MetalinkParserStateMachine>& stm):stm_(stm) {}
};
} // namespace

namespace {
template<typename InputIterator>
void splitNsName
(std::string& localname, std::string& prefix, std::string& nsUri,
 InputIterator first, InputIterator last)
{
  std::pair<Scip, Scip> nsNamePair;
  util::divide(nsNamePair, first, last, '\t');
  if(nsNamePair.second.first == nsNamePair.second.second) {
    localname.assign(nsNamePair.first.first, nsNamePair.first.second);
  } else {
    nsUri.assign(nsNamePair.first.first, nsNamePair.first.second);
    localname.assign(nsNamePair.second.first, nsNamePair.second.second);
  }
}
} // namespace

namespace {
void mlStartElement(void* userData, const char* nsName, const char** attrs)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);

  std::vector<XmlAttr> xmlAttrs;
  if(attrs) {
    const char** p = attrs;
    while(*p != 0) {
      const char* attrNsName = *p++;
      if(*p == 0) {
        break;
      }
      XmlAttr xa;
      splitNsName(xa.localname, xa.prefix, xa.nsUri,
                  attrNsName, attrNsName+strlen(attrNsName));
      const char* value = *p++;
      xa.value.assign(value, value+strlen(value));
      xmlAttrs.push_back(xa);
    }
  }
  std::string localname;
  std::string prefix;
  std::string nsUri;
  splitNsName(localname, prefix, nsUri, nsName, nsName+strlen(nsName));
  sd->stm_->beginElement(localname, prefix, nsUri, xmlAttrs);
  if(sd->stm_->needsCharactersBuffering()) {
    sd->charactersStack_.push_front(A2STR::NIL);
  }
}
} // namespace

namespace {
void mlEndElement(void* userData, const char* nsName)
{
  std::string localname;
  std::string prefix;
  std::string nsUri;
  splitNsName(localname, prefix, nsUri, nsName, nsName+strlen(nsName));

  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::string characters;
  if(sd->stm_->needsCharactersBuffering()) {
    characters = sd->charactersStack_.front();
    sd->charactersStack_.pop_front();
  }
  sd->stm_->endElement(localname, prefix, nsUri, characters);
}
} // namespace

namespace {
void mlCharacters(void* userData, const char* ch, int len)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  if(sd->stm_->needsCharactersBuffering()) {
    sd->charactersStack_.front().append(&ch[0], &ch[len]);
  }
}
} // namespace

namespace {
XML_Parser createParser(const SharedHandle<SessionData>& sessionData)
{
  XML_Parser parser = XML_ParserCreateNS(0, static_cast<const XML_Char>('\t'));
  XML_SetUserData(parser, sessionData.get());
  XML_SetElementHandler(parser, &mlStartElement, &mlEndElement);
  XML_SetCharacterDataHandler(parser, &mlCharacters);
  return parser;
}
} // namespace

namespace {
void checkError(XML_Parser parser)
{
  if(XML_Parse(parser, 0, 0, 1) == XML_STATUS_ERROR) {
    throw DL_ABORT_EX2(MSG_CANNOT_PARSE_METALINK,
                       error_code::METALINK_PARSE_ERROR);
  }
  SessionData* sessionData =
    reinterpret_cast<SessionData*>(XML_GetUserData(parser));
  const SharedHandle<MetalinkParserStateMachine>& stm = sessionData->stm_;
  if(!stm->finished()) {
    throw DL_ABORT_EX2(MSG_CANNOT_PARSE_METALINK,
                       error_code::METALINK_PARSE_ERROR);
  }
  if(!stm->getErrors().empty()) {
    throw DL_ABORT_EX2(stm->getErrorString(),
                       error_code::METALINK_PARSE_ERROR);
  }
}
} // namespace

SharedHandle<Metalinker>
MetalinkProcessor::parseFile
(const std::string& filename,
 const std::string& baseUri)
{
  BufferedFile* fp = 0;
  auto_delete_d<BufferedFile*> deleter(fp);
  if(filename == DEV_STDIN) {
    fp = new BufferedFile(stdin);
    return parseFile(*fp);
  } else {
    fp = new BufferedFile(filename, BufferedFile::READ);
    return parseFile(*fp, baseUri);
  }
}

SharedHandle<Metalinker>
MetalinkProcessor::parseFile
(BufferedFile& fp, const std::string& baseUri)
{
  stm_.reset(new MetalinkParserStateMachine());
  stm_->setBaseUri(baseUri);
  char buf[4096];

  SharedHandle<SessionData> sessionData(new SessionData(stm_));
  XML_Parser parser = createParser(sessionData);
  auto_delete<XML_Parser> deleter(parser, XML_ParserFree);
  while(1) {
    size_t res = fp.read(buf, sizeof(buf));
    if(XML_Parse(parser, buf, res, 0) == XML_STATUS_ERROR) {
      throw DL_ABORT_EX2(MSG_CANNOT_PARSE_METALINK,
                         error_code::METALINK_PARSE_ERROR);
    }
    if(res < sizeof(buf)) {
      break;
    }
  }
  checkError(parser);
  return stm_->getResult();
}
         
SharedHandle<Metalinker>
MetalinkProcessor::parseFromBinaryStream
(const SharedHandle<BinaryStream>& binaryStream,
 const std::string& baseUri)
{
  stm_.reset(new MetalinkParserStateMachine());
  stm_->setBaseUri(baseUri);
  ssize_t bufSize = 4096;
  unsigned char buf[bufSize];

  SharedHandle<SessionData> sessionData(new SessionData(stm_));
  XML_Parser parser = createParser(sessionData);
  auto_delete<XML_Parser> deleter(parser, XML_ParserFree);
  off_t readOffset = 0;
  while(1) {
    ssize_t res = binaryStream->readData(buf, bufSize, readOffset);
    if(res == 0) {
      break;
    }
    if(XML_Parse(parser, reinterpret_cast<const char*>(buf), res, 0) ==
       XML_STATUS_ERROR) {
      throw DL_ABORT_EX2(MSG_CANNOT_PARSE_METALINK,
                         error_code::METALINK_PARSE_ERROR);
    }
    readOffset += res;
  }
  checkError(parser);
  return stm_->getResult();
}

} // namespace aria2
