/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "CreateRequestCommand.h"
#include "InitiateConnectionCommandFactory.h"
#include "RequestGroup.h"
#include "Segment.h"
#include "DownloadContext.h"
#include "DlAbortEx.h"
#include "DownloadEngine.h"
#include "SocketCore.h"
#include "SegmentMan.h"
#include "prefs.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "FileEntry.h"
#include "SocketRecvBuffer.h"
#include "LogFactory.h"
#include "wallclock.h"

namespace aria2 {

CreateRequestCommand::CreateRequestCommand(cuid_t cuid,
                                           RequestGroup* requestGroup,
                                           DownloadEngine* e):
  AbstractCommand
  (cuid, SharedHandle<Request>(), SharedHandle<FileEntry>(), requestGroup, e,
   SharedHandle<SocketCore>(),
   SharedHandle<SocketRecvBuffer>(),
   false)
{
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

bool CreateRequestCommand::executeInternal()
{
  if(getSegments().empty()) {
    setFileEntry(getDownloadContext()->findFileEntryByOffset(0));
  } else {
    // We assume all segments belongs to same file.
    setFileEntry(getDownloadContext()->findFileEntryByOffset
                 (getSegments().front()->getPositionToWrite()));
  }
  std::vector<std::pair<size_t, std::string> > usedHosts;
  if(getOption()->getAsBool(PREF_SELECT_LEAST_USED_HOST)) {
    getDownloadEngine()->getRequestGroupMan()->getUsedHosts(usedHosts);
  }
  setRequest
    (getFileEntry()->getRequest(getRequestGroup()->getURISelector(),
                                getOption()->getAsBool(PREF_REUSE_URI),
                                usedHosts,
                                getOption()->get(PREF_REFERER),
                                // Don't use HEAD request when file
                                // size is known.
                                // Use HEAD for dry-run mode.
                                (getFileEntry()->getLength() == 0 &&
                                 getOption()->getAsBool(PREF_USE_HEAD)) ||
                                getOption()->getAsBool(PREF_DRY_RUN)?
                                Request::METHOD_HEAD:Request::METHOD_GET));
  if(!getRequest()) {
    if(getSegmentMan()) {
      getSegmentMan()->ignoreSegmentFor(getFileEntry());
    }
    // In this case, the error might be already set in RequestGroup,
    // so use it here.
    throw DL_ABORT_EX2("No URI available.",
                       getRequestGroup()->getLastErrorCode());
  } else if(getRequest()->getWakeTime() > global::wallclock()) {
    A2_LOG_DEBUG("This request object is still sleeping.");
    getFileEntry()->poolRequest(getRequest());
    // Reset request of this command. Without this, request is doubly
    // counted (1 for pooled and another one in this command) and
    // AbstractCommand::execute() will behave badly.
    resetRequest();
    getDownloadEngine()->addCommand(this);
    return false;
  }

  Command* command =
    InitiateConnectionCommandFactory::createInitiateConnectionCommand
    (getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
     getDownloadEngine());
  getDownloadEngine()->setNoWait(true);
  getDownloadEngine()->addCommand(command);
  return true;
}

} // namespace aria2
