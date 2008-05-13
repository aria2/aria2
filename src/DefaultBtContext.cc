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
#include "PeerMessageUtil.h"
#include "StringFormat.h"
#include "A2STR.h"
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

size_t DefaultBtContext::getInfoHashLength() const {
  return INFO_HASH_LENGTH;
}

std::string DefaultBtContext::getInfoHashAsString() const {
  return infoHashString;
}

void DefaultBtContext::clear() {
  memset(infoHash, 0, INFO_HASH_LENGTH);
  infoHashString = A2STR::NIL;
  pieceHashes.clear();
  fileEntries.clear();
  totalLength = 0;
  pieceLength = 0;
  fileMode = BtContext::SINGLE;
  numPieces = 0;
  name = A2STR::NIL;
  announceTiers.clear();
  _private = false;
}

void DefaultBtContext::extractPieceHash(const unsigned char* hashData,
					size_t hashDataLength,
					size_t hashLength) {
  size_t numPieces = hashDataLength/hashLength;
  for(size_t i = 0; i < numPieces; i++) {
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
    uint64_t length = 0;
    off_t offset = 0;
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
	throw DlAbortEx
	  (StringFormat(MSG_SOMETHING_MISSING_IN_TORRENT, "file length").str());
      }
      const List* pathList = dynamic_cast<const List*>(fileDic->get("path"));
      if(!pathList) {
	throw DlAbortEx
	  (StringFormat(MSG_SOMETHING_MISSING_IN_TORRENT, "file path list").str());
      }
      const std::deque<MetaEntry*>& paths = pathList->getList();
      std::string path;
      for(size_t i = 0; i < paths.size()-1; i++) {
	const Data* subpath = dynamic_cast<const Data*>(paths[i]);
	if(subpath) {
	  path += subpath->toString()+"/";
	} else {
	  throw DlAbortEx
	    (StringFormat(MSG_SOMETHING_MISSING_IN_TORRENT, "file path element").str());
	}
      }
      const Data* lastPath = dynamic_cast<const Data*>(paths.back());
      if(lastPath) {
	path += lastPath->toString();
      } else {
	throw DlAbortEx
	  (StringFormat(MSG_SOMETHING_MISSING_IN_TORRENT, "file path element").str());
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
      throw DlAbortEx
	(StringFormat(MSG_SOMETHING_MISSING_IN_TORRENT, "file length").str());
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

void DefaultBtContext::extractUrlList(std::deque<std::string>& uris,
				      const MetaEntry* obj)
{
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
}

void DefaultBtContext::extractNodes(const List* nodes)
{
  
  for(std::deque<MetaEntry*>::const_iterator i = nodes->getList().begin();
      i != nodes->getList().end(); ++i) {
    const List* addrPair = dynamic_cast<const List*>(*i);
    if(!addrPair || addrPair->getList().size() != 2) {
      continue;
    }
    const Data* hostname = dynamic_cast<const Data*>(addrPair->getList()[0]);
    if(!hostname) {
      continue;
    }
    std::string h = hostname->toString();
    if(Util::trim(h).empty()) {
      continue;
    }
    const Data* port = dynamic_cast<const Data*>(addrPair->getList()[1]);
    if(!port) {
      continue;
    }
    uint16_t p = port->toInt();
    if(p == 0) {
      continue;
    }
    _nodes.push_back(std::pair<std::string, uint16_t>(h, p));
  }
}

void DefaultBtContext::loadFromMemory(const unsigned char* content,
				      size_t length,
				      const std::string& defaultName)
{
  SharedHandle<MetaEntry> rootEntry(MetaFileUtil::bdecoding(content, length));
  const Dictionary* rootDic = dynamic_cast<const Dictionary*>(rootEntry.get());
  if(!rootDic) {
    throw DlAbortEx
      (StringFormat("torrent file does not contain a root dictionary .").str());
  }
  processRootDictionary(rootDic, defaultName);
}

void DefaultBtContext::load(const std::string& torrentFile) {
  SharedHandle<MetaEntry> rootEntry(MetaFileUtil::parseMetaFile(torrentFile));
  const Dictionary* rootDic = dynamic_cast<const Dictionary*>(rootEntry.get());
  if(!rootDic) {
    throw DlAbortEx
      (StringFormat("torrent file does not contain a root dictionary .").str());
  }
  processRootDictionary(rootDic, torrentFile);
}

void DefaultBtContext::processRootDictionary(const Dictionary* rootDic, const std::string& defaultName)
{
  clear();
  const Dictionary* infoDic = dynamic_cast<const Dictionary*>(rootDic->get("info"));
  if(!infoDic) {
    throw DlAbortEx
      (StringFormat(MSG_SOMETHING_MISSING_IN_TORRENT, "info directory").str());
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
    throw DlAbortEx
      (StringFormat(MSG_SOMETHING_MISSING_IN_TORRENT, "pieces").str());
  }
  if(pieceHashData->getLen() == 0) {
    throw DlAbortEx("The length of piece hash is 0.");
  }
  numPieces = pieceHashData->getLen()/PIECE_HASH_LENGTH;
  if(numPieces == 0) {
    throw DlAbortEx("The number of pieces is 0.");
  }
  // retrieve piece length
  const Data* pieceLengthData = dynamic_cast<const Data*>(infoDic->get("piece length"));
  if(!pieceLengthData) {
    throw DlAbortEx
      (StringFormat(MSG_SOMETHING_MISSING_IN_TORRENT, "piece length").str());
  }
  pieceLength = pieceLengthData->toInt();
  // retrieve piece hashes
  extractPieceHash(pieceHashData->getData(), pieceHashData->getLen(),
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
  std::deque<std::string> urlList;
  extractUrlList(urlList, rootDic->get("url-list"));
  // retrieve file entries
  extractFileEntries(infoDic, defaultName, urlList);
  if((totalLength+pieceLength-1)/pieceLength != numPieces) {
    throw DlAbortEx("Too few/many piece hash.");
  }
  // retrieve announce
  const Data* announceData = dynamic_cast<const Data*>(rootDic->get("announce"));
  const List* announceListData = dynamic_cast<const List*>(rootDic->get("announce-list"));
  if(announceListData) {
    extractAnnounceList(announceListData);
  } else if(announceData) {
    extractAnnounce(announceData);
  }
  // retrieve nodes
  const List* nodes = dynamic_cast<const List*>(rootDic->get("nodes"));
  if(nodes) {
    extractNodes(nodes);
  }
}

std::string DefaultBtContext::getPieceHash(size_t index) const {
  if(index < numPieces) {
    return pieceHashes[index];
  } else {
    return A2STR::NIL;
  }
}

uint64_t DefaultBtContext::getTotalLength() const {
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

size_t DefaultBtContext::getPieceLength() const {
  return pieceLength;
}

size_t DefaultBtContext::getNumPieces() const {
  return numPieces;
}

std::string DefaultBtContext::getActualBasePath() const
{
  return _dir+"/"+name;
}

void DefaultBtContext::computeFastSet
(std::deque<size_t>& fastSet, const std::string& ipaddr, size_t fastSetSize)
{
  unsigned char compact[6];
  if(!PeerMessageUtil::createcompact(compact, ipaddr, 0)) {
    return;
  }
  unsigned char tx[24];
  memcpy(tx, compact, 4);
  if((tx[0] & 0x80) == 0 || (tx[0] & 0x40) == 0) {
    tx[2] = 0x00;
    tx[3] = 0x00;
  } else {
    tx[3] = 0x00;
  }
  memcpy(tx+4, infoHash, 20);
  unsigned char x[20];
  MessageDigestHelper::digest(x, sizeof(x), "sha1", tx, 24);
  while(fastSet.size() < fastSetSize) {
    for(size_t i = 0; i < 5 && fastSet.size() < fastSetSize; i++) {
      size_t j = i*4;
      uint32_t ny;
      memcpy(&ny, x+j, 4);
      uint32_t y = ntohl(ny);
      size_t index = y%numPieces;
      if(std::find(fastSet.begin(), fastSet.end(), index) == fastSet.end()) {
	fastSet.push_back(index);
      }
    }
    unsigned char temp[20];
    MessageDigestHelper::digest(temp, sizeof(temp), "sha1", x, sizeof(x));
    memcpy(x, temp, sizeof(x));
  }
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

std::deque<std::pair<std::string, uint16_t> >&
DefaultBtContext::getNodes()
{
  return _nodes;
}

void DefaultBtContext::setInfoHash(const unsigned char* infoHash)
{
  memcpy(this->infoHash, infoHash, sizeof(this->infoHash));
}

} // namespace aria2
