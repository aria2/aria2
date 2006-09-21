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
#include "UrlRequestInfo.h"

class AccumulateNonP2PUrl {
private:
  Strings* urlsPtr;
  int split;
public:
  AccumulateNonP2PUrl(Strings* urlsPtr,
		int split)
    :urlsPtr(urlsPtr),
     split(split) {}

  void operator()(const MetalinkResource* resource) {
    switch(resource->type) {
    case MetalinkResource::TYPE_HTTP:
    case MetalinkResource::TYPE_HTTPS:
    case MetalinkResource::TYPE_FTP:
      for(int s = 1; s <= split; s++) {
	urlsPtr->push_back(resource->url);
      }
      break;
    }
  }
};

class FindBitTorrentUrl {
public:
  FindBitTorrentUrl() {}

  bool operator()(const MetalinkResource* resource) {
    if(resource->type == MetalinkResource::TYPE_BITTORRENT) {
      return true;
    } else {
      return false;
    }
  }
};

RequestInfo* MetalinkRequestInfo::execute() {
  RequestInfo* next = 0;
  Xml2MetalinkProcessor proc;
  Metalinker* metalinker = 0;
  try {
    metalinker = proc.parseFile(metalinkFile);
    
    MetalinkEntry* entry =
      metalinker->queryEntry(op->get(PREF_METALINK_VERSION),
			     op->get(PREF_METALINK_LANGUAGE),
			     op->get(PREF_METALINK_OS));
    if(entry == 0) {
      printf("No file matched with your preference.\n");
      throw new DlAbortEx("No file matched with your preference.");
    }
    entry->dropUnsupportedResource();
    MetalinkResources::iterator itr =
      find_if(entry->resources.begin(),
	      entry->resources.end(),
	      FindBitTorrentUrl());
    Strings urls;
    int maxConnection = 0;
    Checksum checksum;
    if(itr == entry->resources.end()) {
      entry->reorderResourcesByPreference();
	
      for_each(entry->resources.begin(), entry->resources.end(),
	       AccumulateNonP2PUrl(&urls, op->getAsInt(PREF_SPLIT)));
      maxConnection =
	op->getAsInt(PREF_METALINK_SERVERS)*op->getAsInt(PREF_SPLIT);
	
      // TODO
      // set checksum
      checksum = entry->checksum;
    } else {
      // BitTorrent downloading
      urls.push_back((*itr)->url);
    }
    next = new UrlRequestInfo(urls, maxConnection, op);
    next->setChecksum(checksum);
  } catch(Exception* e) {
    logger->error("Exception caught", e);
    delete e;
    fail = true;
  }
  delete metalinker;
  return next;
}
