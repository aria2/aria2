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
#include "RpcMethodImpl.h"

#include <cassert>
#include <algorithm>
#include <sstream>

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
#include "fmt.h"
#include "RpcRequest.h"
#include "PieceStorage.h"
#include "DownloadContext.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include "prefs.h"
#include "message.h"
#include "FeatureConfig.h"
#include "array_fun.h"
#include "RpcMethodFactory.h"
#include "RpcResponse.h"
#include "SegmentMan.h"
#include "TimedHaltCommand.h"
#include "PeerStat.h"
#include "base64.h"
#include "BitfieldMan.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigest.h"
# include "message_digest_helper.h"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
# include "BtRegistry.h"
# include "PeerStorage.h"
# include "Peer.h"
# include "BtRuntime.h"
# include "BtAnnounce.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace rpc {

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
const std::string KEY_NUM_WAITING = "numWaiting";
const std::string KEY_NUM_STOPPED = "numStopped";
const std::string KEY_NUM_ACTIVE = "numActive";
} // namespace

namespace {
SharedHandle<ValueBase> createGIDResponse(a2_gid_t gid)
{
  return String::g(util::itos(gid));
}
} // namespace

namespace {
SharedHandle<ValueBase>
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
} // namespace

namespace {
bool checkPosParam(const Integer* posParam)
{
  if(posParam) {
    if(posParam->i() >= 0) {
      return true;
    } else {
      throw DL_ABORT_EX("Position must be greater than or equal to 0.");
    }
  }
  return false;
}
} // namespace

namespace {
a2_gid_t str2Gid(const String* str)
{
  assert(str);
  return util::parseLLInt(str->s());
}
} // namespace

namespace {
template<typename OutputIterator>
void extractUris(OutputIterator out, const List* src)
{
  if(src) {
    for(List::ValueType::const_iterator i = src->begin(), eoi = src->end();
        i != eoi; ++i) {
      const String* uri = downcast<String>(*i);
      if(uri) {
        out++ = uri->s();
      }
    }
  }
}
} // namespace

SharedHandle<ValueBase> AddUriRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const List* urisParam = checkRequiredParam<List>(req, 0);
  const Dict* optsParam = checkParam<Dict>(req, 1);
  const Integer* posParam = checkParam<Integer>(req, 2);

  std::vector<std::string> uris;
  extractUris(std::back_inserter(uris), urisParam);
  if(uris.empty()) {
    throw DL_ABORT_EX("URI is not provided.");
  }

  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  gatherRequestOption(requestOption.get(), optsParam);

  bool posGiven = checkPosParam(posParam);
  size_t pos = posGiven ? posParam->i() : 0;

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

#ifdef ENABLE_MESSAGE_DIGEST
namespace {
std::string getHexSha1(const std::string& s)
{
  unsigned char hash[20];
  message_digest::digest(hash, sizeof(hash), MessageDigest::sha1(),
                         s.data(), s.size());
  return util::toHex(hash, sizeof(hash));
}
} // namespace
#endif // ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_BITTORRENT
SharedHandle<ValueBase> AddTorrentRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* torrentParam = checkRequiredParam<String>(req, 0);
  const List* urisParam = checkParam<List>(req, 1);
  const Dict* optsParam = checkParam<Dict>(req, 2);
  const Integer* posParam = checkParam<Integer>(req, 3);

  SharedHandle<String> tempTorrentParam;
  if(req.jsonRpc) {
    tempTorrentParam = String::g
      (base64::decode(torrentParam->s().begin(),
                      torrentParam->s().end()));
    torrentParam = tempTorrentParam.get();
  }
  std::vector<std::string> uris;
  extractUris(std::back_inserter(uris), urisParam);

  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  gatherRequestOption(requestOption.get(), optsParam);

  bool posGiven = checkPosParam(posParam);
  size_t pos = posGiven ? posParam->i() : 0;

  std::string filename = util::applyDir
    (requestOption->get(PREF_DIR), getHexSha1(torrentParam->s())+".torrent");
  std::vector<SharedHandle<RequestGroup> > result;
  // Save uploaded data in order to save this download in
  // --save-session file.
  if(util::saveAs(filename, torrentParam->s(), true)) {
    A2_LOG_INFO(fmt("Uploaded torrent data was saved as %s", filename.c_str()));
    requestOption->put(PREF_TORRENT_FILE, filename);
  } else {
    A2_LOG_INFO(fmt("Uploaded torrent data was not saved."
                    " Failed to write file %s", filename.c_str()));
    filename.clear();
  }
  createRequestGroupForBitTorrent(result, requestOption, uris, filename,
                                  torrentParam->s());

  if(!result.empty()) {
    return addRequestGroup(result.front(), e, posGiven, pos);
  } else {
    throw DL_ABORT_EX("No Torrent to download.");
  }
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
SharedHandle<ValueBase> AddMetalinkRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* metalinkParam = checkRequiredParam<String>(req, 0);
  const Dict* optsParam = checkParam<Dict>(req, 1);
  const Integer* posParam = checkParam<Integer>(req, 2);

  SharedHandle<String> tempMetalinkParam;
  if(req.jsonRpc) {
    tempMetalinkParam = String::g
      (base64::decode(metalinkParam->s().begin(),
                      metalinkParam->s().end()));
    metalinkParam = tempMetalinkParam.get();
  }
  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  gatherRequestOption(requestOption.get(), optsParam);

  bool posGiven = checkPosParam(posParam);
  size_t pos = posGiven ? posParam->i() : 0;

  std::vector<SharedHandle<RequestGroup> > result;
