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
#include "XmlRpcMethodImpl.h"

#include <cassert>
#include <algorithm>

#include "Logger.h"
#include "BDE.h"
#include "DlAbortEx.h"
#include "Option.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "download_helper.h"
#include "util.h"
#include "RequestGroupMan.h"
#include "StringFormat.h"
#include "XmlRpcRequest.h"
#include "PieceStorage.h"
#include "DownloadContext.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include "BtProgressInfoFile.h"
#include "prefs.h"
#include "message.h"
#include "FeatureConfig.h"
#include "array_fun.h"
#include "XmlRpcMethodFactory.h"
#include "XmlRpcResponse.h"
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
# include "BtRegistry.h"
# include "PeerStorage.h"
# include "Peer.h"
# include "BtRuntime.h"
# include "BtAnnounce.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace xmlrpc {

namespace {
const BDE BDE_TRUE = BDE("true");
const BDE BDE_FALSE = BDE("false");
const BDE BDE_OK = BDE("OK");
const BDE BDE_ACTIVE = BDE("active");
const BDE BDE_WAITING = BDE("waiting");
const BDE BDE_REMOVED = BDE("removed");
const BDE BDE_ERROR = BDE("error");
const BDE BDE_COMPLETE = BDE("complete");
const BDE BDE_USED = BDE("used");

const std::string KEY_GID = "gid";
const std::string KEY_ERROR_CODE = "errorCode";
const std::string KEY_STATUS = "status";
const std::string KEY_TOTAL_LENGTH = "totalLength";
const std::string KEY_COMPLETED_LENGTH = "completedLength";
const std::string KEY_DOWNLOAD_SPEED = "downloadSpeed";
const std::string KEY_UPLOAD_SPEED = "uploadSpeed";
const std::string KEY_UPLOAD_LENGTH = "uploadLength";
const std::string KEY_CONNECTIONS = "connections";
const std::string KEY_BITFIELD = "bitfield";
const std::string KEY_PIECE_LENGTH = "pieceLength";
const std::string KEY_NUM_PIECES = "numPieces";
const std::string KEY_FOLLOWED_BY = "followedBy";
const std::string KEY_BELONGS_TO = "belongsTo";
const std::string KEY_INFO_HASH = "infoHash";
const std::string KEY_NUM_SEEDERS = "numSeeders";
const std::string KEY_PEER_ID = "peerId";
const std::string KEY_IP = "ip";
const std::string KEY_PORT = "port";
const std::string KEY_AM_CHOKING = "amChoking";
const std::string KEY_PEER_CHOKING = "peerChoking";
const std::string KEY_SEEDER = "seeder";
const std::string KEY_INDEX = "index";
const std::string KEY_PATH = "path";
const std::string KEY_SELECTED = "selected";
const std::string KEY_LENGTH = "length";
const std::string KEY_URI = "uri";
const std::string KEY_CURRENT_URI = "currentUri";
const std::string KEY_VERSION = "version";
const std::string KEY_ENABLED_FEATURES = "enabledFeatures";
const std::string KEY_METHOD_NAME = "methodName";
const std::string KEY_PARAMS = "params";
const std::string KEY_SESSION_ID = "sessionId";
const std::string KEY_FILES = "files";
const std::string KEY_DIR = "dir";
const std::string KEY_URIS = "uris";
const std::string KEY_BITTORRENT = "bittorrent";
const std::string KEY_INFO = "info";
const std::string KEY_NAME = "name";
const std::string KEY_ANNOUNCE_LIST = "announceList";
const std::string KEY_COMMENT = "comment";
const std::string KEY_CREATION_DATE = "creationDate";
const std::string KEY_MODE = "mode";
const std::string KEY_SERVERS = "servers";
}

static BDE createGIDResponse(int32_t gid)
{
  return BDE(util::itos(gid));
}

static BDE addRequestGroup(const SharedHandle<RequestGroup>& group,
                           DownloadEngine* e,
                           bool posGiven, int pos)
{
  if(posGiven) {
    e->_requestGroupMan->insertReservedGroup(pos, group);
  } else {
    e->_requestGroupMan->addReservedGroup(group);
  }
  return createGIDResponse(group->getGID());
}

