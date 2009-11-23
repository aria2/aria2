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
#include "FileEntry.h"
#include "A2STR.h"
#include "a2functional.h"
#ifdef ENABLE_BITTORRENT
# include "BtDependency.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

Metalink2RequestGroup::Metalink2RequestGroup():
  _logger(LogFactory::getInstance()) {}

class AccumulateNonP2PUrl {
private:
  std::deque<std::string>& urlsPtr;
public:
  AccumulateNonP2PUrl(std::deque<std::string>& urlsPtr)
    :urlsPtr(urlsPtr) {}

  void operator()(const SharedHandle<MetalinkResource>& resource) {
    switch(resource->type) {
    case MetalinkResource::TYPE_HTTP:
    case MetalinkResource::TYPE_HTTPS:
    case MetalinkResource::TYPE_FTP:
      urlsPtr.push_back(resource->url);
      break;
    default:
      break;
    }
  }
};

class FindBitTorrentUrl {
public:
  FindBitTorrentUrl() {}

  bool operator()(const SharedHandle<MetalinkResource>& resource) {
    if(resource->type == MetalinkResource::TYPE_BITTORRENT) {
      return true;
    } else {
      return false;
    }
  }
};

void
Metalink2RequestGroup::generate(std::deque<SharedHandle<RequestGroup> >& groups,
				const std::string& metalinkFile,
				const SharedHandle<Option>& option)
{
  std::deque<SharedHandle<MetalinkEntry> > entries;
  MetalinkHelper::parseAndQuery(entries, metalinkFile, option.get());
  createRequestGroup(groups, entries, option);
}

void
Metalink2RequestGroup::generate(std::deque<SharedHandle<RequestGroup> >& groups,
				const SharedHandle<BinaryStream>& binaryStream,
				const SharedHandle<Option>& option)
{
  std::deque<SharedHandle<MetalinkEntry> > entries;
  MetalinkHelper::parseAndQuery(entries, binaryStream, option.get());
  createRequestGroup(groups, entries, option);
}

void
Metalink2RequestGroup::createRequestGroup
(std::deque<SharedHandle<RequestGroup> >& groups,
 std::deque<SharedHandle<MetalinkEntry> > entries,
 const SharedHandle<Option>& option)
{
  if(entries.size() == 0) {
    _logger->notice(EX_NO_RESULT_WITH_YOUR_PREFS);
    return;
  }
  std::deque<int32_t> selectIndexes =
    util::parseIntRange(option->get(PREF_SELECT_FILE)).flush();
  bool useIndex;
  if(selectIndexes.size()) {
    useIndex = true;
  } else {
    useIndex = false;
  }
  int32_t count = 0;
  for(std::deque<SharedHandle<MetalinkEntry> >::iterator itr = entries.begin(); itr != entries.end();
      ++itr, ++count) {
    SharedHandle<MetalinkEntry>& entry = *itr;
    if(option->defined(PREF_METALINK_LOCATION)) {
      std::deque<std::string> locations;
      util::split(option->get(PREF_METALINK_LOCATION),
		  std::back_inserter(locations), ",", true);
      entry->setLocationPreference(locations, 100);
    }
    if(option->get(PREF_METALINK_PREFERRED_PROTOCOL) != V_NONE) {
      entry->setProtocolPreference(option->get(PREF_METALINK_PREFERRED_PROTOCOL), 100);
    }
    if(useIndex) {
      if(std::find(selectIndexes.begin(), selectIndexes.end(), count+1) ==
	 selectIndexes.end()) {
	continue;
      }
    }
    entry->dropUnsupportedResource();
    if(entry->resources.size() == 0) {
      continue;
    }
    _logger->info(MSG_METALINK_QUEUEING, entry->getPath().c_str());
    std::deque<SharedHandle<MetalinkResource> >::iterator itr =
      std::find_if(entry->resources.begin(), entry->resources.end(), FindBitTorrentUrl());

#ifdef ENABLE_BITTORRENT
    SharedHandle<RequestGroup> torrentRg;
    // there is torrent entry
    if(itr != entry->resources.end()) {
      std::deque<std::string> uris;
      uris.push_back((*itr)->url);
      torrentRg.reset(new RequestGroup(option));
      SharedHandle<DownloadContext> dctx
	(new DownloadContext(option->getAsInt(PREF_SEGMENT_SIZE),
			     0,
			     A2STR::NIL));
      // Since torrent is downloaded in memory, setting dir has no effect.
      //dctx->setDir(_option->get(PREF_DIR));
      dctx->getFirstFileEntry()->setUris(uris);
      torrentRg->setDownloadContext(dctx);
      torrentRg->clearPreDownloadHandler();
      torrentRg->clearPostDownloadHandler();
      // remove "metalink" from Accept Type list to avoid loop in tranparent
      // metalink
      torrentRg->removeAcceptType(RequestGroup::ACCEPT_METALINK);
      // make it in-memory download
      SharedHandle<PreDownloadHandler> preh(new MemoryBufferPreDownloadHandler());
      {
	SharedHandle<RequestGroupCriteria> cri(new TrueRequestGroupCriteria());
	preh->setCriteria(cri);
      }
      torrentRg->addPreDownloadHandler(preh);
      groups.push_back(torrentRg);
    }
#endif // ENABLE_BITTORRENT
    entry->reorderResourcesByPreference();
    std::deque<std::string> uris;
    std::for_each(entry->resources.begin(), entry->resources.end(),
		  AccumulateNonP2PUrl(uris));
    SharedHandle<RequestGroup> rg(new RequestGroup(option));
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
    SharedHandle<DownloadContext> dctx
      (new DownloadContext
       (pieceLength,
	entry->getLength(),
	strconcat(option->get(PREF_DIR), "/", entry->file->getPath())));
    dctx->setDir(option->get(PREF_DIR));
    dctx->getFirstFileEntry()->setUris(uris);
    if(option->getAsBool(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL)) {
      dctx->getFirstFileEntry()->disableSingleHostMultiConnection();
    }
#ifdef ENABLE_MESSAGE_DIGEST
    if(entry->chunkChecksum.isNull()) {
      if(!entry->checksum.isNull()) {
	dctx->setChecksum(entry->checksum->getMessageDigest());
	dctx->setChecksumHashAlgo(entry->checksum->getAlgo());
      }
    } else {
      dctx->setPieceHashes(entry->chunkChecksum->getChecksums().begin(),
			   entry->chunkChecksum->getChecksums().end());
      dctx->setPieceHashAlgo(entry->chunkChecksum->getAlgo());
    }
#endif // ENABLE_MESSAGE_DIGEST
    dctx->setSignature(entry->getSignature());
    rg->setDownloadContext(dctx);
    rg->setNumConcurrentCommand
      (entry->maxConnections < 0 ?
       option->getAsInt(PREF_METALINK_SERVERS) :
       std::min(option->getAsInt(PREF_METALINK_SERVERS),
		static_cast<int32_t>(entry->maxConnections)));
    // remove "metalink" from Accept Type list to avoid loop in tranparent
    // metalink
    rg->removeAcceptType(RequestGroup::ACCEPT_METALINK);

#ifdef ENABLE_BITTORRENT
    // Inject depenency between rg and torrentRg here if torrentRg.isNull() == false
    if(!torrentRg.isNull()) {
      SharedHandle<Dependency> dep(new BtDependency(rg, torrentRg));
      rg->dependsOn(dep);
    }
#endif // ENABLE_BITTORRENT
    groups.push_back(rg);
  }
}

} // namespace aria2