#ifdef ENABLE_MESSAGE_DIGEST
  // TODO RFC5854 Metalink has the extension .meta4 and Metalink
  // Version 3 uses .metalink extension. We use .meta4 for both
  // RFC5854 Metalink and Version 3. aria2 can detect which of which
  // by reading content rather than extension.
  std::string filename = util::applyDir
    (requestOption->get(PREF_DIR), getHexSha1(metalinkParam->s())+".meta4");
  // Save uploaded data in order to save this download in
  // --save-session file.
  if(util::saveAs(filename, metalinkParam->s(), true)) {
    A2_LOG_INFO(fmt("Uploaded metalink data was saved as %s",
                    filename.c_str()));
    requestOption->put(PREF_METALINK_FILE, filename);
    createRequestGroupForMetalink(result, requestOption);
  } else {
    A2_LOG_INFO(fmt("Uploaded metalink data was not saved."
                    " Failed to write file %s", filename.c_str()));
    createRequestGroupForMetalink(result, requestOption, metalinkParam->s());
  }
#else // !ENABLE_MESSAGE_DIGEST
  createRequestGroupForMetalink(result, requestOption, metalinkParam->s());
#endif // !ENABLE_MESSAGE_DIGEST
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

namespace {
SharedHandle<ValueBase> removeDownload
(const RpcRequest& req, DownloadEngine* e, bool forceRemove)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(group) {
    if(group->getState() == RequestGroup::STATE_ACTIVE) {
      if(forceRemove) {
        group->setForceHaltRequested(true, RequestGroup::USER_REQUEST);
      } else {
        group->setHaltRequested(true, RequestGroup::USER_REQUEST);
      }
      e->setRefreshInterval(0);
    } else {
      if(group->isDependencyResolved()) {
        e->getRequestGroupMan()->removeReservedGroup(gid);
      } else {
        throw DL_ABORT_EX(fmt("GID#%" PRId64 " cannot be removed now", gid));
      }
    }
  } else {
    throw DL_ABORT_EX(fmt("Active Download not found for GID#%" PRId64,
                          gid));
  }
  return createGIDResponse(gid);
}
} // namespace

SharedHandle<ValueBase> RemoveRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  return removeDownload(req, e, false);
}

SharedHandle<ValueBase> ForceRemoveRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  return removeDownload(req, e, true);
}

namespace {
bool pauseRequestGroup
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
} // namespace

namespace {
SharedHandle<ValueBase> pauseDownload
(const RpcRequest& req, DownloadEngine* e, bool forcePause)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(group) {
    bool reserved = group->getState() == RequestGroup::STATE_WAITING;
    if(pauseRequestGroup(group, reserved, forcePause)) {
      e->setRefreshInterval(0);
      return createGIDResponse(gid);
    }
  }
  throw DL_ABORT_EX(fmt("GID#%" PRId64 " cannot be paused now", gid));
}
} // namespace

SharedHandle<ValueBase> PauseRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  return pauseDownload(req, e, false);
}

SharedHandle<ValueBase> ForcePauseRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  return pauseDownload(req, e, true);
}

namespace {
template<typename InputIterator>
void pauseRequestGroups
(InputIterator first, InputIterator last, bool reserved, bool forcePause)
{
  for(; first != last; ++first) {
    pauseRequestGroup(*first, reserved, forcePause);
  }
}
} // namespace

namespace {
SharedHandle<ValueBase> pauseAllDownloads
(const RpcRequest& req, DownloadEngine* e, bool forcePause)
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
} // namespace

SharedHandle<ValueBase> PauseAllRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  return pauseAllDownloads(req, e, false);
}

SharedHandle<ValueBase> ForcePauseAllRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  return pauseAllDownloads(req, e, true);
}

SharedHandle<ValueBase> UnpauseRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(!group ||
     group->getState() != RequestGroup::STATE_WAITING ||
     !group->isPauseRequested()) {
    throw DL_ABORT_EX(fmt("GID#%" PRId64 " cannot be unpaused now", gid));
  } else {
    group->setPauseRequested(false);
    e->getRequestGroupMan()->requestQueueCheck();    
  }
  return createGIDResponse(gid);
}

SharedHandle<ValueBase> UnpauseAllRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const std::deque<SharedHandle<RequestGroup> >& groups =
    e->getRequestGroupMan()->getReservedGroups();
  std::for_each(groups.begin(), groups.end(),
                std::bind2nd(mem_fun_sh(&RequestGroup::setPauseRequested),
                             false));
  e->getRequestGroupMan()->requestQueueCheck();    
  return VLB_OK;
}

namespace {
template<typename InputIterator>
void createUriEntry
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
} // namespace

namespace {
void createUriEntry
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
} // namespace

