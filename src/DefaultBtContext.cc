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

#include <cstring>
#include <ostream>
#include <functional>
#include <algorithm>
#include <vector>

#include "DlAbortEx.h"
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
#include "bencode.h"

namespace aria2 {

const std::string DefaultBtContext::DEFAULT_PEER_ID_PREFIX("-aria2-");

DefaultBtContext::DefaultBtContext():_peerIdPrefix(DEFAULT_PEER_ID_PREFIX),
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

const std::string& DefaultBtContext::getInfoHashAsString() const {
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

void DefaultBtContext::extractPieceHash(const std::string& hashData,
					size_t hashLength)
{
  size_t numPieces = hashData.size()/hashLength;
  for(size_t i = 0; i < numPieces; i++) {
    pieceHashes.push_back(Util::toHex(hashData.data()+i*hashLength,
				      hashLength));
  }
}

void DefaultBtContext::extractFileEntries(const BDE& infoDict,
					  const std::string& defaultName,
					  const std::string& overrideName,
					  const std::deque<std::string>& urlList)
{
  if(overrideName.empty()) {
    const BDE& nameData = infoDict[BtContext::C_NAME];
    if(nameData.isString()) {
      name = nameData.s();
    } else {
      name = File(defaultName).getBasename()+".file";
    }
  } else {
    name = overrideName;
  }
  const BDE& filesList = infoDict[BtContext::C_FILES];
  if(filesList.isList()) {
    uint64_t length = 0;
    off_t offset = 0;
    // multi-file mode
    fileMode = BtContext::MULTI;
    for(BDE::List::const_iterator itr = filesList.listBegin();
	itr != filesList.listEnd(); ++itr) {
      const BDE& fileDict = *itr;
      if(!fileDict.isDict()) {
	continue;
      }

      const BDE& fileLengthData = fileDict[BtContext::C_LENGTH];
      if(!fileLengthData.isInteger()) {
	throw DlAbortEx(StringFormat(MSG_MISSING_BT_INFO,
				     BtContext::C_LENGTH.c_str()).str());
      }
      length += fileLengthData.i();

      const BDE& pathList = fileDict[BtContext::C_PATH];
      if(!pathList.isList() || pathList.empty()) {
	throw DlAbortEx("Path is empty.");
      }
      
      std::vector<std::string> pathelem(pathList.size());
      std::transform(pathList.listBegin(), pathList.listEnd(), pathelem.begin(),
		     std::mem_fun_ref(&BDE::s));
      std::string path =
	name+"/"+Util::joinPath(pathelem.begin(), pathelem.end());
      // Split path with '/' again because each pathList element can
      // contain "/" inside.
      std::deque<std::string> elements;
      Util::slice(elements, path, '/');
      path = Util::joinPath(elements.begin(), elements.end());

      std::deque<std::string> uris;
      std::transform(urlList.begin(), urlList.end(), std::back_inserter(uris),
		     std::bind2nd(std::plus<std::string>(), "/"+path));
      FileEntryHandle fileEntry(new FileEntry(_dir+"/"+path, fileLengthData.i(),
					      offset, uris));
      fileEntries.push_back(fileEntry);
      offset += fileEntry->getLength();
    }
    totalLength = length;
  } else {
    // single-file mode;
    fileMode = BtContext::SINGLE;
    const BDE& lengthData = infoDict[BtContext::C_LENGTH];
    if(!lengthData.isInteger()) {
	throw DlAbortEx(StringFormat(MSG_MISSING_BT_INFO,
				     BtContext::C_LENGTH.c_str()).str());      
    }
    totalLength = lengthData.i();
    // Slice path by '/' just in case nasty ".." is included in name
    std::deque<std::string> pathelems;
    Util::slice(pathelems, name, '/');

    // For each uri in urlList, if it ends with '/', then
    // concatenate name to it. Specification just says so.
    std::deque<std::string> uris;
    for(std::deque<std::string>::const_iterator i = urlList.begin();
	i != urlList.end(); ++i) {
      if(Util::endsWith(*i, "/")) {
	uris.push_back((*i)+name);
      } else {
	uris.push_back(*i);
      }
    }

    SharedHandle<FileEntry> fileEntry
      (new FileEntry(_dir+"/"+Util::joinPath(pathelems.begin(),pathelems.end()),
		     totalLength, 0, uris));
    fileEntries.push_back(fileEntry);
  }
}

void DefaultBtContext::extractAnnounceURI(const BDE& announceData)
{
  // Assumed announceData is string
  std::deque<std::string> urls;
  urls.push_back(Util::trim(announceData.s()));
  announceTiers.push_back(AnnounceTierHandle(new AnnounceTier(urls)));
}

void DefaultBtContext::extractAnnounceList(const BDE& announceList)
{
  // Assumed announceList is string
  for(BDE::List::const_iterator itr = announceList.listBegin();
      itr != announceList.listEnd(); ++itr) {
    const BDE& elemList = *itr;
    if(!elemList.isList()) {
      continue;
    }
    std::deque<std::string> urls;
    for(BDE::List::const_iterator elemItr = elemList.listBegin();
	elemItr != elemList.listEnd(); ++elemItr) {
      const BDE& url = (*elemItr);
      if(url.isString()) {
	urls.push_back(Util::trim(url.s()));
      }
    }
    if(!urls.empty()) {
      AnnounceTierHandle tier(new AnnounceTier(urls));
      announceTiers.push_back(tier);
    }
  }
}

void DefaultBtContext::extractAnnounce(const BDE& rootDict)
{
  const BDE& announceList = rootDict[BtContext::C_ANNOUNCE_LIST];
  if(announceList.isList()) {
    extractAnnounceList(announceList);
  } else {
    const BDE& announce = rootDict[BtContext::C_ANNOUNCE];
    if(announce.isString()) {
      extractAnnounceURI(announce);
    }
  }
}

void DefaultBtContext::extractUrlList(std::deque<std::string>& uris,
				      const BDE& bde)
{
  if(bde.isList()) {
    for(BDE::List::const_iterator itr = bde.listBegin();
	itr != bde.listEnd(); ++itr) {
      if((*itr).isString()) {
	uris.push_back((*itr).s());
      }
    }
  } else if(bde.isString()) {
    uris.push_back(bde.s());
  }
}

void DefaultBtContext::extractNodes(const BDE& nodesList)
{
  if(!nodesList.isList()) {
    return;
  }
  for(BDE::List::const_iterator i = nodesList.listBegin();
      i != nodesList.listEnd(); ++i) {
    const BDE& addrPairList = (*i);
    if(!addrPairList.isList() || addrPairList.size() != 2) {
      continue;
    }
    const BDE& hostname = addrPairList[0];
    if(!hostname.isString()) {
      continue;
    }
    if(Util::trim(hostname.s()).empty()) {
      continue;
    }
    const BDE& port = addrPairList[1];
    if(!port.isInteger() || !(0 < port.i() && port.i() < 65536)) {
      continue;
    }
    _nodes.push_back(std::pair<std::string, uint16_t>(hostname.s(), port.i()));
  }
}

void DefaultBtContext::loadFromMemory(const unsigned char* content,
				      size_t length,
				      const std::string& defaultName,
				      const std::string& overrideName)
{
  processRootDictionary(bencode::decode(content, length), defaultName,
			overrideName);
}

void DefaultBtContext::load(const std::string& torrentFile,
			    const std::string& overrideName) {
  processRootDictionary(bencode::decodeFromFile(torrentFile), torrentFile,
			overrideName);
}

void DefaultBtContext::processRootDictionary(const BDE& rootDict,
					     const std::string& defaultName,
					     const std::string& overrideName)
{
  clear();
  if(!rootDict.isDict()) {
    throw DlAbortEx("torrent file does not contain a root dictionary.");
  }
  const BDE& infoDict = rootDict[BtContext::C_INFO];
  if(!infoDict.isDict()) {
    throw DlAbortEx(StringFormat(MSG_MISSING_BT_INFO,
				 BtContext::C_INFO.c_str()).str());
  }
  // retrieve infoHash
  std::string encodedInfoDict = bencode::encode(infoDict);
  MessageDigestHelper::digest(infoHash, INFO_HASH_LENGTH,
			      MessageDigestContext::SHA1,
			      encodedInfoDict.data(),
			      encodedInfoDict.size());
  infoHashString = Util::toHex(infoHash, INFO_HASH_LENGTH);
  // calculate the number of pieces
  const BDE& piecesData = infoDict[BtContext::C_PIECES];
  if(!piecesData.isString()) {
    throw DlAbortEx(StringFormat(MSG_MISSING_BT_INFO,
				 BtContext::C_PIECES.c_str()).str());
  }
  if(piecesData.s().empty()) {
    throw DlAbortEx("The length of piece hash is 0.");
  }
  numPieces = piecesData.s().size()/PIECE_HASH_LENGTH;
  if(numPieces == 0) {
    throw DlAbortEx("The number of pieces is 0.");
  }
  // retrieve piece length
  const BDE& pieceLengthData = infoDict[BtContext::C_PIECE_LENGTH];
  if(!pieceLengthData.isInteger()) {
    throw DlAbortEx(StringFormat(MSG_MISSING_BT_INFO,
				 BtContext::C_PIECE_LENGTH.c_str()).str());
  }
  pieceLength = pieceLengthData.i();
  // retrieve piece hashes
  extractPieceHash(piecesData.s(), PIECE_HASH_LENGTH);
  // private flag
  const BDE& privateData = infoDict[BtContext::C_PRIVATE];
  if(privateData.isInteger()) {
    _private = (privateData.i() == 1);
  }
  // retrieve uri-list.
  // This implemantation obeys HTTP-Seeding specification:
  // see http://www.getright.com/seedtorrent.html
  std::deque<std::string> urlList;
  extractUrlList(urlList, rootDict[BtContext::C_URL_LIST]);

  // retrieve file entries
  extractFileEntries(infoDict, defaultName, overrideName, urlList);
  if((totalLength+pieceLength-1)/pieceLength != numPieces) {
    throw DlAbortEx("Too few/many piece hash.");
  }
  // retrieve announce
  extractAnnounce(rootDict);
  // retrieve nodes
  extractNodes(rootDict[BtContext::C_NODES]);
}

const std::string& DefaultBtContext::getPieceHash(size_t index) const {
  if(index < numPieces) {
    return pieceHashes[index];
  } else {
    return A2STR::NIL;
  }
}

uint64_t DefaultBtContext::getTotalLength() const {
  return totalLength;
}

bool DefaultBtContext::knowsTotalLength() const
{
  return true;
}

BtContext::FILE_MODE DefaultBtContext::getFileMode() const {
  return fileMode;
}

FileEntries DefaultBtContext::getFileEntries() const {
  return fileEntries;
}

const std::string& DefaultBtContext::getPieceHashAlgo() const
{
  return MessageDigestContext::SHA1;
}

const std::deque<SharedHandle<AnnounceTier> >&
DefaultBtContext::getAnnounceTiers() const
{
  return announceTiers;
}

const std::string& DefaultBtContext::getName() const {
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
  if(numPieces < fastSetSize) {
    fastSetSize = numPieces;
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
  MessageDigestHelper::digest(x, sizeof(x), MessageDigestContext::SHA1, tx, 24);
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
    MessageDigestHelper::digest(temp, sizeof(temp), MessageDigestContext::SHA1, x, sizeof(x));
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

void DefaultBtContext::setFilePathWithIndex
(size_t index, const std::string& path)
{
  if(0 < index && index <= fileEntries.size()) {
    fileEntries[index-1]->setPath(path);
  } else {
    throw DlAbortEx(StringFormat("No such file with index=%u",
				 static_cast<unsigned int>(index)).str());
  }
}

} // namespace aria2
