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

#include "a2io.h"
#include "ParserStateMachine.h"
#include "A2STR.h"
#include "a2functional.h"
#include "XmlAttr.h"

namespace aria2 {

namespace xml {

namespace {
void splitNsName(const char** localname, const char** nsUri, const char* src)
{
  const char* sep = strchr(src, '\t');
  if (sep) {
    *localname = sep + 1;
    size_t nsUriLen = sep - src;
    auto temp = new char[nsUriLen + 1];
    memcpy(temp, src, nsUriLen);
    temp[nsUriLen] = '\0';
    *nsUri = temp;
  }
  else {
    *localname = src;
  }
}
} // namespace

namespace {
void mlStartElement(void* userData, const char* nsName, const char** attrs)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::vector<XmlAttr> xmlAttrs;
  if (attrs) {
    const char** p = attrs;
    while (*p) {
      XmlAttr xa;
      const char* attrNsName = *p++;
      if (!*p) {
        break;
      }
      splitNsName(&xa.localname, &xa.nsUri, attrNsName);
      const char* value = *p++;
      xa.value = value;
      xa.valueLength = strlen(value);
      xmlAttrs.push_back(xa);
    }
  }
  const char* localname = nullptr;
  const char* prefix = nullptr;
  const char* nsUri = nullptr;
  splitNsName(&localname, &nsUri, nsName);
  sd->psm->beginElement(localname, prefix, nsUri, xmlAttrs);
  delete[] nsUri;
  for (auto& a : xmlAttrs) {
    delete[] a.nsUri;
  }
  if (sd->psm->needsCharactersBuffering()) {
    sd->charactersStack.push_front(A2STR::NIL);
  }
}
} // namespace

namespace {
void mlEndElement(void* userData, const char* nsName)
{
  const char* localname = nullptr;
  const char* prefix = nullptr;
  const char* nsUri = nullptr;
  splitNsName(&localname, &nsUri, nsName);
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::string characters;
  if (sd->psm->needsCharactersBuffering()) {
    characters = std::move(sd->charactersStack.front());
    sd->charactersStack.pop_front();
  }
  sd->psm->endElement(localname, prefix, nsUri, std::move(characters));
  delete[] nsUri;
}
} // namespace

namespace {
void mlCharacters(void* userData, const char* ch, int len)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  if (sd->psm->needsCharactersBuffering()) {
    sd->charactersStack.front().append(&ch[0], &ch[len]);
  }
}
} // namespace

namespace {
void setupParser(XML_Parser parser, SessionData* sd)
{
  XML_SetUserData(parser, sd);
  XML_SetElementHandler(parser, &mlStartElement, &mlEndElement);
  XML_SetCharacterDataHandler(parser, &mlCharacters);
}
} // namespace

XmlParser::XmlParser(ParserStateMachine* psm)
    : psm_(psm),
      sessionData_(psm_),
      ctx_(XML_ParserCreateNS(nullptr, static_cast<const XML_Char>('\t'))),
      lastError_(0)
{
  setupParser(ctx_, &sessionData_);
}

XmlParser::~XmlParser() { XML_ParserFree(ctx_); }

ssize_t XmlParser::parseUpdate(const char* data, size_t size)
{
  if (lastError_ != 0) {
    return lastError_;
  }
  XML_Status rv = XML_Parse(ctx_, data, size, 0);
  if (rv == XML_STATUS_ERROR) {
    return lastError_ = ERR_XML_PARSE;
  }
  else {
    return size;
  }
}

ssize_t XmlParser::parseFinal(const char* data, size_t size)
{
  if (lastError_ != 0) {
    return lastError_;
  }
  XML_Status rv = XML_Parse(ctx_, data, size, 1);
  if (rv == XML_STATUS_ERROR) {
    return lastError_ = ERR_XML_PARSE;
  }
  else {
    return size;
  }
}

int XmlParser::reset()
{
  psm_->reset();
  sessionData_.reset();
  XML_Bool rv = XML_ParserReset(ctx_, nullptr);
  if (rv == XML_FALSE) {
    return lastError_ = ERR_RESET;
  }
  else {
    setupParser(ctx_, &sessionData_);
    return 0;
  }
}

} // namespace xml

} // namespace aria2
