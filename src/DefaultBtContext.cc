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
#include "DefaultBtContext.h"
#include "MetaFileUtil.h"
#include "Dictionary.h"
#include "List.h"
#include "Data.h"
#include "DlAbortEx.h"
#include "ShaVisitor.h"
#include "Util.h"
#include "MessageDigestHelper.h"
#include "a2netcompat.h"
#include <libgen.h>

DefaultBtContext::DefaultBtContext():_peerIdPrefix("-aria2-") {}

DefaultBtContext::~DefaultBtContext() {}

string DefaultBtContext::generatePeerId() const {
  string peerId = _peerIdPrefix;
  peerId += Util::randomAlpha(20-_peerIdPrefix.size());
  if(peerId.size() > 20) {
    peerId.erase(20);
  }
  return peerId;
}

const unsigned char* DefaultBtContext::getInfoHash() const {
  return infoHash;
}

int32_t DefaultBtContext::getInfoHashLength() const {
  return INFO_HASH_LENGTH;
}

string DefaultBtContext::getInfoHashAsString() const {
  return infoHashString;
}

void DefaultBtContext::clear() {
  memset(infoHash, 0, INFO_HASH_LENGTH);
  infoHashString = "";
  pieceHashes.clear();
  fileEntries.clear();
  totalLength = 0;
  pieceLength = 0;
  fileMode = BtContext::SINGLE;
  numPieces = 0;
  name = "";
  announceTiers.clear();
}

void DefaultBtContext::extractPieceHash(const unsigned char* hashData,
					int32_t hashDataLength,
					int32_t hashLength) {
  assert(hashDataLength > 0);
  assert(hashLength > 0);
  int32_t numPieces = hashDataLength/hashLength;
  assert(numPieces > 0);

  for(int32_t i = 0; i < numPieces; i++) {
    pieceHashes.push_back(Util::toHex(&hashData[i*hashLength],
				      hashLength));
  }
}

void DefaultBtContext::extractFileEntries(Dictionary* infoDic,
					  const string& defaultName) {
  // TODO use dynamic_cast
  Data* nameData = (Data*)infoDic->get("name");
  if(nameData) {
    name = nameData->toString();
  } else {
    char* basec = strdup(defaultName.c_str());
    name = string(basename(basec))+".file";
    free(basec);
  }
  // TODO use dynamic_cast
  List* files = (List*)infoDic->get("files");
  if(files) {
    int64_t length = 0;
    int64_t offset = 0;
    // multi-file mode
    fileMode = BtContext::MULTI;
    const MetaList& metaList = files->getList();
    for(MetaList::const_iterator itr = metaList.begin();
	itr != metaList.end(); itr++) {
      Dictionary* fileDic = (Dictionary*)(*itr);
      // TODO use dynamic_cast
      Data* lengthData = (Data*)fileDic->get("length");
      length += lengthData->toLLInt();
      // TODO use dynamic_cast
      List* pathList = (List*)fileDic->get("path");
      const MetaList& paths = pathList->getList();
      string path;
      for(int32_t i = 0; i < (int32_t)paths.size()-1; i++) {
	Data* subpath = (Data*)paths[i];
	path += subpath->toString()+"/";
      }
      // TODO use dynamic_cast
      Data* lastPath = (Data*)paths.back();
      path += lastPath->toString();

      FileEntryHandle fileEntry(new FileEntry(path,
					      lengthData->toLLInt(),
					      offset));
      fileEntries.push_back(fileEntry);
      offset += fileEntry->getLength();
    }
    totalLength = length;
  } else {
    // single-file mode;
    fileMode = BtContext::SINGLE;
    Data* length = (Data*)infoDic->get("length");
    totalLength = length->toLLInt();
    FileEntryHandle fileEntry(new FileEntry(name, totalLength, 0));
    fileEntries.push_back(fileEntry);
  }
}

void DefaultBtContext::extractAnnounce(Data* announceData) {
  Strings urls;
  urls.push_back(announceData->toString());
  announceTiers.push_back(AnnounceTierHandle(new AnnounceTier(urls)));
}

