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
#include "MetalinkResource.h"
#include "FileEntry.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <deque>

class MetalinkEntry;

typedef SharedHandle<MetalinkEntry> MetalinkEntryHandle;
typedef deque<MetalinkEntryHandle> MetalinkEntries;

class MetalinkEntry {
public:
  FileEntryHandle file;
  string version;
  string language;
  string os;
  MetalinkResources resources;
  int32_t maxConnections;
#ifdef ENABLE_MESSAGE_DIGEST
  ChecksumHandle checksum;
  ChunkChecksumHandle chunkChecksum;
#endif // ENABLE_MESSAGE_DIGEST
public:
  MetalinkEntry();

  ~MetalinkEntry();

  MetalinkEntry& operator=(const MetalinkEntry& metalinkEntry) {
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

  string getPath() const
  {
    return file->getPath();
  }

  int64_t getLength() const
  {
    return file->getLength();
  }

  void dropUnsupportedResource();

  void reorderResourcesByPreference();
  
  void setLocationPreference(const Strings& locations, int32_t preferenceToAdd);
  void setProtocolPreference(const string& protocol, int32_t preferenceToAdd);

  static FileEntries toFileEntry(const MetalinkEntries& metalinkEntries);
};

#endif // _D_METALINK_ENTRY_H_
