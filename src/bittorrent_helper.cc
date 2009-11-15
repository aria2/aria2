/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "bittorrent_helper.h"

#include <cassert>
#include <cstring>
#include <algorithm>

#include "DownloadContext.h"
#include "Randomizer.h"
#include "bencode.h"
#include "util.h"
#include "DlAbortEx.h"
#include "message.h"
#include "StringFormat.h"
#include "BtConstants.h"
#include "messageDigest.h"
#include "MessageDigestHelper.h"
#include "SimpleRandomizer.h"
#include "a2netcompat.h"
#include "BtConstants.h"
#include "bitfield.h"

namespace aria2 {

namespace bittorrent {

static const std::string C_NAME("name");

static const std::string C_NAME_UTF8("name.utf-8");

static const std::string C_FILES("files");

static const std::string C_LENGTH("length");

static const std::string C_PATH("path");

static const std::string C_PATH_UTF8("path.utf-8");

static const std::string C_INFO("info");

static const std::string C_PIECES("pieces");

static const std::string C_PIECE_LENGTH("piece length");

static const std::string C_PRIVATE("private");

static const std::string C_URL_LIST("url-list");

static const std::string C_ANNOUNCE("announce");

static const std::string C_ANNOUNCE_LIST("announce-list");

static const std::string C_NODES("nodes");

static const std::string C_CREATION_DATE("creation date");

static const std::string C_COMMENT("comment");

static const std::string C_COMMENT_UTF8("comment.utf-8");

static const std::string C_CREATED_BY("created by");

static const std::string DEFAULT_PEER_ID_PREFIX("aria2-");

const std::string INFO_HASH("infoHash");

const std::string MODE("mode");

const std::string PRIVATE("private");

const std::string ANNOUNCE_LIST("announceList");

const std::string NODES("nodes");

const std::string HOSTNAME("hostname");

const std::string PORT("port");

const std::string NAME("name");

const std::string URL_LIST("urlList");

const std::string CREATION_DATE("creationDate");

const std::string COMMENT("comment");

const std::string CREATED_BY("createdBy");

const std::string BITTORRENT("bittorrent");

const std::string MULTI("multi");

const std::string SINGLE("single");

static void extractPieceHash(const SharedHandle<DownloadContext>& ctx,
			     const std::string& hashData,
			     size_t hashLength,
			     size_t numPieces)
{
  std::vector<std::string> pieceHashes;
  pieceHashes.reserve(numPieces);
  for(size_t i = 0; i < numPieces; ++i) {
    pieceHashes.push_back(util::toHex(hashData.data()+i*hashLength,
				      hashLength));
  }
  ctx->setPieceHashes(pieceHashes.begin(), pieceHashes.end());
  ctx->setPieceHashAlgo(MessageDigestContext::SHA1);
}

static void extractUrlList
(BDE& torrent, std::vector<std::string>& uris, const BDE& bde)
{
  if(bde.isList()) {
    for(BDE::List::const_iterator itr = bde.listBegin();
	itr != bde.listEnd(); ++itr) {
      if((*itr).isString()) {
	uris.push_back((*itr).s());
      }
    }
    torrent[URL_LIST] = bde;
  } else if(bde.isString()) {
    uris.push_back(bde.s());
    BDE urlList = BDE::list();
    urlList << bde;
    torrent[URL_LIST] = urlList;
  } else {
    torrent[URL_LIST] = BDE::list();
  }
}

template<typename InputIterator, typename OutputIterator>
static OutputIterator createUri
(InputIterator first, InputIterator last, OutputIterator out,
 const std::string& filePath)
{
  for(; first != last; ++first) {
    if(util::endsWith(*first, "/")) {
      *out++ = (*first)+filePath;
    } else {
      *out++ = (*first)+"/"+filePath;
    }
  }
  return out;
}

static void extractFileEntries
(const SharedHandle<DownloadContext>& ctx,
 BDE& torrent,
 const BDE& infoDict,
 const std::string& defaultName,
 const std::string& overrideName,
 const std::vector<std::string>& urlList)
{
  std::string name;
  if(overrideName.empty()) {
    std::string nameKey;
    if(infoDict.containsKey(C_NAME_UTF8)) {
      nameKey = C_NAME_UTF8;
    } else {
      nameKey = C_NAME;
    }
    const BDE& nameData = infoDict[nameKey];
    if(nameData.isString()) {
      // Split path by '/' just in case nasty ".." is included in name
      std::vector<std::string> pathelems;
      util::split(nameData.s(), std::back_inserter(pathelems), "/");
      name = util::joinPath(pathelems.begin(), pathelems.end());
      torrent[NAME] = nameData.s();
    } else {
      name = strconcat(File(defaultName).getBasename(), ".file");
      torrent[NAME] = name;
    }
  } else {
    name = overrideName;
    torrent[NAME] = name;
  }

  const BDE& filesList = infoDict[C_FILES];
  std::vector<SharedHandle<FileEntry> > fileEntries;
  if(filesList.isList()) {
    fileEntries.reserve(filesList.size());
    uint64_t length = 0;
    off_t offset = 0;
    // multi-file mode
    torrent[MODE] = MULTI;
    for(BDE::List::const_iterator itr = filesList.listBegin();
	itr != filesList.listEnd(); ++itr) {
      const BDE& fileDict = *itr;
      if(!fileDict.isDict()) {
	continue;
      }
      const BDE& fileLengthData = fileDict[C_LENGTH];
      if(!fileLengthData.isInteger()) {
	throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
				       C_LENGTH.c_str()).str());
      }
      length += fileLengthData.i();

      std::string pathKey;
      if(fileDict.containsKey(C_PATH_UTF8)) {
	pathKey = C_PATH_UTF8;
      } else {
	pathKey = C_PATH;
      }
      const BDE& pathList = fileDict[pathKey];
      if(!pathList.isList() || pathList.empty()) {
	throw DL_ABORT_EX("Path is empty.");
      }
      
      std::vector<std::string> pathelem(pathList.size());
      std::transform(pathList.listBegin(), pathList.listEnd(), pathelem.begin(),
		     std::mem_fun_ref(&BDE::s));
      std::string path = name;
      strappend(path, "/", util::joinPath(pathelem.begin(), pathelem.end()));
      // Split path with '/' again because each pathList element can
      // contain "/" inside.
      std::vector<std::string> elements;
      util::split(path, std::back_inserter(elements), "/");
      path = util::joinPath(elements.begin(), elements.end());

      std::deque<std::string> uris;
      createUri(urlList.begin(), urlList.end(), std::back_inserter(uris), path);
      SharedHandle<FileEntry> fileEntry
	(new FileEntry(strconcat(ctx->getDir(), "/", path),
		       fileLengthData.i(),
		       offset, uris));
      fileEntries.push_back(fileEntry);
      offset += fileEntry->getLength();
    }
  } else {
    // single-file mode;
    torrent[MODE] = SINGLE;
    const BDE& lengthData = infoDict[C_LENGTH];
    if(!lengthData.isInteger()) {
	throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
				       C_LENGTH.c_str()).str());      
    }
    uint64_t totalLength = lengthData.i();

    // For each uri in urlList, if it ends with '/', then
    // concatenate name to it. Specification just says so.
    std::deque<std::string> uris;
    for(std::vector<std::string>::const_iterator i = urlList.begin();
	i != urlList.end(); ++i) {
      if(util::endsWith(*i, "/")) {
	uris.push_back((*i)+name);
      } else {
	uris.push_back(*i);
      }
    }

    SharedHandle<FileEntry> fileEntry
      (new FileEntry(strconcat(ctx->getDir(), "/", name),totalLength, 0,
		     uris));
    fileEntries.push_back(fileEntry);
  }
  ctx->setFileEntries(fileEntries.begin(), fileEntries.end());
  if(torrent[MODE].s() == MULTI) {
    ctx->setBasePath(strconcat(ctx->getDir(), "/", name));
  }
}

