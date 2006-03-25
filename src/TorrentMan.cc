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
#include "TorrentMan.h"
#include "Dictionary.h"
#include "List.h"
#include "ShaVisitor.h"
#include "Util.h"
#include "MetaFileUtil.h"
#include "DlAbortEx.h"
#include "File.h"
#include "message.h"
#include "PreAllocationDiskWriter.h"
#include <errno.h>
#include <libgen.h>
#include <string.h>

TorrentMan::TorrentMan():bitfield(NULL),
			 peerEntryIdCounter(0), cuidCounter(0),
			 downloadedSize(0), uploadedSize(0),
			 deltaDownload(0), deltaUpload(0),
			 storeDir("."),
			 multiFileTopDir(NULL),
			 interval(DEFAULT_ANNOUNCE_INTERVAL),
			 minInterval(DEFAULT_ANNOUNCE_MIN_INTERVAL),
			 complete(0), incomplete(0),
			 connections(0), diskWriter(NULL) {}

TorrentMan::~TorrentMan() {
  if(bitfield != NULL) {
    delete bitfield;
  }
  if(multiFileTopDir != NULL) {
    delete multiFileTopDir;
  }
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
    delete *itr;
  }
  if(diskWriter != NULL) {
    delete diskWriter;
  }
}

// TODO do not use this method in application code
void TorrentMan::updatePeers(const Peers& peers) {
  this->peers = peers;
}

bool TorrentMan::addPeer(Peer* peer, bool duplicate) {
  if(duplicate) {
    for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
      Peer* p = *itr;
      if(p->ipaddr == peer->ipaddr && p->port == peer->port && p->error > 0) {
	return false;
      }
    }
  } else {
    if(peers.size() >= MAX_PEER_LIST_SIZE) {
      deleteOldErrorPeers(100);
      if(peers.size() >= MAX_PEER_LIST_SIZE) {
	return false;
      }
    }
    for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
      Peer* p = *itr;
      if(p->ipaddr == peer->ipaddr && p->port == peer->port) {
	return false;
      }
    }
  }
  ++peerEntryIdCounter;
  peer->entryId = peerEntryIdCounter;
  peers.push_back(peer);
  return true;
}

/*
void TorrentMan::updatePeer(const Peer& peer) {
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
    Peer& p = *itr;
    if(p.eid == peer.eid) {
      p = peer;
      break;
    }
  }
}
*/

bool TorrentMan::isPeerAvailable() const {
  return getPeer() != Peer::nullPeer;
}

int TorrentMan::deleteOldErrorPeers(int maxNum) {
  int counter = 0;
  for(Peers::iterator itr = peers.begin(); itr != peers.end();) {
    Peer* p = *itr;
    if(p->error != 0 && p->cuid == 0) {
      delete p;
      itr = peers.erase(itr);
      counter++;
      if(maxNum <= counter) {
	break;
      }
    } else {
      itr++;
    }
  }
  return counter;
}

Peer* TorrentMan::getPeer() const {
  for(Peers::const_iterator itr = peers.begin(); itr != peers.end(); itr++) {
    Peer* p = *itr;
    if(p->cuid == 0 && p->error == 0) {
      return p;
    }
  }
  return Peer::nullPeer;
}

bool TorrentMan::isEndGame() const {
  return bitfield->countMissingBlock() <= END_GAME_PIECE_NUM;
}

Piece TorrentMan::getMissingPiece(const Peer* peer) {
  int index = -1;
  if(isEndGame()) {
    index = bitfield->getMissingIndex(peer->getBitfield(), peer->getBitfieldLength());
  } else {
    index = bitfield->getMissingUnusedIndex(peer->getBitfield(), peer->getBitfieldLength());
  }
  if(index == -1) {
    return Piece::nullPiece;
  }
  bitfield->setUseBit(index);

  Piece piece = findUsedPiece(index);
  if(Piece::isNull(piece)) {
    Piece piece(index, bitfield->getBlockLength(index));
    addUsedPiece(piece);
    return piece;
  } else {
    return piece;
  }
}

