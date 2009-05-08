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
#include "CheckIntegrityEntry.h"
#include "IteratableValidator.h"
#include "RequestGroup.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"
#include "prefs.h"
#include "FileAllocationEntry.h"
#include "DownloadEngine.h"
#include "Option.h"

namespace aria2 {

CheckIntegrityEntry::CheckIntegrityEntry(RequestGroup* requestGroup,
					 Command* nextCommand):
  RequestGroupEntry(requestGroup, nextCommand)
{}

CheckIntegrityEntry::~CheckIntegrityEntry() {}

void CheckIntegrityEntry::validateChunk()
{
  _validator->validateChunk();
}

uint64_t CheckIntegrityEntry::getTotalLength()
{
  if(_validator.isNull()) {
    return 0;
  } else {
    return _validator->getTotalLength();
  }
}

off_t CheckIntegrityEntry::getCurrentLength()
{
  if(_validator.isNull()) {
    return 0;
  } else {
    return _validator->getCurrentOffset();
  }
}

bool CheckIntegrityEntry::finished()
{
  return _validator->finished();
}

void CheckIntegrityEntry::cutTrailingGarbage()
{
  _requestGroup->getPieceStorage()->getDiskAdaptor()->cutTrailingGarbage();
}

void CheckIntegrityEntry::proceedFileAllocation
(std::deque<Command*>& commands,
 const SharedHandle<FileAllocationEntry>& entry,
 DownloadEngine* e)
{
  if(_requestGroup->needsFileAllocation()) {
    e->_fileAllocationMan->pushEntry(entry);
  } else {
    entry->prepareForNextAction(commands, e);
  }
  // Disable directIO when fallocation() is going to be used.
  if(_requestGroup->getOption()->get(PREF_FILE_ALLOCATION) == V_FALLOC) {
    entry->disableDirectIO();
  }
}


} // namespace aria2
