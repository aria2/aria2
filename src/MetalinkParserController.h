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
#ifndef D_METALINK_PARSER_CONTROLLER_H
#define D_METALINK_PARSER_CONTROLLER_H

#include "common.h"

#include <string>
#include <utility>
#include <vector>
#include <memory>

namespace aria2 {

class Metalinker;
class MetalinkEntry;
class MetalinkResource;
class MetalinkMetaurl;
class Signature;

class Checksum;
class ChunkChecksum;

class MetalinkParserController {
private:
  std::unique_ptr<Metalinker> metalinker_;

  std::unique_ptr<MetalinkEntry> tEntry_;

  std::unique_ptr<MetalinkResource> tResource_;

  std::unique_ptr<MetalinkMetaurl> tMetaurl_;

  std::unique_ptr<Checksum> tChecksum_;

  std::unique_ptr<ChunkChecksum> tChunkChecksumV4_; // Metalink4Spec

  std::vector<std::string> tempChunkChecksumsV4_; // Metalink4Spec

  std::unique_ptr<ChunkChecksum> tChunkChecksum_; // Metalink3Spec

  std::vector<std::pair<size_t, std::string>>
      tempChunkChecksums_; // Metalink3Spec

  std::pair<size_t, std::string> tempHashPair_; // Metalink3Spec

  std::unique_ptr<Signature> tSignature_;
  std::string baseUri_;

public:
  MetalinkParserController();

  ~MetalinkParserController();

  void reset();

  std::unique_ptr<Metalinker> getResult();

  void newEntryTransaction();

  void setFileNameOfEntry(std::string filename);

  void setFileLengthOfEntry(int64_t length);

  void setVersionOfEntry(std::string version);

  void setLanguageOfEntry(std::string language);

  void setOSOfEntry(std::string os);

  void setMaxConnectionsOfEntry(int maxConnections);

  void commitEntryTransaction();

  void cancelEntryTransaction();

  void newResourceTransaction();

  void setURLOfResource(std::string url);

  void setTypeOfResource(std::string type);

  void setLocationOfResource(std::string location);

  void setPriorityOfResource(int priority);

  void setMaxConnectionsOfResource(int maxConnections);

  void commitResourceTransaction();

  void cancelResourceTransaction();

  void newChecksumTransaction();

  void setTypeOfChecksum(std::string type);

  void setHashOfChecksum(std::string md);

  void commitChecksumTransaction();

  void cancelChecksumTransaction();

  void newChunkChecksumTransactionV4(); // Metalink4Spec

  void setTypeOfChunkChecksumV4(std::string type); // Metalink4Spec

  void setLengthOfChunkChecksumV4(size_t length); // Metalink4Spec

  void addHashOfChunkChecksumV4(std::string md); // Metalink4Spec

  void commitChunkChecksumTransactionV4(); // Metalink4Spec

  void cancelChunkChecksumTransactionV4(); // Metalink4Spec

  void newChunkChecksumTransaction(); // Metalink3Spec

  void setTypeOfChunkChecksum(std::string type); // Metalink3Spec

  void setLengthOfChunkChecksum(size_t length); // Metalink3Spec

  void addHashOfChunkChecksum(size_t order, std::string md); // Metalink3Spec

  void createNewHashOfChunkChecksum(size_t order); // Metalink3Spec

  void setMessageDigestOfChunkChecksum(std::string md); // Metalink3Spec

  void addHashOfChunkChecksum(); // Metalink3Spec

  void commitChunkChecksumTransaction(); // Metalink3Spec

  void cancelChunkChecksumTransaction(); // Metalink3Spec

  void newSignatureTransaction();

  void setTypeOfSignature(std::string type);

  void setFileOfSignature(std::string file);

  void setBodyOfSignature(std::string body);

  void commitSignatureTransaction();

  void cancelSignatureTransaction();

  void newMetaurlTransaction();

  void setURLOfMetaurl(std::string url);

  void setMediatypeOfMetaurl(std::string mediatype);

  void setPriorityOfMetaurl(int priority);

  void setNameOfMetaurl(std::string name);

  void commitMetaurlTransaction();

  void cancelMetaurlTransaction();

  void setBaseUri(std::string baseUri) { baseUri_ = std::move(baseUri); }
};

} // namespace aria2

#endif // D_METALINK_PARSER_CONTROLLER_H
