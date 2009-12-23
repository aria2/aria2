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
#include "UTMetadataPostDownloadHandler.h"
#include "BDE.h"
#include "bittorrent_helper.h"
#include "RequestGroup.h"
#include "download_helper.h"
#include "RecoverableException.h"
#include "A2STR.h"
#include "DownloadContext.h"
#include "Logger.h"
#include "LogFactory.h"
#include "util.h"
#include "a2functional.h"
#include "DiskAdaptor.h"
#include "PieceStorage.h"
#include "bencode.h"
#include "message.h"
#include "prefs.h"
#include "Option.h"

namespace aria2 {

bool UTMetadataPostDownloadHandler::Criteria::match
(const RequestGroup* requestGroup) const
{
  SharedHandle<DownloadContext> dctx =
    requestGroup->getDownloadContext();
  if(dctx->hasAttribute(bittorrent::BITTORRENT)) {
    const BDE& attrs = dctx->getAttribute(bittorrent::BITTORRENT);
    if(!attrs.containsKey(bittorrent::METADATA)) {
      return true;
    }
  }
  return false;
}

UTMetadataPostDownloadHandler::UTMetadataPostDownloadHandler():
  _logger(LogFactory::getInstance())
{
  setCriteria(SharedHandle<Criteria>(new Criteria()));
}

void UTMetadataPostDownloadHandler::getNextRequestGroups
(std::deque<SharedHandle<RequestGroup> >& groups, RequestGroup* requestGroup)
{
  SharedHandle<DownloadContext> dctx = requestGroup->getDownloadContext();
  const BDE& attrs = dctx->getAttribute(bittorrent::BITTORRENT);
  std::string metadata =
    util::toString(requestGroup->getPieceStorage()->getDiskAdaptor());
  std::string torrent = bittorrent::metadata2Torrent(metadata, attrs);

  std::deque<SharedHandle<RequestGroup> > newRgs;
  createRequestGroupForBitTorrent(newRgs, requestGroup->getOption(),
				  std::deque<std::string>(), torrent);
  requestGroup->followedBy(newRgs.begin(), newRgs.end());
  groups.insert(groups.end(), newRgs.begin(), newRgs.end());

  if(!newRgs.empty() &&
     requestGroup->getOption()->getAsBool(PREF_BT_SAVE_METADATA)) {
    SharedHandle<DownloadContext> dctx = newRgs.front()->getDownloadContext();
    assert(dctx->hasAttribute(bittorrent::BITTORRENT));
    std::string filename = requestGroup->getOption()->get(PREF_DIR);
    filename += A2STR::SLASH_C;
    filename +=
      dctx->getAttribute(bittorrent::BITTORRENT)[bittorrent::NAME].s();
    filename += ".torrent";
    if(util::saveAs(filename, torrent)) {
      _logger->notice(MSG_METADATA_SAVED, filename.c_str());
    } else {
      _logger->notice(MSG_METADATA_NOT_SAVED, filename.c_str());
    }
  }
}

} // namespace aria2