void DefaultBtContext::extractAnnounceList(List* announceListData) {
  for(MetaList::const_iterator itr = announceListData->getList().begin();
      itr != announceListData->getList().end(); itr++) {
    const List* elem = (List*)*itr;
    Strings urls;
    for(MetaList::const_iterator elemItr = elem->getList().begin();
	elemItr != elem->getList().end(); elemItr++) {
      const Data* data = (Data*)*elemItr;
      urls.push_back(data->toString());
    }
    if(urls.size()) {
      AnnounceTierHandle tier(new AnnounceTier(urls));
      announceTiers.push_back(tier);
    }
  }
}

void DefaultBtContext::load(const string& torrentFile) {
  clear();
  MetaEntry* rootEntry = MetaFileUtil::parseMetaFile(torrentFile);
  if(!dynamic_cast<Dictionary*>(rootEntry)) {
    throw new DlAbortEx("torrent file does not contain a root dictionary .");
  }
  SharedHandle<Dictionary> rootDic =
    SharedHandle<Dictionary>((Dictionary*)rootEntry);
  Dictionary* infoDic = (Dictionary*)rootDic->get("info");
  // retrieve infoHash
  ShaVisitor v;
  infoDic->accept(&v);
  int32_t len;
  v.getHash(infoHash, len);
  infoHashString = Util::toHex(infoHash, INFO_HASH_LENGTH);
  // calculate the number of pieces
  Data* pieceHashData = (Data*)infoDic->get("pieces");
  numPieces = pieceHashData->getLen()/PIECE_HASH_LENGTH;
  // retrieve piece length
  Data* pieceLengthData = (Data*)infoDic->get("piece length");
  pieceLength = pieceLengthData->toInt();
  // retrieve piece hashes
  extractPieceHash((unsigned char*)pieceHashData->getData(),
		   pieceHashData->getLen(),
		   PIECE_HASH_LENGTH);
  // retrieve file entries
  extractFileEntries(infoDic, torrentFile);
  // retrieve announce
  Data* announceData = (Data*)rootDic->get("announce");
  List* announceListData = (List*)rootDic->get("announce-list");
  if(announceListData) {
    extractAnnounceList(announceListData);
  } else if(announceData) {
    extractAnnounce(announceData);
  }
  if(!announceTiers.size()) {
    throw new DlAbortEx("No announce URL found.");
  }
}

string DefaultBtContext::getPieceHash(int32_t index) const {
  if(index < 0 || numPieces <= index) {
    return "";
  }
  return pieceHashes[index];
}

int64_t DefaultBtContext::getTotalLength() const {
  return totalLength;
}

BtContext::FILE_MODE DefaultBtContext::getFileMode() const {
  return fileMode;
}

FileEntries DefaultBtContext::getFileEntries() const {
  return fileEntries;
}

AnnounceTiers DefaultBtContext::getAnnounceTiers() const {
  return announceTiers;
}

string DefaultBtContext::getName() const {
  return name;
}

int32_t DefaultBtContext::getPieceLength() const {
  return pieceLength;
}

int32_t DefaultBtContext::getNumPieces() const {
  return numPieces;
}

Integers DefaultBtContext::computeFastSet(const string& ipaddr, int32_t fastSetSize)
{
  Integers fastSet;
  struct in_addr saddr;
  if(inet_aton(ipaddr.c_str(), &saddr) == 0) {
    abort();
  }
  unsigned char tx[24];
  memcpy(tx, (void*)&saddr.s_addr, 4);
  if((tx[0] & 0x80) == 0 || (tx[0] & 0x40) == 0) {
    tx[2] = 0x00;
    tx[3] = 0x00;
  } else {
    tx[3] = 0x00;
  }
  memcpy(tx+4, infoHash, 20);
  unsigned char x[20];
  MessageDigestHelper::digest(x, sizeof(x), "sha1", tx, 24);
  while((int32_t)fastSet.size() < fastSetSize) {
    for(int32_t i = 0; i < 5 && (int32_t)fastSet.size() < fastSetSize; i++) {
      int32_t j = i*4;
      uint32_t ny;
      memcpy(&ny, x+j, 4);
      uint32_t y = ntohl(ny);
      int32_t index = y%numPieces;
      if(find(fastSet.begin(), fastSet.end(), index) == fastSet.end()) {
	fastSet.push_back(index);
      }
    }
    unsigned char temp[20];
    MessageDigestHelper::digest(temp, sizeof(temp), "sha1", x, sizeof(x));
    memcpy(x, temp, sizeof(x));
  }
  return fastSet;
}
