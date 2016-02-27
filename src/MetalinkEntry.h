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
#ifndef D_METALINK_ENTRY_H
#define D_METALINK_ENTRY_H

#include "common.h"

#include <string>
#include <vector>
#include <memory>

namespace aria2 {

class MetalinkResource;
class MetalinkMetaurl;
class FileEntry;
class Checksum;
class ChunkChecksum;
class Signature;

class MetalinkEntry {
public:
  std::unique_ptr<FileEntry> file;
  std::string version;
  std::vector<std::string> languages;
  std::vector<std::string> oses;
  // True if size is specified in Metalink document.
  bool sizeKnown;
  std::vector<std::unique_ptr<MetalinkResource>> resources;
  std::vector<std::unique_ptr<MetalinkMetaurl>> metaurls;
  int maxConnections; // Metalink3Spec
  std::unique_ptr<Checksum> checksum;
  std::unique_ptr<ChunkChecksum> chunkChecksum;

private:
  std::unique_ptr<Signature> signature_;

public:
  MetalinkEntry();

  ~MetalinkEntry();

  const std::string& getPath() const;

  int64_t getLength() const;

  const std::unique_ptr<FileEntry>& getFile() const { return file; }

  std::unique_ptr<FileEntry> popFile();

  void dropUnsupportedResource();

  void reorderResourcesByPriority();

  void reorderMetaurlsByPriority();

  bool containsLanguage(const std::string& lang) const;

  bool containsOS(const std::string& os) const;

  void setLocationPriority(const std::vector<std::string>& locations,
                           int priorityToAdd);

  void setProtocolPriority(const std::string& protocol, int priorityToAdd);

  static std::vector<std::unique_ptr<FileEntry>>
  toFileEntry(std::vector<std::unique_ptr<MetalinkEntry>> metalinkEntries);

  void setSignature(std::unique_ptr<Signature> signature);

  const std::unique_ptr<Signature>& getSignature() const { return signature_; }

  std::unique_ptr<Signature> popSignature();
};

} // namespace aria2

#endif // D_METALINK_ENTRY_H
