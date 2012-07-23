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
#include "metalink_helper.h"
#include "Option.h"
#include "MetalinkEntry.h"
#include "MetalinkParserStateMachine.h"
#include "Metalinker.h"
#include "XmlParser.h"
#include "prefs.h"
#include "DlAbortEx.h"
#include "BinaryStream.h"
#include "MetalinkMetaurl.h"

namespace aria2 {

namespace metalink {

namespace {

void query
(std::vector<SharedHandle<MetalinkEntry> >& result,
 const SharedHandle<Metalinker>& metalinker,
 const Option* option)
{
  metalinker->queryEntry(result,
                         option->get(PREF_METALINK_VERSION),
                         option->get(PREF_METALINK_LANGUAGE),
                         option->get(PREF_METALINK_OS));
}

} // namespace

void parseAndQuery
(std::vector<SharedHandle<MetalinkEntry> >& result,
 const std::string& filename,
 const Option* option,
 const std::string& baseUri)
{
  SharedHandle<Metalinker> metalinker = parseFile(filename, baseUri);
  query(result, metalinker, option);
}

void parseAndQuery
(std::vector<SharedHandle<MetalinkEntry> >& result,
 BinaryStream* bs,
 const Option* option,
 const std::string& baseUri)
{
  SharedHandle<Metalinker> metalinker = parseBinaryStream(bs, baseUri);
  query(result, metalinker, option);
}

void groupEntryByMetaurlName
(std::vector<
  std::pair<std::string, std::vector<SharedHandle<MetalinkEntry> > > >& result,
 const std::vector<SharedHandle<MetalinkEntry> >& entries)
{
  for(std::vector<SharedHandle<MetalinkEntry> >::const_iterator eiter =
        entries.begin(), eoi = entries.end(); eiter != eoi; ++eiter) {
    if((*eiter)->metaurls.empty()) {
      std::pair<std::string, std::vector<SharedHandle<MetalinkEntry> > > p;
      p.second.push_back(*eiter);
      result.push_back(p);
    } else {
      std::vector<
      std::pair<std::string,
        std::vector<SharedHandle<MetalinkEntry> > > >::iterator i =
        result.begin();
      if((*eiter)->metaurls[0]->name.empty() ||
         !(*eiter)->sizeKnown) {
        i = result.end();
      }
      for(; i != result.end(); ++i) {
        if((*i).first == (*eiter)->metaurls[0]->url &&
           !(*i).second[0]->metaurls[0]->name.empty()) {
          (*i).second.push_back(*eiter);
          break;
        }
      }
      if(i == result.end()) {
        std::pair<std::string, std::vector<SharedHandle<MetalinkEntry> > > p;
        p.first = (*eiter)->metaurls[0]->url;
        p.second.push_back(*eiter);
        result.push_back(p);
      }
    }
  }
}

SharedHandle<Metalinker> parseFile
(const std::string& filename,
 const std::string& baseUri)
{
  MetalinkParserStateMachine psm;
  psm.setBaseUri(baseUri);
  if(!xml::parseFile(filename, &psm)) {
    throw DL_ABORT_EX2("Could not parse Metalink XML document.",
                       error_code::METALINK_PARSE_ERROR);
  }
  if(!psm.getErrors().empty()) {
    throw DL_ABORT_EX2(psm.getErrorString(),
                       error_code::METALINK_PARSE_ERROR);
  }
  return psm.getResult();
}

SharedHandle<Metalinker> parseBinaryStream
(BinaryStream* bs,
 const std::string& baseUri)
{
  MetalinkParserStateMachine psm;
  psm.setBaseUri(baseUri);
  xml::XmlParser ps(&psm);
  unsigned char buf[4096];
  ssize_t nread;
  int64_t offread = 0;
  bool retval = true;
  while((nread = bs->readData(buf, sizeof(buf), offread)) > 0) {
    if(ps.parseUpdate(reinterpret_cast<const char*>(buf), nread) < 0) {
      retval = false;
      break;
    }
    offread += nread;
  }
  if(nread == 0 && retval) {
    if(ps.parseFinal(0, 0) < 0) {
      retval = false;
    }
  }
  if(!retval) {
    throw DL_ABORT_EX2("Could not parse Metalink XML document.",
                       error_code::METALINK_PARSE_ERROR);
  }
  if(!psm.getErrors().empty()) {
    throw DL_ABORT_EX2(psm.getErrorString(),
                       error_code::METALINK_PARSE_ERROR);
  }
  return psm.getResult();
}

} // namespace metalink

} // namespace aria2
