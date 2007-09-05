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
#include "MetalinkRequestInfo.h"
#include "Xml2MetalinkProcessor.h"
#include "prefs.h"
#include "DlAbortEx.h"
#include "MultiUrlRequestInfo.h"
#include "Util.h"
#include "message.h"

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

RequestInfos MetalinkRequestInfo::execute() {
  Xml2MetalinkProcessor proc;
  RequestInfos nextReqInfos;
  try {
    MetalinkerHandle metalinker = proc.parseFile(metalinkFile);
    MetalinkEntries entries =
      metalinker->queryEntry(op->get(PREF_METALINK_VERSION),
			     op->get(PREF_METALINK_LANGUAGE),
			     op->get(PREF_METALINK_OS));
    if(entries.size() == 0) {
      cout << EX_NO_RESULT_WITH_YOUR_PREFS << endl;
      throw new DlAbortEx(EX_NO_RESULT_WITH_YOUR_PREFS);
    }
    if(op->get(PREF_SHOW_FILES) == V_TRUE) {
      Util::toStream(cout, MetalinkEntry::toFileEntry(entries));
      return RequestInfos();
    }
    bool useIndex;
    Integers selectIndexes;
    Util::unfoldRange(op->get(PREF_SELECT_FILE), selectIndexes);
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
      if(op->defined(PREF_METALINK_LOCATION)) {
	entry->setLocationPreference(op->get(PREF_METALINK_LOCATION), 100);
      }
      if(useIndex) {
	if(find(selectIndexes.begin(), selectIndexes.end(), count+1) == selectIndexes.end()) {
	  continue;
	}
      } else if(!targetFiles.empty()) {
	if(find(targetFiles.begin(), targetFiles.end(), entry->getPath()) == targetFiles.end()) {
	  continue;
	}
      }

      entry->dropUnsupportedResource();
      if(entry->resources.size() == 0) {
	continue;
      }
      logger->info(MSG_METALINK_QUEUEING,
		   entry->getPath().c_str());
      MetalinkResources::iterator itr =
	find_if(entry->resources.begin(),
		entry->resources.end(),
		FindBitTorrentUrl());
      Strings urls;
#ifdef ENABLE_MESSAGE_DIGEST
      ChecksumHandle checksum = 0;
#endif // ENABLE_MESSAGE_DIGEST
      if(itr == entry->resources.end()) {
	entry->reorderResourcesByPreference();
	
	for_each(entry->resources.begin(), entry->resources.end(),
		 AccumulateNonP2PUrl(&urls, op->getAsInt(PREF_SPLIT)));
#ifdef ENABLE_MESSAGE_DIGEST
	// TODO
	// set checksum
	checksum = entry->checksum;
#endif // ENABLE_MESSAGE_DIGEST
      } else {
	// BitTorrent downloading
	urls.push_back((*itr)->url);
      }
      RequestGroupHandle rg = new RequestGroup(urls, op);
      if(itr == entry->resources.end()) {
	rg->setHintFilename(entry->file->getBasename());
	rg->setHintTotalLength(entry->getLength());
      }
      rg->setTopDir(entry->file->getDirname());
      rg->setNumConcurrentCommand(entry->maxConnections < 0 ?
				  op->getAsInt(PREF_METALINK_SERVERS) :
				  min<int32_t>(op->getAsInt(PREF_METALINK_SERVERS), entry->maxConnections));

#ifdef ENABLE_MESSAGE_DIGEST
      if(entry->chunkChecksum.isNull()) {
	rg->setChecksum(checksum);
      } else {
	rg->setChunkChecksum(entry->chunkChecksum);
      }
#endif // ENABLE_MESSAGE_DIGEST
      groups.push_front(rg);
    }
    // clear PREF_OUT, because PREF_OUT is a filename for metalink file itself.
    op->put(PREF_OUT, "");
    MultiUrlRequestInfoHandle reqInfo = new MultiUrlRequestInfo(groups, op);
    nextReqInfos.push_back(reqInfo);
  } catch(RecoverableException* ex) {
    logger->error(EX_EXCEPTION_CAUGHT, ex);
    delete ex;
    fail = true;
  }
  return nextReqInfos;
}