int TorrentMan::deleteUsedPiecesByFillRate(int fillRate, int toDelete) {
  int deleted = 0;
  for(UsedPieces::iterator itr = usedPieces.begin();
      itr != usedPieces.end() && deleted < toDelete;) {
    Piece& piece = *itr;
    if(!bitfield->isUseBitSet(piece.getIndex()) &&
       piece.countCompleteBlock() <= piece.countBlock()*(fillRate/100.0)) {
      logger->debug("deleting used piece index=%d, fillRate(%%)=%d<=%d",
		    piece.getIndex(),
		    (piece.countCompleteBlock()*100)/piece.countBlock(),
		    fillRate);
      itr = usedPieces.erase(itr);
      deleted++;
    } else {
      itr++;
    }
  }
  return deleted;
}

void TorrentMan::reduceUsedPieces(int max) {
  int toDelete = usedPieces.size()-max;
  if(toDelete <= 0) {
    return;
  }
  int fillRate = 10;
  while(fillRate < 50) {
    int deleted = deleteUsedPiecesByFillRate(fillRate, toDelete);
    if(deleted == 0) {
      break;
    }
    toDelete -= deleted;
    fillRate += 10;
  }
}

void TorrentMan::addUsedPiece(const Piece& piece) {
  usedPieces.push_back(piece);
}

Piece TorrentMan::findUsedPiece(int index) const {
  for(UsedPieces::const_iterator itr = usedPieces.begin(); itr != usedPieces.end(); itr++) {
    const Piece& piece = *itr;
    if(piece.getIndex() == index) {
      return piece;
    }
  }
  return Piece::nullPiece;
}

void TorrentMan::deleteUsedPiece(const Piece& piece) {
  if(Piece::isNull(piece)) {
    return;
  }
  for(UsedPieces::iterator itr = usedPieces.begin(); itr != usedPieces.end(); itr++) {
    if(itr->getIndex() == piece.getIndex()) {
      usedPieces.erase(itr);
      break;
    }
  }  
}

void TorrentMan::completePiece(const Piece& piece) {
  if(Piece::isNull(piece)) {
    return;
  }
  if(!hasPiece(piece.getIndex())) {
    addDownloadedSize(piece.getLength());
  }
  bitfield->setBit(piece.getIndex());
  bitfield->unsetUseBit(piece.getIndex());
  deleteUsedPiece(piece);
  if(!isEndGame()) {
    reduceUsedPieces(100);
  }
}

void TorrentMan::cancelPiece(const Piece& piece) {
  if(Piece::isNull(piece)) {
    return;
  }
  bitfield->unsetUseBit(piece.getIndex());
  if(!isEndGame()) {
    if(piece.countCompleteBlock() == 0) {
      deleteUsedPiece(piece);
    }
  }
}

void TorrentMan::updatePiece(const Piece& piece) {
  if(Piece::isNull(piece)) {
    return;
  }
  for(UsedPieces::iterator itr = usedPieces.begin(); itr != usedPieces.end(); itr++) {
    if(itr->getIndex() == piece.getIndex()) {
      *itr = piece;
      break;
    }
  }
}

void TorrentMan::syncPiece(Piece& piece) {
  if(Piece::isNull(piece)) {
    return;
  }
  for(UsedPieces::iterator itr = usedPieces.begin(); itr != usedPieces.end(); itr++) {
    if(itr->getIndex() == piece.getIndex()) {
      piece = *itr;
      return;
    }
  }
  // hasPiece(piece.getIndex()) is true, then set all bit of
  // piece.bitfield to 1
  if(hasPiece(piece.getIndex())) {
    piece.setAllBlock();
  }
}

void TorrentMan::initBitfield() {
  if(bitfield != NULL) {
    delete bitfield;
  }
  bitfield = new BitfieldMan(pieceLength, totalSize);
}

void TorrentMan::setBitfield(unsigned char* bitfield, int bitfieldLength) {
  if(this->bitfield == NULL) {
    initBitfield();
  }
  this->bitfield->setBitfield(bitfield, bitfieldLength);
}

bool TorrentMan::downloadComplete() const {
  return bitfield->isAllBitSet();
}

