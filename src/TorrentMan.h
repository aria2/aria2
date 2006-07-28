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
#include "Dictionary.h"
#include "Option.h"
#include "FileEntry.h"
#include "DiskAdaptor.h"
#include "Request.h"
#include "Time.h"
#include <deque>
#include <map>
#include <string>
#include <algorithm>

using namespace std;

#define INFO_HASH_LENGTH 20
#define PEER_ID_LENGTH 20
#define DEFAULT_ANNOUNCE_INTERVAL 1800
#define DEFAULT_ANNOUNCE_MIN_INTERVAL 1800
#define MAX_PEERS 55
#define MIN_PEERS 15
#define MAX_PEER_LIST_SIZE 100
#define END_GAME_PIECE_NUM 20
#define MAX_PEER_ERROR 5

class HaveEntry {
public:
  int cuid;
  int index;
  Time registeredTime;
  HaveEntry(int cuid, int index):
    cuid(cuid),
    index(index) {}
};

typedef deque<PeerHandle> Peers;
typedef deque<HaveEntry> Haves;
typedef deque<int> PieceIndexes;
typedef deque<Piece> Pieces;

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
  Pieces usedPieces;
  bool setupComplete;
  const Logger* logger;
  Peers activePeers;
  bool halt;

  FILE* openSegFile(const string& segFilename, const string& mode) const;
  void read(FILE* file);

  Piece findUsedPiece(int index) const;
  void addUsedPiece(const Piece& piece);
  void deleteUsedPiece(const Piece& piece);
  int deleteUsedPiecesByFillRate(int fillRate, int toDelete);
  void reduceUsedPieces(int max);
  void readFileEntry(FileEntries& fileEntries, Directory** pTopDir, const Dictionary* infoDic, const string& defaultName);
  void setFileFilter(const Strings& filePaths);
  void setupInternal1(const string& metaInfoFile);
  void setupInternal2();
  Piece checkOutPiece(int index);
public:
  int pieceLength;
  int pieces;
  // TODO type char* would be better
  string peerId;
  string key;
  string announce;
  string trackerId;
  string name;
  int interval;
  int minInterval;
  int complete;
  int incomplete;
  int connections;
  // The number of tracker request command currently in the command queue.
  int trackers;
  // tracker request
  Request* req;
public:
  TorrentMan();
  ~TorrentMan();

  DiskAdaptor* diskAdaptor;
  const Option* option;

  int getNewCuid() { return ++cuidCounter; }

  // TODO do not use this method
  void updatePeers(const Peers& peers);
  bool addPeer(const PeerHandle& peer);
  //void updatePeer(const Peer* peer);
  const Peers& getPeers() const { return peers; }
  PeerHandle getPeer() const;
  bool isPeerAvailable() const;
  void deleteUnusedPeer(int delSize);

  bool hasMissingPiece(const PeerHandle& peer) const;
  int getMissingPieceIndex(const PeerHandle& peer) const;
  int getMissingFastPieceIndex(const PeerHandle& peer) const;
  Piece getMissingPiece(const PeerHandle& peer);
  Piece getMissingFastPiece(const PeerHandle& peer);
  void completePiece(const Piece& piece);
  void cancelPiece(const Piece& piece);
  void updatePiece(const Piece& piece);
  void syncPiece(Piece& piece);
  bool hasPiece(int index) const;
  void initBitfield();
  bool isEndGame() const;
  bool downloadComplete() const;
  bool hasAllPieces() const;
  void setBitfield(unsigned char* bitfield, int len);
  const unsigned char* getBitfield() const {
    return bitfield->getBitfield();
  }
  int getBitfieldLength() const { return bitfield->getBitfieldLength(); }
  int getPieceLength(int index) const {
    return bitfield->getBlockLength(index);
  }
  int getPieceLength() const { return bitfield->getBlockLength(); }

  void setInfoHash(const unsigned char* infoHash) {
    memcpy(this->infoHash, infoHash, INFO_HASH_LENGTH);
  }
  const unsigned char* getInfoHash() const {
    return infoHash;
  }

  void setup(const string& metaInfoFile, const Strings& targetFilePaths);
  void setup(const string& metaInfoFile, const Integers& targetFileIndexes);

  string getPieceHash(int index) const;

  void advertisePiece(int cuid, int index);

  PieceIndexes getAdvertisedPieceIndexes(int myCuid, const Time& lastCheckTime) const;

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
  void setStoreDir(const string& dir) { storeDir = dir; }

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

  FileEntries readFileEntryFromMetaInfoFile(const string& metaInfoFile);
  string getName() const;

  void finishSelectiveDownloadingMode();
  bool isSelectiveDownloadingMode() const;

  long long int getCompletedLength() const;
  long long int getSelectedTotalLength() const;

  void onDownloadComplete();

  void addActivePeer(const PeerHandle& peer) {
    activePeers.push_back(peer);
  }

  Peers& getActivePeers() { return this->activePeers; }

  void deleteActivePeer(const PeerHandle& peer) {
    Peers::iterator itr = find(activePeers.begin(), activePeers.end(), peer);
    if(itr != activePeers.end()) {
      activePeers.erase(itr);
    }
  }

  bool isHalt() const { return halt; }
  void setHalt(bool halt) {
    this->halt = halt;
  }

  enum FILE_MODE {
    SINGLE,
    MULTI
  };
};

#endif // _D_TORRENT_MAN_H_
