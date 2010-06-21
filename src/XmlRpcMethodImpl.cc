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
#include "LogFactory.h"
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
#include "SegmentMan.h"
#include "TimedHaltCommand.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "Segment.h"
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
const SharedHandle<String> VLB_TRUE = String::g("true");
const SharedHandle<String> VLB_FALSE = String::g("false");
const SharedHandle<String> VLB_OK = String::g("OK");
const SharedHandle<String> VLB_ACTIVE = String::g("active");
const SharedHandle<String> VLB_WAITING = String::g("waiting");
const SharedHandle<String> VLB_PAUSED = String::g("paused");
const SharedHandle<String> VLB_REMOVED = String::g("removed");
const SharedHandle<String> VLB_ERROR = String::g("error");
const SharedHandle<String> VLB_COMPLETE = String::g("complete");
const SharedHandle<String> VLB_USED = String::g("used");
const SharedHandle<String> VLB_ZERO = String::g("0");

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

static SharedHandle<ValueBase> createGIDResponse(gid_t gid)
{
  return String::g(util::itos(gid));
}

static SharedHandle<ValueBase>
addRequestGroup(const SharedHandle<RequestGroup>& group,
                DownloadEngine* e,
                bool posGiven, int pos)
{
  if(posGiven) {
    e->getRequestGroupMan()->insertReservedGroup(pos, group);
  } else {
    e->getRequestGroupMan()->addReservedGroup(group);
  }
  return createGIDResponse(group->getGID());
}

static
SharedHandle<RequestGroup>
findRequestGroup(const SharedHandle<RequestGroupMan>& rgman, gid_t gid)
{
  SharedHandle<RequestGroup> group = rgman->findRequestGroup(gid);
  if(group.isNull()) {
    group = rgman->findReservedGroup(gid);
  }
  return group;
}

static const String* getStringParam
(const SharedHandle<List>& params, size_t index)
{
  const String* stringParam = 0;
  if(params->size() > index) {
    stringParam = asString(params->get(index));
  }
  return stringParam;
}

static const Integer* getIntegerParam
(const SharedHandle<List>& params, size_t index)
{
  const Integer* integerParam = 0;
  if(params->size() > index) {
    integerParam = asInteger(params->get(index));
  }
  return integerParam;
}

static const List* getListParam(const SharedHandle<List>& params, size_t index)
{
  const List* listParam = 0;
  if(params->size() > index) {
    listParam = asList(params->get(index));
  }
  return listParam;
}

static const Dict* getDictParam(const SharedHandle<List>& params, size_t index)
{
  const Dict* dictParam = 0;
  if(params->size() > index) {
    dictParam = asDict(params->get(index));
  }
  return dictParam;
}

static void getPosParam(const SharedHandle<List>& params, size_t posParamIndex,
                        bool& posGiven, size_t& pos)
{
  if(params->size() > posParamIndex) {
    const Integer* p = asInteger(params->get(posParamIndex));
    if(p) {
      if(p->i() >= 0) {
        pos = p->i();
        posGiven = true;
        return;
      } else {
        throw DL_ABORT_EX("Position must be greater than or equal to 0.");
      }
    }
  }
  posGiven = false;
} 

static gid_t getRequiredGidParam
(const SharedHandle<List>& params, size_t posParamIndex)
{
  const String* gidParam = getStringParam(params, posParamIndex);
  if(gidParam) {
    return util::parseLLInt(gidParam->s());
  } else {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
}

template<typename OutputIterator>
static void extractUris(OutputIterator out, const List* src)
{
  if(src) {
    for(List::ValueType::const_iterator i = src->begin(), eoi = src->end();
        i != eoi; ++i) {
      const String* uri = asString(*i);
      if(uri) {
        out++ = uri->s();
      }
    }
  }
}

SharedHandle<ValueBase> AddUriXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  std::vector<std::string> uris;
  extractUris(std::back_inserter(uris), getListParam(params, 0));
  if(uris.empty()) {
    throw DL_ABORT_EX("URI is not provided.");
  }

  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  gatherRequestOption(requestOption, getDictParam(params, 1));

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
SharedHandle<ValueBase> AddTorrentXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  const String* torrentParam = getStringParam(params, 0);
  if(!torrentParam) {
    throw DL_ABORT_EX("Torrent data is not provided.");
  }
  
  std::vector<std::string> uris;
  extractUris(std::back_inserter(uris), getListParam(params, 1));

  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  gatherRequestOption(requestOption, getDictParam(params, 2));

  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 3, posGiven, pos);

  std::vector<SharedHandle<RequestGroup> > result;
  createRequestGroupForBitTorrent(result, requestOption,
                                  uris, torrentParam->s());

  if(!result.empty()) {
    return addRequestGroup(result.front(), e, posGiven, pos);
  } else {
    throw DL_ABORT_EX("No Torrent to download.");
  }
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
SharedHandle<ValueBase> AddMetalinkXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  const String* metalinkParam = getStringParam(params, 0);
  if(!metalinkParam) {
    throw DL_ABORT_EX("Metalink data is not provided.");
  }
  
  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  gatherRequestOption(requestOption, getDictParam(params, 1));

  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 2, posGiven, pos);

  std::vector<SharedHandle<RequestGroup> > result;
  createRequestGroupForMetalink(result, requestOption, metalinkParam->s());
  SharedHandle<List> gids = List::g();
  if(!result.empty()) {
    if(posGiven) {
      e->getRequestGroupMan()->insertReservedGroup(pos, result);
    } else {
      e->getRequestGroupMan()->addReservedGroup(result);
    }
    for(std::vector<SharedHandle<RequestGroup> >::const_iterator i =
          result.begin(), eoi = result.end(); i != eoi; ++i) {
      gids->append(util::itos((*i)->getGID()));
    }
  }
  return gids;
} 
#endif // ENABLE_METALINK