void TorrentMan::setup(string metaInfoFile) {
  peerId = "-A2****-";
  for(int i = 0; i < 12; i++) {
    peerId += Util::itos((int)(((double)10)*random()/(RAND_MAX+1.0)));
  }

  uploadedSize = 0;
  downloadedSize = 0;
  Dictionary* topDic = (Dictionary*)MetaFileUtil::parseMetaFile(metaInfoFile);
  const Dictionary* infoDic = (const Dictionary*)topDic->get("info");
  ShaVisitor v;
  infoDic->accept(&v);
  unsigned char md[20];
  int len;
  v.getHash(md, len);
  setInfoHash(md);

  Data* topName = (Data*)infoDic->get("name");
  if(topName != NULL) {
    name = topName->toString();
  } else {
    char* basec = strdup(metaInfoFile.c_str());
    name = string(basename(basec))+".file";
    free(basec);
  }
  List* files = (List*)infoDic->get("files");
  if(files == NULL) {
    // single-file mode;
    setFileMode(SINGLE);
    Data* length = (Data*)infoDic->get("length");
    totalSize = length->toLLInt();
  } else {
    long long int length = 0;
    // multi-file mode
    setFileMode(MULTI);
    multiFileTopDir = new Directory(name);
    const MetaList& metaList = files->getList();
    for(MetaList::const_iterator itr = metaList.begin(); itr != metaList.end();
	itr++) {
      Dictionary* fileDic = (Dictionary*)(*itr);
      Data* lengthData = (Data*)fileDic->get("length");
      length += lengthData->toLLInt();
      List* path = (List*)fileDic->get("path");
      const MetaList& paths = path->getList();
      Directory* parentDir = multiFileTopDir;
      string filePath = name;
      for(int i = 0; i < (int)paths.size()-1; i++) {
	Data* subpath = (Data*)paths.at(i);
	Directory* dir = new Directory(subpath->toString());
	parentDir->addFile(dir);
	parentDir = dir;
	filePath.append("/").append(subpath->toString());
      }
      Data* lastpath = (Data*)paths.back();
      filePath.append("/").append(lastpath->toString());
      FileEntry fileEntry(filePath, lengthData->toLLInt());
      multiFileEntries.push_back(fileEntry);
    }
    totalSize = length;
  }
  announce = ((Data*)topDic->get("announce"))->toString();
  pieceLength = ((Data*)infoDic->get("piece length"))->toInt();
  pieces = totalSize/pieceLength+(totalSize%pieceLength ? 1 : 0);
  Data* piecesHashData = (Data*)infoDic->get("pieces");
  if(piecesHashData->getLen() != pieces*20) {
    throw new DlAbortEx("the number of pieces is wrong.");
  }
  for(int index = 0; index < pieces; index++) {
    string hex = Util::toHex((unsigned char*)&piecesHashData->getData()[index*20], 20);
    pieceHashes.push_back(hex);
    logger->debug("piece #%d, hash:%s", index, hex.c_str());
  }

  initBitfield();
  delete topDic;

  diskWriter = new PreAllocationDiskWriter(totalSize);
  if(segmentFileExists()) {
    load();
    diskWriter->openExistingFile(getTempFilePath());
  } else {
    diskWriter->initAndOpenFile(getTempFilePath());
  }
}

bool TorrentMan::hasPiece(int index) const {
  return bitfield->isBitSet(index);
}

string TorrentMan::getPieceHash(int index) const {
  return pieceHashes.at(index);
}

string TorrentMan::getFilePath() const {
  return storeDir+"/"+name;
}

string TorrentMan::getTempFilePath() const {
  return getFilePath()+".a2tmp";
}

string TorrentMan::getSegmentFilePath() const {
  return getFilePath()+".aria2";
}

bool TorrentMan::segmentFileExists() const {
  string segFilename = getSegmentFilePath();
  File f(segFilename);
  if(f.isFile()) {
    logger->info(MSG_SEGMENT_FILE_EXISTS, segFilename.c_str());
    return true;
  } else {
    logger->info(MSG_SEGMENT_FILE_DOES_NOT_EXIST, segFilename.c_str());
    return false;
  }
}