static void extractAnnounce(BDE& torrent, const BDE& rootDict)
{
  const BDE& announceList = rootDict[C_ANNOUNCE_LIST];
  if(announceList.isList()) {
    torrent[ANNOUNCE_LIST] = announceList;
  } else {
    const BDE& announce = rootDict[C_ANNOUNCE];
    BDE announceList = BDE::list();
    if(announce.isString()) {
      announceList << BDE::list();
      announceList[0] << announce;
    }
    torrent[ANNOUNCE_LIST] = announceList;
  }
}

static void extractNodes(BDE& torrent, const BDE& nodesList)
{
  BDE nodes = BDE::list();
  if(nodesList.isList()) {
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
      if(util::trim(hostname.s()).empty()) {
	continue;
      }
      const BDE& port = addrPairList[1];
      if(!port.isInteger() || !(0 < port.i() && port.i() < 65536)) {
	continue;
      }
      BDE node = BDE::dict();
      node[HOSTNAME] = hostname;
      node[PORT] = port;
      nodes << node;
    }
  }
  torrent[NODES] = nodes;
}

static void processRootDictionary
(const SharedHandle<DownloadContext>& ctx,
 const BDE& rootDict,
 const std::string& defaultName,
 const std::string& overrideName,
 const std::deque<std::string>& uris)
{
  if(!rootDict.isDict()) {
    throw DL_ABORT_EX("torrent file does not contain a root dictionary.");
  }
  const BDE& infoDict = rootDict[C_INFO];
  if(!infoDict.isDict()) {
    throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
				   C_INFO.c_str()).str());
  }
  BDE torrent = BDE::dict();

  // retrieve infoHash
  std::string encodedInfoDict = bencode::encode(infoDict);
  unsigned char infoHash[INFO_HASH_LENGTH];
  MessageDigestHelper::digest(infoHash, INFO_HASH_LENGTH,
			      MessageDigestContext::SHA1,
			      encodedInfoDict.data(),
			      encodedInfoDict.size());
  torrent[INFO_HASH] = std::string(&infoHash[0], &infoHash[INFO_HASH_LENGTH]);

  // calculate the number of pieces
  const BDE& piecesData = infoDict[C_PIECES];
  if(!piecesData.isString()) {
    throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
				   C_PIECES.c_str()).str());
  }
  // Commented out To download 0 length torrent.
  //   if(piecesData.s().empty()) {
  //     throw DL_ABORT_EX("The length of piece hash is 0.");
  //   }
  size_t numPieces = piecesData.s().size()/PIECE_HASH_LENGTH;
  // Commented out to download 0 length torrent.
  //   if(numPieces == 0) {
  //     throw DL_ABORT_EX("The number of pieces is 0.");
  //   }
  // retrieve piece length
  const BDE& pieceLengthData = infoDict[C_PIECE_LENGTH];
  if(!pieceLengthData.isInteger()) {
    throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
				   C_PIECE_LENGTH.c_str()).str());
  }
  size_t pieceLength = pieceLengthData.i();
  ctx->setPieceLength(pieceLength);
  // retrieve piece hashes
  extractPieceHash(ctx, piecesData.s(), PIECE_HASH_LENGTH, numPieces);
  // private flag
  const BDE& privateData = infoDict[C_PRIVATE];
  int privatefg = 0;
  if(privateData.isInteger()) {
    if(privateData.i() == 1) {
      privatefg = 1;
    }
  }
  torrent[PRIVATE] = BDE((int64_t)privatefg);
  // retrieve uri-list.
  // This implemantation obeys HTTP-Seeding specification:
  // see http://www.getright.com/seedtorrent.html
  std::vector<std::string> urlList;
  extractUrlList(torrent, urlList, rootDict[C_URL_LIST]);
  urlList.insert(urlList.end(), uris.begin(), uris.end());
  std::sort(urlList.begin(), urlList.end());
  urlList.erase(std::unique(urlList.begin(), urlList.end()), urlList.end());

  // retrieve file entries
  extractFileEntries(ctx, torrent, infoDict, defaultName, overrideName, urlList);
  if((ctx->getTotalLength()+pieceLength-1)/pieceLength != numPieces) {
    throw DL_ABORT_EX("Too few/many piece hash.");
  }
  // retrieve announce
  extractAnnounce(torrent, rootDict);
  // retrieve nodes
  extractNodes(torrent, rootDict[C_NODES]);

  const BDE& creationDate = rootDict[C_CREATION_DATE];
  if(creationDate.isInteger()) {
    torrent[CREATION_DATE] = creationDate;
  }
  const BDE& commentUtf8 = rootDict[C_COMMENT_UTF8];
  if(commentUtf8.isString()) {
    torrent[COMMENT] = commentUtf8;
  } else {
    const BDE& comment = rootDict[C_COMMENT];
    if(comment.isString()) {
      torrent[COMMENT] = comment;
    }
  }
  const BDE& createdBy = rootDict[C_CREATED_BY];
  if(createdBy.isString()) {
    torrent[CREATED_BY] = createdBy;
  }

  ctx->setAttribute(BITTORRENT, torrent);
}

