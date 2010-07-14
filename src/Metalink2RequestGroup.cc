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
#include "MetalinkHelper.h"
#include "BinaryStream.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "TrueRequestGroupCriteria.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include "MetalinkMetaurl.h"
#include "FileEntry.h"
#include "A2STR.h"
#include "a2functional.h"
#include "download_helper.h"
#ifdef ENABLE_BITTORRENT
# include "BtDependency.h"
# include "download_helper.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

Metalink2RequestGroup::Metalink2RequestGroup():
  logger_(LogFactory::getInstance()) {}

class AccumulateNonP2PUri {
private:
  std::vector<std::string>& urisPtr;
public:
  AccumulateNonP2PUri(std::vector<std::string>& urisPtr)
    :urisPtr(urisPtr) {}

  void operator()(const SharedHandle<MetalinkResource>& resource) {
    switch(resource->type) {
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

class FindBitTorrentUri {
public:
  FindBitTorrentUri() {}

  bool operator()(const SharedHandle<MetalinkResource>& resource) {
    if(resource->type == MetalinkResource::TYPE_BITTORRENT) {
      return true;
    } else {
      return false;
    }
  }
};

void
Metalink2RequestGroup::generate
(std::vector<SharedHandle<RequestGroup> >& groups,
 const std::string& metalinkFile,
 const SharedHandle<Option>& option)
{
  std::vector<SharedHandle<MetalinkEntry> > entries;
  MetalinkHelper::parseAndQuery(entries, metalinkFile, option.get());
  std::vector<SharedHandle<RequestGroup> > tempgroups;
  createRequestGroup(tempgroups, entries, option);
  SharedHandle<MetadataInfo> mi;
  if(metalinkFile == DEV_STDIN) {
    mi.reset(new MetadataInfo());
  } else {
    mi.reset(new MetadataInfo(metalinkFile));
  }
  setMetadataInfo(tempgroups.begin(), tempgroups.end(), mi);
  groups.insert(groups.end(), tempgroups.begin(), tempgroups.end());
}

void
Metalink2RequestGroup::generate
(std::vector<SharedHandle<RequestGroup> >& groups,
 const SharedHandle<BinaryStream>& binaryStream,
 const SharedHandle<Option>& option)
{
  std::vector<SharedHandle<MetalinkEntry> > entries;
  MetalinkHelper::parseAndQuery(entries, binaryStream, option.get());
  std::vector<SharedHandle<RequestGroup> > tempgroups;
  createRequestGroup(tempgroups, entries, option);
  SharedHandle<MetadataInfo> mi(new MetadataInfo());
  setMetadataInfo(tempgroups.begin(), tempgroups.end(), mi);
  groups.insert(groups.end(), tempgroups.begin(), tempgroups.end());
}

void
Metalink2RequestGroup::createRequestGroup
(std::vector<SharedHandle<RequestGroup> >& groups,
 const std::vector<SharedHandle<MetalinkEntry> >& entries,
 const SharedHandle<Option>& option)
{
  if(entries.empty()) {
    logger_->notice(EX_NO_RESULT_WITH_YOUR_PREFS);
    return;
  }
  std::vector<int32_t> selectIndexes =
    util::parseIntRange(option->get(PREF_SELECT_FILE)).flush();
  std::sort(selectIndexes.begin(), selectIndexes.end());
  std::vector<std::string> locations;
  if(option->defined(PREF_METALINK_LOCATION)) {
    util::split(option->get(PREF_METALINK_LOCATION),
                std::back_inserter(locations), ",", true);
    std::transform
      (locations.begin(), locations.end(), locations.begin(), util::toLower);
  }
  std::string preferredProtocol;
  if(option->get(PREF_METALINK_PREFERRED_PROTOCOL) != V_NONE) {
    preferredProtocol = option->get(PREF_METALINK_PREFERRED_PROTOCOL);
  }
  std::vector<SharedHandle<MetalinkEntry> > selectedEntries;
  selectedEntries.reserve(entries.size());
  {
    int32_t count = 1;
    for(std::vector<SharedHandle<MetalinkEntry> >::const_iterator i =
          entries.begin(), eoi = entries.end(); i != eoi; ++i, ++count) {
      (*i)->dropUnsupportedResource();
      if((*i)->resources.empty() && (*i)->metaurls.empty()) {
        continue;
      }
      (*i)->setLocationPriority
        (locations, -MetalinkResource::getLowestPriority());
      if(!preferredProtocol.empty()) {
        (*i)->setProtocolPriority
          (preferredProtocol, -MetalinkResource::getLowestPriority());
      }
      if(selectIndexes.empty() ||
         std::binary_search(selectIndexes.begin(), selectIndexes.end(), count)){
        selectedEntries.push_back(*i);
      }
    }
  }
  std::for_each(selectedEntries.begin(), selectedEntries.end(),
                mem_fun_sh(&MetalinkEntry::reorderMetaurlsByPriority));
  std::vector<std::pair<std::string,
    std::vector<SharedHandle<MetalinkEntry> > > > entryGroups;
  MetalinkHelper::groupEntryByMetaurlName(entryGroups, selectedEntries);
  for(std::vector<std::pair<std::string,
        std::vector<SharedHandle<MetalinkEntry> > > >::const_iterator itr =
        entryGroups.begin(), eoi = entryGroups.end(); itr != eoi; ++itr) {
    const std::string& metaurl = (*itr).first;
    const std::vector<SharedHandle<MetalinkEntry> >& mes = (*itr).second;
    logger_->info("Processing metaurl group metaurl=%s", metaurl.c_str());
#ifdef ENABLE_BITTORRENT
    SharedHandle<RequestGroup> torrentRg;
    if(!metaurl.empty()) {
      std::vector<std::string> uris;
      uris.push_back(metaurl);
      {
        std::vector<SharedHandle<RequestGroup> > result;
        createRequestGroupForUri(result, option, uris,
                                 /* ignoreForceSequential = */true,
                                 /* ignoreLocalPath = */true);
        if(!uris.empty()) {
          torrentRg = result[0];
        }
      }
      if(!torrentRg.isNull()) {
        torrentRg->setNumConcurrentCommand(1);
        torrentRg->clearPreDownloadHandler();
        torrentRg->clearPostDownloadHandler();
        // remove "metalink" from Accept Type list to avoid loop in
        // tranparent metalink
        util::removeMetalinkContentTypes(torrentRg);
        // make it in-memory download
        SharedHandle<PreDownloadHandler> preh
          (new MemoryBufferPreDownloadHandler());
        SharedHandle<RequestGroupCriteria> cri(new TrueRequestGroupCriteria());
        preh->setCriteria(cri);
        torrentRg->addPreDownloadHandler(preh);
        groups.push_back(torrentRg);
      }
    }
#endif // ENABLE_BITTORRENT
    SharedHandle<RequestGroup> rg(new RequestGroup(option));
    SharedHandle<DownloadContext> dctx;
    if(mes.size() == 1) {
      SharedHandle<MetalinkEntry> entry = mes[0];
      logger_->info(MSG_METALINK_QUEUEING, entry->getPath().c_str());
      entry->reorderResourcesByPriority();
      std::vector<std::string> uris;
      std::for_each(entry->resources.begin(), entry->resources.end(),
                    AccumulateNonP2PUri(uris));
      // If piece hash is specified in the metalink,
      // make segment size equal to piece hash size.
      size_t pieceLength;
#ifdef ENABLE_MESSAGE_DIGEST
      if(entry->chunkChecksum.isNull()) {
        pieceLength = option->getAsInt(PREF_SEGMENT_SIZE);
      } else {
        pieceLength = entry->chunkChecksum->getChecksumLength();
      }
#else
      pieceLength = option->getAsInt(PREF_SEGMENT_SIZE);
#endif // ENABLE_MESSAGE_DIGEST
      dctx.reset(new DownloadContext
                 (pieceLength,
                  entry->getLength(),
                  util::applyDir(option->get(PREF_DIR),
                                 entry->file->getPath())));
      dctx->getFirstFileEntry()->setUris(uris);
      dctx->getFirstFileEntry()->setMaxConnectionPerServer(1);
      if(option->getAsBool(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL)) {
        dctx->getFirstFileEntry()->setUniqueProtocol(true);
      }
#ifdef ENABLE_MESSAGE_DIGEST
      if(!entry->checksum.isNull()) {
        dctx->setChecksum(entry->checksum->getMessageDigest());
        dctx->setChecksumHashAlgo(entry->checksum->getAlgo());
      }
      if(!entry->chunkChecksum.isNull()) {
        dctx->setPieceHashes(entry->chunkChecksum->getChecksums().begin(),
                             entry->chunkChecksum->getChecksums().end());
        dctx->setPieceHashAlgo(entry->chunkChecksum->getAlgo());
      }
#endif // ENABLE_MESSAGE_DIGEST
      dctx->setSignature(entry->getSignature());
      rg->setNumConcurrentCommand
        (entry->maxConnections < 0 ?
         option->getAsInt(PREF_METALINK_SERVERS) :
         std::min(option->getAsInt(PREF_METALINK_SERVERS),
                  static_cast<int32_t>(entry->maxConnections)));
    } else {
      dctx.reset(new DownloadContext());
      // piece length is overridden by the one in torrent file.
      dctx->setPieceLength(option->getAsInt(PREF_SEGMENT_SIZE));
      std::vector<SharedHandle<FileEntry> > fileEntries;
      off_t offset = 0;
      for(std::vector<SharedHandle<MetalinkEntry> >::const_iterator i =
            mes.begin(), eoi = mes.end(); i != eoi; ++i) {
        logger_->info("Metalink: Queueing %s for download as a member.",
                      (*i)->getPath().c_str());
        logger_->debug("originalName = %s", (*i)->metaurls[0]->name.c_str());
        (*i)->reorderResourcesByPriority();
        std::vector<std::string> uris;
        std::for_each((*i)->resources.begin(), (*i)->resources.end(),
                      AccumulateNonP2PUri(uris));
        SharedHandle<FileEntry> fe
          (new FileEntry
           (util::applyDir(option->get(PREF_DIR), (*i)->file->getPath()),
            (*i)->file->getLength(), offset, uris));
        fe->setMaxConnectionPerServer(1);
        if(option->getAsBool(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL)) {
          fe->setUniqueProtocol(true);
        }
        fe->setOriginalName((*i)->metaurls[0]->name);
        fileEntries.push_back(fe);
        offset += (*i)->file->getLength();
      }
      dctx->setFileEntries(fileEntries.begin(), fileEntries.end());
      rg->setNumConcurrentCommand(option->getAsInt(PREF_METALINK_SERVERS));
    }
    dctx->setDir(option->get(PREF_DIR));
    rg->setDownloadContext(dctx);
    // remove "metalink" from Accept Type list to avoid loop in
    // tranparent metalink
    util::removeMetalinkContentTypes(rg);
#ifdef ENABLE_BITTORRENT
    // Inject depenency between rg and torrentRg here if
    // torrentRg.isNull() == false
    if(!torrentRg.isNull()) {
      SharedHandle<Dependency> dep(new BtDependency(rg, torrentRg));
      rg->dependsOn(dep);
      torrentRg->belongsTo(rg->getGID());
    }
#endif // ENABLE_BITTORRENT
    groups.push_back(rg);
  }
}

} // namespace aria2