static SharedHandle<ValueBase> removeDownload
(const XmlRpcRequest& req, DownloadEngine* e, bool forceRemove)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);

  SharedHandle<RequestGroup> group =
    e->getRequestGroupMan()->findRequestGroup(gid);
  if(group.isNull()) {
    group = e->getRequestGroupMan()->findReservedGroup(gid);
    if(group.isNull()) {
      throw DL_ABORT_EX
        (StringFormat("Active Download not found for GID#%s",
                      util::itos(gid).c_str()).str());
    }
    if(group->isDependencyResolved()) {
      e->getRequestGroupMan()->removeReservedGroup(gid);
    } else {
      throw DL_ABORT_EX
        (StringFormat("GID#%s cannot be removed now",
                      util::itos(gid).c_str()).str());
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

SharedHandle<ValueBase> RemoveXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return removeDownload(req, e, false);
}

SharedHandle<ValueBase> ForceRemoveXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return removeDownload(req, e, true);
}

static bool pauseRequestGroup
(const SharedHandle<RequestGroup>& group, bool reserved,  bool forcePause)
{
  if((reserved && !group->isPauseRequested()) ||
     (!reserved &&
      !group->isForceHaltRequested() &&
      ((forcePause && group->isHaltRequested() && group->isPauseRequested()) ||
       (!group->isHaltRequested() && !group->isPauseRequested())))) {
    if(!reserved) {
      // Call setHaltRequested before setPauseRequested because
      // setHaltRequested calls setPauseRequested(false) internally.
      if(forcePause) {
        group->setForceHaltRequested(true, RequestGroup::NONE);
      } else {
        group->setHaltRequested(true, RequestGroup::NONE);
      }
    }
    group->setPauseRequested(true);
    return true;
  } else {
    return false;
  }
}

static SharedHandle<ValueBase> pauseDownload
(const XmlRpcRequest& req, DownloadEngine* e, bool forcePause)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);

  bool reserved = false;
  SharedHandle<RequestGroup> group =
    e->getRequestGroupMan()->findRequestGroup(gid);
  if(group.isNull()) {
    reserved = true;
    group = e->getRequestGroupMan()->findReservedGroup(gid);
  }
  if(!group.isNull() && pauseRequestGroup(group, reserved, forcePause)) {
    return createGIDResponse(gid);
  } else {
    throw DL_ABORT_EX
      (StringFormat("GID#%s cannot be paused now",
                    util::itos(gid).c_str()).str());
  }
}

SharedHandle<ValueBase> PauseXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return pauseDownload(req, e, false);
}

SharedHandle<ValueBase> ForcePauseXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return pauseDownload(req, e, true);
}

template<typename InputIterator>
static void pauseRequestGroups
(InputIterator first, InputIterator last, bool reserved, bool forcePause)
{
  for(; first != last; ++first) {
    pauseRequestGroup(*first, reserved, forcePause);
  }
}

static SharedHandle<ValueBase> pauseAllDownloads
(const XmlRpcRequest& req, DownloadEngine* e, bool forcePause)
{
  const std::deque<SharedHandle<RequestGroup> >& groups =
    e->getRequestGroupMan()->getRequestGroups();
  pauseRequestGroups(groups.begin(), groups.end(), false, forcePause);
  const std::deque<SharedHandle<RequestGroup> >& reservedGroups =
    e->getRequestGroupMan()->getReservedGroups();
  pauseRequestGroups(reservedGroups.begin(), reservedGroups.end(),
                     true, forcePause);
  return VLB_OK;
}

