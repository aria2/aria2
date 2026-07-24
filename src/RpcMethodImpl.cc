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
#include "SessionSerializer.h"
#include "MessageDigest.h"
#include "message_digest_helper.h"
#include "OpenedFileCounter.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#  include "BtRegistry.h"
#  include "PeerStorage.h"
#  include "Peer.h"
#  include "BtRuntime.h"
#  include "BtAnnounce.h"
#endif // ENABLE_BITTORRENT
#include "CheckIntegrityEntry.h"

namespace aria2 {

namespace rpc {

namespace {
const char VLB_TRUE[] = "true";
const char VLB_FALSE[] = "false";
const char VLB_ACTIVE[] = "active";
const char VLB_WAITING[] = "waiting";
const char VLB_PAUSED[] = "paused";
const char VLB_REMOVED[] = "removed";
const char VLB_ERROR[] = "error";
const char VLB_COMPLETE[] = "complete";
const char VLB_USED[] = "used";
const char VLB_ZERO[] = "0";

const char KEY_GID[] = "gid";
const char KEY_ERROR_CODE[] = "errorCode";
const char KEY_ERROR_MESSAGE[] = "errorMessage";
const char KEY_STATUS[] = "status";
const char KEY_TOTAL_LENGTH[] = "totalLength";
const char KEY_COMPLETED_LENGTH[] = "completedLength";
const char KEY_DOWNLOAD_SPEED[] = "downloadSpeed";
const char KEY_UPLOAD_SPEED[] = "uploadSpeed";
const char KEY_UPLOAD_LENGTH[] = "uploadLength";
const char KEY_CONNECTIONS[] = "connections";
const char KEY_BITFIELD[] = "bitfield";
const char KEY_PIECE_LENGTH[] = "pieceLength";
const char KEY_NUM_PIECES[] = "numPieces";
const char KEY_FOLLOWED_BY[] = "followedBy";
const char KEY_FOLLOWING[] = "following";
const char KEY_BELONGS_TO[] = "belongsTo";
const char KEY_INFO_HASH[] = "infoHash";
const char KEY_NUM_SEEDERS[] = "numSeeders";
const char KEY_PEER_ID[] = "peerId";
const char KEY_IP[] = "ip";
const char KEY_PORT[] = "port";
const char KEY_AM_CHOKING[] = "amChoking";
const char KEY_PEER_CHOKING[] = "peerChoking";
const char KEY_SEEDER[] = "seeder";
const char KEY_INDEX[] = "index";
const char KEY_PATH[] = "path";
const char KEY_SELECTED[] = "selected";
const char KEY_LENGTH[] = "length";
const char KEY_URI[] = "uri";
const char KEY_CURRENT_URI[] = "currentUri";
const char KEY_VERSION[] = "version";
const char KEY_ENABLED_FEATURES[] = "enabledFeatures";
const char KEY_METHOD_NAME[] = "methodName";
const char KEY_PARAMS[] = "params";
const char KEY_SESSION_ID[] = "sessionId";
const char KEY_FILES[] = "files";
const char KEY_DIR[] = "dir";
const char KEY_URIS[] = "uris";
const char KEY_BITTORRENT[] = "bittorrent";
const char KEY_INFO[] = "info";
const char KEY_NAME[] = "name";
const char KEY_ANNOUNCE_LIST[] = "announceList";
const char KEY_COMMENT[] = "comment";
const char KEY_CREATION_DATE[] = "creationDate";
const char KEY_MODE[] = "mode";
const char KEY_SERVERS[] = "servers";
const char KEY_NUM_WAITING[] = "numWaiting";
const char KEY_NUM_STOPPED[] = "numStopped";
const char KEY_NUM_ACTIVE[] = "numActive";
const char KEY_NUM_STOPPED_TOTAL[] = "numStoppedTotal";
const char KEY_VERIFIED_LENGTH[] = "verifiedLength";
const char KEY_VERIFY_PENDING[] = "verifyIntegrityPending";
} // namespace

namespace {
std::unique_ptr<ValueBase> createGIDResponse(a2_gid_t gid)
{
  return String::g(GroupId::toHex(gid));
}
} // namespace

namespace {
std::unique_ptr<ValueBase> createOKResponse() { return String::g("OK"); }
} // namespace

namespace {
std::unique_ptr<ValueBase>
addRequestGroup(const std::shared_ptr<RequestGroup>& group, DownloadEngine* e,
                bool posGiven, int pos)
{
  if (posGiven) {
    e->getRequestGroupMan()->insertReservedGroup(pos, group);
  }
  else {
    e->getRequestGroupMan()->addReservedGroup(group);
  }
  return createGIDResponse(group->getGID());
}
} // namespace

namespace {
bool checkPosParam(const Integer* posParam)
{
  if (posParam) {
    if (posParam->i() >= 0) {
      return true;
    }
    else {
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
  if (str->s().size() > sizeof(a2_gid_t) * 2) {
    throw DL_ABORT_EX(fmt("Invalid GID %s", str->s().c_str()));
  }
  a2_gid_t n;
  switch (GroupId::expandUnique(n, str->s().c_str())) {
  case GroupId::ERR_NOT_UNIQUE:
    throw DL_ABORT_EX(fmt("GID %s is not unique", str->s().c_str()));
  case GroupId::ERR_NOT_FOUND:
    throw DL_ABORT_EX(fmt("GID %s is not found", str->s().c_str()));
  case GroupId::ERR_INVALID:
    throw DL_ABORT_EX(fmt("Invalid GID %s", str->s().c_str()));
  }
  return n;
}
} // namespace

namespace {
template <typename OutputIterator>
void extractUris(OutputIterator out, const List* src)
{
  if (src) {
    for (auto& elem : *src) {
      const String* uri = downcast<String>(elem);
      if (uri) {
        out++ = uri->s();
      }
    }
  }
}
} // namespace

std::unique_ptr<ValueBase> AddUriRpcMethod::process(const RpcRequest& req,
                                                    DownloadEngine* e)
{
  const List* urisParam = checkRequiredParam<List>(req, 0);
  const Dict* optsParam = checkParam<Dict>(req, 1);
  const Integer* posParam = checkParam<Integer>(req, 2);

  std::vector<std::string> uris;
  extractUris(std::back_inserter(uris), urisParam);
  if (uris.empty()) {
    throw DL_ABORT_EX("URI is not provided.");
  }

  auto requestOption = std::make_shared<Option>(*e->getOption());
  gatherRequestOption(requestOption.get(), optsParam);

  bool posGiven = checkPosParam(posParam);
  size_t pos = posGiven ? posParam->i() : 0;

  std::vector<std::shared_ptr<RequestGroup>> result;
  createRequestGroupForUri(result, requestOption, uris,
                           /* ignoreForceSeq = */ true,
                           /* ignoreLocalPath = */ true);

  if (!result.empty()) {
    return addRequestGroup(result.front(), e, posGiven, pos);
  }
  else {
    throw DL_ABORT_EX("No URI to download.");
  }
}

namespace {
std::string getHexSha1(const std::string& s)
{
  unsigned char hash[20];
  message_digest::digest(hash, sizeof(hash), MessageDigest::sha1().get(),
                         s.data(), s.size());
  return util::toHex(hash, sizeof(hash));
}
} // namespace

#ifdef ENABLE_BITTORRENT
std::unique_ptr<ValueBase> AddTorrentRpcMethod::process(const RpcRequest& req,
                                                        DownloadEngine* e)
{
  const String* torrentParam = checkRequiredParam<String>(req, 0);
  const List* urisParam = checkParam<List>(req, 1);
  const Dict* optsParam = checkParam<Dict>(req, 2);
  const Integer* posParam = checkParam<Integer>(req, 3);

  std::unique_ptr<String> tempTorrentParam;
  if (req.jsonRpc) {
    tempTorrentParam = String::g(
        base64::decode(torrentParam->s().begin(), torrentParam->s().end()));
    torrentParam = tempTorrentParam.get();
  }
  std::vector<std::string> uris;
  extractUris(std::back_inserter(uris), urisParam);

  auto requestOption = std::make_shared<Option>(*e->getOption());
  gatherRequestOption(requestOption.get(), optsParam);

  bool posGiven = checkPosParam(posParam);
  size_t pos = posGiven ? posParam->i() : 0;

  std::string filename;
  if (requestOption->getAsBool(PREF_RPC_SAVE_UPLOAD_METADATA)) {
    filename = util::applyDir(requestOption->get(PREF_DIR),
                              getHexSha1(torrentParam->s()) + ".torrent");
    // Save uploaded data in order to save this download in
    // --save-session file.
    if (util::saveAs(filename, torrentParam->s(), true)) {
      A2_LOG_INFO(
          fmt("Uploaded torrent data was saved as %s", filename.c_str()));
      requestOption->put(PREF_TORRENT_FILE, filename);
    }
    else {
      A2_LOG_INFO(fmt("Uploaded torrent data was not saved."
                      " Failed to write file %s",
                      filename.c_str()));
      filename.clear();
    }
  }
  std::vector<std::shared_ptr<RequestGroup>> result;
  createRequestGroupForBitTorrent(result, requestOption, uris, filename,
                                  torrentParam->s());

  if (!result.empty()) {
    return addRequestGroup(result.front(), e, posGiven, pos);
  }
  else {
    throw DL_ABORT_EX("No Torrent to download.");
  }
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
std::unique_ptr<ValueBase> AddMetalinkRpcMethod::process(const RpcRequest& req,
                                                         DownloadEngine* e)
{
  const String* metalinkParam = checkRequiredParam<String>(req, 0);
  const Dict* optsParam = checkParam<Dict>(req, 1);
  const Integer* posParam = checkParam<Integer>(req, 2);

  std::unique_ptr<String> tempMetalinkParam;
  if (req.jsonRpc) {
    tempMetalinkParam = String::g(
        base64::decode(metalinkParam->s().begin(), metalinkParam->s().end()));
    metalinkParam = tempMetalinkParam.get();
  }
  auto requestOption = std::make_shared<Option>(*e->getOption());
  gatherRequestOption(requestOption.get(), optsParam);

  bool posGiven = checkPosParam(posParam);
  size_t pos = posGiven ? posParam->i() : 0;

  std::vector<std::shared_ptr<RequestGroup>> result;
  std::string filename;
  if (requestOption->getAsBool(PREF_RPC_SAVE_UPLOAD_METADATA)) {
    // TODO RFC5854 Metalink has the extension .meta4 and Metalink
    // Version 3 uses .metalink extension. We use .meta4 for both
    // RFC5854 Metalink and Version 3. aria2 can detect which of which
    // by reading content rather than extension.
    filename = util::applyDir(requestOption->get(PREF_DIR),
                              getHexSha1(metalinkParam->s()) + ".meta4");
    // Save uploaded data in order to save this download in
    // --save-session file.
    if (util::saveAs(filename, metalinkParam->s(), true)) {
      A2_LOG_INFO(
          fmt("Uploaded metalink data was saved as %s", filename.c_str()));
      requestOption->put(PREF_METALINK_FILE, filename);
      createRequestGroupForMetalink(result, requestOption);
    }
    else {
      A2_LOG_INFO(fmt("Uploaded metalink data was not saved."
                      " Failed to write file %s",
                      filename.c_str()));
      createRequestGroupForMetalink(result, requestOption, metalinkParam->s());
    }
  }
  else {
    createRequestGroupForMetalink(result, requestOption, metalinkParam->s());
  }
  auto gids = List::g();
  if (!result.empty()) {
    if (posGiven) {
      e->getRequestGroupMan()->insertReservedGroup(pos, result);
    }
    else {
      e->getRequestGroupMan()->addReservedGroup(result);
    }
    for (auto& i : result) {
      gids->append(GroupId::toHex(i->getGID()));
    }
  }
  return std::move(gids);
}
#endif // ENABLE_METALINK

namespace {
std::unique_ptr<ValueBase> removeDownload(const RpcRequest& req,
                                          DownloadEngine* e, bool forceRemove)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (group) {
    if (group->getState() == RequestGroup::STATE_ACTIVE) {
      if (forceRemove) {
        group->setForceHaltRequested(true, RequestGroup::USER_REQUEST);
      }
      else {
        group->setHaltRequested(true, RequestGroup::USER_REQUEST);
      }
      e->setRefreshInterval(std::chrono::milliseconds(0));
    }
    else {
      if (group->isDependencyResolved()) {
        e->getRequestGroupMan()->removeReservedGroup(gid);
      }
      else {
        throw DL_ABORT_EX(
            fmt("GID#%s cannot be removed now", GroupId::toHex(gid).c_str()));
      }
    }
  }
  else {
    throw DL_ABORT_EX(fmt("Active Download not found for GID#%s",
                          GroupId::toHex(gid).c_str()));
  }
  return createGIDResponse(gid);
}
} // namespace

std::unique_ptr<ValueBase> RemoveRpcMethod::process(const RpcRequest& req,
                                                    DownloadEngine* e)
{
  return removeDownload(req, e, false);
}

std::unique_ptr<ValueBase> ForceRemoveRpcMethod::process(const RpcRequest& req,
                                                         DownloadEngine* e)
{
  return removeDownload(req, e, true);
}

namespace {
std::unique_ptr<ValueBase> pauseDownload(const RpcRequest& req,
                                         DownloadEngine* e, bool forcePause)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (group) {
    bool reserved = group->getState() == RequestGroup::STATE_WAITING;
    if (pauseRequestGroup(group, reserved, forcePause)) {
      e->setRefreshInterval(std::chrono::milliseconds(0));
      return createGIDResponse(gid);
    }
  }
  throw DL_ABORT_EX(
      fmt("GID#%s cannot be paused now", GroupId::toHex(gid).c_str()));
}
} // namespace

std::unique_ptr<ValueBase> PauseRpcMethod::process(const RpcRequest& req,
                                                   DownloadEngine* e)
{
  return pauseDownload(req, e, false);
}

std::unique_ptr<ValueBase> ForcePauseRpcMethod::process(const RpcRequest& req,
                                                        DownloadEngine* e)
{
  return pauseDownload(req, e, true);
}

namespace {
template <typename InputIterator>
void pauseRequestGroups(InputIterator first, InputIterator last, bool reserved,
                        bool forcePause)
{
  for (; first != last; ++first) {
    pauseRequestGroup(*first, reserved, forcePause);
  }
}
} // namespace

namespace {
std::unique_ptr<ValueBase> pauseAllDownloads(const RpcRequest& req,
                                             DownloadEngine* e, bool forcePause)
{
  auto& groups = e->getRequestGroupMan()->getRequestGroups();
  pauseRequestGroups(groups.begin(), groups.end(), false, forcePause);
  auto& reservedGroups = e->getRequestGroupMan()->getReservedGroups();
  pauseRequestGroups(reservedGroups.begin(), reservedGroups.end(), true,
                     forcePause);
  return createOKResponse();
}
} // namespace

std::unique_ptr<ValueBase> PauseAllRpcMethod::process(const RpcRequest& req,
                                                      DownloadEngine* e)
{
  return pauseAllDownloads(req, e, false);
}

std::unique_ptr<ValueBase>
ForcePauseAllRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  return pauseAllDownloads(req, e, true);
}

std::unique_ptr<ValueBase> UnpauseRpcMethod::process(const RpcRequest& req,
                                                     DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (!group || group->getState() != RequestGroup::STATE_WAITING ||
      !group->isPauseRequested()) {
    throw DL_ABORT_EX(
        fmt("GID#%s cannot be unpaused now", GroupId::toHex(gid).c_str()));
  }
  else {
    group->setPauseRequested(false);
    e->getRequestGroupMan()->requestQueueCheck();
  }
  return createGIDResponse(gid);
}

std::unique_ptr<ValueBase> UnpauseAllRpcMethod::process(const RpcRequest& req,
                                                        DownloadEngine* e)
{
  auto& groups = e->getRequestGroupMan()->getReservedGroups();
  for (auto& group : groups) {
    group->setPauseRequested(false);
  }
  e->getRequestGroupMan()->requestQueueCheck();
  return createOKResponse();
}

namespace {
template <typename InputIterator>
void createUriEntry(List* uriList, InputIterator first, InputIterator last,
                    const std::string& status)
{
  for (; first != last; ++first) {
    auto entry = Dict::g();
    entry->put(KEY_URI, *first);
    entry->put(KEY_STATUS, status);
    uriList->append(std::move(entry));
  }
}
} // namespace

namespace {
void createUriEntry(List* uriList, const std::shared_ptr<FileEntry>& file)
{
  createUriEntry(uriList, std::begin(file->getSpentUris()),
                 std::end(file->getSpentUris()), VLB_USED);
  createUriEntry(uriList, std::begin(file->getRemainingUris()),
                 std::end(file->getRemainingUris()), VLB_WAITING);
}
} // namespace

namespace {
template <typename InputIterator>
void createFileEntry(List* files, InputIterator first, InputIterator last,
                     const BitfieldMan* bf)
{
  size_t index = 1;
  for (; first != last; ++first, ++index) {
    auto entry = Dict::g();
    entry->put(KEY_INDEX, util::uitos(index));
    entry->put(KEY_PATH, (*first)->getPath());
    entry->put(KEY_SELECTED, (*first)->isRequested() ? VLB_TRUE : VLB_FALSE);
    entry->put(KEY_LENGTH, util::itos((*first)->getLength()));
    int64_t completedLength = bf->getOffsetCompletedLength(
        (*first)->getOffset(), (*first)->getLength());
    entry->put(KEY_COMPLETED_LENGTH, util::itos(completedLength));

    auto uriList = List::g();
    createUriEntry(uriList.get(), *first);
    entry->put(KEY_URIS, std::move(uriList));
    files->append(std::move(entry));
  }
}
} // namespace

namespace {
template <typename InputIterator>
void createFileEntry(List* files, InputIterator first, InputIterator last,
                     int64_t totalLength, int32_t pieceLength,
                     const std::string& bitfield)
{
  BitfieldMan bf(pieceLength, totalLength);
  bf.setBitfield(reinterpret_cast<const unsigned char*>(bitfield.data()),
                 bitfield.size());
  createFileEntry(files, first, last, &bf);
}
} // namespace

namespace {
template <typename InputIterator>
void createFileEntry(List* files, InputIterator first, InputIterator last,
                     int64_t totalLength, int32_t pieceLength,
                     const std::shared_ptr<PieceStorage>& ps)
{
  BitfieldMan bf(pieceLength, totalLength);
  if (ps) {
    bf.setBitfield(ps->getBitfield(), ps->getBitfieldLength());
  }
  createFileEntry(files, first, last, &bf);
}
} // namespace

namespace {
bool requested_key(const std::vector<std::string>& keys, const std::string& k)
{
  return keys.empty() || std::find(keys.begin(), keys.end(), k) != keys.end();
}
} // namespace

void gatherProgressCommon(Dict* entryDict,
                          const std::shared_ptr<RequestGroup>& group,
                          const std::vector<std::string>& keys)
{
  auto& ps = group->getPieceStorage();
  if (requested_key(keys, KEY_GID)) {
    entryDict->put(KEY_GID, GroupId::toHex(group->getGID()).c_str());
  }
  if (requested_key(keys, KEY_TOTAL_LENGTH)) {
    // This is "filtered" total length if --select-file is used.
    entryDict->put(KEY_TOTAL_LENGTH, util::itos(group->getTotalLength()));
  }
  if (requested_key(keys, KEY_COMPLETED_LENGTH)) {
    // This is "filtered" total length if --select-file is used.
    entryDict->put(KEY_COMPLETED_LENGTH,
                   util::itos(group->getCompletedLength()));
  }
  TransferStat stat = group->calculateStat();
  if (requested_key(keys, KEY_DOWNLOAD_SPEED)) {
    entryDict->put(KEY_DOWNLOAD_SPEED, util::itos(stat.downloadSpeed));
  }
  if (requested_key(keys, KEY_UPLOAD_SPEED)) {
    entryDict->put(KEY_UPLOAD_SPEED, util::itos(stat.uploadSpeed));
  }
  if (requested_key(keys, KEY_UPLOAD_LENGTH)) {
    entryDict->put(KEY_UPLOAD_LENGTH, util::itos(stat.allTimeUploadLength));
  }
  if (requested_key(keys, KEY_CONNECTIONS)) {
    entryDict->put(KEY_CONNECTIONS, util::itos(group->getNumConnection()));
  }
  if (requested_key(keys, KEY_BITFIELD)) {
    if (ps) {
      if (ps->getBitfieldLength() > 0) {
        entryDict->put(KEY_BITFIELD,
                       util::toHex(ps->getBitfield(), ps->getBitfieldLength()));
      }
    }
  }
  auto& dctx = group->getDownloadContext();
  if (requested_key(keys, KEY_PIECE_LENGTH)) {
    entryDict->put(KEY_PIECE_LENGTH, util::itos(dctx->getPieceLength()));
  }
  if (requested_key(keys, KEY_NUM_PIECES)) {
    entryDict->put(KEY_NUM_PIECES, util::uitos(dctx->getNumPieces()));
  }
  if (requested_key(keys, KEY_FOLLOWED_BY)) {
    if (!group->followedBy().empty()) {
      auto list = List::g();
      // The element is GID.
      for (auto& gid : group->followedBy()) {
        list->append(GroupId::toHex(gid));
      }
      entryDict->put(KEY_FOLLOWED_BY, std::move(list));
    }
  }
  if (requested_key(keys, KEY_FOLLOWING)) {
    if (group->following()) {
      entryDict->put(KEY_FOLLOWING, GroupId::toHex(group->following()));
    }
  }
  if (requested_key(keys, KEY_BELONGS_TO)) {
    if (group->belongsTo()) {
      entryDict->put(KEY_BELONGS_TO, GroupId::toHex(group->belongsTo()));
    }
  }
  if (requested_key(keys, KEY_FILES)) {
    auto files = List::g();
    createFileEntry(files.get(), std::begin(dctx->getFileEntries()),
                    std::end(dctx->getFileEntries()), dctx->getTotalLength(),
                    dctx->getPieceLength(), ps);
    entryDict->put(KEY_FILES, std::move(files));
  }
  if (requested_key(keys, KEY_DIR)) {
    entryDict->put(KEY_DIR, group->getOption()->get(PREF_DIR));
  }
}

#ifdef ENABLE_BITTORRENT
void gatherBitTorrentMetadata(Dict* btDict, TorrentAttribute* torrentAttrs)
{
  if (!torrentAttrs->comment.empty()) {
    btDict->put(KEY_COMMENT, torrentAttrs->comment);
  }
  if (torrentAttrs->creationDate) {
    btDict->put(KEY_CREATION_DATE, Integer::g(torrentAttrs->creationDate));
  }
  if (torrentAttrs->mode) {
    btDict->put(KEY_MODE, bittorrent::getModeString(torrentAttrs->mode));
  }
  auto destAnnounceList = List::g();
  for (auto& annlist : torrentAttrs->announceList) {
    auto destAnnounceTier = List::g();
    for (auto& ann : annlist) {
      destAnnounceTier->append(ann);
    }
    destAnnounceList->append(std::move(destAnnounceTier));
  }
  btDict->put(KEY_ANNOUNCE_LIST, std::move(destAnnounceList));
  if (!torrentAttrs->metadata.empty()) {
    auto infoDict = Dict::g();
    infoDict->put(KEY_NAME, torrentAttrs->name);
    btDict->put(KEY_INFO, std::move(infoDict));
  }
}

namespace {
void gatherProgressBitTorrent(Dict* entryDict,
                              const std::shared_ptr<RequestGroup>& group,
                              TorrentAttribute* torrentAttrs,
                              BtObject* btObject,
                              const std::vector<std::string>& keys)
{
  if (requested_key(keys, KEY_INFO_HASH)) {
    entryDict->put(KEY_INFO_HASH, util::toHex(torrentAttrs->infoHash));
  }
  if (requested_key(keys, KEY_BITTORRENT)) {
    auto btDict = Dict::g();
    gatherBitTorrentMetadata(btDict.get(), torrentAttrs);
    entryDict->put(KEY_BITTORRENT, std::move(btDict));
  }
  if (requested_key(keys, KEY_NUM_SEEDERS)) {
    if (!btObject) {
      entryDict->put(KEY_NUM_SEEDERS, VLB_ZERO);
    }
    else {
      auto& peerStorage = btObject->peerStorage;
      assert(peerStorage);
      auto& peers = peerStorage->getUsedPeers();
      entryDict->put(KEY_NUM_SEEDERS,
                     util::uitos(countSeeder(peers.begin(), peers.end())));
    }
  }
  if (requested_key(keys, KEY_SEEDER)) {
    entryDict->put(KEY_SEEDER, group->isSeeder() ? VLB_TRUE : VLB_FALSE);
  }
}
} // namespace

namespace {
void gatherPeer(List* peers, const std::shared_ptr<PeerStorage>& ps)
{
  auto& usedPeers = ps->getUsedPeers();
  for (auto& peer : usedPeers) {
    if (!peer->isActive()) {
      continue;
    }
    auto peerEntry = Dict::g();
    peerEntry->put(KEY_PEER_ID, util::torrentPercentEncode(peer->getPeerId(),
                                                           PEER_ID_LENGTH));
    peerEntry->put(KEY_IP, peer->getIPAddress());
    if (peer->isIncomingPeer()) {
      peerEntry->put(KEY_PORT, VLB_ZERO);
    }
    else {
      peerEntry->put(KEY_PORT, util::uitos(peer->getPort()));
    }
    peerEntry->put(KEY_BITFIELD,
                   util::toHex(peer->getBitfield(), peer->getBitfieldLength()));
    peerEntry->put(KEY_AM_CHOKING, peer->amChoking() ? VLB_TRUE : VLB_FALSE);
    peerEntry->put(KEY_PEER_CHOKING,
                   peer->peerChoking() ? VLB_TRUE : VLB_FALSE);
    peerEntry->put(KEY_DOWNLOAD_SPEED,
                   util::itos(peer->calculateDownloadSpeed()));
    peerEntry->put(KEY_UPLOAD_SPEED, util::itos(peer->calculateUploadSpeed()));
    peerEntry->put(KEY_SEEDER, peer->isSeeder() ? VLB_TRUE : VLB_FALSE);
    peers->append(std::move(peerEntry));
  }
}
} // namespace
#endif // ENABLE_BITTORRENT

namespace {
void gatherProgress(Dict* entryDict, const std::shared_ptr<RequestGroup>& group,
                    DownloadEngine* e, const std::vector<std::string>& keys)
{
  gatherProgressCommon(entryDict, group, keys);
#ifdef ENABLE_BITTORRENT
  if (group->getDownloadContext()->hasAttribute(CTX_ATTR_BT)) {
    gatherProgressBitTorrent(
        entryDict, group,
        bittorrent::getTorrentAttrs(group->getDownloadContext()),
        e->getBtRegistry()->get(group->getGID()), keys);
  }
#endif // ENABLE_BITTORRENT
  if (e->getCheckIntegrityMan()) {
    if (e->getCheckIntegrityMan()->isPicked(
            [&group](const CheckIntegrityEntry& ent) {
              return ent.getRequestGroup() == group.get();
            })) {
      entryDict->put(
          KEY_VERIFIED_LENGTH,
          util::itos(
              e->getCheckIntegrityMan()->getPickedEntry()->getCurrentLength()));
    }
    if (e->getCheckIntegrityMan()->isQueued(
            [&group](const CheckIntegrityEntry& ent) {
              return ent.getRequestGroup() == group.get();
            })) {
      entryDict->put(KEY_VERIFY_PENDING, VLB_TRUE);
    }
  }
}
} // namespace

void gatherStoppedDownload(Dict* entryDict,
                           const std::shared_ptr<DownloadResult>& ds,
                           const std::vector<std::string>& keys)
{
  if (requested_key(keys, KEY_GID)) {
    entryDict->put(KEY_GID, ds->gid->toHex());
  }
  if (requested_key(keys, KEY_ERROR_CODE)) {
    entryDict->put(KEY_ERROR_CODE, util::itos(static_cast<int>(ds->result)));
  }
  if (requested_key(keys, KEY_ERROR_MESSAGE)) {
    entryDict->put(KEY_ERROR_MESSAGE, ds->resultMessage);
  }
  if (requested_key(keys, KEY_STATUS)) {
    if (ds->result == error_code::REMOVED) {
      entryDict->put(KEY_STATUS, VLB_REMOVED);
    }
    else if (ds->result == error_code::FINISHED) {
      entryDict->put(KEY_STATUS, VLB_COMPLETE);
    }
    else {
      entryDict->put(KEY_STATUS, VLB_ERROR);
    }
  }
  if (requested_key(keys, KEY_FOLLOWED_BY)) {
    if (!ds->followedBy.empty()) {
      auto list = List::g();
      // The element is GID.
      for (auto gid : ds->followedBy) {
        list->append(GroupId::toHex(gid));
      }
      entryDict->put(KEY_FOLLOWED_BY, std::move(list));
    }
  }
  if (requested_key(keys, KEY_FOLLOWING)) {
    if (ds->following) {
      entryDict->put(KEY_FOLLOWING, GroupId::toHex(ds->following));
    }
  }
  if (requested_key(keys, KEY_BELONGS_TO)) {
    if (ds->belongsTo) {
      entryDict->put(KEY_BELONGS_TO, GroupId::toHex(ds->belongsTo));
    }
  }
  if (requested_key(keys, KEY_FILES)) {
    auto files = List::g();
    createFileEntry(files.get(), std::begin(ds->fileEntries),
                    std::end(ds->fileEntries), ds->totalLength, ds->pieceLength,
                    ds->bitfield);
    entryDict->put(KEY_FILES, std::move(files));
  }
  if (requested_key(keys, KEY_TOTAL_LENGTH)) {
    entryDict->put(KEY_TOTAL_LENGTH, util::itos(ds->totalLength));
  }
  if (requested_key(keys, KEY_COMPLETED_LENGTH)) {
    entryDict->put(KEY_COMPLETED_LENGTH, util::itos(ds->completedLength));
  }
  if (requested_key(keys, KEY_UPLOAD_LENGTH)) {
    entryDict->put(KEY_UPLOAD_LENGTH, util::itos(ds->uploadLength));
  }
  if (requested_key(keys, KEY_BITFIELD)) {
    if (!ds->bitfield.empty()) {
      entryDict->put(KEY_BITFIELD, util::toHex(ds->bitfield));
    }
  }
  if (requested_key(keys, KEY_DOWNLOAD_SPEED)) {
    entryDict->put(KEY_DOWNLOAD_SPEED, VLB_ZERO);
  }
  if (requested_key(keys, KEY_UPLOAD_SPEED)) {
    entryDict->put(KEY_UPLOAD_SPEED, VLB_ZERO);
  }
  if (!ds->infoHash.empty()) {
    if (requested_key(keys, KEY_INFO_HASH)) {
      entryDict->put(KEY_INFO_HASH, util::toHex(ds->infoHash));
    }
    if (requested_key(keys, KEY_NUM_SEEDERS)) {
      entryDict->put(KEY_NUM_SEEDERS, VLB_ZERO);
    }
  }
  if (requested_key(keys, KEY_PIECE_LENGTH)) {
    entryDict->put(KEY_PIECE_LENGTH, util::itos(ds->pieceLength));
  }
  if (requested_key(keys, KEY_NUM_PIECES)) {
    entryDict->put(KEY_NUM_PIECES, util::uitos(ds->numPieces));
  }
  if (requested_key(keys, KEY_CONNECTIONS)) {
    entryDict->put(KEY_CONNECTIONS, VLB_ZERO);
  }
  if (requested_key(keys, KEY_DIR)) {
    entryDict->put(KEY_DIR, ds->dir);
  }

#ifdef ENABLE_BITTORRENT
  if (ds->attrs.size() > CTX_ATTR_BT && ds->attrs[CTX_ATTR_BT]) {
    const auto attrs =
        static_cast<TorrentAttribute*>(ds->attrs[CTX_ATTR_BT].get());
    if (requested_key(keys, KEY_BITTORRENT)) {
      auto btDict = Dict::g();
      gatherBitTorrentMetadata(btDict.get(), attrs);
      entryDict->put(KEY_BITTORRENT, std::move(btDict));
    }
  }
#endif // ENABLE_BITTORRENT
}

std::unique_ptr<ValueBase> GetFilesRpcMethod::process(const RpcRequest& req,
                                                      DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  auto files = List::g();
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (!group) {
    auto dr = e->getRequestGroupMan()->findDownloadResult(gid);
    if (!dr) {
      throw DL_ABORT_EX(fmt("No file data is available for GID#%s",
                            GroupId::toHex(gid).c_str()));
    }
    else {
      createFileEntry(files.get(), std::begin(dr->fileEntries),
                      std::end(dr->fileEntries), dr->totalLength,
                      dr->pieceLength, dr->bitfield);
    }
  }
  else {
    auto& dctx = group->getDownloadContext();
    createFileEntry(files.get(),
                    std::begin(group->getDownloadContext()->getFileEntries()),
                    std::end(group->getDownloadContext()->getFileEntries()),
                    dctx->getTotalLength(), dctx->getPieceLength(),
                    group->getPieceStorage());
  }
  return std::move(files);
}

std::unique_ptr<ValueBase> GetUrisRpcMethod::process(const RpcRequest& req,
                                                     DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (!group) {
    throw DL_ABORT_EX(fmt("No URI data is available for GID#%s",
                          GroupId::toHex(gid).c_str()));
  }
  auto uriList = List::g();
  // TODO Current implementation just returns first FileEntry's URIs.
  if (!group->getDownloadContext()->getFileEntries().empty()) {
    createUriEntry(uriList.get(),
                   group->getDownloadContext()->getFirstFileEntry());
  }
  return std::move(uriList);
}

#ifdef ENABLE_BITTORRENT
std::unique_ptr<ValueBase> GetPeersRpcMethod::process(const RpcRequest& req,
                                                      DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (!group) {
    throw DL_ABORT_EX(fmt("No peer data is available for GID#%s",
                          GroupId::toHex(gid).c_str()));
  }
  auto peers = List::g();
  auto btObject = e->getBtRegistry()->get(group->getGID());
  if (btObject) {
    assert(btObject->peerStorage);
    gatherPeer(peers.get(), btObject->peerStorage);
  }
  return std::move(peers);
}
#endif // ENABLE_BITTORRENT

std::unique_ptr<ValueBase> TellStatusRpcMethod::process(const RpcRequest& req,
                                                        DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);
  const List* keysParam = checkParam<List>(req, 1);

