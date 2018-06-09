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
#include "Metalink2RequestGroup.h"

#include <algorithm>

#include "RequestGroup.h"
#include "Option.h"
#include "LogFactory.h"
#include "Logger.h"
#include "prefs.h"
#include "util.h"
#include "message.h"
#include "DownloadContext.h"
#include "metalink_helper.h"
#include "BinaryStream.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include "MetalinkMetaurl.h"
#include "FileEntry.h"
#include "A2STR.h"
#include "a2functional.h"
#include "download_helper.h"
#include "fmt.h"
#include "SegList.h"
#include "DownloadFailureException.h"
#include "Signature.h"
#include "download_handlers.h"
#include "RequestGroupCriteria.h"
#ifdef ENABLE_BITTORRENT
#  include "BtDependency.h"
#  include "download_helper.h"
#endif // ENABLE_BITTORRENT
#include "Checksum.h"
#include "ChunkChecksum.h"

namespace aria2 {

Metalink2RequestGroup::Metalink2RequestGroup() = default;

namespace {
class AccumulateNonP2PUri {
private:
  std::vector<std::string>& urisPtr;

public:
  AccumulateNonP2PUri(std::vector<std::string>& urisPtr) : urisPtr(urisPtr) {}

  void operator()(const std::unique_ptr<MetalinkResource>& resource)
  {
    switch (resource->type) {
    case MetalinkResource::TYPE_HTTP:
    case MetalinkResource::TYPE_HTTPS:
    case MetalinkResource::TYPE_FTP:
      urisPtr.push_back(resource->url);
      break;
    default:
      break;
    }
  }
};
} // namespace

namespace {
class FindBitTorrentUri {
public:
  FindBitTorrentUri() = default;