namespace {
template<typename InputIterator>
void createFileEntry
(const SharedHandle<List>& files,
 InputIterator first, InputIterator last,
 const BitfieldMan* bf)
{
  size_t index = 1;
  for(; first != last; ++first, ++index) {
    SharedHandle<Dict> entry = Dict::g();
    entry->put(KEY_INDEX, util::uitos(index));
    entry->put(KEY_PATH, (*first)->getPath());
    entry->put(KEY_SELECTED, (*first)->isRequested()?VLB_TRUE:VLB_FALSE);
    entry->put(KEY_LENGTH, util::itos((*first)->getLength()));
    int64_t completedLength = bf->getOffsetCompletedLength
      ((*first)->getOffset(), (*first)->getLength());
    entry->put(KEY_COMPLETED_LENGTH, util::itos(completedLength));

    SharedHandle<List> uriList = List::g();
    createUriEntry(uriList, *first);
    entry->put(KEY_URIS, uriList);
    files->append(entry);
  }
}
} // namespace

namespace {
template<typename InputIterator>
void createFileEntry
(const SharedHandle<List>& files,
 InputIterator first, InputIterator last,
 int64_t totalLength,
 int32_t pieceLength,
 const std::string& bitfield)
{
  BitfieldMan bf(pieceLength, totalLength);
  bf.setBitfield(reinterpret_cast<const unsigned char*>(bitfield.data()),
                 bitfield.size());
  createFileEntry(files, first, last, &bf);
}
} // namespace

namespace {
template<typename InputIterator>
void createFileEntry
(const SharedHandle<List>& files,
 InputIterator first, InputIterator last,
 int64_t totalLength,
 int32_t pieceLength,
 const SharedHandle<PieceStorage>& ps)
{
  BitfieldMan bf(pieceLength, totalLength);
  if(ps) {
    bf.setBitfield(ps->getBitfield(), ps->getBitfieldLength());
  }
  createFileEntry(files, first, last, &bf);
}
} // namespace

namespace {
bool requested_key
(const std::vector<std::string>& keys, const std::string& k)
{
  return keys.empty() || std::find(keys.begin(), keys.end(), k) != keys.end();
}
} // namespace

void gatherProgressCommon
(const SharedHandle<Dict>& entryDict,
 const SharedHandle<RequestGroup>& group,
 const std::vector<std::string>& keys)
{
  const SharedHandle<PieceStorage>& ps = group->getPieceStorage();
  if(requested_key(keys, KEY_GID)) {
    entryDict->put(KEY_GID, util::itos(group->getGID()));
  }
  if(requested_key(keys, KEY_TOTAL_LENGTH)) {
    // This is "filtered" total length if --select-file is used.
    entryDict->put(KEY_TOTAL_LENGTH, util::itos(group->getTotalLength()));
  }
  if(requested_key(keys, KEY_COMPLETED_LENGTH)) {
    // This is "filtered" total length if --select-file is used.
    entryDict->put
      (KEY_COMPLETED_LENGTH,util::itos(group->getCompletedLength()));
  }
  TransferStat stat = group->calculateStat();
  if(requested_key(keys, KEY_DOWNLOAD_SPEED)) {
    entryDict->put(KEY_DOWNLOAD_SPEED, util::itos(stat.getDownloadSpeed()));
  }
  if(requested_key(keys, KEY_UPLOAD_SPEED)) {
    entryDict->put(KEY_UPLOAD_SPEED, util::itos(stat.getUploadSpeed()));
  }
  if(requested_key(keys, KEY_UPLOAD_LENGTH)) {
    entryDict->put
      (KEY_UPLOAD_LENGTH, util::itos(stat.getAllTimeUploadLength()));
  }
  if(requested_key(keys, KEY_CONNECTIONS)) {
    entryDict->put(KEY_CONNECTIONS, util::itos(group->getNumConnection()));
  }
  if(requested_key(keys, KEY_BITFIELD)) {
    if(ps) {
      if(ps->getBitfieldLength() > 0) {
        entryDict->put(KEY_BITFIELD,
                       util::toHex(ps->getBitfield(), ps->getBitfieldLength()));
      }
    }
  }
  const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
  if(requested_key(keys, KEY_PIECE_LENGTH)) {
    entryDict->put(KEY_PIECE_LENGTH, util::itos(dctx->getPieceLength()));
  }
  if(requested_key(keys, KEY_NUM_PIECES)) {
    entryDict->put(KEY_NUM_PIECES, util::uitos(dctx->getNumPieces()));
  }
  if(requested_key(keys, KEY_FOLLOWED_BY)) {
    if(!group->followedBy().empty()) {
      SharedHandle<List> list = List::g();
      // The element is GID.
      for(std::vector<a2_gid_t>::const_iterator i = group->followedBy().begin(),
            eoi = group->followedBy().end(); i != eoi; ++i) {
        list->append(util::itos(*i));
      }
      entryDict->put(KEY_FOLLOWED_BY, list);
    }
  }
  if(requested_key(keys, KEY_BELONGS_TO)) {
    if(group->belongsTo()) {
      entryDict->put(KEY_BELONGS_TO, util::itos(group->belongsTo()));
    }
  }
  if(requested_key(keys, KEY_FILES)) {
    SharedHandle<List> files = List::g();
    createFileEntry
      (files, dctx->getFileEntries().begin(), dctx->getFileEntries().end(),
       dctx->getTotalLength(), dctx->getPieceLength(), ps);
    entryDict->put(KEY_FILES, files);
  }
  if(requested_key(keys, KEY_DIR)) {
    entryDict->put(KEY_DIR, group->getOption()->get(PREF_DIR));
  }
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

namespace {
void gatherProgressBitTorrent
(const SharedHandle<Dict>& entryDict,
 const SharedHandle<TorrentAttribute>& torrentAttrs,
 const SharedHandle<BtObject>& btObject,
 const std::vector<std::string>& keys)
{
  if(requested_key(keys, KEY_INFO_HASH)) {
    entryDict->put(KEY_INFO_HASH, util::toHex(torrentAttrs->infoHash));
  }
  if(requested_key(keys, KEY_BITTORRENT)) {
    SharedHandle<Dict> btDict = Dict::g();
    gatherBitTorrentMetadata(btDict, torrentAttrs);
    entryDict->put(KEY_BITTORRENT, btDict);
  }
  if(requested_key(keys, KEY_NUM_SEEDERS)) {
    if(!btObject) {
      entryDict->put(KEY_NUM_SEEDERS, VLB_ZERO);
    } else {
      const SharedHandle<PeerStorage>& peerStorage = btObject->peerStorage;
      assert(peerStorage);
      std::vector<SharedHandle<Peer> > peers;
      peerStorage->getActivePeers(peers);
      entryDict->put(KEY_NUM_SEEDERS,
                     util::uitos(countSeeder(peers.begin(), peers.end())));
    }
  }
}
} // namespace

namespace {
void gatherPeer
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
    peerEntry->put(KEY_DOWNLOAD_SPEED, util::itos(stat.getDownloadSpeed()));
    peerEntry->put(KEY_UPLOAD_SPEED, util::itos(stat.getUploadSpeed()));
    peerEntry->put(KEY_SEEDER, (*i)->isSeeder()?VLB_TRUE:VLB_FALSE);
    peers->append(peerEntry);
  }
}
} // namespace
#endif // ENABLE_BITTORRENT

