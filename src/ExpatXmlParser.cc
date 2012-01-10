/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#include "ExpatXmlParser.h"

#include <cstdio>
#include <cstring>
#include <deque>

#include <expat.h>

#include "a2io.h"
#include "BinaryStream.h"
#include "BufferedFile.h"
#include "ParserStateMachine.h"
#include "A2STR.h"
#include "a2functional.h"
#include "XmlAttr.h"

namespace aria2 {

namespace {
struct SessionData {
  std::deque<std::string> charactersStack_;
  ParserStateMachine* psm_;
  SessionData(ParserStateMachine* psm)
    : psm_(psm)
  {}
};
} // namespace

namespace {
void splitNsName(const char** localname, const char** nsUri, const char* src)
{
  const char* sep = strchr(src, '\t');
  if(sep) {
    *localname = sep+1;
    size_t nsUriLen = sep-src;
    char* temp = new char[nsUriLen+1];
    memcpy(temp, src, nsUriLen);
    temp[nsUriLen] = '\0';
    *nsUri = temp;
  } else {
    *localname = src;
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
      XmlAttr xa;
      const char* attrNsName = *p++;
      if(*p == 0) {
        break;
      }
      splitNsName(&xa.localname, &xa.nsUri, attrNsName);
      const char* value = *p++;
      xa.value = value;
      xa.valueLength = strlen(value);
      xmlAttrs.push_back(xa);
    }
  }
  const char* localname = 0;
  const char* prefix = 0;
  const char* nsUri = 0;
  splitNsName(&localname, &nsUri, nsName);
  sd->psm_->beginElement(localname, prefix, nsUri, xmlAttrs);
  delete [] nsUri;
  for(std::vector<XmlAttr>::iterator i = xmlAttrs.begin(),
        eoi = xmlAttrs.end(); i != eoi; ++i) {
    delete [] (*i).nsUri;
  }
  if(sd->psm_->needsCharactersBuffering()) {
    sd->charactersStack_.push_front(A2STR::NIL);
  }
}
} // namespace

namespace {
void mlEndElement(void* userData, const char* nsName)
{
  const char* localname = 0;
  const char* prefix = 0;
  const char* nsUri = 0;
  splitNsName(&localname, &nsUri, nsName);
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::string characters;
  if(sd->psm_->needsCharactersBuffering()) {
    characters = sd->charactersStack_.front();
    sd->charactersStack_.pop_front();
  }
  sd->psm_->endElement(localname, prefix, nsUri, characters);
  delete [] nsUri;
}
} // namespace

namespace {
void mlCharacters(void* userData, const char* ch, int len)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  if(sd->psm_->needsCharactersBuffering()) {
    sd->charactersStack_.front().append(&ch[0], &ch[len]);
  }
}
} // namespace

XmlParser::XmlParser(ParserStateMachine* psm)
  : psm_(psm)
{}

XmlParser::~XmlParser() {}

namespace {
XML_Parser createParser(SessionData* sd)
{
  XML_Parser parser = XML_ParserCreateNS(0, static_cast<const XML_Char>('\t'));
  XML_SetUserData(parser, sd);
  XML_SetElementHandler(parser, &mlStartElement, &mlEndElement);
  XML_SetCharacterDataHandler(parser, &mlCharacters);
  return parser;
}
} // namespace

bool XmlParser::parseFile(const char* filename)
{
  BufferedFile* fp = 0;
  if(strcmp(filename, DEV_STDIN) == 0) {
    fp = new BufferedFile(stdin);
    auto_delete_d<BufferedFile*> deleter(fp);
    return parseFile(fp);
  } else {
    fp = new BufferedFile(filename, BufferedFile::READ);
    auto_delete_d<BufferedFile*> deleter(fp);
    return parseFile(fp);
  }
}

bool XmlParser::parseFile(BufferedFile* fp)
{
  char buf[4096];
  SessionData sessionData(psm_);
  XML_Parser parser = createParser(&sessionData);
  auto_delete<XML_Parser> deleter(parser, XML_ParserFree);
  while(1) {
    size_t res = fp->read(buf, sizeof(buf));
    if(XML_Parse(parser, buf, res, 0) == XML_STATUS_ERROR) {
      return false;
    }
    if(res < sizeof(buf)) {
      break;
    }
  }
  return XML_Parse(parser, 0, 0, 1) != XML_STATUS_ERROR && psm_->finished();
}
         
bool XmlParser::parseBinaryStream(BinaryStream* bs)
{
  const ssize_t bufSize = 4096;
  unsigned char buf[bufSize];
  SessionData sessionData(psm_);
  XML_Parser parser = createParser(&sessionData);
  auto_delete<XML_Parser> deleter(parser, XML_ParserFree);
  off_t readOffset = 0;
  while(1) {
    ssize_t res = bs->readData(buf, bufSize, readOffset);
    if(res == 0) {
      break;
    }
    if(XML_Parse(parser, reinterpret_cast<const char*>(buf), res, 0) ==
       XML_STATUS_ERROR) {
      return false;
    }
    readOffset += res;
  }
  return XML_Parse(parser, 0, 0, 1) != XML_STATUS_ERROR && psm_->finished();
}

bool XmlParser::parseMemory(const char* xml, size_t size)
{
  SessionData sessionData(psm_);
  XML_Parser parser = createParser(&sessionData);
  auto_delete<XML_Parser> deleter(parser, XML_ParserFree);
  if(XML_Parse(parser, xml, size, 0) == XML_STATUS_ERROR) {
    return false;
  }
  return XML_Parse(parser, 0, 0, 1) != XML_STATUS_ERROR && psm_->finished();
}

} // namespace aria2
