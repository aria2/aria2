/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#include "Xml2XmlParser.h"

#include <cassert>
#include <cstring>

#include "a2io.h"
#include "ParserStateMachine.h"
#include "A2STR.h"
#include "a2functional.h"
#include "XmlAttr.h"

namespace aria2 {

namespace xml {

namespace {
void mlStartElement
(void* userData,
 const xmlChar* localname,
 const xmlChar* prefix,
 const xmlChar* nsUri,
 int numNamespaces,
 const xmlChar** namespaces,
 int numAttrs,
 int numDefaulted,
 const xmlChar** attrs)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::vector<XmlAttr> xmlAttrs;
  const char** pattrs = reinterpret_cast<const char**>(attrs);
  for(size_t i = 0, max = numAttrs*5; i < max; i += 5) {
    XmlAttr xmlAttr;
    assert(pattrs[i]);
    xmlAttr.localname = pattrs[i];
    if(pattrs[i+1]) {
      xmlAttr.prefix = pattrs[i+1];
    }
    if(attrs[i+2]) {
      xmlAttr.nsUri = pattrs[i+2];
    }
    xmlAttr.value = pattrs[i+3];
    xmlAttr.valueLength = pattrs[i+4]-xmlAttr.value;
    xmlAttrs.push_back(xmlAttr);
  }
  sd->psm->beginElement
    (reinterpret_cast<const char*>(localname),
     reinterpret_cast<const char*>(prefix),
     reinterpret_cast<const char*>(nsUri),
     xmlAttrs);
  if(sd->psm->needsCharactersBuffering()) {
    sd->charactersStack.push_front(A2STR::NIL);
  }
}
} // namespace

namespace {
void mlEndElement
(void* userData,
 const xmlChar* localname,
 const xmlChar* prefix,
 const xmlChar* nsUri)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::string characters;
  if(sd->psm->needsCharactersBuffering()) {
    characters = sd->charactersStack.front();
    sd->charactersStack.pop_front();
  }
  sd->psm->endElement
    (reinterpret_cast<const char*>(localname),
     reinterpret_cast<const char*>(prefix),
     reinterpret_cast<const char*>(nsUri),
     characters);
}
} // namespace

namespace {
void mlCharacters(void* userData, const xmlChar* ch, int len)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  if(sd->psm->needsCharactersBuffering()) {
    sd->charactersStack.front().append(&ch[0], &ch[len]);
  }
}
} // namespace

namespace {
xmlSAXHandler mySAXHandler =
  {
    0, // internalSubsetSAXFunc
    0, // isStandaloneSAXFunc
    0, // hasInternalSubsetSAXFunc
    0, // hasExternalSubsetSAXFunc
    0, // resolveEntitySAXFunc
    0, // getEntitySAXFunc
    0, // entityDeclSAXFunc
    0, // notationDeclSAXFunc
    0, // attributeDeclSAXFunc
    0, // elementDeclSAXFunc
    0, //   unparsedEntityDeclSAXFunc
    0, //   setDocumentLocatorSAXFunc
    0, //   startDocumentSAXFunc
    0, //   endDocumentSAXFunc
    0, //   startElementSAXFunc
    0, //   endElementSAXFunc
    0, //   referenceSAXFunc
    &mlCharacters, //   charactersSAXFunc
    0, //   ignorableWhitespaceSAXFunc
    0, //   processingInstructionSAXFunc
    0, //   commentSAXFunc
    0, //   warningSAXFunc
    0, //   errorSAXFunc
    0, //   fatalErrorSAXFunc
    0, //   getParameterEntitySAXFunc
    0, //   cdataBlockSAXFunc
    0, //   externalSubsetSAXFunc
    XML_SAX2_MAGIC, //   unsigned int        initialized
    0, //   void *      _private
    &mlStartElement, //   startElementNsSAX2Func
    &mlEndElement, //   endElementNsSAX2Func
    0, //   xmlStructuredErrorFunc
  };
} // namespace

XmlParser::XmlParser(ParserStateMachine* psm)
  : psm_(psm),
    sessionData_(psm),
    ctx_(xmlCreatePushParserCtxt(&mySAXHandler, &sessionData_, 0, 0, 0)),
    lastError_(0)
{}

XmlParser::~XmlParser()
{
  xmlFreeParserCtxt(ctx_);
}

ssize_t XmlParser::parseUpdate(const char* data, size_t size)
{
  if(lastError_ != 0) {
    return lastError_;
  }
  int rv = xmlParseChunk(ctx_, data, size, 0);
  if(rv != 0) {
    return lastError_ = ERR_XML_PARSE;
  } else {
    return size;
  }
}

ssize_t XmlParser::parseFinal(const char* data, size_t size)
{
  if(lastError_ != 0) {
    return lastError_;
  }
  int rv = xmlParseChunk(ctx_, data, size, 1);
  if(rv != 0) {
    return lastError_ = ERR_XML_PARSE;
  } else {
    return size;
  }
}

int XmlParser::reset()
{
  psm_->reset();
  sessionData_.reset();
  int rv = xmlCtxtResetPush(ctx_, 0, 0, 0, 0);
  if(rv != 0) {
    return lastError_ = ERR_RESET;
  } else {
    return 0;
  }
}

} // namespace xml

} // namespace aria2