  a2_gid_t gid = str2Gid(gidParam);
  std::vector<std::string> keys;
  toStringList(std::back_inserter(keys), keysParam);

  auto group = e->getRequestGroupMan()->findGroup(gid);
  auto entryDict = Dict::g();
  if (!group) {
    auto ds = e->getRequestGroupMan()->findDownloadResult(gid);
    if (!ds) {
      throw DL_ABORT_EX(
          fmt("No such download for GID#%s", GroupId::toHex(gid).c_str()));
    }
    gatherStoppedDownload(entryDict.get(), ds, keys);
  }
  else {
    if (requested_key(keys, KEY_STATUS)) {
      if (group->getState() == RequestGroup::STATE_ACTIVE) {
        entryDict->put(KEY_STATUS, VLB_ACTIVE);
      }
      else {
        if (group->isPauseRequested()) {
          entryDict->put(KEY_STATUS, VLB_PAUSED);
        }
        else {
          entryDict->put(KEY_STATUS, VLB_WAITING);
        }
      }
    }
    gatherProgress(entryDict.get(), group, e, keys);
  }
  return std::move(entryDict);
}

std::unique_ptr<ValueBase> TellActiveRpcMethod::process(const RpcRequest& req,
                                                        DownloadEngine* e)
{
  const List* keysParam = checkParam<List>(req, 0);
  std::vector<std::string> keys;
  toStringList(std::back_inserter(keys), keysParam);
  auto list = List::g();
  bool statusReq = requested_key(keys, KEY_STATUS);
  for (auto& group : e->getRequestGroupMan()->getRequestGroups()) {
    auto entryDict = Dict::g();
    if (statusReq) {
      entryDict->put(KEY_STATUS, VLB_ACTIVE);
    }
    gatherProgress(entryDict.get(), group, e, keys);
    list->append(std::move(entryDict));
  }
  return std::move(list);
}

const RequestGroupList& TellWaitingRpcMethod::getItems(DownloadEngine* e) const
{
  return e->getRequestGroupMan()->getReservedGroups();
}

void TellWaitingRpcMethod::createEntry(
    Dict* entryDict, const std::shared_ptr<RequestGroup>& item,
    DownloadEngine* e, const std::vector<std::string>& keys) const
{
  if (requested_key(keys, KEY_STATUS)) {
    if (item->isPauseRequested()) {
      entryDict->put(KEY_STATUS, VLB_PAUSED);
    }
    else {
      entryDict->put(KEY_STATUS, VLB_WAITING);
    }
  }
  gatherProgress(entryDict, item, e, keys);
}

const DownloadResultList&
TellStoppedRpcMethod::getItems(DownloadEngine* e) const
{
  return e->getRequestGroupMan()->getDownloadResults();
}

void TellStoppedRpcMethod::createEntry(
    Dict* entryDict, const std::shared_ptr<DownloadResult>& item,
    DownloadEngine* e, const std::vector<std::string>& keys) const
{
  gatherStoppedDownload(entryDict, item, keys);
}

std::unique_ptr<ValueBase>
PurgeDownloadResultRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  e->getRequestGroupMan()->purgeDownloadResult();
  return createOKResponse();
}