SharedHandle<ValueBase> PauseAllXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return pauseAllDownloads(req, e, false);
}

SharedHandle<ValueBase> ForcePauseAllXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return pauseAllDownloads(req, e, true);
}

SharedHandle<ValueBase> UnpauseXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);
  SharedHandle<RequestGroup> group =
    e->getRequestGroupMan()->findReservedGroup(gid);
  if(group.isNull() || !group->isPauseRequested()) {
    throw DL_ABORT_EX
      (StringFormat("GID#%s cannot be unpaused now",
                    util::itos(gid).c_str()).str());
  } else {
    group->setPauseRequested(false);
    e->getRequestGroupMan()->requestQueueCheck();    
  }
  return createGIDResponse(gid);
}

SharedHandle<ValueBase> UnpauseAllXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const std::deque<SharedHandle<RequestGroup> >& groups =
    e->getRequestGroupMan()->getReservedGroups();
  std::for_each(groups.begin(), groups.end(),
                std::bind2nd(mem_fun_sh(&RequestGroup::setPauseRequested),
                             false));
  e->getRequestGroupMan()->requestQueueCheck();    
  return VLB_OK;
}

template<typename InputIterator>
static void createUriEntry
(const SharedHandle<List>& uriList,
 InputIterator first, InputIterator last,
 const SharedHandle<String>& status)
{
  for(; first != last; ++first) {
    SharedHandle<Dict> entry = Dict::g();
    entry->put(KEY_URI, *first);
    entry->put(KEY_STATUS, status);
    uriList->append(entry);
  }
}

static void createUriEntry
(const SharedHandle<List>& uriList, const SharedHandle<FileEntry>& file)
{
  createUriEntry(uriList,
                 file->getSpentUris().begin(),
                 file->getSpentUris().end(),
                 VLB_USED);
  createUriEntry(uriList,
                 file->getRemainingUris().begin(),
                 file->getRemainingUris().end(),
                 VLB_WAITING);
}

template<typename InputIterator>
static void createFileEntry
(const SharedHandle<List>& files, InputIterator first, InputIterator last)
{
  size_t index = 1;
  for(; first != last; ++first, ++index) {
    SharedHandle<Dict> entry = Dict::g();
    entry->put(KEY_INDEX, util::uitos(index));
    entry->put(KEY_PATH, (*first)->getPath());
    entry->put(KEY_SELECTED, (*first)->isRequested()?VLB_TRUE:VLB_FALSE);
    entry->put(KEY_LENGTH, util::uitos((*first)->getLength()));

    SharedHandle<List> uriList = List::g();
    createUriEntry(uriList, *first);
    entry->put(KEY_URIS, uriList);
    files->append(entry);
  }
}

void gatherProgressCommon
(const SharedHandle<Dict>& entryDict, const SharedHandle<RequestGroup>& group)
{
  entryDict->put(KEY_GID, util::itos(group->getGID()));
  // This is "filtered" total length if --select-file is used.
  entryDict->put(KEY_TOTAL_LENGTH, util::uitos(group->getTotalLength()));
  // This is "filtered" total length if --select-file is used.
  entryDict->put(KEY_COMPLETED_LENGTH,util::uitos(group->getCompletedLength()));
  TransferStat stat = group->calculateStat();
  entryDict->put(KEY_DOWNLOAD_SPEED, util::uitos(stat.getDownloadSpeed()));
  entryDict->put(KEY_UPLOAD_SPEED, util::uitos(stat.getUploadSpeed()));
  entryDict->put(KEY_UPLOAD_LENGTH, util::uitos(stat.getAllTimeUploadLength()));
  entryDict->put(KEY_CONNECTIONS, util::uitos(group->getNumConnection()));
  SharedHandle<PieceStorage> ps = group->getPieceStorage();
  if(!ps.isNull()) {
    if(ps->getBitfieldLength() > 0) {
      entryDict->put(KEY_BITFIELD,
                     util::toHex(ps->getBitfield(), ps->getBitfieldLength()));
    }
  }
  const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
  entryDict->put(KEY_PIECE_LENGTH, util::uitos(dctx->getPieceLength()));
  entryDict->put(KEY_NUM_PIECES, util::uitos(dctx->getNumPieces()));
  if(!group->followedBy().empty()) {
    SharedHandle<List> list = List::g();
    // The element is GID.
    for(std::vector<gid_t>::const_iterator i = group->followedBy().begin(),
          eoi = group->followedBy().end(); i != eoi; ++i) {
      list->append(util::itos(*i));
    }
    entryDict->put(KEY_FOLLOWED_BY, list);
  }
  if(group->belongsTo()) {
    entryDict->put(KEY_BELONGS_TO, util::itos(group->belongsTo()));
  }
  SharedHandle<List> files = List::g();
  createFileEntry
    (files, dctx->getFileEntries().begin(), dctx->getFileEntries().end());
  entryDict->put(KEY_FILES, files);
  entryDict->put(KEY_DIR, dctx->getDir());
}