  bool operator()(const std::shared_ptr<MetalinkResource>& resource)
  {
    if (resource->type == MetalinkResource::TYPE_BITTORRENT) {
      return true;
    }
    else {
      return false;
    }
  }
};
} // namespace

void Metalink2RequestGroup::generate(
    std::vector<std::shared_ptr<RequestGroup>>& groups,
    const std::string& metalinkFile, const std::shared_ptr<Option>& option,
    const std::string& baseUri)
{
  std::vector<std::shared_ptr<RequestGroup>> tempgroups;
  createRequestGroup(
      tempgroups, metalink::parseAndQuery(metalinkFile, option.get(), baseUri),
      option);
  std::shared_ptr<MetadataInfo> mi;
  if (metalinkFile == DEV_STDIN) {
    mi = std::make_shared<MetadataInfo>();
  }
  else {
    // TODO Downloads from local metalink file does not save neither
    // its gid nor MetadataInfo's gid.
    mi = std::make_shared<MetadataInfo>(GroupId::create(), metalinkFile);
  }
  setMetadataInfo(std::begin(tempgroups), std::end(tempgroups), mi);
  groups.insert(std::end(groups), std::begin(tempgroups), std::end(tempgroups));
}

void Metalink2RequestGroup::generate(
    std::vector<std::shared_ptr<RequestGroup>>& groups,
    const std::shared_ptr<BinaryStream>& binaryStream,
    const std::shared_ptr<Option>& option, const std::string& baseUri)
{
  std::vector<std::shared_ptr<RequestGroup>> tempgroups;
  createRequestGroup(
      tempgroups,
      metalink::parseAndQuery(binaryStream.get(), option.get(), baseUri),
      option);
  auto mi = std::make_shared<MetadataInfo>();
  setMetadataInfo(std::begin(tempgroups), std::end(tempgroups), mi);
  groups.insert(std::end(groups), std::begin(tempgroups), std::end(tempgroups));
}

void Metalink2RequestGroup::createRequestGroup(
    std::vector<std::shared_ptr<RequestGroup>>& groups,
    std::vector<std::unique_ptr<MetalinkEntry>> entries,
    const std::shared_ptr<Option>& optionTemplate)
{
  if (entries.empty()) {
    A2_LOG_NOTICE(EX_NO_RESULT_WITH_YOUR_PREFS);
    return;
  }
  std::vector<std::string> locations;
  if (optionTemplate->defined(PREF_METALINK_LOCATION)) {
    auto& loc = optionTemplate->get(PREF_METALINK_LOCATION);
    util::split(std::begin(loc), std::end(loc), std::back_inserter(locations),
                ',', true);
    for (auto& s : locations) {
      util::lowercase(s);
    }
  }
  std::string preferredProtocol;
  if (optionTemplate->get(PREF_METALINK_PREFERRED_PROTOCOL) != V_NONE) {
    preferredProtocol = optionTemplate->get(PREF_METALINK_PREFERRED_PROTOCOL);
  }
  for (auto& entry : entries) {
    entry->dropUnsupportedResource();
    if (entry->resources.empty() && entry->metaurls.empty()) {
      continue;
    }
    entry->setLocationPriority(locations,
                               -MetalinkResource::getLowestPriority());
    if (!preferredProtocol.empty()) {
      entry->setProtocolPriority(preferredProtocol,
                                 -MetalinkResource::getLowestPriority());
    }
  }
  auto sgl = util::parseIntSegments(optionTemplate->get(PREF_SELECT_FILE));
  sgl.normalize();
  if (sgl.hasNext()) {
    size_t inspoint = 0;
    for (size_t i = 0, len = entries.size(); i < len && sgl.hasNext(); ++i) {
      size_t j = sgl.peek() - 1;
      if (i == j) {
        if (inspoint != i) {
          entries[inspoint] = std::move(entries[i]);
        }
        ++inspoint;
        sgl.next();
      }
    }
    entries.resize(inspoint);
  }
  std::for_each(std::begin(entries), std::end(entries),
                std::mem_fn(&MetalinkEntry::reorderMetaurlsByPriority));
  auto entryGroups = metalink::groupEntryByMetaurlName(entries);
  for (auto& entryGroup : entryGroups) {
    auto& metaurl = entryGroup.first;
    auto& mes = entryGroup.second;
    A2_LOG_INFO(fmt("Processing metaurl group metaurl=%s", metaurl.c_str()));
#ifdef ENABLE_BITTORRENT
    std::shared_ptr<RequestGroup> torrentRg;
    if (!metaurl.empty()) {
      std::vector<std::string> uris;
      uris.push_back(metaurl);
      {
        std::vector<std::shared_ptr<RequestGroup>> result;
        createRequestGroupForUri(result, optionTemplate, uris,
                                 /* ignoreForceSequential = */ true,
                                 /* ignoreLocalPath = */ true);
        if (!result.empty()) {
          torrentRg = result[0];
        }
      }
      if (torrentRg) {
        torrentRg->setNumConcurrentCommand(1);
        torrentRg->clearPreDownloadHandler();
        torrentRg->clearPostDownloadHandler();
        // remove "metalink" from Accept Type list to avoid loop in
        // transparent metalink
        torrentRg->getDownloadContext()->setAcceptMetalink(false);
        // make it in-memory download
        torrentRg->addPreDownloadHandler(
            download_handlers::getMemoryPreDownloadHandler());
        torrentRg->markInMemoryDownload();
        groups.push_back(torrentRg);
      }
    }
#endif // ENABLE_BITTORRENT
    auto option = util::copy(optionTemplate);
    auto rg = std::make_shared<RequestGroup>(GroupId::create(), option);
    std::shared_ptr<DownloadContext> dctx;
    int numSplit = option->getAsInt(PREF_SPLIT);
    int maxConn = option->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
    if (mes.size() == 1) {
      auto entry = mes[0];
      A2_LOG_INFO(fmt(MSG_METALINK_QUEUEING, entry->getPath().c_str()));
      entry->reorderResourcesByPriority();
      for (auto& mr : entry->resources) {
        A2_LOG_DEBUG(fmt("priority=%d url=%s", mr->priority, mr->url.c_str()));
      }
      std::vector<std::string> uris;
      std::for_each(std::begin(entry->resources), std::end(entry->resources),
                    AccumulateNonP2PUri(uris));
      // If piece hash is specified in the metalink,
      // make segment size equal to piece hash size.
      int32_t pieceLength;
      if (!entry->chunkChecksum) {
        pieceLength = option->getAsInt(PREF_PIECE_LENGTH);
      }
      else {
        pieceLength = entry->chunkChecksum->getPieceLength();
      }
      dctx = std::make_shared<DownloadContext>(
          pieceLength, entry->getLength(),
          util::applyDir(option->get(PREF_DIR), entry->file->getPath()));
      dctx->getFirstFileEntry()->setUris(uris);
      dctx->getFirstFileEntry()->setMaxConnectionPerServer(maxConn);
      dctx->getFirstFileEntry()->setSuffixPath(entry->file->getPath());
      if (!entry->metaurls.empty()) {
        dctx->getFirstFileEntry()->setOriginalName(entry->metaurls[0]->name);
      }

      if (option->getAsBool(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL)) {
        dctx->getFirstFileEntry()->setUniqueProtocol(true);
      }
      if (entry->checksum) {
        dctx->setDigest(entry->checksum->getHashType(),
                        entry->checksum->getDigest());
      }
      if (entry->chunkChecksum) {
        dctx->setPieceHashes(entry->chunkChecksum->getHashType(),
                             std::begin(entry->chunkChecksum->getPieceHashes()),
                             std::end(entry->chunkChecksum->getPieceHashes()));
      }
      dctx->setSignature(entry->popSignature());
      rg->setNumConcurrentCommand(
          entry->maxConnections < 0
              ? numSplit
              : std::min(numSplit, entry->maxConnections));
    }
    else {
      dctx = std::make_shared<DownloadContext>();
      // piece length is overridden by the one in torrent file.
      dctx->setPieceLength(option->getAsInt(PREF_PIECE_LENGTH));
      std::vector<std::shared_ptr<FileEntry>> fileEntries;
      int64_t offset = 0;
      for (auto entry : mes) {
        A2_LOG_INFO(fmt("Metalink: Queueing %s for download as a member.",
                        entry->getPath().c_str()));
        A2_LOG_DEBUG(
            fmt("originalName = %s", entry->metaurls[0]->name.c_str()));
        entry->reorderResourcesByPriority();
        std::vector<std::string> uris;
        std::for_each(std::begin(entry->resources), std::end(entry->resources),
                      AccumulateNonP2PUri(uris));
        auto fe = std::make_shared<FileEntry>(
            util::applyDir(option->get(PREF_DIR), entry->file->getPath()),
            entry->file->getLength(), offset, uris);
        fe->setMaxConnectionPerServer(maxConn);
        if (option->getAsBool(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL)) {
          fe->setUniqueProtocol(true);
        }
        fe->setOriginalName(entry->metaurls[0]->name);
        fe->setSuffixPath(entry->file->getPath());
        fileEntries.push_back(fe);
        if (offset >
            std::numeric_limits<int64_t>::max() - entry->file->getLength()) {
          throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, offset));
        }
        offset += entry->file->getLength();
      }
      dctx->setFileEntries(std::begin(fileEntries), std::end(fileEntries));
      rg->setNumConcurrentCommand(numSplit);
    }
    rg->setDownloadContext(dctx);