void load(const std::string& torrentFile,
	  const SharedHandle<DownloadContext>& ctx,
	  const std::string& overrideName)
{
  processRootDictionary(ctx,
			bencode::decodeFromFile(torrentFile),
			torrentFile,
			overrideName,
			std::deque<std::string>());
}

void load(const std::string& torrentFile,
	  const SharedHandle<DownloadContext>& ctx,
	  const std::deque<std::string>& uris,
	  const std::string& overrideName)
{
  processRootDictionary(ctx,
			bencode::decodeFromFile(torrentFile),
			torrentFile,
			overrideName,
			uris);
}

void loadFromMemory(const unsigned char* content,
		    size_t length,
		    const SharedHandle<DownloadContext>& ctx,
		    const std::string& defaultName,
		    const std::string& overrideName)
{
  processRootDictionary(ctx,
			bencode::decode(content, length),
			defaultName,
			overrideName,
			std::deque<std::string>());
}

void loadFromMemory(const unsigned char* content,
		    size_t length,
		    const SharedHandle<DownloadContext>& ctx,
		    const std::deque<std::string>& uris,
		    const std::string& defaultName,
		    const std::string& overrideName)
{
  processRootDictionary(ctx,
			bencode::decode(content, length),
			defaultName,
			overrideName,
			uris);
}

