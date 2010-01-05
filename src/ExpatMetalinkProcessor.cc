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
#include "DefaultDiskWriter.h"
#include "MetalinkParserStateMachine.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "util.h"
#include "message.h"
#include "DlAbortEx.h"

namespace aria2 {

class SessionData {
public:
  SharedHandle<MetalinkParserStateMachine> _stm;

  std::deque<std::string> _charactersStack;

  SessionData(const SharedHandle<MetalinkParserStateMachine>& stm):_stm(stm) {}
};

static void mlStartElement(void* userData, const char* name, const char** attrs)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);

  std::map<std::string, std::string> attrmap;
  if(attrs) {
    const char** p = attrs;
    while(*p != 0) {
      std::string name = *p++;
      if(*p == 0) {
        break;
      }
      std::string value = util::trim(*p++);
      attrmap[name] = value;
    }
  }
  sd->_stm->beginElement(name, attrmap);
  if(sd->_stm->needsCharactersBuffering()) {
    sd->_charactersStack.push_front(std::string());
  }
}

static void mlEndElement(void* userData, const char* name)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  std::string characters;
  if(sd->_stm->needsCharactersBuffering()) {
    characters = util::trim(sd->_charactersStack.front());
    sd->_charactersStack.pop_front();
  }
  sd->_stm->endElement(name, characters);
}

static void mlCharacters(void* userData, const char* ch, int len)
{
  SessionData* sd = reinterpret_cast<SessionData*>(userData);
  if(sd->_stm->needsCharactersBuffering()) {
    sd->_charactersStack.front() += std::string(&ch[0], &ch[len]);
  }
}

SharedHandle<Metalinker>
MetalinkProcessor::parseFile(const std::string& filename)
{
  SharedHandle<DefaultDiskWriter> dw(new DefaultDiskWriter(filename));
  dw->openExistingFile();

  return parseFromBinaryStream(dw);
}
         
SharedHandle<Metalinker>
MetalinkProcessor::parseFromBinaryStream(const SharedHandle<BinaryStream>& binaryStream)
{
  _stm.reset(new MetalinkParserStateMachine());
  ssize_t bufSize = 4096;
  unsigned char buf[bufSize];

  SharedHandle<SessionData> sessionData(new SessionData(_stm));
  XML_Parser parser = XML_ParserCreate(0);
  try {
    XML_SetUserData(parser, sessionData.get());
    XML_SetElementHandler(parser, &mlStartElement, &mlEndElement);
    XML_SetCharacterDataHandler(parser, &mlCharacters);

    off_t readOffset = 0;
    while(1) {
      ssize_t res = binaryStream->readData(buf, bufSize, readOffset);
      if(res == 0) {
        break;
      }
      if(XML_Parse(parser, reinterpret_cast<const char*>(buf), res, 0) ==
         XML_STATUS_ERROR) {
        throw DL_ABORT_EX(MSG_CANNOT_PARSE_METALINK);
      }
      readOffset += res;
    }
    if(XML_Parse(parser, 0, 0, 1) == XML_STATUS_ERROR) {
      throw DL_ABORT_EX(MSG_CANNOT_PARSE_METALINK);
    }
  } catch(Exception& e) {
    XML_ParserFree(parser);
    throw;
  }
  XML_ParserFree(parser);
  if(!_stm->finished()) {
    throw DL_ABORT_EX(MSG_CANNOT_PARSE_METALINK);
  }
  return _stm->getResult();
}

} // namespace aria2