#ifdef ENABLE_BITTORRENT
void gatherBitTorrentMetadata
(const SharedHandle<Dict>& btDict,
 const SharedHandle<TorrentAttribute>& torrentAttrs)
{
  if(!torrentAttrs->comment.empty()) {
    btDict->put(KEY_COMMENT, torrentAttrs->comment);
  }
  if(torrentAttrs->creationDate) {
    btDict->put(KEY_CREATION_DATE, Integer::g(torrentAttrs->creationDate));
  }
  if(!torrentAttrs->mode.empty()) {
    btDict->put(KEY_MODE, torrentAttrs->mode);
  }
  SharedHandle<List> destAnnounceList = List::g();
  for(std::vector<std::vector<std::string> >::const_iterator l =
        torrentAttrs->announceList.begin(),
        eoi = torrentAttrs->announceList.end(); l != eoi; ++l) {
    SharedHandle<List> destAnnounceTier = List::g();
    for(std::vector<std::string>::const_iterator t = (*l).begin(),
          eoi2 = (*l).end(); t != eoi2; ++t) {
      destAnnounceTier->append(*t);
    }
    destAnnounceList->append(destAnnounceTier);
  }
  btDict->put(KEY_ANNOUNCE_LIST, destAnnounceList);
  if(!torrentAttrs->metadata.empty()) {
    SharedHandle<Dict> infoDict = Dict::g();
    infoDict->put(KEY_NAME, torrentAttrs->name);
    btDict->put(KEY_INFO, infoDict);
  }
}

static void gatherProgressBitTorrent
(const SharedHandle<Dict>& entryDict,
 const SharedHandle<TorrentAttribute>& torrentAttrs,
 const BtObject& btObject)
{
  entryDict->put(KEY_INFO_HASH, util::toHex(torrentAttrs->infoHash));
  SharedHandle<Dict> btDict = Dict::g();
  gatherBitTorrentMetadata(btDict, torrentAttrs);
  entryDict->put(KEY_BITTORRENT, btDict);
  if(btObject.isNull()) {
    entryDict->put(KEY_NUM_SEEDERS, VLB_ZERO);
  } else {
    SharedHandle<PeerStorage> peerStorage = btObject.peerStorage_;
    assert(!peerStorage.isNull());
    std::vector<SharedHandle<Peer> > peers;
    peerStorage->getActivePeers(peers);
    entryDict->put(KEY_NUM_SEEDERS,
                   util::uitos(countSeeder(peers.begin(), peers.end())));
  }
}

static void gatherPeer
(const SharedHandle<List>& peers, const SharedHandle<PeerStorage>& ps)
{
  std::vector<SharedHandle<Peer> > activePeers;
  ps->getActivePeers(activePeers);
  for(std::vector<SharedHandle<Peer> >::const_iterator i =
        activePeers.begin(), eoi = activePeers.end(); i != eoi; ++i) {
    SharedHandle<Dict> peerEntry = Dict::g();
    peerEntry->put(KEY_PEER_ID, util::torrentPercentEncode((*i)->getPeerId(),
                                                           PEER_ID_LENGTH));
    peerEntry->put(KEY_IP, (*i)->getIPAddress());
    if((*i)->isIncomingPeer()) {
      peerEntry->put(KEY_PORT, VLB_ZERO);
    } else {
      peerEntry->put(KEY_PORT, util::uitos((*i)->getPort()));
    }
    peerEntry->put(KEY_BITFIELD,
                   util::toHex((*i)->getBitfield(), (*i)->getBitfieldLength()));
    peerEntry->put(KEY_AM_CHOKING, (*i)->amChoking()?VLB_TRUE:VLB_FALSE);
    peerEntry->put(KEY_PEER_CHOKING, (*i)->peerChoking()?VLB_TRUE:VLB_FALSE);
    TransferStat stat = ps->getTransferStatFor(*i);
    peerEntry->put(KEY_DOWNLOAD_SPEED, util::uitos(stat.getDownloadSpeed()));
    peerEntry->put(KEY_UPLOAD_SPEED, util::uitos(stat.getUploadSpeed()));
    peerEntry->put(KEY_SEEDER, (*i)->isSeeder()?VLB_TRUE:VLB_FALSE);
    peers->append(peerEntry);
  }
}
#endif // ENABLE_BITTORRENT