void loadFromMemory(const std::string& context,
		    const SharedHandle<DownloadContext>& ctx,
		    const std::string& defaultName,
		    const std::string& overrideName)
{
  processRootDictionary
    (ctx,
     bencode::decode(reinterpret_cast<const unsigned char*>(context.c_str()),
		     context.size()),
     defaultName, overrideName,
     std::deque<std::string>());
}

void loadFromMemory(const std::string& context,
		    const SharedHandle<DownloadContext>& ctx,
		    const std::deque<std::string>& uris,
		    const std::string& defaultName,
		    const std::string& overrideName)
{
  processRootDictionary
    (ctx,
     bencode::decode(reinterpret_cast<const unsigned char*>(context.c_str()),
		     context.size()),
     defaultName, overrideName,
     uris);
}

const unsigned char*
getInfoHash(const SharedHandle<DownloadContext>& downloadContext)
{
  return downloadContext->getAttribute(BITTORRENT)[INFO_HASH].uc();
}

std::string
getInfoHashString(const SharedHandle<DownloadContext>& downloadContext)
{
  return util::toHex(downloadContext->getAttribute(BITTORRENT)[INFO_HASH].s());
}

void print(std::ostream& o, const SharedHandle<DownloadContext>& dctx)
{
  const BDE& torrentAttrs = dctx->getAttribute(BITTORRENT);
  o << "*** BitTorrent File Information ***" << "\n";
  if(torrentAttrs.containsKey(COMMENT)) {
    o << "Comment: " << torrentAttrs[COMMENT].s() << "\n";
  }
  if(torrentAttrs.containsKey(CREATION_DATE)) {
    struct tm* staticNowtmPtr;
    char buf[26];
    time_t t = torrentAttrs[CREATION_DATE].i();
    if((staticNowtmPtr = localtime(&t)) != 0 &&
       asctime_r(staticNowtmPtr, buf) != 0) {
      // buf includes "\n"
      o << "Creation Date: " << buf;
    }
  }
  if(torrentAttrs.containsKey(CREATED_BY)) {
    o << "Created By: " << torrentAttrs[CREATED_BY].s() << "\n";
  }
  o << "Mode: " << torrentAttrs[MODE].s() << "\n";
  o << "Announce:" << "\n";
  const BDE& announceList = torrentAttrs[ANNOUNCE_LIST];
  for(BDE::List::const_iterator tieritr = announceList.listBegin();
      tieritr != announceList.listEnd(); ++tieritr) {
    if(!(*tieritr).isList()) {
      continue;
    }
    for(BDE::List::const_iterator i = (*tieritr).listBegin();
	i != (*tieritr).listEnd(); ++i) {
      o << " " << (*i).s();
    }
    o << "\n";
  }
  o << "Info Hash: " << util::toHex(torrentAttrs[INFO_HASH].s()) << "\n";
  o << "Piece Length: " << util::abbrevSize(dctx->getPieceLength()) << "B\n";
  o << "The Number of Pieces: " << dctx->getNumPieces() << "\n";
  o << "Total Length: " << util::abbrevSize(dctx->getTotalLength()) << "B ("
    << util::uitos(dctx->getTotalLength(), true) << ")\n";
  if(!torrentAttrs[URL_LIST].empty()) {
    const BDE& urlList = torrentAttrs[URL_LIST];
    o << "URL List: " << "\n";
    for(BDE::List::const_iterator i = urlList.listBegin();
	i != urlList.listEnd(); ++i) {
      o << " " << (*i).s() << "\n";
    }
  }
  o << "Name: " << torrentAttrs[NAME].s() << "\n";
  util::toStream(dctx->getFileEntries().begin(), dctx->getFileEntries().end(), o);
}