static bool hasDictParam(const BDE& params, size_t index)
{
  return params.size() > index && params[index].isDict();
}

static void getPosParam(const BDE& params, size_t posParamIndex,
                        bool& posGiven, size_t& pos)
{
  if(params.size() > posParamIndex && params[posParamIndex].isInteger()) {
    if(params[posParamIndex].i() >= 0) {
      pos = params[posParamIndex].i();
      posGiven = true;
    } else {
      throw DL_ABORT_EX("Position must be greater than or equal to 0.");
    }
  } else {
    posGiven = false;
  }
} 

template<typename OutputIterator>
static void extractUris(OutputIterator out, const BDE& src)
{
  for(BDE::List::const_iterator i = src.listBegin(), eoi = src.listEnd();
      i != eoi; ++i) {
    if((*i).isString()) {
      out++ = (*i).s();
    }
  }
}

BDE AddUriXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isList() || params[0].empty()) {
    throw DL_ABORT_EX("URI is not provided.");
  }
  std::vector<std::string> uris;
  extractUris(std::back_inserter(uris), params[0]);

  SharedHandle<Option> requestOption(new Option(*e->option));
  if(hasDictParam(params, 1)) {
    gatherRequestOption(requestOption, params[1]);
  }
  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 2, posGiven, pos);

  std::vector<SharedHandle<RequestGroup> > result;
  createRequestGroupForUri(result, requestOption, uris,
                           /* ignoreForceSeq = */ true,
                           /* ignoreLocalPath = */ true);

  if(!result.empty()) {
    return addRequestGroup(result.front(), e, posGiven, pos);
  } else {
    throw DL_ABORT_EX("No URI to download.");
  }
}

#ifdef ENABLE_BITTORRENT
BDE AddTorrentXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX("Torrent data is not provided.");
  }
  
  std::vector<std::string> uris;
  if(params.size() > 1 && params[1].isList()) {
    extractUris(std::back_inserter(uris), params[1]);
  }
  SharedHandle<Option> requestOption(new Option(*e->option));
  if(hasDictParam(params, 2)) {
    gatherRequestOption(requestOption, params[2]);
  }
  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 3, posGiven, pos);

  std::vector<SharedHandle<RequestGroup> > result;
  createRequestGroupForBitTorrent(result, requestOption,
                                  uris,
                                  params[0].s());

  if(!result.empty()) {
    return addRequestGroup(result.front(), e, posGiven, pos);
  } else {
    throw DL_ABORT_EX("No Torrent to download.");
  }
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
BDE AddMetalinkXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX("Metalink data is not provided.");
  }
  
  SharedHandle<Option> requestOption(new Option(*e->option));
  if(hasDictParam(params, 1)) {
    gatherRequestOption(requestOption, params[1]);
  };
  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 2, posGiven, pos);

  std::vector<SharedHandle<RequestGroup> > result;
  createRequestGroupForMetalink(result, requestOption, params[0].s());
  if(!result.empty()) {
    if(posGiven) {
      e->_requestGroupMan->insertReservedGroup(pos, result);
    } else {
      e->_requestGroupMan->addReservedGroup(result);
    }
    BDE gids = BDE::list();
    for(std::vector<SharedHandle<RequestGroup> >::const_iterator i =
          result.begin(), eoi = result.end(); i != eoi; ++i) {
      gids << BDE(util::itos((*i)->getGID()));
    }
    return gids;
  } else {
    throw DL_ABORT_EX("No files to download.");
  }
} 
#endif // ENABLE_METALINK