namespace {
void gatherProgress
(const SharedHandle<Dict>& entryDict,
 const SharedHandle<RequestGroup>& group,
 DownloadEngine* e,
 const std::vector<std::string>& keys)
{
  gatherProgressCommon(entryDict, group, keys);
#ifdef ENABLE_BITTORRENT
  if(group->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT)) {
    SharedHandle<TorrentAttribute> torrentAttrs =
      bittorrent::getTorrentAttrs(group->getDownloadContext());
    const SharedHandle<BtObject>& btObject =
      e->getBtRegistry()->get(group->getGID());
    gatherProgressBitTorrent(entryDict, torrentAttrs, btObject, keys);
  }
#endif // ENABLE_BITTORRENT
}
} // namespace

void gatherStoppedDownload
(const SharedHandle<Dict>& entryDict, const SharedHandle<DownloadResult>& ds,
 const std::vector<std::string>& keys)
{
  if(requested_key(keys, KEY_GID)) {
    entryDict->put(KEY_GID, util::itos(ds->gid));
  }
  if(requested_key(keys, KEY_ERROR_CODE)) {
    entryDict->put(KEY_ERROR_CODE, util::itos(static_cast<int>(ds->result)));
  }
  if(requested_key(keys, KEY_STATUS)) {
    if(ds->result == error_code::REMOVED) {
      entryDict->put(KEY_STATUS, VLB_REMOVED);
    } else if(ds->result == error_code::FINISHED) {
      entryDict->put(KEY_STATUS, VLB_COMPLETE);
    } else {
      entryDict->put(KEY_STATUS, VLB_ERROR);
    }
  }
  if(requested_key(keys, KEY_FOLLOWED_BY)) {
    if(!ds->followedBy.empty()) {
      SharedHandle<List> list = List::g();
      // The element is GID.
      for(std::vector<a2_gid_t>::const_iterator i = ds->followedBy.begin(),
            eoi = ds->followedBy.end(); i != eoi; ++i) {
        list->append(util::itos(*i));
      }
      entryDict->put(KEY_FOLLOWED_BY, list);
    }
  }
  if(requested_key(keys, KEY_BELONGS_TO)) {
    if(ds->belongsTo) {
      entryDict->put(KEY_BELONGS_TO, util::itos(ds->belongsTo));
    }
  }
  if(requested_key(keys, KEY_FILES)) {
    SharedHandle<List> files = List::g();
    createFileEntry(files, ds->fileEntries.begin(), ds->fileEntries.end(),
                    ds->totalLength, ds->pieceLength, ds->bitfield);
    entryDict->put(KEY_FILES, files);
  }
  if(requested_key(keys, KEY_TOTAL_LENGTH)) {
    entryDict->put(KEY_TOTAL_LENGTH, util::itos(ds->totalLength));
  }
  if(requested_key(keys, KEY_COMPLETED_LENGTH)) {
    entryDict->put(KEY_COMPLETED_LENGTH, util::itos(ds->completedLength));
  }
  if(requested_key(keys, KEY_UPLOAD_LENGTH)) {
    entryDict->put(KEY_UPLOAD_LENGTH, util::itos(ds->uploadLength));
  }
  if(requested_key(keys, KEY_BITFIELD)) {
    if(!ds->bitfield.empty()) {
      entryDict->put(KEY_BITFIELD, util::toHex(ds->bitfield));
    }
  }
  if(requested_key(keys, KEY_DOWNLOAD_SPEED)) {
    entryDict->put(KEY_DOWNLOAD_SPEED, VLB_ZERO);
  }
  if(requested_key(keys, KEY_UPLOAD_SPEED)) {
    entryDict->put(KEY_UPLOAD_SPEED, VLB_ZERO);
  }
  if(!ds->infoHash.empty()) {
    if(requested_key(keys, KEY_INFO_HASH)) {
      entryDict->put(KEY_INFO_HASH, util::toHex(ds->infoHash));
    }
    if(requested_key(keys, KEY_NUM_SEEDERS)) {
      entryDict->put(KEY_NUM_SEEDERS, VLB_ZERO);
    }
  }
  if(requested_key(keys, KEY_PIECE_LENGTH)) {
    entryDict->put(KEY_PIECE_LENGTH, util::itos(ds->pieceLength));
  }
  if(requested_key(keys, KEY_NUM_PIECES)) {
    entryDict->put(KEY_NUM_PIECES, util::uitos(ds->numPieces));
  }
  if(requested_key(keys, KEY_CONNECTIONS)) {
    entryDict->put(KEY_CONNECTIONS, VLB_ZERO);
  }
  if(requested_key(keys, KEY_DIR)) {
    entryDict->put(KEY_DIR, ds->dir);
  }
}

