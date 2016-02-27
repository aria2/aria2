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
#include "BtFileAllocationEntry.h"

#include <algorithm>

#include "BtSetup.h"
#include "RequestGroup.h"
#include "Command.h"
#include "DownloadEngine.h"
#include "DownloadContext.h"
#include "FileEntry.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"
#include "Option.h"
#include "prefs.h"
#include "LogFactory.h"

namespace aria2 {

BtFileAllocationEntry::BtFileAllocationEntry(RequestGroup* requestGroup)
    : FileAllocationEntry(requestGroup, nullptr)
{
}

BtFileAllocationEntry::~BtFileAllocationEntry() {}

void BtFileAllocationEntry::prepareForNextAction(
    std::vector<std::unique_ptr<Command>>& commands, DownloadEngine* e)
{
  auto& option = getRequestGroup()->getOption();

  BtSetup().setup(commands, getRequestGroup(), e, option.get());
  if (option->getAsBool(PREF_ENABLE_MMAP) &&
      option->get(PREF_FILE_ALLOCATION) != V_NONE &&
      getRequestGroup()->getPieceStorage()->getDiskAdaptor()->size() <=
          option->getAsLLInt(PREF_MAX_MMAP_LIMIT)) {
    getRequestGroup()->getPieceStorage()->getDiskAdaptor()->enableMmap();
  }
  if (!getRequestGroup()->downloadFinished()) {
    // For DownloadContext::resetDownloadStartTime(), see also
    // RequestGroup::createInitialCommand()
    getRequestGroup()->getDownloadContext()->resetDownloadStartTime();
    const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
        getRequestGroup()->getDownloadContext()->getFileEntries();
    if (isUriSuppliedForRequsetFileEntry(std::begin(fileEntries),
                                         std::end(fileEntries))) {
      getRequestGroup()->createNextCommandWithAdj(commands, e, 0);
    }
  }
  else {
#ifdef __MINGW32__
    const std::shared_ptr<DiskAdaptor>& diskAdaptor =
        getRequestGroup()->getPieceStorage()->getDiskAdaptor();
    if (!diskAdaptor->isReadOnlyEnabled()) {
      // On Windows, if aria2 opens files with GENERIC_WRITE access
      // right, some programs cannot open them aria2 is seeding. To
      // avoid this situation, re-open the files with read-only
      // enabled.
      A2_LOG_INFO("Closing files and re-open them with read-only mode"
                  " enabled.");
      diskAdaptor->closeFile();
      diskAdaptor->enableReadOnly();
      diskAdaptor->openFile();
    }
#endif // __MINGW32__
    getRequestGroup()->enableSeedOnly();
  }
}

} // namespace aria2
