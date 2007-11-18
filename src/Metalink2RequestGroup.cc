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
#include "RequestGroup.h"
#include "Option.h"
#include "LogFactory.h"
#include "prefs.h"
#include "Xml2MetalinkProcessor.h"
#include "Util.h"
#include "message.h"
#include "SingleFileDownloadContext.h"
#include "MetalinkHelper.h"
#ifdef ENABLE_BITTORRENT
# include "BtDependency.h"
#endif // ENABLE_BITTORRENT

Metalink2RequestGroup::Metalink2RequestGroup(const Option* option):_option(option), _logger(LogFactory::getInstance()) {}

Metalink2RequestGroup::~Metalink2RequestGroup() {}

class AccumulateNonP2PUrl {
private:
  Strings* urlsPtr;
  int32_t split;
public:
  AccumulateNonP2PUrl(Strings* urlsPtr, int32_t split)
    :urlsPtr(urlsPtr),
     split(split) {}

  void operator()(const MetalinkResourceHandle& resource) {
    int32_t maxConnections;
    if(resource->maxConnections < 0) {
      maxConnections = split;
    } else {
      maxConnections = min<int32_t>(resource->maxConnections, split);
    }
    switch(resource->type) {
    case MetalinkResource::TYPE_HTTP:
    case MetalinkResource::TYPE_HTTPS:
    case MetalinkResource::TYPE_FTP:
      for(int32_t s = 1; s <= maxConnections; s++) {
	urlsPtr->push_back(resource->url);
      }
      break;
    default:
      break;
    }
  }
};

class FindBitTorrentUrl {
public:
  FindBitTorrentUrl() {}

  bool operator()(const MetalinkResourceHandle& resource) {
    if(resource->type == MetalinkResource::TYPE_BITTORRENT) {
      return true;
    } else {
      return false;
    }
  }
};

RequestGroups Metalink2RequestGroup::generate(const string& metalinkFile)
{
  MetalinkEntries entries = MetalinkHelper::parseAndQuery(metalinkFile,
							  _option);
  if(entries.size() == 0) {
    _logger->notice(EX_NO_RESULT_WITH_YOUR_PREFS);
    return RequestGroups();
  }
  bool useIndex;
  Integers selectIndexes;
  Util::unfoldRange(_option->get(PREF_SELECT_FILE), selectIndexes);
  if(selectIndexes.size()) {
    useIndex = true;
  } else {
    useIndex = false;
  }
  RequestGroups groups;
  int32_t count = 0;
  for(MetalinkEntries::iterator itr = entries.begin(); itr != entries.end();
      itr++, ++count) {
    MetalinkEntryHandle& entry = *itr;
    if(_option->defined(PREF_METALINK_LOCATION)) {
      Strings locations;
      Util::slice(locations, _option->get(PREF_METALINK_LOCATION), ',', true);
      entry->setLocationPreference(locations, 100);
    }
    if(useIndex) {
      if(find(selectIndexes.begin(), selectIndexes.end(), count+1) == selectIndexes.end()) {
	continue;
      }
    }
    entry->dropUnsupportedResource();
    if(entry->resources.size() == 0) {
      continue;
    }
    _logger->info(MSG_METALINK_QUEUEING, entry->getPath().c_str());
    MetalinkResources::iterator itr = find_if(entry->resources.begin(),
					      entry->resources.end(),
					      FindBitTorrentUrl());
#ifdef ENABLE_BITTORRENT
    RequestGroupHandle torrentRg = 0;
    // there is torrent entry
    if(itr != entry->resources.end()) {
      Strings uris;
      uris.push_back((*itr)->url);
      torrentRg = new RequestGroup(_option, uris);
      SingleFileDownloadContextHandle dctx =
	new SingleFileDownloadContext(_option->getAsInt(PREF_SEGMENT_SIZE),
				      0,
				      "");
      dctx->setDir(_option->get(PREF_DIR));
      torrentRg->setDownloadContext(dctx);
      torrentRg->clearPostDowloadHandler();
      groups.push_back(torrentRg);
    }
#endif // ENABLE_BITTORRENT
    entry->reorderResourcesByPreference();
    Strings uris;
    for_each(entry->resources.begin(), entry->resources.end(),
	     AccumulateNonP2PUrl(&uris, _option->getAsInt(PREF_SPLIT)));
    RequestGroupHandle rg = new RequestGroup(_option, uris);
    // If piece hash is specified in the metalink,
    // make segment size equal to piece hash size.
    int32_t pieceLength;
#ifdef ENABLE_MESSAGE_DIGEST
    if(entry->chunkChecksum.isNull()) {
      pieceLength = _option->getAsInt(PREF_SEGMENT_SIZE);
    } else {
      pieceLength = entry->chunkChecksum->getChecksumLength();
    }
#else
    pieceLength = _option->getAsInt(PREF_SEGMENT_SIZE);
#endif // ENABLE_MESSAGE_DIGEST
    SingleFileDownloadContextHandle dctx =
      new SingleFileDownloadContext(pieceLength,
				    entry->getLength(),
				    "",
				    entry->file->getPath());
    dctx->setDir(_option->get(PREF_DIR));
#ifdef ENABLE_MESSAGE_DIGEST
    if(entry->chunkChecksum.isNull()) {
      if(!entry->checksum.isNull()) {
	dctx->setChecksum(entry->checksum->getMessageDigest());
	dctx->setChecksumHashAlgo(entry->checksum->getAlgo());
      }
    } else {
      dctx->setPieceHashes(entry->chunkChecksum->getChecksums());
      dctx->setPieceHashAlgo(entry->chunkChecksum->getAlgo());
    }
#endif // ENABLE_MESSAGE_DIGEST
    rg->setDownloadContext(dctx);
    rg->setNumConcurrentCommand(entry->maxConnections < 0 ?
				_option->getAsInt(PREF_METALINK_SERVERS) :
				min<int32_t>(_option->getAsInt(PREF_METALINK_SERVERS), entry->maxConnections));

#ifdef ENABLE_BITTORRENT
    // Inject depenency between rg and torrentRg here if torrentRg.isNull() == false
    if(!torrentRg.isNull()) {
      rg->dependsOn(new BtDependency(rg, torrentRg, _option));
    }
#endif // ENABLE_BITTORRENT
    groups.push_back(rg);
  }
  return groups;
}