std::unique_ptr<ValueBase>
RemoveDownloadResultRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  if (!e->getRequestGroupMan()->removeDownloadResult(gid)) {
    throw DL_ABORT_EX(fmt("Could not remove download result of GID#%s",
                          GroupId::toHex(gid).c_str()));
  }
  return createOKResponse();
}

std::unique_ptr<ValueBase> ChangeOptionRpcMethod::process(const RpcRequest& req,
                                                          DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);
  const Dict* optsParam = checkRequiredParam<Dict>(req, 1);

  a2_gid_t gid = str2Gid(gidParam);
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (group) {
    Option option;
    std::shared_ptr<Option> pendingOption;
    if (group->getState() == RequestGroup::STATE_ACTIVE) {
      pendingOption = std::make_shared<Option>();
      gatherChangeableOption(&option, pendingOption.get(), optsParam);
      if (!pendingOption->emptyLocal()) {
        group->setPendingOption(pendingOption);
        // pauseRequestGroup() may fail if group has been told to
        // stop/pause already.  In that case, we can still apply the
        // pending options on pause.
        if (pauseRequestGroup(group, false, false)) {
          group->setRestartRequested(true);
          e->setRefreshInterval(std::chrono::milliseconds(0));
        }
      }
    }
    else {
      gatherChangeableOptionForReserved(&option, optsParam);
    }
    changeOption(group, option, e);
  }
  else {
    throw DL_ABORT_EX(
        fmt("Cannot change option for GID#%s", GroupId::toHex(gid).c_str()));
  }
  return createOKResponse();
}

