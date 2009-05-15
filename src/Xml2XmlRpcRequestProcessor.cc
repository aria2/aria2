/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "Xml2XmlRpcRequestProcessor.h"

#include <stack>

#include <libxml/parser.h>

#include "XmlRpcRequestParserStateMachine.h"
#include "Util.h"
#include "DlAbortEx.h"

namespace aria2 {

namespace xmlrpc {

struct SessionData {
  XmlRpcRequestParserStateMachine* _stm;

  std::stack<std::string> _charactersStack;

  SessionData(XmlRpcRequestParserStateMachine* stm):_stm(stm) {}
};

static void mlStartElement(void* userData, const xmlChar* name,
			   const xmlChar** attrs)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::map<std::string, std::string> attrmap;
  if(attrs) {
    const xmlChar** p = attrs;
    while(*p != 0) {
      std::string name = (const char*)*p++;
      if(*p == 0) {
	break;
      }
      std::string value = Util::trim((const char*)*p++);
      attrmap[name] = value;
    }
  }
  sd->_stm->beginElement((const char*)name, attrmap);
  if(sd->_stm->needsCharactersBuffering()) {
    sd->_charactersStack.push(std::string());
  }
}

static void mlEndElement(void* userData, const xmlChar* name)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::string characters;
  if(sd->_stm->needsCharactersBuffering()) {
    characters = Util::trim(sd->_charactersStack.top());
    sd->_charactersStack.pop();
  }
  sd->_stm->endElement((const char*)name, characters);
}

static void mlCharacters(void* userData, const xmlChar* ch, int len)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  if(sd->_stm->needsCharactersBuffering()) {
    sd->_charactersStack.top() += std::string(&ch[0], &ch[len]);
  }
}

static xmlSAXHandler mySAXHandler =
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
  &mlStartElement, //   startElementSAXFunc
  &mlEndElement, //   endElementSAXFunc
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
  0, //   unsigned int	initialized
  0, //   void *	_private
  0, //   startElementNsSAX2Func
  0, //   endElementNsSAX2Func
  0, //   xmlStructuredErrorFunc
};

XmlRpcRequest
XmlRpcRequestProcessor::parseMemory(const std::string& xml)
{
  _stm.reset(new XmlRpcRequestParserStateMachine());
  SharedHandle<SessionData> sessionData(new SessionData(_stm.get()));

  int r = xmlSAXUserParseMemory(&mySAXHandler, sessionData.get(),
				xml.data(), xml.size());
  if(r != 0) {
    throw DlAbortEx("Failed to parse xml-rpc request.");
  }
  return XmlRpcRequest(_stm->getMethodName(), _stm->getCurrentFrameValue());
}

} // namespace xmlrpc

} // namespace aria2
