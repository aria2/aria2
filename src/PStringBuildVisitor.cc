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
#include "PStringBuildVisitor.h"
#include "PStringSegment.h"
#include "PStringNumLoop.h"
#include "PStringSelect.h"
#include "a2functional.h"

namespace aria2 {

void PStringBuildVisitor::visit(PStringSegment& segment)
{
  std::string uri;
  if(buildQueue_.empty()) {
    uri += segment.getValue();
  } else {
    uri = buildQueue_.front();
    uri += segment.getValue();
  }
  buildQueue_.push_front(uri);
  if(!segment.hasNext()) {
    uris_.push_back(uri);
  }

  if(!segment.getNext().isNull()) {
    segment.getNext()->accept(*this);
  }

  buildQueue_.pop_front();
}

void PStringBuildVisitor::visit(PStringNumLoop& s)
{
  unsigned int start = s.getStartValue();
  unsigned int end = s.getEndValue();
  unsigned int step = s.getStep();
  for(unsigned int i = start; i <= end; i += step) {
    PStringSegment(s.getNumberDecorator()->decorate(i),
                   s.getNext()).accept(*this);
  }
}

void PStringBuildVisitor::visit(PStringSelect& s)
{
  const std::vector<std::string>& values = s.getValues();
  for(std::vector<std::string>::const_iterator i = values.begin(),
        eoi = values.end(); i != eoi; ++i) {
    PStringSegment(*i, s.getNext()).accept(*this);
  }
}

const std::vector<std::string>& PStringBuildVisitor::getURIs() const
{
  return uris_;
}

void PStringBuildVisitor::reset()
{
  buildQueue_.clear();
  uris_.clear();
}

} // namespace aria2
