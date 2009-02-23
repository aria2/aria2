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
#include "BtCheckIntegrityEntry.h"
#include "BtFileAllocationEntry.h"
#include "RequestGroup.h"
#include "PieceStorage.h"
#include "DownloadEngine.h"
#include "FileAllocationMan.h"
#include "DiskAdaptor.h"
#include "prefs.h"
#include "Option.h"

namespace aria2 {

BtCheckIntegrityEntry::BtCheckIntegrityEntry(RequestGroup* requestGroup):
  PieceHashCheckIntegrityEntry(requestGroup, 0) {}

BtCheckIntegrityEntry::~BtCheckIntegrityEntry() {}

static void proceedFileAllocation
(std::deque<Command*>& commands, DownloadEngine* e, RequestGroup* requestGroup)
{
  FileAllocationEntryHandle entry(new BtFileAllocationEntry(requestGroup));
  if(requestGroup->needsFileAllocation()) {
    e->_fileAllocationMan->pushEntry(entry);
  } else {
    entry->prepareForNextAction(commands, e);
  }
}

void BtCheckIntegrityEntry::onDownloadIncomplete(std::deque<Command*>& commands,
						 DownloadEngine* e)
{
  // Now reopen DiskAdaptor with read only disabled.
  _requestGroup->getPieceStorage()->getDiskAdaptor()->closeFile();
  _requestGroup->getPieceStorage()->getDiskAdaptor()->disableReadOnly();
  _requestGroup->getPieceStorage()->getDiskAdaptor()->openFile();

  proceedFileAllocation(commands, e, _requestGroup);
}

void BtCheckIntegrityEntry::onDownloadFinished(std::deque<Command*>& commands,
					       DownloadEngine* e)
{
  _requestGroup->getPieceStorage()->getDiskAdaptor()->onDownloadComplete();
  // TODO Currently,when all the checksums
  // are valid, then aira2 goes to seeding mode. Sometimes it is better
  // to exit rather than doing seeding. So, it would be good to toggle this
  // behavior.
  if(e->option->getAsBool(PREF_BT_HASH_CHECK_SEED)) {
    proceedFileAllocation(commands, e, _requestGroup);
  }
}


} // namespace aria2
