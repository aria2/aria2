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
#include "Dictionary.h"
#include "Option.h"
#include <deque>
#include <map>
#include <string>

using namespace std;

#define INFO_HASH_LENGTH 20
#define DEFAULT_ANNOUNCE_INTERVAL 300
#define DEFAULT_ANNOUNCE_MIN_INTERVAL 300
#define MAX_PEERS 55
#define MAX_PEER_UPDATE 15
#define MAX_PEER_LIST_SIZE 250
#define END_GAME_PIECE_NUM 20
#define MAX_PEER_ERROR 5

class FileEntry {
public:
  string path;
  long long int length;
  long long int offset;
  bool extracted;
  bool requested;
  FileEntry(string path, long long int length, long long int offset):
    path(path), length(length), offset(offset),
    extracted(false), requested(true) {}
  ~FileEntry() {}
};

typedef deque<Peer*> Peers;
typedef multimap<int, int> Haves;
typedef deque<FileEntry> MultiFileEntries;
typedef deque<Piece> UsedPieces;
typedef deque<int> PieceIndexes;

class TorrentMan {
private:
  Peers peers;
  BitfieldMan* bitfield;
  unsigned char infoHash[INFO_HASH_LENGTH];
  deque<string> pieceHashes;
  int peerEntryIdCounter;
  int cuidCounter;
  long long int totalLength;
  long long int downloadLength;
  long long int uploadLength;
  long long int preDownloadLength;
  long long int preUploadLength;
  int deltaDownloadLength;
  int deltaUploadLength;
  int fileMode;
  string storeDir;
  int port;
  Haves haves;
  UsedPieces usedPieces;
  Directory* multiFileTopDir;
  MultiFileEntries multiFileEntries;
  bool setupComplete;

  FILE* openSegFile(string segFilename, string mode) const;
  void read(FILE* file);

  Piece findUsedPiece(int index) const;
  void addUsedPiece(const Piece& piece);
  void deleteUsedPiece(const Piece& piece);
  int deleteUsedPiecesByFillRate(int fillRate, int toDelete);
  void reduceUsedPieces(int max);
  void readFileEntry(const Dictionary* infoDic, const string& defaultName);
public:
  int pieceLength;
  int pieces;
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
  const Option* option;

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
  void setupDiskWriter();

  string getPieceHash(int index) const;

  void advertisePiece(int cuid, int index) {
    Haves::value_type vt(cuid, index);
    haves.insert(vt);
  }

  PieceIndexes getAdvertisedPieceIndexes(int myCuid) const {
    PieceIndexes indexes;
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

  long long int getTotalLength() const { return totalLength; }
  void setTotalLength(long long int length) { totalLength = length; }

  void addDeltaDownloadLength(int length) { deltaDownloadLength += length; }
  int getDeltaDownloadLength() const { return deltaDownloadLength; }
  void resetDeltaDownloadLength() { deltaDownloadLength = 0; }

  void addDeltaUploadLength(int length) { deltaUploadLength += length; }
  int getDeltaUploadLength() const { return deltaUploadLength; }
  void resetDeltaUploadLength() { deltaUploadLength = 0; }

  void addDownloadLength(int deltaLength) { downloadLength += deltaLength; }
  long long int getDownloadLength() const { return downloadLength; }
  void setDownloadLength(long long int length) { downloadLength = length; }

  void addUploadLength(int deltaLength) { uploadLength += deltaLength; }
  long long int getUploadLength() const { return uploadLength; }
  void setUploadLength(long long int length) { uploadLength = length; }

  long long int getSessionDownloadLength() const {
    return downloadLength-preDownloadLength;
  }
  long long int getSessionUploadLength() const {
    return uploadLength-preUploadLength;
  }

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

  void copySingleFile() const;
  void splitMultiFile();
  void fixFilename();
  void deleteTempFile() const;

  void setPort(int port) { this->port = port; }
  int getPort() const { return port; }

  int countUsedPiece() const { return usedPieces.size(); }
  int countAdvertisedPiece() const { return haves.size(); }

  void readFileEntryFromMetaInfoFile(const string& metaInfoFile);
  const MultiFileEntries& getMultiFileEntries() const;
  string getName() const;

  //bool unextractedFileEntryExists() const;
  
  void setAllMultiFileRequestedState(bool state);
  void finishPartialDownloadingMode();
  bool isPartialDownloadingMode() const;

  void setFileEntriesToDownload(const Strings& filePaths);

  long long int getCompletedLength() const;
  long long int getPartialTotalLength() const;

  void onDownloadComplete();

  enum FILE_MODE {
    SINGLE,
    MULTI
  };
};

#endif // _D_TORRENT_MAN_H_