static BDE removeDownload
(const XmlRpcRequest& req, DownloadEngine* e, bool forceRemove)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = e->_requestGroupMan->findRequestGroup(gid);

  if(group.isNull()) {
    group = e->_requestGroupMan->findReservedGroup(gid);
    if(group.isNull()) {
      throw DL_ABORT_EX
        (StringFormat("Active Download not found for GID#%d", gid).str());
    }
    if(group->isDependencyResolved()) {
      e->_requestGroupMan->removeReservedGroup(gid);
    } else {
      throw DL_ABORT_EX
        (StringFormat("GID#%d cannot be removed now", gid).str());
    }
  } else {
    if(forceRemove) {
      group->setForceHaltRequested(true, RequestGroup::USER_REQUEST);
    } else {
      group->setHaltRequested(true, RequestGroup::USER_REQUEST);
    }
  }

  return createGIDResponse(gid);

}

BDE RemoveXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  return removeDownload(req, e, false);
}

BDE ForceRemoveXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return removeDownload(req, e, true);
}

static void createUriEntry(BDE& uriList, const SharedHandle<FileEntry>& file)
{
  {
    const std::deque<std::string>& uris = file->getSpentUris();
    for(std::deque<std::string>::const_iterator i = uris.begin(),
          eoi = uris.end(); i != eoi; ++i) {
      BDE entry = BDE::dict();
      entry[KEY_URI] = *i;
      entry[KEY_STATUS] = BDE_USED;
      uriList << entry;
    }
  }
  {
    const std::deque<std::string>& uris = file->getRemainingUris();
    for(std::deque<std::string>::const_iterator i = uris.begin(),
          eoi = uris.end(); i != eoi; ++i) {
      BDE entry = BDE::dict();
      entry[KEY_URI] = *i;
      entry[KEY_STATUS] = BDE_WAITING;
      uriList << entry;
    }
  }
}

template<typename InputIterator>
static void createFileEntry(BDE& files, InputIterator first, InputIterator last)
{
  size_t index = 1;
  for(; first != last; ++first, ++index) {
    BDE entry = BDE::dict();
    entry[KEY_INDEX] = util::uitos(index);
    entry[KEY_PATH] = (*first)->getPath();
    entry[KEY_SELECTED] = (*first)->isRequested()?BDE_TRUE:BDE_FALSE;
    entry[KEY_LENGTH] = util::uitos((*first)->getLength());

    BDE uriList = BDE::list();
    createUriEntry(uriList, *first);
    entry[KEY_URIS] = uriList;
    files << entry;
  }
}

void gatherProgressCommon
(BDE& entryDict, const SharedHandle<RequestGroup>& group)
{
  entryDict[KEY_GID] = util::itos(group->getGID());
  // This is "filtered" total length if --select-file is used.
  entryDict[KEY_TOTAL_LENGTH] = util::uitos(group->getTotalLength());
  // This is "filtered" total length if --select-file is used.
  entryDict[KEY_COMPLETED_LENGTH] = util::uitos(group->getCompletedLength());
  TransferStat stat = group->calculateStat();
  entryDict[KEY_DOWNLOAD_SPEED] = util::uitos(stat.getDownloadSpeed());
  entryDict[KEY_UPLOAD_SPEED] = util::uitos(stat.getUploadSpeed());
  entryDict[KEY_UPLOAD_LENGTH] = util::uitos(stat.getAllTimeUploadLength());
  entryDict[KEY_CONNECTIONS] = util::uitos(group->getNumConnection());
  SharedHandle<PieceStorage> ps = group->getPieceStorage();
  if(!ps.isNull()) {
    if(ps->getBitfieldLength() > 0) {
      entryDict[KEY_BITFIELD] = util::toHex(ps->getBitfield(),
                                            ps->getBitfieldLength());
    }
  }
  const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
  entryDict[KEY_PIECE_LENGTH] = util::uitos(dctx->getPieceLength());
  entryDict[KEY_NUM_PIECES] = util::uitos(dctx->getNumPieces());
  if(!group->followedBy().empty()) {
    BDE list = BDE::list();
    // The element is GID.
    for(std::vector<int32_t>::const_iterator i = group->followedBy().begin(),
          eoi = group->followedBy().end(); i != eoi; ++i) {
      list << util::itos(*i);
    }
    entryDict[KEY_FOLLOWED_BY] = list;
  }
  if(group->belongsTo()) {
    entryDict[KEY_BELONGS_TO] = util::itos(group->belongsTo());
  }
  BDE files = BDE::list();
  createFileEntry
    (files, dctx->getFileEntries().begin(), dctx->getFileEntries().end());
  entryDict[KEY_FILES] = files;
  entryDict[KEY_DIR] = group->getOption()->get(PREF_DIR);
}

