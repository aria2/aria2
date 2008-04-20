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
#include "ContentTypeRequestGroupCriteria.h"
#include "RequestGroup.h"
#include "SingleFileDownloadContext.h"
#include "Util.h"
#include "FileEntry.h"

namespace aria2 {

ContentTypeRequestGroupCriteria::ContentTypeRequestGroupCriteria(const std::deque<std::string>& contentTypes,
								 const std::deque<std::string>& extensions):
  _contentTypes(contentTypes),
  _extensions(extensions) {}

ContentTypeRequestGroupCriteria::~ContentTypeRequestGroupCriteria() {}

bool ContentTypeRequestGroupCriteria::match(const RequestGroup* requestGroup) const
{
  if(forwardMatch(requestGroup->getFilePath(), _extensions)) {
    return true;
  } else {
    SingleFileDownloadContextHandle dctx(dynamic_pointer_cast<SingleFileDownloadContext>(requestGroup->getDownloadContext()));
    if(dctx.isNull()) {
      return false;
    } else {
      return exactMatch(dctx->getContentType(), _contentTypes);
    }
  }
}

bool ContentTypeRequestGroupCriteria::forwardMatch(const std::string& target, const std::deque<std::string>& candidates) const
{
  for(std::deque<std::string>::const_iterator itr = candidates.begin(); itr != candidates.end(); ++itr) {
    if(Util::endsWith(target, *itr)) {
      return true;
    }
  }
  return false;
}

bool ContentTypeRequestGroupCriteria::exactMatch(const std::string& target, const std::deque<std::string>& candidates) const
{
  for(std::deque<std::string>::const_iterator itr = candidates.begin(); itr != candidates.end(); ++itr) {
    if(target == *itr) {
      return true;
    }
  }
  return false;
}

} // namespace aria2
