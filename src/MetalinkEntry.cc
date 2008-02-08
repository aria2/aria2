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
#include "MetalinkResource.h"
#include "FileEntry.h"
#include "Util.h"
#include "a2functional.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <algorithm>

namespace aria2 {

MetalinkEntry::MetalinkEntry():
  file(0),
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
  std::deque<std::string> _locations;
  int32_t _preferenceToAdd;
public:
  AddLocationPreference(const std::deque<std::string>& locations, int32_t preferenceToAdd):
    _locations(locations), _preferenceToAdd(preferenceToAdd)
  {
    std::transform(_locations.begin(), _locations.end(), _locations.begin(), Util::toUpper);
    std::sort(_locations.begin(), _locations.end());
  }

  void operator()(SharedHandle<MetalinkResource>& res) {
    if(std::binary_search(_locations.begin(), _locations.end(), res->location)) {
      res->preference += _preferenceToAdd;
    }
  }
};

MetalinkEntry& MetalinkEntry::operator=(const MetalinkEntry& metalinkEntry)
{
  if(this != &metalinkEntry) {
    this->file = metalinkEntry.file;
    this->version = metalinkEntry.version;
    this->language = metalinkEntry.language;
    this->os = metalinkEntry.os;
    this->maxConnections = metalinkEntry.maxConnections;
#ifdef ENABLE_MESSAGE_DIGEST
    this->checksum = metalinkEntry.checksum;
    this->chunkChecksum = metalinkEntry.chunkChecksum;
#endif // ENABLE_MESSAGE_DIGEST
  }
  return *this;
}

std::string MetalinkEntry::getPath() const
{
  return file->getPath();
}

int64_t MetalinkEntry::getLength() const
{
  return file->getLength();
}

void MetalinkEntry::setLocationPreference(const std::deque<std::string>& locations,
					  int32_t preferenceToAdd)
{
  std::for_each(resources.begin(), resources.end(),
		AddLocationPreference(locations, preferenceToAdd));
}

class AddProtocolPreference {
private:
  const std::string& _protocol;
  int32_t _preferenceToAdd;
public:
  AddProtocolPreference(const std::string& protocol, int32_t prefToAdd):
    _protocol(protocol), _preferenceToAdd(prefToAdd) {}

  void operator()(const SharedHandle<MetalinkResource>& res) const
  {
    if(_protocol == MetalinkResource::getTypeString(res->type)) {
      res->preference += _preferenceToAdd;
    }
  }
};

void MetalinkEntry::setProtocolPreference(const std::string& protocol,
					  int32_t preferenceToAdd)
{
  std::for_each(resources.begin(), resources.end(),
		AddProtocolPreference(protocol, preferenceToAdd));
}

class PrefOrder {
public:
  bool operator()(const SharedHandle<MetalinkResource>& res1,
		  const SharedHandle<MetalinkResource>& res2) {
    return res1->preference > res2->preference;
  }
};

void MetalinkEntry::reorderResourcesByPreference() {
  std::random_shuffle(resources.begin(), resources.end());
  std::sort(resources.begin(), resources.end(), PrefOrder());
}

class Supported:public std::unary_function<SharedHandle<MetalinkResource>, bool> {
public:
  bool operator()(const SharedHandle<MetalinkResource>& res) const
  {
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
  resources.erase(std::remove_if(resources.begin(), resources.end(),
				 std::not1(Supported())),
		  resources.end());
}

std::deque<SharedHandle<FileEntry> >
MetalinkEntry::toFileEntry(const std::deque<SharedHandle<MetalinkEntry> >& metalinkEntries)
{
  std::deque<SharedHandle<FileEntry> > entries;
  std::transform(metalinkEntries.begin(), metalinkEntries.end(), std::back_inserter(entries),
		 mem_fun_sh(&MetalinkEntry::getFile));
  return entries;
}

SharedHandle<FileEntry> MetalinkEntry::getFile() const
{
  return file;
}

} // namespace aria2