#ifdef ENABLE_BITTORRENT
void gatherBitTorrentMetadata(BDE& btDict, const BDE& torrentAttrs)
{
  if(torrentAttrs.containsKey(bittorrent::COMMENT)) {
    btDict[KEY_COMMENT] = torrentAttrs[bittorrent::COMMENT];
  }
  if(torrentAttrs.containsKey(bittorrent::CREATION_DATE)) {
    btDict[KEY_CREATION_DATE] = torrentAttrs[bittorrent::CREATION_DATE];
  }
  if(torrentAttrs.containsKey(bittorrent::MODE)) {
    btDict[KEY_MODE] = torrentAttrs[bittorrent::MODE];
  }
  // Copy announceList to avoid modification on entyDict to be
  // affected original announceList.
  // TODO Would it be good to add copy() method in BDE?
  const BDE& announceList = torrentAttrs[bittorrent::ANNOUNCE_LIST];
  BDE destAnnounceList = BDE::list();
  for(BDE::List::const_iterator l = announceList.listBegin(),
        eoi = announceList.listEnd(); l != eoi; ++l) {
    BDE destAnnounceTier = BDE::list();
    for(BDE::List::const_iterator t = (*l).listBegin(),
          eoi2 = (*l).listEnd(); t != eoi2; ++t) {
      destAnnounceTier <<  (*t);
    }
    destAnnounceList << destAnnounceTier;
  }
  btDict[KEY_ANNOUNCE_LIST] = destAnnounceList;
  if(torrentAttrs.containsKey(bittorrent::METADATA)) {
    BDE infoDict = BDE::dict();
    infoDict[KEY_NAME] = torrentAttrs[bittorrent::NAME];
    btDict[KEY_INFO] = infoDict;
  }
}

static void gatherProgressBitTorrent
(BDE& entryDict, const BDE& torrentAttrs, const BtObject& btObject)
{
  const std::string& infoHash = torrentAttrs[bittorrent::INFO_HASH].s();
  entryDict[KEY_INFO_HASH] = util::toHex(infoHash);
  BDE btDict = BDE::dict();
  gatherBitTorrentMetadata(btDict, torrentAttrs);
  entryDict[KEY_BITTORRENT] = btDict;
  if(!btObject.isNull()) {
    SharedHandle<PeerStorage> peerStorage = btObject._peerStorage;
    assert(!peerStorage.isNull());

    std::vector<SharedHandle<Peer> > peers;
    peerStorage->getActivePeers(peers);
    entryDict[KEY_NUM_SEEDERS] = countSeeder(peers.begin(), peers.end());
  }
}

static void gatherPeer(BDE& peers, const SharedHandle<PeerStorage>& ps)
{
  std::vector<SharedHandle<Peer> > activePeers;
  ps->getActivePeers(activePeers);
  for(std::vector<SharedHandle<Peer> >::const_iterator i =
        activePeers.begin(), eoi = activePeers.end(); i != eoi; ++i) {
    BDE peerEntry = BDE::dict();
    peerEntry[KEY_PEER_ID] = util::torrentPercentEncode((*i)->getPeerId(),
                                                        PEER_ID_LENGTH);
    peerEntry[KEY_IP] = (*i)->ipaddr;
    peerEntry[KEY_PORT] = util::uitos((*i)->port);
    peerEntry[KEY_BITFIELD] = util::toHex((*i)->getBitfield(),
                                          (*i)->getBitfieldLength());
    peerEntry[KEY_AM_CHOKING] = (*i)->amChoking()?BDE_TRUE:BDE_FALSE;
    peerEntry[KEY_PEER_CHOKING] = (*i)->peerChoking()?BDE_TRUE:BDE_FALSE;
    TransferStat stat = ps->getTransferStatFor(*i);
    peerEntry[KEY_DOWNLOAD_SPEED] = util::uitos(stat.getDownloadSpeed());
    peerEntry[KEY_UPLOAD_SPEED] = util::uitos(stat.getUploadSpeed());
    peerEntry[KEY_SEEDER] = (*i)->isSeeder()?BDE_TRUE:BDE_FALSE;
    peers << peerEntry;
  }
}
#endif // ENABLE_BITTORRENT