SharedHandle<ValueBase> GetFilesRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<List> files = List::g();
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(!group) {
    SharedHandle<DownloadResult> dr =
      e->getRequestGroupMan()->findDownloadResult(gid);
    if(!dr) {
      throw DL_ABORT_EX(fmt("No file data is available for GID#%" PRId64 "",
                            gid));
    } else {
      createFileEntry(files, dr->fileEntries.begin(), dr->fileEntries.end(),
                      dr->totalLength, dr->pieceLength, dr->bitfield);
    }
  } else {
    const SharedHandle<PieceStorage>& ps = group->getPieceStorage();
    const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
    createFileEntry(files,
                    group->getDownloadContext()->getFileEntries().begin(),
                    group->getDownloadContext()->getFileEntries().end(),
                    dctx->getTotalLength(),
                    dctx->getPieceLength(),
                    ps);
  }
  return files;
}

SharedHandle<ValueBase> GetUrisRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(!group) {
    throw DL_ABORT_EX(fmt("No URI data is available for GID#%" PRId64, gid));
  }
  SharedHandle<List> uriList = List::g();
  // TODO Current implementation just returns first FileEntry's URIs.
  if(!group->getDownloadContext()->getFileEntries().empty()) {
    createUriEntry(uriList, group->getDownloadContext()->getFirstFileEntry());
  }
  return uriList;
}

#ifdef ENABLE_BITTORRENT
SharedHandle<ValueBase> GetPeersRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(!group) {
    throw DL_ABORT_EX(fmt("No peer data is available for GID#%" PRId64, gid));
  }
  SharedHandle<List> peers = List::g();
  const SharedHandle<BtObject>& btObject =
    e->getBtRegistry()->get(group->getGID());
  if(btObject) {
    assert(btObject->peerStorage);
    gatherPeer(peers, btObject->peerStorage);
  }
  return peers;
}
#endif // ENABLE_BITTORRENT

SharedHandle<ValueBase> TellStatusRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);
  const List* keysParam = checkParam<List>(req, 1);

  a2_gid_t gid = str2Gid(gidParam);
  std::vector<std::string> keys;
  toStringList(std::back_inserter(keys), keysParam);

  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);

  SharedHandle<Dict> entryDict = Dict::g();
  if(!group) {
    SharedHandle<DownloadResult> ds =
      e->getRequestGroupMan()->findDownloadResult(gid);
    if(!ds) {
      throw DL_ABORT_EX(fmt("No such download for GID#%" PRId64 "", gid));
    }
    gatherStoppedDownload(entryDict, ds, keys);
  } else {
    if(requested_key(keys, KEY_STATUS)) {
      if(group->getState() == RequestGroup::STATE_ACTIVE) {
        entryDict->put(KEY_STATUS, VLB_ACTIVE);
      } else {
        if(group->isPauseRequested()) {
          entryDict->put(KEY_STATUS, VLB_PAUSED);
        } else {
          entryDict->put(KEY_STATUS, VLB_WAITING);
        }
      }
    }
    gatherProgress(entryDict, group, e, keys);
  }
  return entryDict;
}

SharedHandle<ValueBase> TellActiveRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const List* keysParam = checkParam<List>(req, 0);
  std::vector<std::string> keys;
  toStringList(std::back_inserter(keys), keysParam);
  SharedHandle<List> list = List::g();
  const std::deque<SharedHandle<RequestGroup> >& groups =
    e->getRequestGroupMan()->getRequestGroups();
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator i =
        groups.begin(), eoi = groups.end(); i != eoi; ++i) {
    SharedHandle<Dict> entryDict = Dict::g();
    if(requested_key(keys, KEY_STATUS)) {
      entryDict->put(KEY_STATUS, VLB_ACTIVE);
    }
    gatherProgress(entryDict, *i, e, keys);
    list->append(entryDict);
  }
  return list;
}