std::unique_ptr<ValueBase>
ChangeGlobalOptionRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  const Dict* optsParam = checkRequiredParam<Dict>(req, 0);

  Option option;
  gatherChangeableGlobalOption(&option, optsParam);
  changeGlobalOption(option, e);
  return createOKResponse();
}

std::unique_ptr<ValueBase> GetVersionRpcMethod::process(const RpcRequest& req,
                                                        DownloadEngine* e)
{
  auto result = Dict::g();
  result->put(KEY_VERSION, PACKAGE_VERSION);
  auto featureList = List::g();
  for (int feat = 0; feat < MAX_FEATURE; ++feat) {
    const char* name = strSupportedFeature(feat);
    if (name) {
      featureList->append(name);
    }
  }
  result->put(KEY_ENABLED_FEATURES, std::move(featureList));
  return std::move(result);
}

namespace {
void pushRequestOption(Dict* dict, const std::shared_ptr<Option>& option,
                       const std::shared_ptr<OptionParser>& oparser)
{
  for (size_t i = 1, len = option::countOption(); i < len; ++i) {
    PrefPtr pref = option::i2p(i);
    const OptionHandler* h = oparser->find(pref);
    if (h && h->getInitialOption() && option->defined(pref)) {
      dict->put(pref->k, option->get(pref));
    }
  }
}
} // namespace