FILE* TorrentMan::openSegFile(string segFilename, string mode) const {
  FILE* segFile = fopen(segFilename.c_str(), mode.c_str());
  if(segFile == NULL) {
    throw new DlAbortEx(strerror(errno));
  }
  return segFile;
}

void TorrentMan::load() {
  string segFilename = getSegmentFilePath();
  logger->info(MSG_LOADING_SEGMENT_FILE, segFilename.c_str());
  FILE* segFile = openSegFile(segFilename, "r+");
  read(segFile);
  fclose(segFile);
  logger->info(MSG_LOADED_SEGMENT_FILE);
}

void TorrentMan::read(FILE* file) {
  assert(file != NULL);
  unsigned char savedInfoHash[INFO_HASH_LENGTH];
  if(fread(savedInfoHash, INFO_HASH_LENGTH, 1, file) < 1) {
    throw new DlAbortEx(strerror(errno));
  }
  if(Util::toHex(savedInfoHash, INFO_HASH_LENGTH) != Util::toHex(infoHash, INFO_HASH_LENGTH)) {
    throw new DlAbortEx("info hash mismatch");
  }
  unsigned char* savedBitfield = new unsigned char[bitfield->getBitfieldLength()];
  try {
    if(fread(savedBitfield, bitfield->getBitfieldLength(), 1, file) < 1) {
      throw new DlAbortEx(strerror(errno));
    }
    setBitfield(savedBitfield, bitfield->getBitfieldLength());
    if(fread(&downloadedSize, sizeof(downloadedSize), 1, file) < 1) {
      throw new DlAbortEx(strerror(errno));
    }
    if(fread(&uploadedSize, sizeof(uploadedSize), 1, file) < 1) {
      throw new DlAbortEx(strerror(errno));
    }
    delete [] savedBitfield;
  } catch(Exception* ex) {
    delete [] savedBitfield;
    throw;
  }
}

void TorrentMan::save() const {
  if(downloadedSize == 0) {
    return;
  }
  string segFilename = getSegmentFilePath();
  logger->info(MSG_SAVING_SEGMENT_FILE, segFilename.c_str());
  FILE* file = openSegFile(segFilename, "w");
  if(fwrite(infoHash, INFO_HASH_LENGTH, 1, file) < 1) {
    throw new DlAbortEx(strerror(errno));
  }
  if(fwrite(bitfield->getBitfield(), bitfield->getBitfieldLength(), 1, file) < 1) {
    throw new DlAbortEx(strerror(errno));
  }
  if(fwrite(&downloadedSize, sizeof(downloadedSize), 1, file) < 1) {
    throw new DlAbortEx(strerror(errno));
  }
  if(fwrite(&uploadedSize, sizeof(uploadedSize), 1, file) < 1) {
    throw new DlAbortEx(strerror(errno));
  }
  fclose(file);
  logger->info(MSG_SAVED_SEGMENT_FILE);
}

void TorrentMan::remove() const {
  if(segmentFileExists()) {
    File f(getSegmentFilePath());
    f.remove();
  }
}

void TorrentMan::fixFilename() const {
  if(fileMode == SINGLE) {
    copySingleFile();
  } else {
    splitMultiFile();
  }
}

void TorrentMan::copySingleFile() const {
  logger->info("writing file %s", getFilePath().c_str());
  Util::fileCopy(getFilePath(), getTempFilePath());
}

void TorrentMan::splitMultiFile() const {
  logger->info("creating directories");
  multiFileTopDir->createDir(storeDir, true);
  long long int offset = 0;
  for(MultiFileEntries::const_iterator itr = multiFileEntries.begin();
      itr != multiFileEntries.end(); itr++) {
    string dest = storeDir+"/"+itr->path;
    logger->info("writing file %s", dest.c_str());
    Util::rangedFileCopy(dest, getTempFilePath(), offset, itr->length);
    offset += itr->length;
  }
}

void TorrentMan::deleteTempFile() const {
  unlink(getTempFilePath().c_str());
}