static void gatherProgress
(BDE& entryDict, const SharedHandle<RequestGroup>& group, DownloadEngine* e)
{
  gatherProgressCommon(entryDict, group);
#ifdef ENABLE_BITTORRENT
  if(group->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT)) {
    const BDE& torrentAttrs =
      group->getDownloadContext()->getAttribute(bittorrent::BITTORRENT);
    BtObject btObject = e->getBtRegistry()->get(group->getGID());
    gatherProgressBitTorrent(entryDict, torrentAttrs, btObject);
  }
#endif // ENABLE_BITTORRENT
}

void gatherStoppedDownload
(BDE& entryDict, const SharedHandle<DownloadResult>& ds)
{
  entryDict[KEY_GID] = util::itos(ds->gid);
  entryDict[KEY_ERROR_CODE] = util::itos(static_cast<int>(ds->result));
  if(ds->result == downloadresultcode::IN_PROGRESS) {
    entryDict[KEY_STATUS] = BDE_REMOVED;
  } else if(ds->result == downloadresultcode::FINISHED) {
    entryDict[KEY_STATUS] = BDE_COMPLETE;
  } else {
    entryDict[KEY_STATUS] = BDE_ERROR;
  }
  if(!ds->followedBy.empty()) {
    BDE list = BDE::list();
    // The element is GID.
    for(std::vector<int32_t>::const_iterator i = ds->followedBy.begin(),
          eoi = ds->followedBy.end(); i != eoi; ++i) {
      list << util::itos(*i);
    }
    entryDict[KEY_FOLLOWED_BY] = list;
  }
  if(ds->belongsTo) {
    entryDict[KEY_BELONGS_TO] = util::itos(ds->belongsTo);
  }
  BDE files = BDE::list();
  createFileEntry(files, ds->fileEntries.begin(), ds->fileEntries.end());
  entryDict[KEY_FILES] = files;
}

static
SharedHandle<RequestGroup>
findRequestGroup(const SharedHandle<RequestGroupMan>& rgman, int32_t gid)
{
  SharedHandle<RequestGroup> group = rgman->findRequestGroup(gid);
  if(group.isNull()) {
    group = rgman->findReservedGroup(gid);
  }
  return group;
}

BDE GetFilesXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = util::parseInt(params[0].s());

  BDE files = BDE::list();
  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    SharedHandle<DownloadResult> dr =
      e->_requestGroupMan->findDownloadResult(gid);
    if(dr.isNull()) {
      throw DL_ABORT_EX
        (StringFormat("No file data is available for GID#%d", gid).str());
    } else {
      createFileEntry(files, dr->fileEntries.begin(), dr->fileEntries.end());
    }
  } else {
    createFileEntry(files,
                    group->getDownloadContext()->getFileEntries().begin(),
                    group->getDownloadContext()->getFileEntries().end());
  }
  return files;
}

BDE GetUrisXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("No URI data is available for GID#%d", gid).str());
  }
  BDE uriList = BDE::list();
  // TODO Current implementation just returns first FileEntry's URIs.
  if(!group->getDownloadContext()->getFileEntries().empty()) {
    createUriEntry(uriList, group->getDownloadContext()->getFirstFileEntry());
  }
  return uriList;
}

#ifdef ENABLE_BITTORRENT
BDE GetPeersXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("No peer data is available for GID#%d", gid).str());
  }
  BDE peers = BDE::list();
  BtObject btObject = e->getBtRegistry()->get(group->getGID());
  if(!btObject.isNull()) {
    assert(!btObject._peerStorage.isNull());
    gatherPeer(peers, btObject._peerStorage);
  }
  return peers;
}
#endif // ENABLE_BITTORRENT