std::unique_ptr<ValueBase> GetOptionRpcMethod::process(const RpcRequest& req,
                                                       DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  auto group = e->getRequestGroupMan()->findGroup(gid);
  auto result = Dict::g();
  if (!group) {
    auto dr = e->getRequestGroupMan()->findDownloadResult(gid);
    if (!dr) {
      throw DL_ABORT_EX(
          fmt("Cannot get option for GID#%s", GroupId::toHex(gid).c_str()));
    }
    pushRequestOption(result.get(), dr->option, getOptionParser());
  }
  else {
    pushRequestOption(result.get(), group->getOption(), getOptionParser());
  }
  return std::move(result);
}

std::unique_ptr<ValueBase>
GetGlobalOptionRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  auto result = Dict::g();
  for (size_t i = 0, len = e->getOption()->getTable().size(); i < len; ++i) {
    PrefPtr pref = option::i2p(i);
    if (pref == PREF_RPC_SECRET || !e->getOption()->defined(pref)) {
      continue;
    }
    const OptionHandler* h = getOptionParser()->find(pref);
    if (h) {
      result->put(pref->k, e->getOption()->get(pref));
    }
  }
  return std::move(result);
}

std::unique_ptr<ValueBase>
ChangePositionRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);
  const Integer* posParam = checkRequiredParam<Integer>(req, 1);
  const String* howParam = checkRequiredParam<String>(req, 2);

  a2_gid_t gid = str2Gid(gidParam);
  int pos = posParam->i();
  const std::string& howStr = howParam->s();
  OffsetMode how;
  if (howStr == "POS_SET") {
    how = OFFSET_MODE_SET;
  }
  else if (howStr == "POS_CUR") {
    how = OFFSET_MODE_CUR;
  }
  else if (howStr == "POS_END") {
    how = OFFSET_MODE_END;
  }
  else {
    throw DL_ABORT_EX("Illegal argument.");
  }
  size_t destPos =
      e->getRequestGroupMan()->changeReservedGroupPosition(gid, pos, how);
  return Integer::g(destPos);
}

