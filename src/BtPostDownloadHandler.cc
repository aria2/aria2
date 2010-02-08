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
#include "BtPostDownloadHandler.h"
#include "prefs.h"
#include "RequestGroup.h"
#include "Option.h"
#include "Logger.h"
#include "DownloadHandlerConstants.h"
#include "File.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"
#include "util.h"
#include "ContentTypeRequestGroupCriteria.h"
#include "Exception.h"
#include "DownloadContext.h"
#include "download_helper.h"

namespace aria2 {

BtPostDownloadHandler::BtPostDownloadHandler()
{
  SharedHandle<RequestGroupCriteria> cri
    (new ContentTypeRequestGroupCriteria
     (DownloadHandlerConstants::getBtContentTypes().begin(),
      DownloadHandlerConstants::getBtContentTypes().end(),
      DownloadHandlerConstants::getBtExtensions().begin(),
      DownloadHandlerConstants::getBtExtensions().end()));
  setCriteria(cri);
}

BtPostDownloadHandler::~BtPostDownloadHandler() {}

void BtPostDownloadHandler::getNextRequestGroups
(std::deque<SharedHandle<RequestGroup> >& groups,
 RequestGroup* requestGroup)
{
  _logger->info("Generating RequestGroups for Torrent file %s",
                requestGroup->getFirstFilePath().c_str());
  std::string content;
  try {
    requestGroup->getPieceStorage()->getDiskAdaptor()->openExistingFile();
    content = util::toString(requestGroup->getPieceStorage()->getDiskAdaptor());
    requestGroup->getPieceStorage()->getDiskAdaptor()->closeFile();    
  } catch(Exception& e) {
    requestGroup->getPieceStorage()->getDiskAdaptor()->closeFile();
    throw;
  }
  std::deque<SharedHandle<RequestGroup> > newRgs;
  createRequestGroupForBitTorrent(newRgs, requestGroup->getOption(),
                                  std::deque<std::string>(),
                                  content);
  requestGroup->followedBy(newRgs.begin(), newRgs.end());
  groups.insert(groups.end(), newRgs.begin(), newRgs.end());
}

} // namespace aria2
