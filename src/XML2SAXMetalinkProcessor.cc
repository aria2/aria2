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
#include "XML2SAXMetalinkProcessor.h"

#include <cassert>

#include "BinaryStream.h"
#include "MetalinkParserStateMachine.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "util.h"
#include "message.h"
#include "DlAbortEx.h"
#include "A2STR.h"
#include "error_code.h"

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
void mlStartElement
(void* userData,
 const xmlChar* srcLocalname,
 const xmlChar* srcPrefix,
 const xmlChar* srcNsUri,
 int numNamespaces,
 const xmlChar **namespaces,
 int numAttrs,
 int numDefaulted,
 const xmlChar **attrs)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::vector<XmlAttr> xmlAttrs;
  size_t index = 0;
  for(int attrIndex = 0; attrIndex < numAttrs; ++attrIndex, index += 5) {
    XmlAttr xmlAttr;
    assert(attrs[index]);
    xmlAttr.localname = reinterpret_cast<const char*>(attrs[index]);
    if(attrs[index+1]) {
      xmlAttr.prefix = reinterpret_cast<const char*>(attrs[index+1]);
    }
    if(attrs[index+2]) {
      xmlAttr.nsUri = reinterpret_cast<const char*>(attrs[index+2]);
    }
    const char* valueBegin = reinterpret_cast<const char*>(attrs[index+3]);
    const char* valueEnd = reinterpret_cast<const char*>(attrs[index+4]);
    xmlAttr.value = std::string(valueBegin, valueEnd);
    xmlAttrs.push_back(xmlAttr);
  }
  assert(srcLocalname);
  std::string localname = reinterpret_cast<const char*>(srcLocalname);
  std::string prefix;
  std::string nsUri;
  if(srcPrefix) {
    prefix = reinterpret_cast<const char*>(srcPrefix);
  }
  if(srcNsUri) {
    nsUri = reinterpret_cast<const char*>(srcNsUri);
  }
  sd->stm_->beginElement(localname, prefix, nsUri, xmlAttrs);
  if(sd->stm_->needsCharactersBuffering()) {
    sd->charactersStack_.push_front(A2STR::NIL);
  }
}
} // namespace

namespace {
void mlEndElement
(void* userData,
 const xmlChar* srcLocalname,
 const xmlChar* srcPrefix,
 const xmlChar* srcNsUri)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::string characters;
  if(sd->stm_->needsCharactersBuffering()) {
    characters = sd->charactersStack_.front();
    sd->charactersStack_.pop_front();
  }
  std::string localname = reinterpret_cast<const char*>(srcLocalname);
  std::string prefix;
  std::string nsUri;
  if(srcPrefix) {
    prefix = reinterpret_cast<const char*>(srcPrefix);
  }
  if(srcNsUri) {
    nsUri = reinterpret_cast<const char*>(srcNsUri);
  }
  sd->stm_->endElement(localname, prefix, nsUri, characters);
}
} // namespace

namespace {
void mlCharacters(void* userData, const xmlChar* ch, int len)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  if(sd->stm_->needsCharactersBuffering()) {
    sd->charactersStack_.front() += std::string(&ch[0], &ch[len]);
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

MetalinkProcessor::MetalinkProcessor() {}

MetalinkProcessor::~MetalinkProcessor() {}

SharedHandle<Metalinker>
MetalinkProcessor::parseFile
(const std::string& filename,
 const std::string& baseUri)
{
  stm_.reset(new MetalinkParserStateMachine());
  stm_->setBaseUri(baseUri);
  SharedHandle<SessionData> sessionData(new SessionData(stm_));
  // Old libxml2(at least 2.7.6, Ubuntu 10.04LTS) does not read stdin
  // when "/dev/stdin" is passed as filename while 2.7.7 does. So we
  // convert DEV_STDIN to "-" for compatibility.
  std::string nfilename;
  if(filename == DEV_STDIN) {
    nfilename = "-";
  } else {
    nfilename = filename;
  }
  int retval = xmlSAXUserParseFile(&mySAXHandler, sessionData.get(),
                                   nfilename.c_str());
  if(retval != 0) {
    throw DL_ABORT_EX2(MSG_CANNOT_PARSE_METALINK,
                       error_code::METALINK_PARSE_ERROR);
  }
  if(!stm_->finished()) {
    throw DL_ABORT_EX2(MSG_CANNOT_PARSE_METALINK,
                       error_code::METALINK_PARSE_ERROR);
  }
  if(!stm_->getErrors().empty()) {
    throw DL_ABORT_EX2(stm_->getErrorString(),
                       error_code::METALINK_PARSE_ERROR);
  }
  return stm_->getResult();
}
         
SharedHandle<Metalinker>
MetalinkProcessor::parseFromBinaryStream
(const SharedHandle<BinaryStream>& binaryStream,
 const std::string& baseUri)
{
  stm_.reset(new MetalinkParserStateMachine());
  stm_->setBaseUri(baseUri);
  size_t bufSize = 4096;
  unsigned char buf[bufSize];

  ssize_t res = binaryStream->readData(buf, 4, 0);
  if(res != 4) {
    throw DL_ABORT_EX2("Too small data for parsing XML.",
                       error_code::METALINK_PARSE_ERROR);
  }

  SharedHandle<SessionData> sessionData(new SessionData(stm_));
  xmlParserCtxtPtr ctx = xmlCreatePushParserCtxt
    (&mySAXHandler, sessionData.get(),
     reinterpret_cast<const char*>(buf), res, 0);
  auto_delete<xmlParserCtxtPtr> deleter(ctx, xmlFreeParserCtxt);

  off_t readOffset = res;
  while(1) {
    ssize_t res = binaryStream->readData(buf, bufSize, readOffset);
    if(res == 0) {
      break;
    }
    if(xmlParseChunk(ctx, reinterpret_cast<const char*>(buf), res, 0) != 0) {
      throw DL_ABORT_EX2(MSG_CANNOT_PARSE_METALINK,
                         error_code::METALINK_PARSE_ERROR);
    }
    readOffset += res;
  }
  xmlParseChunk(ctx, reinterpret_cast<const char*>(buf), 0, 1);

  if(!stm_->finished()) {
    throw DL_ABORT_EX2(MSG_CANNOT_PARSE_METALINK,
                       error_code::METALINK_PARSE_ERROR);
  }
  if(!stm_->getErrors().empty()) {
    throw DL_ABORT_EX2(stm_->getErrorString(),
                       error_code::METALINK_PARSE_ERROR);
  }
  return stm_->getResult();
}

} // namespace aria2
