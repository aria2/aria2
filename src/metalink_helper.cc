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

#include <array>

#include "Option.h"
#include "MetalinkEntry.h"
#include "MetalinkParserStateMachine.h"
#include "Metalinker.h"
#include "XmlParser.h"
#include "prefs.h"
#include "DlAbortEx.h"
#include "BinaryStream.h"
#include "MetalinkMetaurl.h"
#include "a2functional.h"

namespace aria2 {

namespace metalink {

namespace {

std::vector<std::unique_ptr<MetalinkEntry>>
query(const std::shared_ptr<Metalinker>& metalinker, const Option* option)
{
  return metalinker->queryEntry(option->get(PREF_METALINK_VERSION),
                                option->get(PREF_METALINK_LANGUAGE),
                                option->get(PREF_METALINK_OS));
}

} // namespace

std::vector<std::unique_ptr<MetalinkEntry>>
parseAndQuery(const std::string& filename, const Option* option,
              const std::string& baseUri)
{
  return query(parseFile(filename, baseUri), option);
}

std::vector<std::unique_ptr<MetalinkEntry>>
parseAndQuery(BinaryStream* bs, const Option* option,
              const std::string& baseUri)
{
  return query(parseBinaryStream(bs, baseUri), option);
}

std::vector<std::pair<std::string, std::vector<MetalinkEntry*>>>
groupEntryByMetaurlName(
    const std::vector<std::unique_ptr<MetalinkEntry>>& entries)
{
  std::vector<std::pair<std::string, std::vector<MetalinkEntry*>>> result;
  for (auto& entry : entries) {
    if (entry->metaurls.empty()) {
      // std::pair<std::string, std::vector<MetalinkEntry*>> p;
      // p.second.push_back(entry.get());
      result.push_back({"", {entry.get()}});
    }
    else {
      auto i = std::begin(result);
      if (entry->metaurls[0]->name.empty() || !entry->sizeKnown) {
        i = std::end(result);
      }
      for (; i != std::end(result); ++i) {
        if ((*i).first == entry->metaurls[0]->url &&
            !(*i).second[0]->metaurls[0]->name.empty()) {
          (*i).second.push_back(entry.get());
          break;
        }
      }
      if (i == std::end(result)) {
        result.push_back({entry->metaurls[0]->url, {entry.get()}});
      }
    }
  }
  return result;
}

std::unique_ptr<Metalinker> parseFile(const std::string& filename,
                                      const std::string& baseUri)
{
  MetalinkParserStateMachine psm;
  psm.setBaseUri(baseUri);
  if (!xml::parseFile(filename, &psm)) {
    throw DL_ABORT_EX2("Could not parse Metalink XML document.",
                       error_code::METALINK_PARSE_ERROR);
  }
  if (!psm.getErrors().empty()) {
    throw DL_ABORT_EX2(psm.getErrorString(), error_code::METALINK_PARSE_ERROR);
  }
  return psm.getResult();
}

std::unique_ptr<Metalinker> parseBinaryStream(BinaryStream* bs,
                                              const std::string& baseUri)
{
  MetalinkParserStateMachine psm;
  psm.setBaseUri(baseUri);
  xml::XmlParser ps(&psm);
  std::array<unsigned char, 4_k> buf;
  ssize_t nread;
  int64_t offread = 0;
  bool retval = true;
  while ((nread = bs->readData(buf.data(), buf.size(), offread)) > 0) {
    if (ps.parseUpdate(reinterpret_cast<const char*>(buf.data()), nread) < 0) {
      retval = false;
      break;
    }
    offread += nread;
  }
  if (nread == 0 && retval) {
    if (ps.parseFinal(nullptr, 0) < 0) {
      retval = false;
    }
  }
  if (!retval) {
    throw DL_ABORT_EX2("Could not parse Metalink XML document.",
                       error_code::METALINK_PARSE_ERROR);
  }
  if (!psm.getErrors().empty()) {
    throw DL_ABORT_EX2(psm.getErrorString(), error_code::METALINK_PARSE_ERROR);
  }
  return psm.getResult();
}

} // namespace metalink

} // namespace aria2
