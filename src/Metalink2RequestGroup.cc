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
#include "TrueRequestGroupCriteria.h"
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
#ifdef ENABLE_BITTORRENT
# include "BtDependency.h"
# include "download_helper.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

Metalink2RequestGroup::Metalink2RequestGroup() {}

namespace {
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
} // namespace

namespace {
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
} // namespace

void
Metalink2RequestGroup::generate
(std::vector<SharedHandle<RequestGroup> >& groups,
 const std::string& metalinkFile,
 const SharedHandle<Option>& option,
 const std::string& baseUri)
{
  std::vector<SharedHandle<MetalinkEntry> > entries;
  metalink::parseAndQuery(entries, metalinkFile, option.get(), baseUri);
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
 const SharedHandle<Option>& option,
 const std::string& baseUri)
{
  std::vector<SharedHandle<MetalinkEntry> > entries;
  metalink::parseAndQuery(entries, binaryStream.get(), option.get(), baseUri);
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
 const SharedHandle<Option>& optionTemplate)
{
  if(entries.empty()) {
    A2_LOG_NOTICE(EX_NO_RESULT_WITH_YOUR_PREFS);
    return;
  }
  std::vector<std::string> locations;
  if(optionTemplate->defined(PREF_METALINK_LOCATION)) {
    const std::string& loc = optionTemplate->get(PREF_METALINK_LOCATION);
    util::split(loc.begin(), loc.end(),
                std::back_inserter(locations), ',', true);
    for(std::vector<std::string>::iterator i = locations.begin(),
          eoi = locations.end(); i != eoi; ++i) {
      util::lowercase(*i);
    }
  }
  std::string preferredProtocol;
  if(optionTemplate->get(PREF_METALINK_PREFERRED_PROTOCOL) != V_NONE) {
    preferredProtocol = optionTemplate->get(PREF_METALINK_PREFERRED_PROTOCOL);
  }
  for(std::vector<SharedHandle<MetalinkEntry> >::const_iterator i =
        entries.begin(), eoi = entries.end(); i != eoi; ++i) {
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
  }
  std::vector<SharedHandle<MetalinkEntry> > selectedEntries;
  SegList<int> sgl;
  util::parseIntSegments(sgl, optionTemplate->get(PREF_SELECT_FILE));
  sgl.normalize();
  if(!sgl.hasNext()) {
    selectedEntries.assign(entries.begin(), entries.end());
  } else {
    selectedEntries.reserve(entries.size());
    for(size_t i = 0, len = entries.size(); i < len && sgl.hasNext(); ++i) {
      size_t j = sgl.peek()-1;
      if(i == j) {
        selectedEntries.push_back(entries[i]);
        sgl.next();
      }
    }
  }
  std::for_each(selectedEntries.begin(), selectedEntries.end(),
                mem_fun_sh(&MetalinkEntry::reorderMetaurlsByPriority));
  std::vector<std::pair<std::string,
    std::vector<SharedHandle<MetalinkEntry> > > > entryGroups;
  metalink::groupEntryByMetaurlName(entryGroups, selectedEntries);
  for(std::vector<std::pair<std::string,
        std::vector<SharedHandle<MetalinkEntry> > > >::const_iterator itr =
        entryGroups.begin(), eoi = entryGroups.end(); itr != eoi; ++itr) {
    const std::string& metaurl = (*itr).first;
    const std::vector<SharedHandle<MetalinkEntry> >& mes = (*itr).second;
    A2_LOG_INFO(fmt("Processing metaurl group metaurl=%s", metaurl.c_str()));
#ifdef ENABLE_BITTORRENT
    SharedHandle<RequestGroup> torrentRg;
    if(!metaurl.empty()) {
      std::vector<std::string> uris;
      uris.push_back(metaurl);
      {
        std::vector<SharedHandle<RequestGroup> > result;
        createRequestGroupForUri(result, optionTemplate, uris,
                                 /* ignoreForceSequential = */true,
                                 /* ignoreLocalPath = */true);
        if(!uris.empty()) {
          torrentRg = result[0];
        }
      }
      if(torrentRg) {
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
    SharedHandle<Option> option = util::copy(optionTemplate);
    SharedHandle<RequestGroup> rg(new RequestGroup(option));
    SharedHandle<DownloadContext> dctx;
    int numSplit = option->getAsInt(PREF_SPLIT);
    int maxConn = option->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
    if(mes.size() == 1) {
      SharedHandle<MetalinkEntry> entry = mes[0];
      A2_LOG_INFO(fmt(MSG_METALINK_QUEUEING, entry->getPath().c_str()));
      entry->reorderResourcesByPriority();
      std::vector<std::string> uris;
      std::for_each(entry->resources.begin(), entry->resources.end(),
                    AccumulateNonP2PUri(uris));
      // If piece hash is specified in the metalink,
      // make segment size equal to piece hash size.
      int32_t pieceLength;
#ifdef ENABLE_MESSAGE_DIGEST
      if(!entry->chunkChecksum) {
        pieceLength = option->getAsInt(PREF_PIECE_LENGTH);
      } else {
        pieceLength = entry->chunkChecksum->getPieceLength();
      }
#else
      pieceLength = option->getAsInt(PREF_PIECE_LENGTH);
#endif // ENABLE_MESSAGE_DIGEST
      dctx.reset(new DownloadContext
                 (pieceLength,
                  entry->getLength(),
                  util::applyDir(option->get(PREF_DIR),
                                 entry->file->getPath())));
      dctx->getFirstFileEntry()->setUris(uris);
      dctx->getFirstFileEntry()->setMaxConnectionPerServer(maxConn);
      if(option->getAsBool(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL)) {
        dctx->getFirstFileEntry()->setUniqueProtocol(true);
      }
#ifdef ENABLE_MESSAGE_DIGEST
      if(entry->checksum) {
        dctx->setDigest(entry->checksum->getHashType(),
                        entry->checksum->getDigest());
      }
      if(entry->chunkChecksum) {
        dctx->setPieceHashes(entry->chunkChecksum->getHashType(),
                             entry->chunkChecksum->getPieceHashes().begin(),
                             entry->chunkChecksum->getPieceHashes().end());
      }
#endif // ENABLE_MESSAGE_DIGEST
      dctx->setSignature(entry->getSignature());
      rg->setNumConcurrentCommand
        (entry->maxConnections < 0 ?
         numSplit : std::min(numSplit, entry->maxConnections));
    } else {
      dctx.reset(new DownloadContext());
      // piece length is overridden by the one in torrent file.
      dctx->setPieceLength(option->getAsInt(PREF_PIECE_LENGTH));
      std::vector<SharedHandle<FileEntry> > fileEntries;
      int64_t offset = 0;
      for(std::vector<SharedHandle<MetalinkEntry> >::const_iterator i =
            mes.begin(), eoi = mes.end(); i != eoi; ++i) {
        A2_LOG_INFO(fmt("Metalink: Queueing %s for download as a member.",
                        (*i)->getPath().c_str()));
        A2_LOG_DEBUG(fmt("originalName = %s", (*i)->metaurls[0]->name.c_str()));
        (*i)->reorderResourcesByPriority();
        std::vector<std::string> uris;
        std::for_each((*i)->resources.begin(), (*i)->resources.end(),
                      AccumulateNonP2PUri(uris));
        SharedHandle<FileEntry> fe
          (new FileEntry
           (util::applyDir(option->get(PREF_DIR), (*i)->file->getPath()),
            (*i)->file->getLength(), offset, uris));
        fe->setMaxConnectionPerServer(maxConn);
        if(option->getAsBool(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL)) {
          fe->setUniqueProtocol(true);
        }
        fe->setOriginalName((*i)->metaurls[0]->name);
        fileEntries.push_back(fe);
        if(offset >
           std::numeric_limits<int64_t>::max() - (*i)->file->getLength()) {
          throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, offset));
        }
        offset += (*i)->file->getLength();
      }
      dctx->setFileEntries(fileEntries.begin(), fileEntries.end());
      rg->setNumConcurrentCommand(numSplit);
    }
    rg->setDownloadContext(dctx);
    rg->setPauseRequested(option->getAsBool(PREF_PAUSE));
    removeOneshotOption(option);
    // remove "metalink" from Accept Type list to avoid loop in
    // tranparent metalink
    util::removeMetalinkContentTypes(rg);
#ifdef ENABLE_BITTORRENT
    // Inject depenency between rg and torrentRg here if
    // torrentRg is true
    if(torrentRg) {
      SharedHandle<Dependency> dep(new BtDependency(rg.get(), torrentRg));
      rg->dependsOn(dep);
      torrentRg->belongsTo(rg->getGID());
      // metadata download may take very long time. If URIs are
      // available, give up metadata download in at most 30 seconds.
      const time_t btStopTimeout = 30;
      time_t currentBtStopTimeout =
        torrentRg->getOption()->getAsInt(PREF_BT_STOP_TIMEOUT);
      if(currentBtStopTimeout == 0 || currentBtStopTimeout > btStopTimeout) {
        std::vector<SharedHandle<FileEntry> >::const_iterator i;
        std::vector<SharedHandle<FileEntry> >::const_iterator eoi
          = dctx->getFileEntries().end();
        for(i = dctx->getFileEntries().begin(); i != eoi; ++i) {
          if((*i)->getRemainingUris().empty()) {
            break;
          }
        }
        if(i == dctx->getFileEntries().end()) {
          torrentRg->getOption()->put
            (PREF_BT_STOP_TIMEOUT, util::itos(btStopTimeout));
        }
      }
    }
#endif // ENABLE_BITTORRENT
    groups.push_back(rg);
  }
}

} // namespace aria2