BDE TellStatusXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = e->_requestGroupMan->findRequestGroup(gid);

  BDE entryDict = BDE::dict();
  if(group.isNull()) {
    group = e->_requestGroupMan->findReservedGroup(gid);
    if(group.isNull()) {
      SharedHandle<DownloadResult> ds =
        e->_requestGroupMan->findDownloadResult(gid);
      if(ds.isNull()) {
        throw DL_ABORT_EX
          (StringFormat("No such download for GID#%d", gid).str());
      }
      gatherStoppedDownload(entryDict, ds);
    } else {
      entryDict[KEY_STATUS] = BDE_WAITING;
      gatherProgress(entryDict, group, e);
    }
  } else {
    entryDict[KEY_STATUS] = BDE_ACTIVE;
    gatherProgress(entryDict, group, e);
  }
  return entryDict;
}

BDE TellActiveXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  BDE list = BDE::list();
  const std::deque<SharedHandle<RequestGroup> >& groups =
    e->_requestGroupMan->getRequestGroups();
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator i =
        groups.begin(), eoi = groups.end(); i != eoi; ++i) {
    BDE entryDict = BDE::dict();
    entryDict[KEY_STATUS] = BDE_ACTIVE;
    gatherProgress(entryDict, *i, e);
    list << entryDict;
  }
  return list;
}

const std::deque<SharedHandle<RequestGroup> >&
TellWaitingXmlRpcMethod::getItems(DownloadEngine* e) const
{
  return e->_requestGroupMan->getReservedGroups();
}

void TellWaitingXmlRpcMethod::createEntry
(BDE& entryDict, const SharedHandle<RequestGroup>& item,
 DownloadEngine* e) const
{
  entryDict[KEY_STATUS] = BDE_WAITING;
  gatherProgress(entryDict, item, e);
}

const std::deque<SharedHandle<DownloadResult> >&
TellStoppedXmlRpcMethod::getItems(DownloadEngine* e) const
{
  return e->_requestGroupMan->getDownloadResults();
}

void TellStoppedXmlRpcMethod::createEntry
(BDE& entryDict, const SharedHandle<DownloadResult>& item,
 DownloadEngine* e) const
{
  gatherStoppedDownload(entryDict, item);
}

BDE PurgeDownloadResultXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  e->_requestGroupMan->purgeDownloadResult();
  return BDE_OK;
}

BDE ChangeOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }  
  int32_t gid = util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("Cannot change option for GID#%d", gid).str());
  }
  SharedHandle<Option> option(new Option());
  if(params.size() > 1 && params[1].isDict()) {
    gatherChangeableOption(option, params[1]);
    applyChangeableOption(group->getOption().get(), option.get());
    if(option->defined(PREF_MAX_DOWNLOAD_LIMIT)) {
      group->setMaxDownloadSpeedLimit
        (option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
    }
    if(option->defined(PREF_MAX_UPLOAD_LIMIT)) {
      group->setMaxUploadSpeedLimit(option->getAsInt(PREF_MAX_UPLOAD_LIMIT));
    }
#ifdef ENABLE_BITTORRENT
    BtObject btObject = e->getBtRegistry()->get(group->getGID());
    if(!btObject.isNull()) {
      if(option->defined(PREF_BT_MAX_PEERS)) {
        btObject._btRuntime->setMaxPeers(option->getAsInt(PREF_BT_MAX_PEERS));
      }
    }
#endif // ENABLE_BITTORRENT
  }
  return BDE_OK;
}

BDE ChangeGlobalOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isDict()) {
    return BDE_OK;
  }
  SharedHandle<Option> option(new Option());
  gatherChangeableGlobalOption(option, params[0]);
  applyChangeableGlobalOption(e->option, option.get());

  if(option->defined(PREF_MAX_OVERALL_DOWNLOAD_LIMIT)) {
    e->_requestGroupMan->setMaxOverallDownloadSpeedLimit
      (option->getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
  }
  if(option->defined(PREF_MAX_OVERALL_UPLOAD_LIMIT)) {
    e->_requestGroupMan->setMaxOverallUploadSpeedLimit
      (option->getAsInt(PREF_MAX_OVERALL_UPLOAD_LIMIT));
  }
  if(option->defined(PREF_MAX_CONCURRENT_DOWNLOADS)) {
    e->_requestGroupMan->setMaxSimultaneousDownloads
      (option->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS));
    e->_requestGroupMan->requestQueueCheck();
  }
  return BDE_OK;
}