static void gatherProgress
(const SharedHandle<Dict>& entryDict,
 const SharedHandle<RequestGroup>& group, DownloadEngine* e)
{
  gatherProgressCommon(entryDict, group);
#ifdef ENABLE_BITTORRENT
  if(group->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT)) {
    SharedHandle<TorrentAttribute> torrentAttrs =
      bittorrent::getTorrentAttrs(group->getDownloadContext());
    BtObject btObject = e->getBtRegistry()->get(group->getGID());
    gatherProgressBitTorrent(entryDict, torrentAttrs, btObject);
  }
#endif // ENABLE_BITTORRENT
}

void gatherStoppedDownload
(const SharedHandle<Dict>& entryDict, const SharedHandle<DownloadResult>& ds)
{
  entryDict->put(KEY_GID, util::itos(ds->gid));
  entryDict->put(KEY_ERROR_CODE, util::itos(static_cast<int>(ds->result)));
  if(ds->result == downloadresultcode::IN_PROGRESS) {
    entryDict->put(KEY_STATUS, VLB_REMOVED);
  } else if(ds->result == downloadresultcode::FINISHED) {
    entryDict->put(KEY_STATUS, VLB_COMPLETE);
  } else {
    entryDict->put(KEY_STATUS, VLB_ERROR);
  }
  if(!ds->followedBy.empty()) {
    SharedHandle<List> list = List::g();
    // The element is GID.
    for(std::vector<gid_t>::const_iterator i = ds->followedBy.begin(),
          eoi = ds->followedBy.end(); i != eoi; ++i) {
      list->append(util::itos(*i));
    }
    entryDict->put(KEY_FOLLOWED_BY, list);
  }
  if(ds->belongsTo) {
    entryDict->put(KEY_BELONGS_TO, util::itos(ds->belongsTo));
  }
  SharedHandle<List> files = List::g();
  createFileEntry(files, ds->fileEntries.begin(), ds->fileEntries.end());
  entryDict->put(KEY_FILES, files);
  entryDict->put(KEY_TOTAL_LENGTH, util::uitos(ds->totalLength));
  entryDict->put(KEY_COMPLETED_LENGTH, util::uitos(ds->completedLength));
  entryDict->put(KEY_UPLOAD_LENGTH, util::uitos(ds->uploadLength));
  if(!ds->bitfieldStr.empty()) {
    entryDict->put(KEY_BITFIELD, ds->bitfieldStr);
  }
  entryDict->put(KEY_DOWNLOAD_SPEED, VLB_ZERO);
  entryDict->put(KEY_UPLOAD_SPEED, VLB_ZERO);
  if(!ds->infoHashStr.empty()) {
    entryDict->put(KEY_INFO_HASH, ds->infoHashStr);
    entryDict->put(KEY_NUM_SEEDERS, VLB_ZERO);
  }
  entryDict->put(KEY_PIECE_LENGTH, util::uitos(ds->pieceLength));
  entryDict->put(KEY_NUM_PIECES, util::uitos(ds->numPieces));
  entryDict->put(KEY_CONNECTIONS, VLB_ZERO);
  entryDict->put(KEY_DIR, ds->dir);
}

SharedHandle<ValueBase> GetFilesXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);
  SharedHandle<List> files = List::g();
  SharedHandle<RequestGroup> group =
    findRequestGroup(e->getRequestGroupMan(), gid);
  if(group.isNull()) {
    SharedHandle<DownloadResult> dr =
      e->getRequestGroupMan()->findDownloadResult(gid);
    if(dr.isNull()) {
      throw DL_ABORT_EX
        (StringFormat("No file data is available for GID#%s",
                      util::itos(gid).c_str()).str());
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

SharedHandle<ValueBase> GetUrisXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);
  SharedHandle<RequestGroup> group =
    findRequestGroup(e->getRequestGroupMan(), gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("No URI data is available for GID#%s",
                    util::itos(gid).c_str()).str());
  }
  SharedHandle<List> uriList = List::g();
  // TODO Current implementation just returns first FileEntry's URIs.
  if(!group->getDownloadContext()->getFileEntries().empty()) {
    createUriEntry(uriList, group->getDownloadContext()->getFirstFileEntry());
  }
  return uriList;
}

#ifdef ENABLE_BITTORRENT
SharedHandle<ValueBase> GetPeersXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);

  SharedHandle<RequestGroup> group =
    findRequestGroup(e->getRequestGroupMan(), gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("No peer data is available for GID#%s",
                    util::itos(gid).c_str()).str());
  }
  SharedHandle<List> peers = List::g();
  BtObject btObject = e->getBtRegistry()->get(group->getGID());
  if(!btObject.isNull()) {
    assert(!btObject.peerStorage_.isNull());
    gatherPeer(peers, btObject.peerStorage_);
  }
  return peers;
}
#endif // ENABLE_BITTORRENT