std::unique_ptr<ValueBase>
GetSessionInfoRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  auto result = Dict::g();
  result->put(KEY_SESSION_ID, util::toHex(e->getSessionId()));
  return std::move(result);
}

std::unique_ptr<ValueBase> GetServersRpcMethod::process(const RpcRequest& req,
                                                        DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);

  a2_gid_t gid = str2Gid(gidParam);
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (!group || group->getState() != RequestGroup::STATE_ACTIVE) {
    throw DL_ABORT_EX(
        fmt("No active download for GID#%s", GroupId::toHex(gid).c_str()));
  }
  auto result = List::g();
  size_t index = 1;
  for (auto& fe : group->getDownloadContext()->getFileEntries()) {
    auto fileEntry = Dict::g();
    fileEntry->put(KEY_INDEX, util::uitos(index++));
    auto servers = List::g();
    for (auto& req : fe->getInFlightRequests()) {
      auto ps = req->getPeerStat();
      if (ps) {
        auto serverEntry = Dict::g();
        serverEntry->put(KEY_URI, req->getUri());
        serverEntry->put(KEY_CURRENT_URI, req->getCurrentUri());
        serverEntry->put(KEY_DOWNLOAD_SPEED,
                         util::itos(ps->calculateDownloadSpeed()));
        servers->append(std::move(serverEntry));
      }
    }
    fileEntry->put(KEY_SERVERS, std::move(servers));
    result->append(std::move(fileEntry));
  }
  return std::move(result);
}

