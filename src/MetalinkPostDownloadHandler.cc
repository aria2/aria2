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
#include "MetalinkPostDownloadHandler.h"

#include <deque>

#include "RequestGroup.h"
#include "Metalink2RequestGroup.h"
#include "Logger.h"
#include "LogFactory.h"
#include "DiskAdaptor.h"
#include "PieceStorage.h"
#include "DownloadHandlerConstants.h"
#include "ContentTypeRequestGroupCriteria.h"
#include "Exception.h"
#include "prefs.h"
#include "Option.h"
#include "DownloadContext.h"
#include "download_helper.h"
#include "fmt.h"
#include "FileEntry.h"
#include "RequestGroupMan.h"

namespace aria2 {

MetalinkPostDownloadHandler::MetalinkPostDownloadHandler()
{
  setCriteria(make_unique<ContentTypeRequestGroupCriteria>(
      getMetalinkContentTypes(), getMetalinkExtensions()));
}

namespace {
const std::string& getBaseUri(RequestGroup* requestGroup)
{
  auto& dctx = requestGroup->getDownloadContext();
  if (dctx->getFileEntries().empty()) {
    return A2STR::NIL;
  }
  else {
    // TODO Check download result for each URI
    auto& entry = dctx->getFirstFileEntry();
    auto& spentUris = entry->getSpentUris();
    if (spentUris.empty()) {
      auto& remainingUris = entry->getRemainingUris();
      if (remainingUris.empty()) {
        return A2STR::NIL;
      }
      else {
        return remainingUris.front();
      }
    }
    else {
      return spentUris.back();
    }
  }
}
} // namespace

void MetalinkPostDownloadHandler::getNextRequestGroups(
    std::vector<std::shared_ptr<RequestGroup>>& groups,
    RequestGroup* requestGroup) const
{
  A2_LOG_DEBUG(fmt("Generating RequestGroups for Metalink file %s",
                   requestGroup->getFirstFilePath().c_str()));
  auto diskAdaptor = requestGroup->getPieceStorage()->getDiskAdaptor();
  try {
    diskAdaptor->openExistingFile();
    // requestOption.put(PREF_DIR,
    // requestGroup->getDownloadContext()->getDir());
    const std::string& baseUri = getBaseUri(requestGroup);
    std::vector<std::shared_ptr<RequestGroup>> newRgs;
    Metalink2RequestGroup().generate(newRgs, diskAdaptor,
                                     requestGroup->getOption(), baseUri);
    requestGroup->followedBy(newRgs.begin(), newRgs.end());
    for (auto& rg : newRgs) {
      rg->following(requestGroup->getGID());
    }
    auto mi = createMetadataInfoFromFirstFileEntry(
        requestGroup->getGroupId(), requestGroup->getDownloadContext());
    if (mi) {
      setMetadataInfo(newRgs.begin(), newRgs.end(), mi);
    }

    auto rgman = requestGroup->getRequestGroupMan();

    if (rgman && rgman->getKeepRunning() &&
        requestGroup->getOption()->getAsBool(PREF_PAUSE_METADATA)) {
      for (auto& rg : newRgs) {
        rg->setPauseRequested(true);
      }
    }

    groups.insert(groups.end(), newRgs.begin(), newRgs.end());
    diskAdaptor->closeFile();
  }
  catch (Exception& e) {
    diskAdaptor->closeFile();
    throw;
  }
}

} // namespace aria2