const std::deque<SharedHandle<RequestGroup> >&
TellWaitingRpcMethod::getItems(DownloadEngine* e) const
{
  return e->getRequestGroupMan()->getReservedGroups();
}

void TellWaitingRpcMethod::createEntry
(const SharedHandle<Dict>& entryDict,
 const SharedHandle<RequestGroup>& item,
 DownloadEngine* e,
 const std::vector<std::string>& keys) const
{
  if(requested_key(keys, KEY_STATUS)) {
    if(item->isPauseRequested()) {
      entryDict->put(KEY_STATUS, VLB_PAUSED);
    } else {
      entryDict->put(KEY_STATUS, VLB_WAITING);
    }
  }
  gatherProgress(entryDict, item, e, keys);
}

const std::deque<SharedHandle<DownloadResult> >&
TellStoppedRpcMethod::getItems(DownloadEngine* e) const
{
  return e->getRequestGroupMan()->getDownloadResults();
}

void TellStoppedRpcMethod::createEntry
(const SharedHandle<Dict>& entryDict,
 const SharedHandle<DownloadResult>& item,
 DownloadEngine* e,
 const std::vector<std::string>& keys) const
{
  gatherStoppedDownload(entryDict, item, keys);
}

SharedHandle<ValueBase> PurgeDownloadResultRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  e->getRequestGroupMan()->purgeDownloadResult();
  return VLB_OK;
}

SharedHandle<ValueBase> RemoveDownloadResultRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  if(!e->getRequestGroupMan()->removeDownloadResult(gid)) {
    throw DL_ABORT_EX(fmt("Could not remove download result of GID#%" PRId64 "", gid));
  }
  return VLB_OK;
}

namespace {
void changeOption
(const SharedHandle<RequestGroup>& group,
 const Option& option,
 DownloadEngine* e)
{
  const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
  const SharedHandle<Option>& grOption = group->getOption();
  grOption->merge(option);
  if(option.defined(PREF_CHECKSUM)) {
    const std::string& checksum = grOption->get(PREF_CHECKSUM);
    std::pair<Scip, Scip> p;
    util::divide(p, checksum.begin(), checksum.end(), '=');
    std::string hashType(p.first.first, p.first.second);
    util::lowercase(hashType);
    dctx->setDigest(hashType, util::fromHex(p.second.first, p.second.second));
  }
  if(option.defined(PREF_SELECT_FILE)) {
    SegList<int> sgl;
    util::parseIntSegments(sgl, grOption->get(PREF_SELECT_FILE));
    sgl.normalize();
    dctx->setFileFilter(sgl);
  }
  if(option.defined(PREF_SPLIT)) {
    group->setNumConcurrentCommand(grOption->getAsInt(PREF_SPLIT));
  }
  if(option.defined(PREF_MAX_CONNECTION_PER_SERVER)) {
    int maxConn = grOption->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
    const std::vector<SharedHandle<FileEntry> >& files = dctx->getFileEntries();
    for(std::vector<SharedHandle<FileEntry> >::const_iterator i = files.begin(),
          eoi = files.end(); i != eoi; ++i) {
      (*i)->setMaxConnectionPerServer(maxConn);
    }
  }
  if(option.defined(PREF_DIR) || option.defined(PREF_OUT)) {
    if(dctx->getFileEntries().size() == 1
#ifdef ENABLE_BITTORRENT
       && !dctx->hasAttribute(bittorrent::BITTORRENT)
#endif // ENABLE_BITTORRENT
       ) {
      dctx->getFirstFileEntry()->setPath
        (grOption->blank(PREF_OUT) ? A2STR::NIL :
         util::applyDir(grOption->get(PREF_DIR), grOption->get(PREF_OUT)));
    }
  }
#ifdef ENABLE_BITTORRENT
  if(option.defined(PREF_DIR) || option.defined(PREF_INDEX_OUT)) {
    if(dctx->hasAttribute(bittorrent::BITTORRENT)) {
      std::istringstream indexOutIn(grOption->get(PREF_INDEX_OUT));
      std::vector<std::pair<size_t, std::string> > indexPaths =
        util::createIndexPaths(indexOutIn);
      for(std::vector<std::pair<size_t, std::string> >::const_iterator i =
            indexPaths.begin(), eoi = indexPaths.end(); i != eoi; ++i) {
        dctx->setFilePathWithIndex
          ((*i).first,
           util::applyDir(grOption->get(PREF_DIR), (*i).second));
      }
    }
  }
#endif // ENABLE_BITTORRENT
  if(option.defined(PREF_MAX_DOWNLOAD_LIMIT)) {
    group->setMaxDownloadSpeedLimit
      (grOption->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
  }
  if(option.defined(PREF_MAX_UPLOAD_LIMIT)) {
    group->setMaxUploadSpeedLimit(grOption->getAsInt(PREF_MAX_UPLOAD_LIMIT));
  }
#ifdef ENABLE_BITTORRENT
  const SharedHandle<BtObject>& btObject =
    e->getBtRegistry()->get(group->getGID());
  if(btObject) {
    if(option.defined(PREF_BT_MAX_PEERS)) {
      btObject->btRuntime->setMaxPeers(grOption->getAsInt(PREF_BT_MAX_PEERS));
    }
  }
#endif // ENABLE_BITTORRENT
}
} // namespace

SharedHandle<ValueBase> ChangeOptionRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);
  const Dict* optsParam = checkRequiredParam<Dict>(req, 1);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  Option option;
  if(group) {
    if(group->getState() == RequestGroup::STATE_ACTIVE) {
      gatherChangeableOption(&option, optsParam);
    } else {
      gatherChangeableOptionForReserved(&option, optsParam);
    }
    changeOption(group, option, e);
  } else {
    throw DL_ABORT_EX(fmt("Cannot change option for GID#%" PRId64, gid));
  }
  return VLB_OK;
}

