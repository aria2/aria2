/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "MetalinkParserStateImpl.h"

#include <cstring>

#include "MetalinkParserStateV3Impl.h"
#include "MetalinkParserStateV4Impl.h"
#include "MetalinkParserStateMachine.h"
#include "XmlAttr.h"

namespace aria2 {

namespace {
class FindAttr {
private:
  const char* localname_;
  const char* nsUri_;
public:
  FindAttr(const char* localname, const char* nsUri)
    : localname_(localname),
      nsUri_(nsUri)
  {}

  bool operator()(const XmlAttr& attr) const
  {
    return strcmp(attr.localname, localname_) == 0 &&
      (attr.nsUri == 0 || strcmp(attr.nsUri, nsUri_) == 0);
  }
};
} // namespace

std::vector<XmlAttr>::const_iterator findAttr
(const std::vector<XmlAttr>& attrs,
 const char* localname,
 const char* nsUri)
{
  return std::find_if(attrs.begin(), attrs.end(), FindAttr(localname, nsUri));
}

void InitialMetalinkParserState::beginElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::vector<XmlAttr>& attrs)
{
  if(!nsUri || strcmp(localname, "metalink") != 0) {
    psm->setSkipTagState();
  } else if(strcmp(nsUri, METALINK4_NAMESPACE_URI) == 0) {
    psm->setMetalinkStateV4();
  } else if(strcmp(nsUri, METALINK3_NAMESPACE_URI) == 0) {
    psm->setMetalinkState();
  } else {
    psm->setSkipTagState();
  }
}

void SkipTagMetalinkParserState::beginElement
(MetalinkParserStateMachine* psm,
 const char* localname,
 const char* prefix,
 const char* nsUri,
 const std::vector<XmlAttr>& attrs)
{
  psm->setSkipTagState();
}

} // namespace aria2
