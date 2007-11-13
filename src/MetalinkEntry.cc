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
#include "MetalinkEntry.h"
#include "Util.h"
#include <algorithm>

MetalinkEntry::MetalinkEntry():
  maxConnections(-1)
#ifdef ENABLE_MESSAGE_DIGEST
  ,
  checksum(0),
  chunkChecksum(0)
#endif // ENABLE_MESSAGE_DIGEST
{}

MetalinkEntry::~MetalinkEntry() {}

class AddLocationPreference {
private:
  Strings _locations;
  int32_t _preferenceToAdd;
public:
  AddLocationPreference(const Strings& locations, int32_t preferenceToAdd):
    _locations(locations), _preferenceToAdd(preferenceToAdd)
  {
    transform(_locations.begin(), _locations.end(), _locations.begin(), Util::toUpper);
    sort(_locations.begin(), _locations.end());
  }

  void operator()(MetalinkResourceHandle& res) {
    if(binary_search(_locations.begin(), _locations.end(), res->location)) {
      res->preference += _preferenceToAdd;
    }
  }
};

void MetalinkEntry::setLocationPreference(const Strings& locations,
					  int32_t preferenceToAdd)
{
  for_each(resources.begin(), resources.end(),
	   AddLocationPreference(locations, preferenceToAdd));
}

class PrefOrder {
public:
  bool operator()(const MetalinkResourceHandle& res1,
		  const MetalinkResourceHandle& res2) {
    return res1->preference > res2->preference;
  }
};

void MetalinkEntry::reorderResourcesByPreference() {
  random_shuffle(resources.begin(), resources.end());
  sort(resources.begin(), resources.end(), PrefOrder());
}

class Supported {
public:
  bool operator()(const MetalinkResourceHandle& res) {
    switch(res->type) {
    case MetalinkResource::TYPE_FTP:
    case MetalinkResource::TYPE_HTTP:
#ifdef ENABLE_SSL
    case MetalinkResource::TYPE_HTTPS:
#endif // ENABLE_SSL
#ifdef ENABLE_BITTORRENT
    case MetalinkResource::TYPE_BITTORRENT:
#endif // ENABLE_BITTORRENT
      return true;
    default:
      return false;
    }
  }
};

void MetalinkEntry::dropUnsupportedResource() {
  MetalinkResources::iterator split =
    partition(resources.begin(), resources.end(), Supported());
  resources.erase(split, resources.end());
}

FileEntries MetalinkEntry::toFileEntry(const MetalinkEntries& metalinkEntries)
{
  FileEntries entries;
  for(MetalinkEntries::const_iterator itr = metalinkEntries.begin();
      itr != metalinkEntries.end(); ++itr) {
    entries.push_back((*itr)->file);
  }
  return entries;
}