std::unique_ptr<ValueBase> ChangeUriRpcMethod::process(const RpcRequest& req,
                                                       DownloadEngine* e)
{
  const String* gidParam = checkRequiredParam<String>(req, 0);
  const Integer* indexParam = checkRequiredInteger(req, 1, IntegerGE(1));
  const List* delUrisParam = checkRequiredParam<List>(req, 2);
  const List* addUrisParam = checkRequiredParam<List>(req, 3);
  const Integer* posParam = checkParam<Integer>(req, 4);

  a2_gid_t gid = str2Gid(gidParam);
  bool posGiven = checkPosParam(posParam);
  size_t pos = posGiven ? posParam->i() : 0;
  size_t index = indexParam->i() - 1;
  auto group = e->getRequestGroupMan()->findGroup(gid);
  if (!group) {
    throw DL_ABORT_EX(
        fmt("Cannot remove URIs from GID#%s", GroupId::toHex(gid).c_str()));
  }
  auto& files = group->getDownloadContext()->getFileEntries();
  if (files.size() <= index) {
    throw DL_ABORT_EX(fmt("fileIndex is out of range"));
  }
  auto& s = files[index];
  size_t delcount = 0;
  for (auto& elem : *delUrisParam) {
    const String* uri = downcast<String>(elem);
    if (uri && s->removeUri(uri->s())) {
      ++delcount;
    }
  }
  size_t addcount = 0;
  if (posGiven) {
    for (auto& elem : *addUrisParam) {
      const String* uri = downcast<String>(elem);
      if (uri && s->insertUri(uri->s(), pos)) {
        ++addcount;
        ++pos;
      }
    }
  }
  else {
    for (auto& elem : *addUrisParam) {
      const String* uri = downcast<String>(elem);
      if (uri && s->addUri(uri->s())) {
        ++addcount;
      }
    }
  }
  if (addcount && group->getPieceStorage()) {
    std::vector<std::unique_ptr<Command>> commands;
    group->createNextCommand(commands, e);
    e->addCommand(std::move(commands));
    group->getSegmentMan()->recognizeSegmentFor(s);
  }
  auto res = List::g();
  res->append(Integer::g(delcount));
  res->append(Integer::g(addcount));
  return std::move(res);
}

namespace {
std::unique_ptr<ValueBase> goingShutdown(const RpcRequest& req,
                                         DownloadEngine* e, bool forceHalt)
{
  // Schedule shutdown after 3seconds to give time to client to
  // receive RPC response.
  e->addRoutineCommand(
      make_unique<TimedHaltCommand>(e->newCUID(), e, 3_s, forceHalt));
  A2_LOG_INFO("Scheduled shutdown in 3 seconds.");
  return createOKResponse();
}
} // namespace

std::unique_ptr<ValueBase> ShutdownRpcMethod::process(const RpcRequest& req,
                                                      DownloadEngine* e)
{
  return goingShutdown(req, e, false);
}

std::unique_ptr<ValueBase>
ForceShutdownRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  return goingShutdown(req, e, true);
}

std::unique_ptr<ValueBase>
GetGlobalStatRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  auto& rgman = e->getRequestGroupMan();
  auto ts = rgman->calculateStat();
  auto res = Dict::g();
  res->put(KEY_DOWNLOAD_SPEED, util::itos(ts.downloadSpeed));
  res->put(KEY_UPLOAD_SPEED, util::itos(ts.uploadSpeed));
  res->put(KEY_NUM_WAITING, util::uitos(rgman->getReservedGroups().size()));
  res->put(KEY_NUM_STOPPED, util::uitos(rgman->getDownloadResults().size()));
  res->put(KEY_NUM_STOPPED_TOTAL, util::uitos(rgman->getNumStoppedTotal()));
  res->put(KEY_NUM_ACTIVE, util::uitos(rgman->getRequestGroups().size()));
  return std::move(res);
}

std::unique_ptr<ValueBase> SaveSessionRpcMethod::process(const RpcRequest& req,
                                                         DownloadEngine* e)
{
  const std::string& filename = e->getOption()->get(PREF_SAVE_SESSION);
  if (filename.empty()) {
    throw DL_ABORT_EX("Filename is not given.");
  }
  SessionSerializer sessionSerializer(e->getRequestGroupMan().get());
  if (sessionSerializer.save(filename)) {
    A2_LOG_NOTICE(
        fmt(_("Serialized session to '%s' successfully."), filename.c_str()));
    return createOKResponse();
  }
  throw DL_ABORT_EX(
      fmt("Failed to serialize session to '%s'.", filename.c_str()));
}

std::unique_ptr<ValueBase>
SystemMulticallRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  // Should never get here, since SystemMulticallRpcMethod overrides execute().
  assert(false);
  return nullptr;
}

RpcResponse SystemMulticallRpcMethod::execute(RpcRequest req, DownloadEngine* e)
{
  auto authorized = RpcResponse::AUTHORIZED;
  try {
    const List* methodSpecs = checkRequiredParam<List>(req, 0);
    auto list = List::g();
    for (auto& methodSpec : *methodSpecs) {
      Dict* methodDict = downcast<Dict>(methodSpec);
      if (!methodDict) {
        list->append(createErrorResponse(
            DL_ABORT_EX("system.multicall expected struct."), req));
        continue;
      }
      const String* methodName =
          downcast<String>(methodDict->get(KEY_METHOD_NAME));
      if (!methodName) {
        list->append(
            createErrorResponse(DL_ABORT_EX("Missing methodName."), req));
        continue;
      }
      if (methodName->s() == getMethodName()) {
        list->append(createErrorResponse(
            DL_ABORT_EX("Recursive system.multicall forbidden."), req));
        continue;
      }
      // TODO what if params missing?
      auto tempParamsList = methodDict->get(KEY_PARAMS);
      std::unique_ptr<List> paramsList;
      if (downcast<List>(tempParamsList)) {
        paramsList.reset(
            static_cast<List*>(methodDict->popValue(KEY_PARAMS).release()));
      }
      else {
        paramsList = List::g();
      }
      RpcRequest r = {methodName->s(), std::move(paramsList), nullptr,
                      req.jsonRpc};
      RpcResponse res = getMethod(methodName->s())->execute(std::move(r), e);
      if (rpc::not_authorized(res)) {
        authorized = RpcResponse::NOTAUTHORIZED;
      }
      if (res.code == 0) {
        auto l = List::g();
        l->append(std::move(res.param));
        list->append(std::move(l));
      }
      else {
        list->append(std::move(res.param));
      }
    }
    return RpcResponse(0, authorized, std::move(list), std::move(req.id));
  }
  catch (RecoverableException& ex) {
    A2_LOG_DEBUG_EX(EX_EXCEPTION_CAUGHT, ex);
    return RpcResponse(1, authorized, createErrorResponse(ex, req),
                       std::move(req.id));
  }
}

std::unique_ptr<ValueBase>
SystemListMethodsRpcMethod::process(const RpcRequest& req, DownloadEngine* e)
{
  auto list = List::g();
  for (auto& s : allMethodNames()) {
    list->append(s);
  }

  return std::move(list);
}

RpcResponse SystemListMethodsRpcMethod::execute(RpcRequest req,
                                                DownloadEngine* e)
{
  auto r = process(req, e);
  return RpcResponse(0, RpcResponse::AUTHORIZED, std::move(r),
                     std::move(req.id));
}

std::unique_ptr<ValueBase>
SystemListNotificationsRpcMethod::process(const RpcRequest& req,
                                          DownloadEngine* e)
{
  auto list = List::g();
  for (auto& s : allNotificationsNames()) {
    list->append(s);
  }

  return std::move(list);
}