    if (option->getAsBool(PREF_ENABLE_RPC)) {
      rg->setPauseRequested(option->getAsBool(PREF_PAUSE));
    }

    removeOneshotOption(option);
    // remove "metalink" from Accept Type list to avoid loop in
    // transparent metalink
    dctx->setAcceptMetalink(false);
#ifdef ENABLE_BITTORRENT
    // Inject dependency between rg and torrentRg here if
    // torrentRg is true
    if (torrentRg) {
      auto dep = std::make_shared<BtDependency>(rg.get(), torrentRg);
      rg->dependsOn(dep);
      torrentRg->belongsTo(rg->getGID());
      // metadata download may take very long time. If URIs are
      // available, give up metadata download in at most 30 seconds.
      const time_t btStopTimeout = 30;
      time_t currentBtStopTimeout =
          torrentRg->getOption()->getAsInt(PREF_BT_STOP_TIMEOUT);
      if (currentBtStopTimeout == 0 || currentBtStopTimeout > btStopTimeout) {
        bool allHaveUri = true;
        for (auto& fe : dctx->getFileEntries()) {
          if (fe->getRemainingUris().empty()) {
            allHaveUri = false;
            break;
          }
        }
        if (allHaveUri) {
          torrentRg->getOption()->put(PREF_BT_STOP_TIMEOUT,
                                      util::itos(btStopTimeout));
        }
      }
    }
#endif // ENABLE_BITTORRENT
    groups.push_back(rg);
  }
}

} // namespace aria2