SharedHandle<ValueBase> ChangeGlobalOptionRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const Dict* optsParam = checkRequiredParam<Dict>(req, 0);

  Option option;
  gatherChangeableGlobalOption(&option, optsParam);
  e->getOption()->merge(option);

  if(option.defined(PREF_MAX_OVERALL_DOWNLOAD_LIMIT)) {
    e->getRequestGroupMan()->setMaxOverallDownloadSpeedLimit
      (option.getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
  }
  if(option.defined(PREF_MAX_OVERALL_UPLOAD_LIMIT)) {
    e->getRequestGroupMan()->setMaxOverallUploadSpeedLimit
      (option.getAsInt(PREF_MAX_OVERALL_UPLOAD_LIMIT));
  }
  if(option.defined(PREF_MAX_CONCURRENT_DOWNLOADS)) {
    e->getRequestGroupMan()->setMaxSimultaneousDownloads
      (option.getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS));
    e->getRequestGroupMan()->requestQueueCheck();
  }
  if(option.defined(PREF_MAX_DOWNLOAD_RESULT)) {
    e->getRequestGroupMan()->setMaxDownloadResult
      (option.getAsInt(PREF_MAX_DOWNLOAD_RESULT));
  }
  if(option.defined(PREF_LOG_LEVEL)) {
    LogFactory::setLogLevel(option.get(PREF_LOG_LEVEL));
  }
  if(option.defined(PREF_LOG)) {
    LogFactory::setLogFile(option.get(PREF_LOG));
    try {
      LogFactory::reconfigure();
    } catch(RecoverableException& e) {
      // TODO no exception handling
    }
  }

  return VLB_OK;
}

SharedHandle<ValueBase> GetVersionRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
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

namespace {
void pushRequestOption
(const SharedHandle<Dict>& dict,
 const SharedHandle<Option>& option,
 const SharedHandle<OptionParser>& oparser)
{
  for(size_t i = 1, len = option::countOption(); i < len; ++i) {
    const Pref* pref = option::i2p(i);
    const SharedHandle<OptionHandler>& h = oparser->find(pref);
    if(h && h->getInitialOption() && option->defined(pref)) {
      dict->put(pref->k, option->get(pref));
    }
  }
}
} // namespace

SharedHandle<ValueBase> GetOptionRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(!group) {
    throw DL_ABORT_EX(fmt("Cannot get option for GID#%" PRId64 "", gid));
  }
  SharedHandle<Dict> result = Dict::g();
  SharedHandle<Option> option = group->getOption();
  pushRequestOption(result, option, getOptionParser());
  return result;
}

SharedHandle<ValueBase> GetGlobalOptionRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  SharedHandle<Dict> result = Dict::g();
  for(size_t i = 0, len = e->getOption()->getTable().size(); i < len; ++i) {
    const Pref* pref = option::i2p(i);
    if(!e->getOption()->defined(pref)) {
      continue;
    }
    const SharedHandle<OptionHandler>& h = getOptionParser()->find(pref);
    if(h) {
      result->put(pref->k, e->getOption()->get(pref));
    }
  }
  return result;
}

SharedHandle<ValueBase> ChangePositionRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);
  const Integer* posParam = checkRequiredParam<Integer>(req, 1);
  const String* howParam = checkRequiredParam<String>(req, 2);

  a2_gid_t gid = str2Gid(gidParam);
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

SharedHandle<ValueBase> GetSessionInfoRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  SharedHandle<Dict> result = Dict::g();
  result->put(KEY_SESSION_ID, util::toHex(e->getSessionId()));
  return result;
}

SharedHandle<ValueBase> GetServersRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(!group || group->getState() != RequestGroup::STATE_ACTIVE) {
    throw DL_ABORT_EX(fmt("No active download for GID#%" PRId64, gid));
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
    const FileEntry::InFlightRequestSet& requests =
      (*fi)->getInFlightRequests();
    for(FileEntry::InFlightRequestSet::iterator ri =requests.begin(),
          eoi = requests.end(); ri != eoi; ++ri) {
      SharedHandle<PeerStat> ps = (*ri)->getPeerStat();
      if(ps) {
        SharedHandle<Dict> serverEntry = Dict::g();
        serverEntry->put(KEY_URI, (*ri)->getUri());
        serverEntry->put(KEY_CURRENT_URI, (*ri)->getCurrentUri());
        serverEntry->put(KEY_DOWNLOAD_SPEED,
                         util::itos(ps->calculateDownloadSpeed()));
        servers->append(serverEntry);
      }
    }
    fileEntry->put(KEY_SERVERS, servers);
    result->append(fileEntry);
  }
  return result;
}

