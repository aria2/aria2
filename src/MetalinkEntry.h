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
#ifndef _D_METALINK_ENTRY_H_
#define _D_METALINK_ENTRY_H_

#include "common.h"

#include <string>
#include <deque>

#include "SharedHandle.h"

namespace aria2 {

class MetalinkResource;
class FileEntry;
#ifdef ENABLE_MESSAGE_DIGEST
class Checksum;
class ChunkChecksum;
#endif // ENABLE_MESSAGE_DIGEST
class Signature;

class MetalinkEntry {
public:
  SharedHandle<FileEntry> file;
  std::string version;
  std::string language;
  std::string os;
  std::deque<SharedHandle<MetalinkResource> > resources;
  int maxConnections;
#ifdef ENABLE_MESSAGE_DIGEST
  SharedHandle<Checksum> checksum;
  SharedHandle<ChunkChecksum> chunkChecksum;
#endif // ENABLE_MESSAGE_DIGEST
private:
  SharedHandle<Signature> _signature;
public:
  MetalinkEntry();

  ~MetalinkEntry();

  MetalinkEntry& operator=(const MetalinkEntry& metalinkEntry);

  std::string getPath() const;

  uint64_t getLength() const;

  const SharedHandle<FileEntry>& getFile() const
  {
    return file;
  }

  void dropUnsupportedResource();

  void reorderResourcesByPreference();
  
  void setLocationPreference(const std::deque<std::string>& locations, int preferenceToAdd);
  void setProtocolPreference(const std::string& protocol, int preferenceToAdd);

  static void toFileEntry
  (std::deque<SharedHandle<FileEntry> >& fileEntries,
   const std::deque<SharedHandle<MetalinkEntry> >& metalinkEntries);

  void setSignature(const SharedHandle<Signature>& signature);

  const SharedHandle<Signature>& getSignature() const
  {
    return _signature;
  }

};

} // namespace aria2

#endif // _D_METALINK_ENTRY_H_