SharedHandle<ValueBase> TellStatusXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);

  SharedHandle<RequestGroup> group =
    e->getRequestGroupMan()->findRequestGroup(gid);

  SharedHandle<Dict> entryDict = Dict::g();
  if(group.isNull()) {
    group = e->getRequestGroupMan()->findReservedGroup(gid);
    if(group.isNull()) {
      SharedHandle<DownloadResult> ds =
        e->getRequestGroupMan()->findDownloadResult(gid);
      if(ds.isNull()) {
        throw DL_ABORT_EX
          (StringFormat("No such download for GID#%s",
                        util::itos(gid).c_str()).str());
      }
      gatherStoppedDownload(entryDict, ds);
    } else {
      if(group->isPauseRequested()) {
        entryDict->put(KEY_STATUS, VLB_PAUSED);
      } else {
        entryDict->put(KEY_STATUS, VLB_WAITING);
      }
      gatherProgress(entryDict, group, e);
    }
  } else {
    entryDict->put(KEY_STATUS, VLB_ACTIVE);
    gatherProgress(entryDict, group, e);
  }
  return entryDict;
}

SharedHandle<ValueBase> TellActiveXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  SharedHandle<List> list = List::g();
  const std::deque<SharedHandle<RequestGroup> >& groups =
    e->getRequestGroupMan()->getRequestGroups();
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator i =
        groups.begin(), eoi = groups.end(); i != eoi; ++i) {
    SharedHandle<Dict> entryDict = Dict::g();
    entryDict->put(KEY_STATUS, VLB_ACTIVE);
    gatherProgress(entryDict, *i, e);
    list->append(entryDict);
  }
  return list;
}

const std::deque<SharedHandle<RequestGroup> >&
TellWaitingXmlRpcMethod::getItems(DownloadEngine* e) const
{
  return e->getRequestGroupMan()->getReservedGroups();
}

void TellWaitingXmlRpcMethod::createEntry
(const SharedHandle<Dict>& entryDict, const SharedHandle<RequestGroup>& item,
 DownloadEngine* e) const
{
  if(item->isPauseRequested()) {
    entryDict->put(KEY_STATUS, VLB_PAUSED);
  } else {
    entryDict->put(KEY_STATUS, VLB_WAITING);
  }
  gatherProgress(entryDict, item, e);
}

const std::deque<SharedHandle<DownloadResult> >&
TellStoppedXmlRpcMethod::getItems(DownloadEngine* e) const
{
  return e->getRequestGroupMan()->getDownloadResults();
}

void TellStoppedXmlRpcMethod::createEntry
(const SharedHandle<Dict>& entryDict, const SharedHandle<DownloadResult>& item,
 DownloadEngine* e) const
{
  gatherStoppedDownload(entryDict, item);
}

SharedHandle<ValueBase> PurgeDownloadResultXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  e->getRequestGroupMan()->purgeDownloadResult();
  return VLB_OK;
}

SharedHandle<ValueBase> ChangeOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);

  SharedHandle<RequestGroup> group =
    findRequestGroup(e->getRequestGroupMan(), gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("Cannot change option for GID#%s",
                    util::itos(gid).c_str()).str());
  }
  SharedHandle<Option> option(new Option());
  const Dict* optionsParam = getDictParam(params, 1);
  if(optionsParam) {
    gatherChangeableOption(option, optionsParam);
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
        btObject.btRuntime_->setMaxPeers(option->getAsInt(PREF_BT_MAX_PEERS));
      }
    }
#endif // ENABLE_BITTORRENT
  }
  return VLB_OK;
}

SharedHandle<ValueBase> ChangeGlobalOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  const Dict* optionsParam = getDictParam(params, 0);
  if(!optionsParam) {
    return VLB_OK;
  }
  SharedHandle<Option> option(new Option());
  gatherChangeableGlobalOption(option, optionsParam);
  applyChangeableGlobalOption(e->getOption(), option.get());

  if(option->defined(PREF_MAX_OVERALL_DOWNLOAD_LIMIT)) {
    e->getRequestGroupMan()->setMaxOverallDownloadSpeedLimit
      (option->getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
  }
  if(option->defined(PREF_MAX_OVERALL_UPLOAD_LIMIT)) {
    e->getRequestGroupMan()->setMaxOverallUploadSpeedLimit
      (option->getAsInt(PREF_MAX_OVERALL_UPLOAD_LIMIT));
  }
  if(option->defined(PREF_MAX_CONCURRENT_DOWNLOADS)) {
    e->getRequestGroupMan()->setMaxSimultaneousDownloads
      (option->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS));
    e->getRequestGroupMan()->requestQueueCheck();
  }
  return VLB_OK;
}

