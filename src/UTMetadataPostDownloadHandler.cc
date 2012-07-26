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
#include "UTMetadataPostDownloadHandler.h"
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
#include "bencode2.h"
#include "message.h"
#include "prefs.h"
#include "Option.h"
#include "fmt.h"

namespace aria2 {

bool UTMetadataPostDownloadHandler::Criteria::match
(const RequestGroup* requestGroup) const
{
  const SharedHandle<DownloadContext>& dctx =
    requestGroup->getDownloadContext();
  if(dctx->hasAttribute(bittorrent::BITTORRENT)) {
    SharedHandle<TorrentAttribute> attrs = bittorrent::getTorrentAttrs(dctx);
    if(attrs->metadata.empty()) {
      return true;
    }
  }
  return false;
}

UTMetadataPostDownloadHandler::UTMetadataPostDownloadHandler()
{
  SharedHandle<Criteria> cri(new Criteria());
  setCriteria(cri);
}

void UTMetadataPostDownloadHandler::getNextRequestGroups
(std::vector<SharedHandle<RequestGroup> >& groups, RequestGroup* requestGroup)
{
  const SharedHandle<DownloadContext>& dctx =requestGroup->getDownloadContext();
  SharedHandle<TorrentAttribute> attrs = bittorrent::getTorrentAttrs(dctx);
  std::string metadata =
    util::toString(requestGroup->getPieceStorage()->getDiskAdaptor());
  std::string torrent = bittorrent::metadata2Torrent(metadata, attrs);

  if(requestGroup->getOption()->getAsBool(PREF_BT_SAVE_METADATA)) {
    std::string filename =
      util::applyDir(requestGroup->getOption()->get(PREF_DIR),
                     util::toHex(attrs->infoHash)+".torrent");
    if(util::saveAs(filename, torrent)) {
      A2_LOG_NOTICE(fmt(MSG_METADATA_SAVED, filename.c_str()));
    } else {
      A2_LOG_NOTICE(fmt(MSG_METADATA_NOT_SAVED, filename.c_str()));
    }
  }
  if(!requestGroup->getOption()->getAsBool(PREF_BT_METADATA_ONLY)) {
    std::vector<SharedHandle<RequestGroup> > newRgs;
    // Don't adjust announce URI because it has been done when
    // RequestGroup is created with magnet URI.
    createRequestGroupForBitTorrent(newRgs, requestGroup->getOption(),
                                    std::vector<std::string>(),
                                    A2STR::NIL, torrent, false);
    requestGroup->followedBy(newRgs.begin(), newRgs.end());
    if(requestGroup->getMetadataInfo()) {
      setMetadataInfo(newRgs.begin(), newRgs.end(),
                      requestGroup->getMetadataInfo());
    }
    groups.insert(groups.end(), newRgs.begin(), newRgs.end());
  }
}

} // namespace aria2