SharedHandle<ValueBase> ChangeUriRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);
  const Integer* indexParam = checkRequiredInteger(req, 1, IntegerGE(1));
  const List* delUrisParam = checkRequiredParam<List>(req, 2);
  const List* addUrisParam = checkRequiredParam<List>(req, 3);
  const Integer* posParam = checkParam<Integer>(req, 4);

  a2_gid_t gid = str2Gid(gidParam);
  bool posGiven = checkPosParam(posParam);
  size_t pos = posGiven ? posParam->i() : 0;
  size_t index = indexParam->i()-1;
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(!group) {
    throw DL_ABORT_EX(fmt("Cannot remove URIs from GID#%" PRId64 "", gid));
  }
  const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
  const std::vector<SharedHandle<FileEntry> >& files = dctx->getFileEntries();
  if(files.size() <= index) {
    throw DL_ABORT_EX(fmt("fileIndex is out of range"));
  }
  SharedHandle<FileEntry> s = files[index];
  size_t delcount = 0;
  for(List::ValueType::const_iterator i = delUrisParam->begin(),
        eoi = delUrisParam->end(); i != eoi; ++i) {
    const String* uri = downcast<String>(*i);
    if(uri && s->removeUri(uri->s())) {
      ++delcount;
    }
  }
  size_t addcount = 0;
  if(posGiven) {
    for(List::ValueType::const_iterator i = addUrisParam->begin(),
          eoi = addUrisParam->end(); i != eoi; ++i) {
      const String* uri = downcast<String>(*i);
      if(uri && s->insertUri(uri->s(), pos)) {
        ++addcount;
        ++pos;
      }
    }
  } else {
    for(List::ValueType::const_iterator i = addUrisParam->begin(),
          eoi = addUrisParam->end(); i != eoi; ++i) {
      const String* uri = downcast<String>(*i);
      if(uri && s->addUri(uri->s())) {
        ++addcount;
      }
    }
  }
  if(addcount && group->getPieceStorage()) {
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

namespace {
SharedHandle<ValueBase> goingShutdown
(const RpcRequest& req, DownloadEngine* e, bool forceHalt)
{
  // Schedule shutdown after 3seconds to give time to client to
  // receive RPC response.
  e->addRoutineCommand(new TimedHaltCommand(e->newCUID(), e, 3, forceHalt));
  A2_LOG_INFO("Scheduled shutdown in 3 seconds.");
  return VLB_OK;
}
} // namespace

SharedHandle<ValueBase> ShutdownRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  return goingShutdown(req, e, false);
}

SharedHandle<ValueBase> ForceShutdownRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  return goingShutdown(req, e, true);
}

SharedHandle<ValueBase> GetGlobalStatRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const SharedHandle<RequestGroupMan>& rgman = e->getRequestGroupMan();
  TransferStat ts = rgman->calculateStat();
  SharedHandle<Dict> res = Dict::g();
  res->put(KEY_DOWNLOAD_SPEED, util::itos(ts.downloadSpeed));
  res->put(KEY_UPLOAD_SPEED, util::itos(ts.uploadSpeed));
  res->put(KEY_NUM_WAITING, util::uitos(rgman->getReservedGroups().size()));
  res->put(KEY_NUM_STOPPED, util::uitos(rgman->getDownloadResults().size()));
  res->put(KEY_NUM_ACTIVE, util::uitos(rgman->getRequestGroups().size()));
  return res;
}

SharedHandle<ValueBase> SystemMulticallRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  const List* methodSpecs = checkRequiredParam<List>(req, 0);
  SharedHandle<List> list = List::g();
  for(List::ValueType::const_iterator i = methodSpecs->begin(),
        eoi = methodSpecs->end(); i != eoi; ++i) {
    const Dict* methodDict = downcast<Dict>(*i);
    if(!methodDict) {
      list->append(createErrorResponse
                   (DL_ABORT_EX("system.multicall expected struct."), req));
      continue;
    }
    const String* methodName = downcast<String>(methodDict->get(KEY_METHOD_NAME));
    if(!methodName) {
      list->append(createErrorResponse
                   (DL_ABORT_EX("Missing methodName."), req));
      continue;
    }
    if(methodName->s() == getMethodName()) {
      list->append(createErrorResponse
                   (DL_ABORT_EX("Recursive system.multicall forbidden."), req));
      continue;
    }
    const SharedHandle<ValueBase>& tempParamsList = methodDict->get(KEY_PARAMS);
    SharedHandle<List> paramsList;
    if(downcast<List>(tempParamsList)) {
      paramsList = static_pointer_cast<List>(tempParamsList);
    } else {
      paramsList = List::g();
    }
    SharedHandle<RpcMethod> method = RpcMethodFactory::create(methodName->s());
    RpcRequest innerReq(methodName->s(), paramsList);
    innerReq.jsonRpc = req.jsonRpc;
    RpcResponse res = method->execute(innerReq, e);
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

SharedHandle<ValueBase> NoSuchMethodRpcMethod::process
(const RpcRequest& req, DownloadEngine* e)
{
  throw DL_ABORT_EX(fmt("No such method: %s", req.methodName.c_str()));
}

} // namespace rpc

} // namespace aria2