SharedHandle<ValueBase> GetVersionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  SharedHandle<Dict> result = Dict::g();
  result->put(KEY_VERSION, PACKAGE_VERSION);
  SharedHandle<List> featureList = List::g();
  const FeatureMap& features = FeatureConfig::getInstance()->getFeatures();
  for(FeatureMap::const_iterator i = features.begin(), eoi = features.end();
      i != eoi;++i){
    if((*i).second) {
      featureList->append((*i).first);
    }
  }
  result->put(KEY_ENABLED_FEATURES, featureList);
  return result;
}

template<typename InputIterator>
static void pushRequestOption
(const SharedHandle<Dict>& dict,
 InputIterator optionFirst, InputIterator optionLast)
{
  const std::set<std::string>& requestOptions = listRequestOptions();
  for(; optionFirst != optionLast; ++optionFirst) {
    if(requestOptions.count((*optionFirst).first)) {
      dict->put((*optionFirst).first, (*optionFirst).second);
    }
  }
}

SharedHandle<ValueBase> GetOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);

  SharedHandle<RequestGroup> group =
    findRequestGroup(e->getRequestGroupMan(), gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("Cannot get option for GID#%s",
                    util::itos(gid).c_str()).str());
  }
  SharedHandle<Dict> result = Dict::g();
  SharedHandle<Option> option = group->getOption();
  pushRequestOption(result, option->begin(), option->end());
  return result;
}

SharedHandle<ValueBase> GetGlobalOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  SharedHandle<Dict> result = Dict::g();
  for(std::map<std::string, std::string>::const_iterator i =
        e->getOption()->begin(), eoi = e->getOption()->end(); i != eoi; ++i) {
    SharedHandle<OptionHandler> h = getOptionParser()->findByName((*i).first);
    if(!h.isNull() && !h->isHidden()) {
      result->put((*i).first, (*i).second);
    }
  }
  return result;
}

SharedHandle<ValueBase> ChangePositionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);
  const Integer* posParam = getIntegerParam(params, 1);
  const String* howParam = getStringParam(params, 2);

  if(!posParam || !howParam) {
    throw DL_ABORT_EX("Illegal argument.");
  }
  int pos = posParam->i();
  const std::string& howStr = howParam->s();
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
    e->getRequestGroupMan()->changeReservedGroupPosition(gid, pos, how);
  SharedHandle<Integer> result = Integer::g(destPos);
  return result;
}

SharedHandle<ValueBase> GetSessionInfoXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  SharedHandle<Dict> result = Dict::g();
  result->put(KEY_SESSION_ID, util::toHex(e->getSessionId()));
  return result;
}

SharedHandle<ValueBase> GetServersXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);
  SharedHandle<RequestGroup> group =
    e->getRequestGroupMan()->findRequestGroup(gid);
  if(group.isNull()) {
    throw DL_ABORT_EX(StringFormat("No active download for GID#%s",
                                   util::itos(gid).c_str()).str());
  }
  const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
  const std::vector<SharedHandle<FileEntry> >& files = dctx->getFileEntries();
  SharedHandle<List> result = List::g();
  size_t index = 1;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator fi = files.begin(),
        eoi = files.end(); fi != eoi; ++fi, ++index) {
    SharedHandle<Dict> fileEntry = Dict::g();
    fileEntry->put(KEY_INDEX, util::uitos(index));
    SharedHandle<List> servers = List::g();
    const std::deque<SharedHandle<Request> >& requests =
      (*fi)->getInFlightRequests();
    for(std::deque<SharedHandle<Request> >::const_iterator ri =requests.begin(),
          eoi = requests.end(); ri != eoi; ++ri) {
      SharedHandle<PeerStat> ps = (*ri)->getPeerStat();
      if(!ps.isNull()) {
        SharedHandle<Dict> serverEntry = Dict::g();
        serverEntry->put(KEY_URI, (*ri)->getUri());
        serverEntry->put(KEY_CURRENT_URI, (*ri)->getCurrentUri());
        serverEntry->put(KEY_DOWNLOAD_SPEED,
                         util::uitos(ps->calculateDownloadSpeed()));
        servers->append(serverEntry);
      }
    }
    fileEntry->put(KEY_SERVERS, servers);
    result->append(fileEntry);
  }
  return result;
}

