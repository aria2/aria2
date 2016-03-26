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
#include "LogFactory.h"
#include "DownloadHandlerConstants.h"
#include "File.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"
#include "util.h"
#include "ContentTypeRequestGroupCriteria.h"
#include "Exception.h"
#include "DownloadContext.h"
#include "download_helper.h"
#include "fmt.h"
#include "ValueBaseBencodeParser.h"
#include "DiskWriter.h"
#include "AbstractSingleDiskAdaptor.h"
#include "BencodeDiskWriter.h"
#include "RequestGroupMan.h"

namespace aria2 {

BtPostDownloadHandler::BtPostDownloadHandler()
{
  setCriteria(make_unique<ContentTypeRequestGroupCriteria>(getBtContentTypes(),
                                                           getBtExtensions()));
}

void BtPostDownloadHandler::getNextRequestGroups(
    std::vector<std::shared_ptr<RequestGroup>>& groups,
    RequestGroup* requestGroup) const
{
  A2_LOG_INFO(fmt("Generating RequestGroups for Torrent file %s",
                  requestGroup->getFirstFilePath().c_str()));
  std::unique_ptr<ValueBase> torrent;
  if (requestGroup->inMemoryDownload()) {
    auto& dw = static_cast<AbstractSingleDiskAdaptor*>(
                   requestGroup->getPieceStorage()->getDiskAdaptor().get())
                   ->getDiskWriter();
    auto bdw = static_cast<bittorrent::BencodeDiskWriter*>(dw.get());
    int error = bdw->finalize();
    if (error == 0) {
      torrent = bdw->getResult();
    }
  }
  else {
    std::string content;
    try {
      requestGroup->getPieceStorage()->getDiskAdaptor()->openExistingFile();
      content =
          util::toString(requestGroup->getPieceStorage()->getDiskAdaptor());
      requestGroup->getPieceStorage()->getDiskAdaptor()->closeFile();
    }
    catch (Exception& e) {
      requestGroup->getPieceStorage()->getDiskAdaptor()->closeFile();
      throw;
    }
    ssize_t error;
    torrent = bittorrent::ValueBaseBencodeParser().parseFinal(
        content.c_str(), content.size(), error);
  }
  if (!torrent) {
    throw DL_ABORT_EX2("Could not parse BitTorrent metainfo",
                       error_code::BENCODE_PARSE_ERROR);
  }
  std::vector<std::shared_ptr<RequestGroup>> newRgs;
  createRequestGroupForBitTorrent(newRgs, requestGroup->getOption(),
                                  std::vector<std::string>(), "",
                                  torrent.get());
  requestGroup->followedBy(std::begin(newRgs), std::end(newRgs));
  for (auto& rg : newRgs) {
    rg->following(requestGroup->getGID());
  }
  auto mi = createMetadataInfoFromFirstFileEntry(
      requestGroup->getGroupId(), requestGroup->getDownloadContext());
  if (mi) {
    setMetadataInfo(std::begin(newRgs), std::end(newRgs), mi);
  }

  auto rgman = requestGroup->getRequestGroupMan();

  if (rgman && rgman->getKeepRunning() &&
      requestGroup->getOption()->getAsBool(PREF_PAUSE_METADATA)) {
    for (auto& rg : newRgs) {
      rg->setPauseRequested(true);
    }
  }

  groups.insert(std::end(groups), std::begin(newRgs), std::end(newRgs));
}

} // namespace aria2
