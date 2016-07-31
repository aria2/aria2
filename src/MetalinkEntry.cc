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
#include "MetalinkEntry.h"

#include <algorithm>

#include "MetalinkResource.h"
#include "MetalinkMetaurl.h"
#include "FileEntry.h"
#include "util.h"
#include "a2functional.h"
#include "Checksum.h"
#include "ChunkChecksum.h"
#include "Signature.h"
#include "SimpleRandomizer.h"

namespace aria2 {

MetalinkEntry::MetalinkEntry() : sizeKnown(false), maxConnections(-1) {}

MetalinkEntry::~MetalinkEntry() = default;

const std::string& MetalinkEntry::getPath() const { return file->getPath(); }

int64_t MetalinkEntry::getLength() const { return file->getLength(); }

void MetalinkEntry::setLocationPriority(
    const std::vector<std::string>& locations, int priorityToAdd)
{
  for (auto& res : resources) {
    if (std::find(std::begin(locations), std::end(locations), res->location) !=
        std::end(locations)) {
      res->priority += priorityToAdd;
    }
  }
}

void MetalinkEntry::setProtocolPriority(const std::string& protocol,
                                        int priorityToAdd)
{
  for (auto& res : resources) {
    if (protocol == MetalinkResource::getTypeString(res->type)) {
      res->priority += priorityToAdd;
    }
  }
}

namespace {
template <typename T> class PriorityHigher {
public:
  bool operator()(const T& res1, const T& res2)
  {
    return res1->priority < res2->priority;
  }
};
} // namespace

void MetalinkEntry::reorderResourcesByPriority()
{
  std::shuffle(std::begin(resources), std::end(resources),
               *SimpleRandomizer::getInstance());
  std::sort(std::begin(resources), std::end(resources),
            PriorityHigher<std::unique_ptr<MetalinkResource>>{});
}

void MetalinkEntry::reorderMetaurlsByPriority()
{
  std::sort(std::begin(metaurls), std::end(metaurls),
            PriorityHigher<std::unique_ptr<MetalinkMetaurl>>{});
}

namespace {
class Supported
    : public std::unary_function<std::shared_ptr<MetalinkResource>, bool> {
public:
  bool operator()(const std::shared_ptr<MetalinkResource>& res) const
  {
    switch (res->type) {
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
} // namespace

void MetalinkEntry::dropUnsupportedResource()
{
  resources.erase(
      std::remove_if(std::begin(resources), std::end(resources),
                     [](const std::unique_ptr<MetalinkResource>& res) {
                       switch (res->type) {
                       case MetalinkResource::TYPE_FTP:
                       case MetalinkResource::TYPE_HTTP:
#ifdef ENABLE_SSL
                       case MetalinkResource::TYPE_HTTPS:
#endif // ENABLE_SSL
#ifdef ENABLE_BITTORRENT
                       case MetalinkResource::TYPE_BITTORRENT:
#endif // ENABLE_BITTORRENT
                         return false;
                       default:
                         return true;
                       }
                     }),
      std::end(resources));
}

std::vector<std::unique_ptr<FileEntry>> MetalinkEntry::toFileEntry(
    std::vector<std::unique_ptr<MetalinkEntry>> metalinkEntries)
{
  std::vector<std::unique_ptr<FileEntry>> res;
  res.reserve(metalinkEntries.size());
  for (auto& entry : metalinkEntries) {
    res.push_back(entry->popFile());
  }
  return res;
}

void MetalinkEntry::setSignature(std::unique_ptr<Signature> signature)
{
  signature_ = std::move(signature);
}

bool MetalinkEntry::containsLanguage(const std::string& lang) const
{
  return std::find(languages.begin(), languages.end(), lang) != languages.end();
}

bool MetalinkEntry::containsOS(const std::string& os) const
{
  return std::find(oses.begin(), oses.end(), os) != oses.end();
}

std::unique_ptr<Signature> MetalinkEntry::popSignature()
{
  return std::move(signature_);
}

std::unique_ptr<FileEntry> MetalinkEntry::popFile() { return std::move(file); }

} // namespace aria2