BDE GetVersionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  BDE result = BDE::dict();
  result[KEY_VERSION] = std::string(PACKAGE_VERSION);
  BDE featureList = BDE::list();
  const FeatureMap& features = FeatureConfig::getInstance()->getFeatures();
  for(FeatureMap::const_iterator i = features.begin(), eoi = features.end();
      i != eoi;++i){
    if((*i).second) {
      featureList << (*i).first;
    }
  }
  result[KEY_ENABLED_FEATURES] = featureList;
  return result;
}

template<typename InputIterator>
static void pushRequestOption
(BDE& dict, InputIterator optionFirst, InputIterator optionLast)
{
  const std::set<std::string>& requestOptions = listRequestOptions();
  for(; optionFirst != optionLast; ++optionFirst) {
    if(requestOptions.count((*optionFirst).first)) {
      dict[(*optionFirst).first] = (*optionFirst).second;
    }
  }
}

BDE GetOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }  
  int32_t gid = util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("Cannot get option for GID#%d", gid).str());
  }
  BDE result = BDE::dict();
  SharedHandle<Option> option = group->getOption();
  pushRequestOption(result, option->begin(), option->end());
  return result;
}

BDE GetGlobalOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  BDE result = BDE::dict();
  for(std::map<std::string, std::string>::const_iterator i = e->option->begin(),
        eoi = e->option->end(); i != eoi; ++i) {
    SharedHandle<OptionHandler> h = _optionParser->findByName((*i).first);
    if(!h.isNull() && !h->isHidden()) {
      result[(*i).first] = (*i).second;
    }
  }
  return result;
}

BDE ChangePositionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.size() != 3 ||
     !params[0].isString() || !params[1].isInteger() || !params[2].isString()) {
    throw DL_ABORT_EX("Illegal argument.");
  }
  int32_t gid = util::parseInt(params[0].s());
  int pos = params[1].i();
  const std::string& howStr = params[2].s();
  RequestGroupMan::HOW how;
  if(howStr == "POS_SET") {
    how = RequestGroupMan::POS_SET;
  } else if(howStr == "POS_CUR") {
    how = RequestGroupMan::POS_CUR;
  } else if(howStr == "POS_END") {
    how = RequestGroupMan::POS_END;
  } else {
    throw DL_ABORT_EX("Illegal argument.");
  }
  size_t destPos =
    e->_requestGroupMan->changeReservedGroupPosition(gid, pos, how);
  BDE result(destPos);
  return result;
}

BDE GetSessionInfoXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  BDE result = BDE::dict();
  result[KEY_SESSION_ID] = util::toHex(e->getSessionId());
  return result;
}

BDE GetServersXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX("Bad request");
  }
  int32_t gid = util::parseInt(params[0].s());
  SharedHandle<RequestGroup> group = e->_requestGroupMan->findRequestGroup(gid);
  if(group.isNull()) {
    throw DL_ABORT_EX(StringFormat("No active download for GID#%d", gid).str());
  }
  SharedHandle<DownloadContext> dctx = group->getDownloadContext();
  const std::vector<SharedHandle<FileEntry> >& files = dctx->getFileEntries();
  BDE result = BDE::list();
  size_t index = 1;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator fi = files.begin(),
        eoi = files.end(); fi != eoi; ++fi, ++index) {
    BDE fileEntry = BDE::dict();
    fileEntry[KEY_INDEX] = util::uitos(index);
    BDE servers = BDE::list();
    const std::deque<SharedHandle<Request> >& requests =
      (*fi)->getInFlightRequests();
    for(std::deque<SharedHandle<Request> >::const_iterator ri =requests.begin(),
          eoi = requests.end(); ri != eoi; ++ri) {
      SharedHandle<PeerStat> ps = (*ri)->getPeerStat();
      if(!ps.isNull()) {
        BDE serverEntry = BDE::dict();
        serverEntry[KEY_URI] = (*ri)->getUri();
        serverEntry[KEY_CURRENT_URI] = (*ri)->getCurrentUri();
        serverEntry[KEY_DOWNLOAD_SPEED] =
          util::uitos(ps->calculateDownloadSpeed());
        servers << serverEntry;
      }
    }
    fileEntry[KEY_SERVERS] = servers;
    result << fileEntry;
  }
  return result;
}

