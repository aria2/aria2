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
#include "ExpatMetalinkProcessor.h"
#include "BinaryStream.h"
#include "MetalinkParserStateMachine.h"
#include "Util.h"
#include "message.h"
#include "DefaultDiskWriter.h"

class SessionData {
public:
  MetalinkParserStateMachineHandle _stm;

  Strings _charactersStack;

  SessionData(const MetalinkParserStateMachineHandle& stm):_stm(stm) {}
};

typedef SharedHandle<SessionData> SessionDataHandle;

static void mlStartElement(void* userData, const char* name, const char** attrs)
{
  ((SessionData*)userData)->_charactersStack.push_front(string());
  map<string, string> attrmap;
  if(attrs) {
    const char** p = attrs;
    while(*p != 0) {
      string name = *p++;
      if(*p == 0) {
	break;
      }
      string value = Util::trim(*p++);
      attrmap[name] = value;
    }
  }
  ((SessionData*)userData)->_stm->beginElement(name, attrmap);
}

static void mlEndElement(void* userData, const char* name)
{
  SessionData* sd = (SessionData*)userData;

  sd->_stm->endElement(name, Util::trim(sd->_charactersStack.front()));
  sd->_charactersStack.pop_front();
}

static void mlCharacters(void* userData, const char* ch, int len)
{
  ((SessionData*)userData)->_charactersStack.front() += string(&ch[0], &ch[len]);
}

ExpatMetalinkProcessor::ExpatMetalinkProcessor():
  _stm(0)
{}
	 
	 
MetalinkerHandle ExpatMetalinkProcessor::parseFile(const string& filename)
{
  DefaultDiskWriterHandle dw = new DefaultDiskWriter();
  dw->openExistingFile(filename);

  return parseFromBinaryStream(dw);
}
	 
MetalinkerHandle ExpatMetalinkProcessor::parseFromBinaryStream(const BinaryStreamHandle& binaryStream)
{
  _stm = new MetalinkParserStateMachine();
  int32_t bufSize = 4096;
  unsigned char buf[bufSize];

  SessionDataHandle sessionData = new SessionData(_stm);
  XML_Parser parser = XML_ParserCreate(0);
  try {
    XML_SetUserData(parser, sessionData.get());
    XML_SetElementHandler(parser, &mlStartElement, &mlEndElement);
    XML_SetCharacterDataHandler(parser, &mlCharacters);

    int64_t readOffset = 0;
    while(1) {
      int32_t res = binaryStream->readData(buf, bufSize, readOffset);
      if(res == 0) {
	break;
      }
      if(XML_Parse(parser, (const char*)buf, res, 0) == XML_STATUS_ERROR) {
	throw new DlAbortEx(MSG_CANNOT_PARSE_METALINK);
      }
      readOffset += res;
    }
    if(XML_Parse(parser, 0, 0, 1) == XML_STATUS_ERROR) {
      throw new DlAbortEx(MSG_CANNOT_PARSE_METALINK);
    }
  } catch(Exception* e) {
    XML_ParserFree(parser);
    throw;
  }
  XML_ParserFree(parser);
  if(!_stm->finished()) {
    throw new DlAbortEx(MSG_CANNOT_PARSE_METALINK);
  }
  return _stm->getResult();
}
