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
#ifndef _D_P_STRING_BUILD_VISITOR_H_
#define _D_P_STRING_BUILD_VISITOR_H_

#include "PStringVisitor.h"
#include "PStringSegment.h"

namespace aria2 {

class PStringBuildVisitor : public PStringVisitor, public PStringSegmentVisitor
{
private:

  std::deque<std::string> _buildQueue;

  std::deque<std::string> _uris;

public:
  
  virtual void hello(PStringSegment* segment);

  virtual void goodbye(PStringSegment* segment);

  const std::deque<std::string>& getURIs() const
  {
    return _uris;
  }

  void reset()
  {
    _buildQueue.clear();
    _uris.clear();
  }
};

typedef SharedHandle<PStringBuildVisitor> PStringBuildVisitorHandle;

} // namespace aria2

#endif // _D_P_STRING_BUILD_VISITOR_H_