SharedHandle<ValueBase> ChangeUriXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  gid_t gid = getRequiredGidParam(params, 0);
  const Integer* indexParam = getIntegerParam(params, 1);
  const List* delUrisParam = getListParam(params, 2);
  const List* addUrisParam = getListParam(params, 3);
  if(!indexParam || !delUrisParam || ! addUrisParam) {
    throw DL_ABORT_EX("Bad request");
  }
  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 4, posGiven, pos);

  size_t index = indexParam->i()-1;
  SharedHandle<RequestGroup> group =
    findRequestGroup(e->getRequestGroupMan(), gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("Cannot remove URIs from GID#%s",
                    util::itos(gid).c_str()).str());
  }
  const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
  const std::vector<SharedHandle<FileEntry> >& files = dctx->getFileEntries();
  if(files.size() <= index) {
    throw DL_ABORT_EX(StringFormat("fileIndex is out of range").str());
  }
  SharedHandle<FileEntry> s = files[index];
  size_t delcount = 0;
  for(List::ValueType::const_iterator i = delUrisParam->begin(),
        eoi = delUrisParam->end(); i != eoi; ++i) {
    const String* uri = asString(*i);
    if(uri && s->removeUri(uri->s())) {
      ++delcount;
    }
  }
  size_t addcount = 0;
  if(posGiven) {
    for(List::ValueType::const_iterator i = addUrisParam->begin(),
          eoi = addUrisParam->end(); i != eoi; ++i) {
      const String* uri = asString(*i);
      if(uri && s->insertUri(uri->s(), pos)) {
        ++addcount;
        ++pos;
      }
    }
  } else {
    for(List::ValueType::const_iterator i = addUrisParam->begin(),
          eoi = addUrisParam->end(); i != eoi; ++i) {
      const String* uri = asString(*i);
      if(uri && s->addUri(uri->s())) {
        ++addcount;
      }
    }
  }
  if(addcount && !group->getPieceStorage().isNull()) {
    std::vector<Command*> commands;
    group->createNextCommand(commands, e);
    e->addCommand(commands);
    group->getSegmentMan()->recognizeSegmentFor(s);
  }
  SharedHandle<List> res = List::g();
  res->append(Integer::g(delcount));
  res->append(Integer::g(addcount));
  return res;
}

static SharedHandle<ValueBase> goingShutdown
(const XmlRpcRequest& req, DownloadEngine* e, bool forceHalt)
{
  // Schedule shutdown after 3seconds to give time to client to
  // receive XML-RPC response.
  e->addRoutineCommand(new TimedHaltCommand(e->newCUID(), e, 3, forceHalt));
  LogFactory::getInstance()->info("Scheduled shutdown in 3 seconds.");
  return VLB_OK;
}

SharedHandle<ValueBase> ShutdownXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return goingShutdown(req, e, false);
}

SharedHandle<ValueBase> ForceShutdownXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  return goingShutdown(req, e, true);
}

SharedHandle<ValueBase> SystemMulticallXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<List>& params = req.params;
  const List* methodSpecs = getListParam(params, 0);
  if(!methodSpecs) {
    throw DL_ABORT_EX("Illegal argument. One item list is expected.");
  }
  SharedHandle<List> list = List::g();
  for(List::ValueType::const_iterator i = methodSpecs->begin(),
        eoi = methodSpecs->end(); i != eoi; ++i) {
    const Dict* methodDict = asDict(*i);
    if(!methodDict) {
      list->append(createErrorResponse
                   (DL_ABORT_EX("system.multicall expected struct.")));
      continue;
    }
    const String* methodName = asString(methodDict->get(KEY_METHOD_NAME));
    const List* paramsList = asList(methodDict->get(KEY_PARAMS));

    if(!methodName || !paramsList) {
      list->append(createErrorResponse
                   (DL_ABORT_EX("Missing methodName or params.")));
      continue;
    }
    if(methodName->s() == getMethodName()) {
      list->append(createErrorResponse
                   (DL_ABORT_EX("Recursive system.multicall forbidden.")));
      continue;
    }
    SharedHandle<XmlRpcMethod> method =
      XmlRpcMethodFactory::create(methodName->s());
    XmlRpcRequest innerReq
      (methodName->s(), static_pointer_cast<List>(methodDict->get(KEY_PARAMS)));
    XmlRpcResponse res = method->execute(innerReq, e);
    if(res.code == 0) {
      SharedHandle<List> l = List::g();
      l->append(res.param);
      list->append(l);
    } else {
      list->append(res.param);
    }
  }
  return list;
}

SharedHandle<ValueBase> NoSuchMethodXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  throw DL_ABORT_EX
    (StringFormat("No such method: %s", req.methodName.c_str()).str());
}

} // namespace xmlrpc

} // namespace aria2
