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
#include "StreamFileAllocationEntry.h"

#include <algorithm>

#include "DownloadEngine.h"
#include "Option.h"
#include "prefs.h"
#include "RequestGroup.h"
#include "DownloadContext.h"
#include "Command.h"
#include "PeerStat.h"
#include "FileEntry.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"

namespace aria2 {

StreamFileAllocationEntry::StreamFileAllocationEntry(
    RequestGroup* requestGroup, std::unique_ptr<Command> nextCommand)
    : FileAllocationEntry(requestGroup, std::move(nextCommand))
{
}

StreamFileAllocationEntry::~StreamFileAllocationEntry() {}

void StreamFileAllocationEntry::prepareForNextAction(
    std::vector<std::unique_ptr<Command>>& commands, DownloadEngine* e)
{
  auto& option = getRequestGroup()->getOption();

  // For DownloadContext::resetDownloadStartTime(), see also
  // RequestGroup::createInitialCommand()
  getRequestGroup()->getDownloadContext()->resetDownloadStartTime();
  if (option->getAsBool(PREF_ENABLE_MMAP) &&
      option->get(PREF_FILE_ALLOCATION) != V_NONE &&
      getRequestGroup()->getPieceStorage()->getDiskAdaptor()->size() <=
          option->getAsLLInt(PREF_MAX_MMAP_LIMIT)) {
    getRequestGroup()->getPieceStorage()->getDiskAdaptor()->enableMmap();
  }
  if (getNextCommand()) {
    // Reset download start time of PeerStat because it is started
    // before file allocation begins.
    const std::shared_ptr<DownloadContext>& dctx =
        getRequestGroup()->getDownloadContext();
    const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
        dctx->getFileEntries();
    for (auto& f : fileEntries) {
      const auto& reqs = f->getInFlightRequests();
      for (auto& req : reqs) {
        const std::shared_ptr<PeerStat>& peerStat = req->getPeerStat();
        if (peerStat) {
          peerStat->downloadStart();
        }
      }
    }
    // give _nextCommand a chance to execute in the next execution loop.
    getNextCommand()->setStatus(Command::STATUS_ONESHOT_REALTIME);
    e->setNoWait(true);
    commands.push_back(popNextCommand());
    // try remaining uris
    getRequestGroup()->createNextCommandWithAdj(commands, e, -1);
  }
  else {
    getRequestGroup()->createNextCommandWithAdj(commands, e, 0);
  }
}

} // namespace aria2
