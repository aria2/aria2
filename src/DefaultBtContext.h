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

#include "IntSequence.h"

namespace aria2 {

class Randomizer;
class Logger;
class BDE;

#define INFO_HASH_LENGTH 20
#define PIECE_HASH_LENGTH 20

class DefaultBtContext : public BtContext {
private:
  unsigned char infoHash[INFO_HASH_LENGTH];
  std::string infoHashString;
  std::deque<std::string> pieceHashes;
  std::deque<SharedHandle<FileEntry> > fileEntries;
  FILE_MODE fileMode;
  uint64_t totalLength;
  size_t pieceLength;
  std::string name;
  size_t numPieces;
  std::string peerId;
  std::string _peerIdPrefix;
  std::deque<SharedHandle<AnnounceTier> > announceTiers;
  std::deque<std::pair<std::string, uint16_t> > _nodes;
  SharedHandle<Randomizer> _randomizer;

  RequestGroup* _ownerRequestGroup;

  Logger* _logger;

  static const std::string C_NAME;

  static const std::string C_NAME_UTF8;

  static const std::string C_FILES;

  static const std::string C_LENGTH;

  static const std::string C_PATH;

  static const std::string C_PATH_UTF8;

  static const std::string C_INFO;

  static const std::string C_PIECES;

  static const std::string C_PIECE_LENGTH;

  static const std::string C_PRIVATE;

  // This is just a string "1". Used as a value of "private" flag.
  static const std::string C_PRIVATE_ON;

  static const std::string C_URL_LIST;

  static const std::string C_ANNOUNCE;

  static const std::string C_ANNOUNCE_LIST;

  static const std::string C_NODES;

  void clear();
  void extractPieceHash(const std::string& hashData, size_t hashLength);
  void extractFileEntries(const BDE& infoDic,
			  const std::string& defaultName,
			  const std::string& overrideName,
			  const std::deque<std::string>& urlList);
  void extractAnnounceURI(const BDE& announceData);
  void extractAnnounceList(const BDE& announceListData);
  void extractAnnounce(const BDE& rootDict);

  void extractUrlList(std::deque<std::string>& uris, const BDE& obj);

  void extractNodes(const BDE& nodes);

  void processRootDictionary(const BDE& rootDic,
			     const std::string& defaultName,
			     const std::string& overrideName);

 public:
  DefaultBtContext();
  virtual ~DefaultBtContext();

  virtual const unsigned char* getInfoHash() const;

  virtual size_t getInfoHashLength() const;

  virtual const std::string& getInfoHashAsString() const;

  virtual const std::string& getPieceHash(size_t index) const;

  virtual const std::deque<std::string>& getPieceHashes() const
  {
    return pieceHashes;
  }

  virtual uint64_t getTotalLength() const;

  virtual bool knowsTotalLength() const;

  virtual FILE_MODE getFileMode() const;

  virtual std::deque<SharedHandle<FileEntry> > getFileEntries() const;

  virtual const std::string& getPieceHashAlgo() const;

  virtual const std::deque<SharedHandle<AnnounceTier> >&
  getAnnounceTiers() const;

  // Note: Before calling load* function, call setDir() to specify
  // directory.  Setting directory after load* function doesn't change
  // the output file path.
  virtual void load(const std::string& torrentFile,
		    const std::string& overrideName = "");

  void loadFromMemory(const unsigned char* content, size_t length,
		      const std::string& defaultName,
		      const std::string& overrideName = "");

  void loadFromMemory(const std::string& context,
		      const std::string& defaultName,
		      const std::string& overrideName = "")
  {
    loadFromMemory(reinterpret_cast<const unsigned char*>(context.c_str()),
		   context.size(), defaultName, overrideName);
  }

  virtual const std::string& getName() const;

  virtual size_t getPieceLength() const;
  
  virtual size_t getNumPieces() const;

  virtual std::string getActualBasePath() const;

  virtual const unsigned char* getPeerId() {
    if(peerId.empty()) {
      peerId = generatePeerId();
    }
    return reinterpret_cast<const unsigned char*>(peerId.c_str());
  }

  virtual void computeFastSet
  (std::deque<size_t>& fastSet, const std::string& ipaddr, size_t fastSetSize);

  virtual RequestGroup* getOwnerRequestGroup()
  {
    return _ownerRequestGroup;
  }

  virtual std::deque<std::pair<std::string, uint16_t> >& getNodes();

  virtual void setFileFilter(IntSequence seq);

  std::string generatePeerId() const;

  void setPeerIdPrefix(const std::string& peerIdPrefix)
  {
    _peerIdPrefix = peerIdPrefix;
  }

  // for unit test
  void setInfoHash(const unsigned char* infoHash);

  void setNumPieces(size_t numPieces)
  {
    this->numPieces = numPieces;
  }
   
  void setOwnerRequestGroup(RequestGroup* owner)
  {
    _ownerRequestGroup = owner;
  }

  void setRandomizer(const SharedHandle<Randomizer>& randomizer);

  // Sets file path for spcified index. index starts from 1. The index
  // is the same used in BtContext::setFileFilter().  Please note that
  // path is not the actual file path. The actual file path is
  // getDir()+"/"+path.
  void setFilePathWithIndex(size_t index, const std::string& path);

  friend std::ostream& operator<<(std::ostream& o, const DefaultBtContext& ctx);

  static const std::string DEFAULT_PEER_ID_PREFIX;
};

typedef SharedHandle<DefaultBtContext> DefaultBtContextHandle;

} // namespace aria2

#endif // _D_DEFAULT_BT_CONTEXT_H_