RpcResponse SystemListNotificationsRpcMethod::execute(RpcRequest req,
                                                      DownloadEngine* e)
{
  auto r = process(req, e);
  return RpcResponse(0, RpcResponse::AUTHORIZED, std::move(r),
                     std::move(req.id));
}

std::unique_ptr<ValueBase> NoSuchMethodRpcMethod::process(const RpcRequest& req,
                                                          DownloadEngine* e)
{
  throw DL_ABORT_EX(fmt("No such method: %s", req.methodName.c_str()));
}

} // namespace rpc

bool pauseRequestGroup(const std::shared_ptr<RequestGroup>& group,
                       bool reserved, bool forcePause)
{
  if ((reserved && !group->isPauseRequested()) ||
      (!reserved && !group->isForceHaltRequested() &&
       ((forcePause && group->isHaltRequested() && group->isPauseRequested()) ||
        (!group->isHaltRequested() && !group->isPauseRequested())))) {
    if (!reserved) {
      // Call setHaltRequested before setPauseRequested because
      // setHaltRequested calls setPauseRequested(false) internally.
      if (forcePause) {
        group->setForceHaltRequested(true, RequestGroup::NONE);
      }
      else {
        group->setHaltRequested(true, RequestGroup::NONE);
      }
    }
    group->setPauseRequested(true);
    return true;
  }
  else {
    return false;
  }
}

void changeOption(const std::shared_ptr<RequestGroup>& group,
                  const Option& option, DownloadEngine* e)
{
  const std::shared_ptr<DownloadContext>& dctx = group->getDownloadContext();
  const std::shared_ptr<Option>& grOption = group->getOption();
  grOption->merge(option);
  if (option.defined(PREF_CHECKSUM)) {
    const std::string& checksum = grOption->get(PREF_CHECKSUM);
    auto p = util::divide(std::begin(checksum), std::end(checksum), '=');
    std::string hashType(p.first.first, p.first.second);
    util::lowercase(hashType);
    dctx->setDigest(hashType, util::fromHex(p.second.first, p.second.second));
  }
  if (option.defined(PREF_SELECT_FILE)) {
    auto sgl = util::parseIntSegments(grOption->get(PREF_SELECT_FILE));
    sgl.normalize();
    dctx->setFileFilter(std::move(sgl));
  }
  if (option.defined(PREF_SPLIT)) {
    group->setNumConcurrentCommand(grOption->getAsInt(PREF_SPLIT));
  }
  if (option.defined(PREF_MAX_CONNECTION_PER_SERVER)) {
    int maxConn = grOption->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
    const std::vector<std::shared_ptr<FileEntry>>& files =
        dctx->getFileEntries();
    for (auto& file : files) {
      (file)->setMaxConnectionPerServer(maxConn);
    }
  }
  if (option.defined(PREF_DIR) || option.defined(PREF_OUT)) {
    if (!group->getMetadataInfo()) {

      assert(dctx->getFileEntries().size() == 1);

      auto& fileEntry = dctx->getFirstFileEntry();

      if (!grOption->blank(PREF_OUT)) {
        fileEntry->setPath(
            util::applyDir(grOption->get(PREF_DIR), grOption->get(PREF_OUT)));
        fileEntry->setSuffixPath(A2STR::NIL);
      }
      else if (fileEntry->getSuffixPath().empty()) {
        fileEntry->setPath(A2STR::NIL);
      }
      else {
        fileEntry->setPath(util::applyDir(grOption->get(PREF_DIR),
                                          fileEntry->getSuffixPath()));
      }
    }
    else if (group->getMetadataInfo()
#ifdef ENABLE_BITTORRENT
             && !dctx->hasAttribute(CTX_ATTR_BT)
#endif // ENABLE_BITTORRENT
    ) {
      // In case of Metalink
      for (auto& fileEntry : dctx->getFileEntries()) {
        // PREF_OUT is not applicable to Metalink.  We have always
        // suffixPath set.
        fileEntry->setPath(util::applyDir(grOption->get(PREF_DIR),
                                          fileEntry->getSuffixPath()));
      }
    }
  }
#ifdef ENABLE_BITTORRENT
  if (option.defined(PREF_DIR) || option.defined(PREF_INDEX_OUT)) {
    if (dctx->hasAttribute(CTX_ATTR_BT)) {
      std::istringstream indexOutIn(grOption->get(PREF_INDEX_OUT));
      std::vector<std::pair<size_t, std::string>> indexPaths =
          util::createIndexPaths(indexOutIn);
      for (std::vector<std::pair<size_t, std::string>>::const_iterator
               i = indexPaths.begin(),
               eoi = indexPaths.end();
           i != eoi; ++i) {
        dctx->setFilePathWithIndex(
            (*i).first, util::applyDir(grOption->get(PREF_DIR), (*i).second));
      }
    }
  }
#endif // ENABLE_BITTORRENT
  if (option.defined(PREF_MAX_DOWNLOAD_LIMIT)) {
    group->setMaxDownloadSpeedLimit(
        grOption->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
  }
  if (option.defined(PREF_MAX_UPLOAD_LIMIT)) {
    group->setMaxUploadSpeedLimit(grOption->getAsInt(PREF_MAX_UPLOAD_LIMIT));
  }
#ifdef ENABLE_BITTORRENT
  auto btObject = e->getBtRegistry()->get(group->getGID());
  if (btObject) {
    if (option.defined(PREF_BT_MAX_PEERS)) {
      btObject->btRuntime->setMaxPeers(grOption->getAsInt(PREF_BT_MAX_PEERS));
    }
  }
#endif // ENABLE_BITTORRENT
}

void changeGlobalOption(const Option& option, DownloadEngine* e)
{
  e->getOption()->merge(option);
  if (option.defined(PREF_MAX_OVERALL_DOWNLOAD_LIMIT)) {
    e->getRequestGroupMan()->setMaxOverallDownloadSpeedLimit(
        option.getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
  }
  if (option.defined(PREF_MAX_OVERALL_UPLOAD_LIMIT)) {
    e->getRequestGroupMan()->setMaxOverallUploadSpeedLimit(
        option.getAsInt(PREF_MAX_OVERALL_UPLOAD_LIMIT));
  }
  if (option.defined(PREF_MAX_CONCURRENT_DOWNLOADS)) {
    e->getRequestGroupMan()->setMaxConcurrentDownloads(
        option.getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS));
    e->getRequestGroupMan()->requestQueueCheck();
  }
  if (option.defined(PREF_OPTIMIZE_CONCURRENT_DOWNLOADS)) {
    e->getRequestGroupMan()->setupOptimizeConcurrentDownloads();
    e->getRequestGroupMan()->requestQueueCheck();
  }
  if (option.defined(PREF_MAX_DOWNLOAD_RESULT)) {
    e->getRequestGroupMan()->setMaxDownloadResult(
        option.getAsInt(PREF_MAX_DOWNLOAD_RESULT));
  }
  if (option.defined(PREF_LOG_LEVEL)) {
    LogFactory::setLogLevel(option.get(PREF_LOG_LEVEL));
  }
  if (option.defined(PREF_LOG)) {
    LogFactory::setLogFile(option.get(PREF_LOG));
    try {
      LogFactory::reconfigure();
    }
    catch (RecoverableException& e) {
      // TODO no exception handling
    }
  }
  if (option.defined(PREF_BT_MAX_OPEN_FILES)) {
    auto& openedFileCounter = e->getRequestGroupMan()->getOpenedFileCounter();
    openedFileCounter->setMaxOpenFiles(option.getAsInt(PREF_BT_MAX_OPEN_FILES));
  }
}

} // namespace aria2
