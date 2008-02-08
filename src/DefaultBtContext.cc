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
#include "BencodeVisitor.h"
#include "Util.h"
#include "MessageDigestHelper.h"
#include "a2netcompat.h"
#include "AnnounceTier.h"
#include "SimpleRandomizer.h"
#include "LogFactory.h"
#include "Logger.h"
#include "FileEntry.h"
#include "message.h"
#include <cassert>
#include <cstring>
#include <ostream>
#include <functional>
#include <algorithm>

namespace aria2 {

DefaultBtContext::DefaultBtContext():_peerIdPrefix("-aria2-"),
				     _randomizer(SimpleRandomizer::getInstance()),
				     _ownerRequestGroup(0),
				     _logger(LogFactory::getInstance()) {}

DefaultBtContext::~DefaultBtContext() {}

std::string DefaultBtContext::generatePeerId() const {
  std::string peerId = _peerIdPrefix;
  peerId += Util::randomAlpha(20-_peerIdPrefix.size(), _randomizer);
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

std::string DefaultBtContext::getInfoHashAsString() const {
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
  _private = false;
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

void DefaultBtContext::extractFileEntries(const Dictionary* infoDic,
					  const std::string& defaultName,
					  const std::deque<std::string>& urlList) {
  const Data* nameData = dynamic_cast<const Data*>(infoDic->get("name"));
  if(nameData) {
    name = nameData->toString();
  } else {
    name = File(defaultName).getBasename()+".file";
  }
  const List* files = dynamic_cast<const List*>(infoDic->get("files"));
  if(files) {
    int64_t length = 0;
    int64_t offset = 0;
    // multi-file mode
    fileMode = BtContext::MULTI;
    const std::deque<MetaEntry*>& metaList = files->getList();
    for(std::deque<MetaEntry*>::const_iterator itr = metaList.begin();
	itr != metaList.end(); itr++) {
      const Dictionary* fileDic = dynamic_cast<const Dictionary*>((*itr));
      if(!fileDic) {
	continue;
      }
      const Data* lengthData = dynamic_cast<const Data*>(fileDic->get("length"));
      if(lengthData) {
	length += lengthData->toLLInt();
      } else {
	throw new DlAbortEx(MSG_SOMETHING_MISSING_IN_TORRENT, "file length");
      }
      const List* pathList = dynamic_cast<const List*>(fileDic->get("path"));
      if(!pathList) {
	throw new DlAbortEx(MSG_SOMETHING_MISSING_IN_TORRENT, "file path list");
      }
      const std::deque<MetaEntry*>& paths = pathList->getList();
      std::string path;
      for(int32_t i = 0; i < (int32_t)paths.size()-1; i++) {
	const Data* subpath = dynamic_cast<const Data*>(paths[i]);
	if(subpath) {
	  path += subpath->toString()+"/";
	} else {
	  throw new DlAbortEx(MSG_SOMETHING_MISSING_IN_TORRENT, "file path element");
	}
      }
      const Data* lastPath = dynamic_cast<const Data*>(paths.back());
      if(lastPath) {
	path += lastPath->toString();
      } else {
	throw new DlAbortEx(MSG_SOMETHING_MISSING_IN_TORRENT, "file path element");
      }

      std::deque<std::string> uris;
      std::transform(urlList.begin(), urlList.end(), std::back_inserter(uris),
		     std::bind2nd(std::plus<std::string>(), "/"+name+"/"+path));
      FileEntryHandle fileEntry(new FileEntry(path,
					      lengthData->toLLInt(),
					      offset,
					      uris));
      fileEntries.push_back(fileEntry);
      offset += fileEntry->getLength();
    }
    totalLength = length;
  } else {
    // single-file mode;
    fileMode = BtContext::SINGLE;
    const Data* length = dynamic_cast<const Data*>(infoDic->get("length"));
    if(length) {
      totalLength = length->toLLInt();
    } else {
      throw new DlAbortEx(MSG_SOMETHING_MISSING_IN_TORRENT, "file length");
    }
    FileEntryHandle fileEntry(new FileEntry(name, totalLength, 0, urlList));
    fileEntries.push_back(fileEntry);
  }
}

void DefaultBtContext::extractAnnounce(const Data* announceData) {
  std::deque<std::string> urls;
  urls.push_back(Util::trim(announceData->toString()));
  announceTiers.push_back(AnnounceTierHandle(new AnnounceTier(urls)));
}

void DefaultBtContext::extractAnnounceList(const List* announceListData) {
  for(std::deque<MetaEntry*>::const_iterator itr = announceListData->getList().begin();
      itr != announceListData->getList().end(); itr++) {
    const List* elem = dynamic_cast<const List*>(*itr);
    if(!elem) {
      continue;
    }
    std::deque<std::string> urls;
    for(std::deque<MetaEntry*>::const_iterator elemItr = elem->getList().begin();
	elemItr != elem->getList().end(); elemItr++) {
      const Data* data = dynamic_cast<const Data*>(*elemItr);
      if(data) {
	urls.push_back(Util::trim(data->toString()));
      }
    }
    if(urls.size()) {
      AnnounceTierHandle tier(new AnnounceTier(urls));
      announceTiers.push_back(tier);
    }
  }
}

std::deque<std::string> DefaultBtContext::extractUrlList(const MetaEntry* obj)
{
  std::deque<std::string> uris;
  if(dynamic_cast<const List*>(obj)) {
    const List* urlList = reinterpret_cast<const List*>(obj);
    for(std::deque<MetaEntry*>::const_iterator itr = urlList->getList().begin();
	itr != urlList->getList().end(); ++itr) {
      const Data* data = dynamic_cast<const Data*>(*itr);
      if(data) {
	uris.push_back(data->toString());
      }
    }
  } else if(dynamic_cast<const Data*>(obj)) {
    const Data* urlData = reinterpret_cast<const Data*>(obj);
    uris.push_back(urlData->toString());
  }
  return uris;
}

void DefaultBtContext::loadFromMemory(const char* content, int32_t length, const std::string& defaultName)
{
  SharedHandle<MetaEntry> rootEntry = MetaFileUtil::bdecoding(content, length);
  const Dictionary* rootDic = dynamic_cast<const Dictionary*>(rootEntry.get());
  if(!rootDic) {
    throw new DlAbortEx("torrent file does not contain a root dictionary .");
  }
  processRootDictionary(rootDic, defaultName);
}

void DefaultBtContext::load(const std::string& torrentFile) {
  SharedHandle<MetaEntry> rootEntry = MetaFileUtil::parseMetaFile(torrentFile);
  const Dictionary* rootDic = dynamic_cast<const Dictionary*>(rootEntry.get());
  if(!rootDic) {
    throw new DlAbortEx("torrent file does not contain a root dictionary .");
  }
  processRootDictionary(rootDic, torrentFile);
}

void DefaultBtContext::processRootDictionary(const Dictionary* rootDic, const std::string& defaultName)
{
  clear();
  const Dictionary* infoDic = dynamic_cast<const Dictionary*>(rootDic->get("info"));
  if(!infoDic) {
    throw new DlAbortEx(MSG_SOMETHING_MISSING_IN_TORRENT, "info directory");
  }
  // retrieve infoHash
  BencodeVisitor v;
  infoDic->accept(&v);
  MessageDigestHelper::digest(infoHash, INFO_HASH_LENGTH, "sha1",
			      v.getBencodedData().c_str(),
			      v.getBencodedData().size());
  infoHashString = Util::toHex(infoHash, INFO_HASH_LENGTH);
  // calculate the number of pieces
  const Data* pieceHashData = dynamic_cast<const Data*>(infoDic->get("pieces"));
  if(!pieceHashData) {
    throw new DlAbortEx(MSG_SOMETHING_MISSING_IN_TORRENT, "pieces");
  }
  numPieces = pieceHashData->getLen()/PIECE_HASH_LENGTH;
  // retrieve piece length
  const Data* pieceLengthData = dynamic_cast<const Data*>(infoDic->get("piece length"));
  if(!pieceLengthData) {
    throw new DlAbortEx(MSG_SOMETHING_MISSING_IN_TORRENT, "piece length");
  }
  pieceLength = pieceLengthData->toInt();
  // retrieve piece hashes
  extractPieceHash((unsigned char*)pieceHashData->getData(),
		   pieceHashData->getLen(),
		   PIECE_HASH_LENGTH);
  const Data* privateFlag = dynamic_cast<const Data*>(infoDic->get("private"));
  if(privateFlag) {
    if(privateFlag->toString() == "1") {
      _private = true;
    }
  }
  // retrieve uri-list.
  // This implemantation obeys HTTP-Seeding specification:
  // see http://www.getright.com/seedtorrent.html
  std::deque<std::string> urlList = extractUrlList(rootDic->get("url-list"));
  // retrieve file entries
  extractFileEntries(infoDic, defaultName, urlList);
  if((totalLength+pieceLength-1)/pieceLength != numPieces) {
    throw new DlAbortEx("Too few/many piece hash.");
  }
  // retrieve announce
  const Data* announceData = dynamic_cast<const Data*>(rootDic->get("announce"));
  const List* announceListData = dynamic_cast<const List*>(rootDic->get("announce-list"));
  if(announceListData) {
    extractAnnounceList(announceListData);
  } else if(announceData) {
    extractAnnounce(announceData);
  }
  if(!announceTiers.size()) {
    throw new DlAbortEx("No announce URL found.");
  }
}

std::string DefaultBtContext::getPieceHash(int32_t index) const {
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

std::string DefaultBtContext::getName() const {
  return name;
}

int32_t DefaultBtContext::getPieceLength() const {
  return pieceLength;
}

int32_t DefaultBtContext::getNumPieces() const {
  return numPieces;
}

std::string DefaultBtContext::getActualBasePath() const
{
  return _dir+"/"+name;
}

std::deque<int32_t> DefaultBtContext::computeFastSet(const std::string& ipaddr, int32_t fastSetSize)
{
  std::deque<int32_t> fastSet;
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
      if(std::find(fastSet.begin(), fastSet.end(), index) == fastSet.end()) {
	fastSet.push_back(index);
      }
    }
    unsigned char temp[20];
    MessageDigestHelper::digest(temp, sizeof(temp), "sha1", x, sizeof(x));
    memcpy(x, temp, sizeof(x));
  }
  return fastSet;
}

std::ostream& operator<<(std::ostream& o, const DefaultBtContext& ctx)
{
  o << "*** BitTorrent File Information ***" << "\n";
  o << "Mode: " << (ctx.getFileMode() == DownloadContext::SINGLE ? "Single File Torrent":"Multi File Torrent") << "\n";
  o << "Announce:" << "\n";
  AnnounceTiers tiers = ctx.getAnnounceTiers();
  for(AnnounceTiers::const_iterator itr = tiers.begin(); itr != tiers.end(); ++itr) {
    const AnnounceTierHandle& tier = *itr;
    for(std::deque<std::string>::const_iterator uriItr = tier->urls.begin(); uriItr != tier->urls.end(); ++uriItr) {
      o << " " << *uriItr;
    }
    o << "\n";
  }
  o << "Info Hash: " << ctx.getInfoHashAsString() << "\n";
  o << "Piece Length: " << Util::abbrevSize(ctx.getPieceLength()) << "B\n";
  o << "The Number of Pieces: " << ctx.getNumPieces() << "\n";
  o << "Total Length: " << Util::abbrevSize(ctx.getTotalLength()) << "B\n";
  if(ctx.getFileMode() == DownloadContext::MULTI) {
    o << "Name: " << ctx.getName() << "\n";
  }
  Util::toStream(o, ctx.getFileEntries());
  return o;
}

void DefaultBtContext::setRandomizer(const RandomizerHandle& randomizer)
{
  _randomizer = randomizer;
}

} // namespace aria2