void computeFastSet
(std::vector<size_t>& fastSet, const std::string& ipaddr,
 size_t numPieces, const unsigned char* infoHash, size_t fastSetSize)
{
  unsigned char compact[6];
  if(!createcompact(compact, ipaddr, 0)) {
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
    for(size_t i = 0; i < 5 && fastSet.size() < fastSetSize; ++i) {
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

std::string generatePeerId(const std::string& peerIdPrefix)
{
  std::string peerId = peerIdPrefix;
  unsigned char buf[20];
  int len = 20-peerIdPrefix.size();
  if(len > 0) {
    util::generateRandomData(buf, len);
    peerId += std::string(&buf[0], &buf[len]);
  } if(peerId.size() > 20) {
    peerId.erase(20);
  }
  return peerId;
}

static std::string peerId;

const std::string& generateStaticPeerId(const std::string& peerIdPrefix)
{
  if(peerId.empty()) {
    peerId = generatePeerId(peerIdPrefix);
  }
  return peerId;
}

void setStaticPeerId(const std::string& newPeerId)
{
  peerId = newPeerId;
}

// If PeerID is not generated, it is created with default peerIdPrefix
// (aria2-).
const unsigned char* getStaticPeerId()
{
  if(peerId.empty()) {
    return
      reinterpret_cast<const unsigned char*>
      (generateStaticPeerId(DEFAULT_PEER_ID_PREFIX).data());
  } else {
    return reinterpret_cast<const unsigned char*>(peerId.data());
  }
}

uint8_t getId(const unsigned char* msg)
{
  return msg[0];
}

uint32_t getIntParam(const unsigned char* msg, size_t pos)
{
  uint32_t nParam;
  memcpy(&nParam, msg+pos, sizeof(nParam));
  return ntohl(nParam);
}

uint16_t getShortIntParam(const unsigned char* msg, size_t pos)
{
  uint16_t nParam;
  memcpy(&nParam, msg+pos, sizeof(nParam));
  return ntohs(nParam);
}

void checkIndex(size_t index, size_t pieces)
{
  if(!(index < pieces)) {
    throw DL_ABORT_EX(StringFormat("Invalid index: %lu",
				 static_cast<unsigned long>(index)).str());
  }
}

void checkBegin(uint32_t begin, size_t pieceLength)
{
  if(!(begin < pieceLength)) {
    throw DL_ABORT_EX(StringFormat("Invalid begin: %u", begin).str());
  }  
}

void checkLength(size_t length)
{
  if(length > MAX_BLOCK_LENGTH) {
    throw DL_ABORT_EX
      (StringFormat("Length too long: %lu > %uKB",
		    static_cast<unsigned long>(length),
		    MAX_BLOCK_LENGTH/1024).str());
  }
  if(length == 0) {
    throw DL_ABORT_EX
      (StringFormat("Invalid length: %lu",
		    static_cast<unsigned long>(length)).str());
  }
}

void checkRange(uint32_t begin, size_t length, size_t pieceLength)
{
  if(!(0 < length)) {
    throw DL_ABORT_EX
      (StringFormat("Invalid range: begin=%u, length=%lu",
		    begin,
		    static_cast<unsigned long>(length)).str());
  }
  uint32_t end = begin+length;
  if(!(end <= pieceLength)) {
    throw DL_ABORT_EX
      (StringFormat("Invalid range: begin=%u, length=%lu",
		    begin,
		    static_cast<unsigned long>(length)).str());
  }
}

void checkBitfield
(const unsigned char* bitfield, size_t bitfieldLength, size_t pieces)
{
  if(!(bitfieldLength == (pieces+7)/8)) {
    throw DL_ABORT_EX
      (StringFormat("Invalid bitfield length: %lu",
		    static_cast<unsigned long>(bitfieldLength)).str());
  }
  // Check if last byte contains garbage set bit.
  if(bitfield[bitfieldLength-1]&~bitfield::lastByteMask(pieces)) {
      throw DL_ABORT_EX("Invalid bitfield");
  }
}

void setIntParam(unsigned char* dest, uint32_t param)
{
  uint32_t nParam = htonl(param);
  memcpy(dest, &nParam, sizeof(nParam));
}

void setShortIntParam(unsigned char* dest, uint16_t param)
{
  uint16_t nParam = htons(param);
  memcpy(dest, &nParam, sizeof(nParam));
}

void createPeerMessageString
(unsigned char* msg, size_t msgLength, size_t payloadLength, uint8_t messageId)
{
  assert(msgLength >= 5);
  memset(msg, 0, msgLength);
  setIntParam(msg, payloadLength);
  msg[4] = messageId;
}

bool createcompact
(unsigned char* compact, const std::string& addr, uint16_t port)
{
  struct addrinfo hints;
  struct addrinfo* res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // since compact peer format is ipv4 only.
  hints.ai_flags = AI_NUMERICHOST;
  if(getaddrinfo(addr.c_str(), 0, &hints, &res)) {
    return false;
  }
  struct sockaddr_in* in = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
  memcpy(compact, &(in->sin_addr.s_addr), sizeof(uint32_t));
  uint16_t port_nworder(htons(port));
  memcpy(compact+4, &port_nworder, sizeof(uint16_t));
  freeaddrinfo(res);
  return true;
}

std::pair<std::string, uint16_t> unpackcompact(const unsigned char* compact)
{
  struct sockaddr_in in;
  memset(&in, 0, sizeof(in));
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
  // For netbsd
  in.sin_len = sizeof(in);
#endif // HAVE_SOCKADDR_IN_SIN_LEN
  in.sin_family = AF_INET;
  memcpy(&(in.sin_addr.s_addr), compact, sizeof(uint32_t));
  in.sin_port = 0;
  char host[NI_MAXHOST];
  int s;
  s = getnameinfo(reinterpret_cast<const struct sockaddr*>(&in), sizeof(in),
		  host, NI_MAXHOST, 0, NI_MAXSERV,
		  NI_NUMERICHOST);
  if(s) {
    return std::pair<std::string, uint16_t>();
  }
  uint16_t port_nworder;
  memcpy(&port_nworder, compact+sizeof(uint32_t), sizeof(uint16_t));
  uint16_t port = ntohs(port_nworder);
  return std::pair<std::string, uint16_t>(host, port);
}


void assertPayloadLengthGreater
(size_t threshold, size_t actual, const std::string& msgName)
{
  if(actual <= threshold) {
    throw DL_ABORT_EX
      (StringFormat(MSG_TOO_SMALL_PAYLOAD_SIZE, msgName.c_str(), actual).str());
  }
}

void assertPayloadLengthEqual
(size_t expected, size_t actual, const std::string& msgName)
{
  if(expected != actual) {
    throw DL_ABORT_EX
      (StringFormat(EX_INVALID_PAYLOAD_SIZE, msgName.c_str(),
		    actual, expected).str());
  }
}

void assertID
(uint8_t expected, const unsigned char* data, const std::string& msgName)
{
  uint8_t id = getId(data);
  if(expected != id) {
    throw DL_ABORT_EX
      (StringFormat(EX_INVALID_BT_MESSAGE_ID, id, msgName.c_str(),
		    expected).str());
  }
}

void generateRandomKey(unsigned char* key)
{
  unsigned char bytes[40];
  util::generateRandomData(bytes, sizeof(bytes));
  MessageDigestHelper::digest(key, 20, MessageDigestContext::SHA1, bytes, sizeof(bytes));
}

} // namespace bittorrent

} // namespace aria2
