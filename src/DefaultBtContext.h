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
#ifndef _D_DEFAULT_BT_CONTEXT_H_
#define _D_DEFAULT_BT_CONTEXT_H_

#include "BtContext.h"
#include <iosfwd>

namespace aria2 {

class Randomizer;
class Logger;
class MetaEntry;
class Dictionary;
class List;
class Data;

#define INFO_HASH_LENGTH 20
#define PIECE_HASH_LENGTH 20

class DefaultBtContext : public BtContext {
private:
  unsigned char infoHash[INFO_HASH_LENGTH];
  std::string infoHashString;
  std::deque<std::string> pieceHashes;
  std::deque<SharedHandle<FileEntry> > fileEntries;
  FILE_MODE fileMode;
  int64_t totalLength;
  int32_t pieceLength;
  std::string name;
  int32_t numPieces;
  std::string peerId;
  std::string _peerIdPrefix;
  std::deque<SharedHandle<AnnounceTier> > announceTiers;
  SharedHandle<Randomizer> _randomizer;

  RequestGroup* _ownerRequestGroup;

  const Logger* _logger;

  void clear();
  void extractPieceHash(const unsigned char* hashData,
			int32_t hashDataLength,
			int32_t hashLength);
  void extractFileEntries(const Dictionary* infoDic,
			  const std::string& defaultName,
			  const std::deque<std::string>& urlList);
  void extractAnnounce(const Data* announceData);
  void extractAnnounceList(const List* announceListData);

  std::deque<std::string> extractUrlList(const MetaEntry* obj);

  void processRootDictionary(const Dictionary* rootDic, const std::string& defaultName);

 public:
  DefaultBtContext();
  virtual ~DefaultBtContext();

  virtual const unsigned char* getInfoHash() const;

  virtual int32_t getInfoHashLength() const;

  virtual std::string getInfoHashAsString() const;

  virtual std::string getPieceHash(int32_t index) const;

  virtual const std::deque<std::string>& getPieceHashes() const
  {
    return pieceHashes;
  }

  virtual int64_t getTotalLength() const;

  virtual FILE_MODE getFileMode() const;

  virtual std::deque<SharedHandle<FileEntry> > getFileEntries() const;

  virtual std::string getPieceHashAlgo() const
  {
    return "sha1";
  }

  virtual std::deque<SharedHandle<AnnounceTier> > getAnnounceTiers() const;

  virtual void load(const std::string& torrentFile);

  void loadFromMemory(const char* content, int32_t length, const std::string& defaultName);

  virtual std::string getName() const;

  virtual int32_t getPieceLength() const;
  
  virtual int32_t getNumPieces() const;

  virtual std::string getActualBasePath() const;

  virtual const unsigned char* getPeerId() {
    if(peerId == "") {
      peerId = generatePeerId();
    }
    return (const unsigned char*)peerId.c_str();
  }

  virtual std::deque<int32_t> computeFastSet(const std::string& ipaddr, int32_t fastSetSize);

  virtual RequestGroup* getOwnerRequestGroup()
  {
    return _ownerRequestGroup;
  }

  std::string generatePeerId() const;

  void setPeerIdPrefix(const std::string& peerIdPrefix)
  {
    _peerIdPrefix = peerIdPrefix;
  }

  // for unit test
  void setInfoHash(const unsigned char* infoHash)
  {
    memcpy(this->infoHash, infoHash, sizeof(this->infoHash));
  }

  void setNumPieces(int32_t numPieces)
  {
    this->numPieces = numPieces;
  }
   
  void setOwnerRequestGroup(RequestGroup* owner)
  {
    _ownerRequestGroup = owner;
  }

  void setRandomizer(const SharedHandle<Randomizer>& randomizer);

  friend std::ostream& operator<<(std::ostream& o, const DefaultBtContext& ctx);
};

typedef SharedHandle<DefaultBtContext> DefaultBtContextHandle;

} // namespace aria2

#endif // _D_DEFAULT_BT_CONTEXT_H_
