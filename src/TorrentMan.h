/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_TORRENT_MAN_H_
#define _D_TORRENT_MAN_H_

#include "Peer.h"
#include "common.h"
#include "Logger.h"
#include "BitfieldMan.h"
#include "DiskWriter.h"
#include "Piece.h"
#include "Directory.h"
#include <deque>
#include <vector>
#include <map>
#include <string>

using namespace std;

#define DEFAULT_BLOCK_LEN 16*1024;
#define MAX_BLOCK_LEN 128*1024;

#define INFO_HASH_LENGTH 20

#define IS_NULL_PIECE(X) (X.index == 0 && X.length == 0)

#define DEFAULT_ANNOUNCE_INTERVAL 1800
#define DEFAULT_ANNOUNCE_MIN_INTERVAL 120
#define MAX_PEERS 55
#define MAX_PEER_LIST_SIZE 250
#define END_GAME_PIECE_NUM 20

class FileEntry {
public:
  string path;
  long long int length;
  FileEntry(string path, long long int length):path(path), length(length) {}
  ~FileEntry() {}
};

typedef deque<Peer*> Peers;
typedef multimap<int, int> Haves;
typedef vector<FileEntry> MultiFileEntries;
typedef deque<Piece> UsedPieces;

class TorrentMan {
private:
  Peers peers;
  BitfieldMan* bitfield;
  unsigned char infoHash[INFO_HASH_LENGTH];
  vector<string> pieceHashes;
  int peerEntryIdCounter;
  int cuidCounter;
  long long int downloadedSize;
  long long int uploadedSize;
  int deltaDownload;
  int deltaUpload;
  int fileMode;
  string storeDir;
  int port;
  Haves haves;
  UsedPieces usedPieces;
  Directory* multiFileTopDir;
  MultiFileEntries multiFileEntries;

  FILE* openSegFile(string segFilename, string mode) const;
  void read(FILE* file);

  Piece findUsedPiece(int index) const;
  void addUsedPiece(const Piece& piece);
  void deleteUsedPiece(const Piece& piece);
  int deleteUsedPiecesByFillRate(int fillRate, int toDelete);
  void reduceUsedPieces(int max);
public:
  int pieceLength;
  int pieces;
  long long int totalSize;
  string peerId;
  string announce;
  string trackerId;
  string name;
  int interval;
  int minInterval;
  int complete;
  int incomplete;
  int connections;
public:
  TorrentMan();
  ~TorrentMan();

  const Logger* logger;
  DiskWriter* diskWriter;

  int getNewCuid() { return ++cuidCounter; }

  // TODO do not use this method
  void updatePeers(const Peers& peers);
  bool addPeer(Peer* peer, bool duplicate = false);
  //void updatePeer(const Peer* peer);
  const Peers& getPeers() const { return peers; }
  Peer* getPeer() const;
  bool isPeerAvailable() const;
  int deleteOldErrorPeers(int maxNum);

  Piece getMissingPiece(const Peer* peer);
  void completePiece(const Piece& piece);
  void cancelPiece(const Piece& piece);
  void updatePiece(const Piece& piece);
  void syncPiece(Piece& piece);
  bool hasPiece(int index) const;
  void initBitfield();
  bool isEndGame() const;
  bool downloadComplete() const;
  void setBitfield(unsigned char* bitfield, int len);
  const unsigned char* getBitfield() const {
    return bitfield->getBitfield();
  }
  int getBitfieldLength() const { return bitfield->getBitfieldLength(); }
  void setInfoHash(const unsigned char* infoHash) {
    memcpy(this->infoHash, infoHash, INFO_HASH_LENGTH);
  }
  const unsigned char* getInfoHash() const {
    return infoHash;
  }

  void setup(string metaInfoFile);

  string getPieceHash(int index) const;

  void advertisePiece(int cuid, int index) {
    Haves::value_type vt(cuid, index);
    haves.insert(vt);
  }

  vector<int> getAdvertisedPieceIndexes(int myCuid) const {
    vector<int> indexes;
    for(Haves::const_iterator itr = haves.begin(); itr != haves.end(); itr++) {
      const Haves::value_type& have = *itr;
      if(have.first == myCuid) {
	continue;
      }
      indexes.push_back(have.second);
    }
    return indexes;
  }

  void unadvertisePiece(int cuid) {
    haves.erase(cuid);
  }

  void addDeltaDownload(int size) { deltaDownload += size; }
  int getDeltaDownload() const { return deltaDownload; }
  void resetDeltaDownload() { deltaDownload = 0; }

  void addDeltaUpload(int size) { deltaUpload += size; }
  int getDeltaUpload() const { return deltaUpload; }
  void resetDeltaUpload() { deltaUpload = 0; }

  void addDownloadedSize(int size) { downloadedSize += size; }
  long long int getDownloadedSize() const { return downloadedSize; }
  void setDownloadedSize(long long int size) { downloadedSize = size; }

  void addUploadedSize(int size) { uploadedSize += size; }
  long long int getUploadedSize() const { return uploadedSize; }
  void setUploadedSize(long long int size) { uploadedSize = size; }

  void setFileMode(int mode) {
    fileMode = mode;
  }

  int getFileMode() const {
    return fileMode;
  }

  string getStoreDir() const { return storeDir; }
  void setStoreDir(string dir) { storeDir = dir; }

  string getFilePath() const;
  string getTempFilePath() const;
  string getSegmentFilePath() const;

  bool segmentFileExists() const;  
  void load();
  void save() const;
  void remove() const;

  void renameSingleFile() const;
  void splitMultiFile() const;
  void fixFilename() const;

  void setPort(int port) { this->port = port; }
  int getPort() const { return port; }

  int countUsedPiece() const { return usedPieces.size(); }
  int countAdvertisedPiece() const { return haves.size(); }

  enum FILE_MODE {
    SINGLE,
    MULTI
  };
};

#endif // _D_TORRENT_MAN_H_