BDE ChangeUriXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.size() < 4 ||
     !params[0].isString() || !params[1].isInteger() ||
     !params[2].isList() || !params[3].isList() ||
     params[1].i() <= 0) {
    throw DL_ABORT_EX("Bad request");
  }
  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 4, posGiven, pos);

  int32_t gid = util::parseInt(params[0].s());
  size_t index = params[1].i()-1;
  const BDE& deluris = params[2];
  const BDE& adduris = params[3];
  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("Cannot remove URIs from GID#%d", gid).str());
  }
  SharedHandle<DownloadContext> dctx = group->getDownloadContext();
  const std::vector<SharedHandle<FileEntry> >& files = dctx->getFileEntries();
  if(files.size() <= index) {
    throw DL_ABORT_EX(StringFormat("fileIndex is out of range").str());
  }
  SharedHandle<FileEntry> s = files[index];
  size_t delcount = 0;
  for(BDE::List::const_iterator i = deluris.listBegin(),
        eoi = deluris.listEnd(); i != eoi; ++i) {
    if(s->removeUri((*i).s())) {
      ++delcount;
    }
  }
  size_t addcount = 0;
  if(posGiven) {
    for(BDE::List::const_iterator i = adduris.listBegin(),
          eoi = adduris.listEnd(); i != eoi; ++i) {
      if(s->insertUri((*i).s(), pos)) {
        ++addcount;
        ++pos;
      }
    }
  } else {
    for(BDE::List::const_iterator i = adduris.listBegin(),
          eoi = adduris.listEnd(); i != eoi; ++i) {
      if(s->addUri((*i).s())) {
        ++addcount;
      }
    }
  }
  if(addcount) {
    std::vector<Command*> commands;
    group->createNextCommand(commands, e);
    e->addCommand(commands);
  }
  BDE res = BDE::list();
  res << delcount;
  res << addcount;
  return res;
}

BDE SystemMulticallXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  
  if(params.size() != 1) {
    throw DL_ABORT_EX("Illegal argument. One item list is expected.");
  }
  const BDE& methodSpecs = params[0];
  BDE list = BDE::list();
  for(BDE::List::const_iterator i = methodSpecs.listBegin(),
        eoi = methodSpecs.listEnd(); i != eoi; ++i) {
    if(!(*i).isDict()) {
      list << createErrorResponse
        (DL_ABORT_EX("system.multicall expected struct."));
      continue;
    }
    if(!(*i).containsKey(KEY_METHOD_NAME) ||
       !(*i).containsKey(KEY_PARAMS)) {
      list << createErrorResponse
        (DL_ABORT_EX("Missing methodName or params."));
      continue;
    }
    const std::string& methodName = (*i)[KEY_METHOD_NAME].s();
    if(methodName == getMethodName()) {
      list << createErrorResponse
        (DL_ABORT_EX("Recursive system.multicall forbidden."));
      continue;
    }
    SharedHandle<XmlRpcMethod> method = XmlRpcMethodFactory::create(methodName);
    XmlRpcRequest innerReq(methodName, (*i)[KEY_PARAMS]);
    XmlRpcResponse res = method->execute(innerReq, e);
    if(res._code == 0) {
      BDE l = BDE::list();
      l << res._param;
      list << l;
    } else {
      list << res._param;
    }
  }
  return list;
}

BDE NoSuchMethodXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  throw DL_ABORT_EX
    (StringFormat("No such method: %s", req._methodName.c_str()).str());
}

} // namespace xmlrpc

} // namespace aria2
