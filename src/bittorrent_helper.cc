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

#include <cstring>
#include <algorithm>
#include <deque>

#include "DownloadContext.h"
#include "Randomizer.h"
#include "bencode.h"
#include "Util.h"
#include "DlAbortEx.h"
#include "message.h"
#include "StringFormat.h"
#include "BtConstants.h"
#include "messageDigest.h"
#include "MessageDigestHelper.h"
#include "PeerMessageUtil.h"
#include "SimpleRandomizer.h"

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

static const std::string DEFAULT_PEER_ID_PREFIX("-aria2-");

const std::string INFO_HASH("infoHash");

const std::string MODE("mode");

const std::string PRIVATE("private");

const std::string ANNOUNCE_LIST("announceList");

const std::string NODES("nodes");

const std::string HOSTNAME("hostname");

const std::string PORT("port");

const std::string NAME("name");

const std::string URL_LIST("urlList");

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
    pieceHashes.push_back(Util::toHex(hashData.data()+i*hashLength,
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
    if(Util::endsWith(*first, "/")) {
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
      // Slice path by '/' just in case nasty ".." is included in name
      std::deque<std::string> pathelems;
      Util::slice(pathelems, nameData.s(), '/');
      name = Util::joinPath(pathelems.begin(), pathelems.end());
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
      strappend(path, "/", Util::joinPath(pathelem.begin(), pathelem.end()));
      // Split path with '/' again because each pathList element can
      // contain "/" inside.
      std::deque<std::string> elements;
      Util::slice(elements, path, '/');
      path = Util::joinPath(elements.begin(), elements.end());

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
      if(Util::endsWith(*i, "/")) {
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
      if(Util::trim(hostname.s()).empty()) {
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
  return Util::toHex(downloadContext->getAttribute(BITTORRENT)[INFO_HASH].s());
}

void print(std::ostream& o, const SharedHandle<DownloadContext>& dctx)
{
  const BDE& torrentAttrs = dctx->getAttribute(BITTORRENT);
  o << "*** BitTorrent File Information ***" << "\n";
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
  o << "Info Hash: " << Util::toHex(torrentAttrs[INFO_HASH].s()) << "\n";
  o << "Piece Length: " << Util::abbrevSize(dctx->getPieceLength()) << "B\n";
  o << "The Number of Pieces: " << dctx->getNumPieces() << "\n";
  o << "Total Length: " << Util::abbrevSize(dctx->getTotalLength()) << "B\n";
  if(!torrentAttrs[URL_LIST].empty()) {
    const BDE& urlList = torrentAttrs[URL_LIST];
    o << "URL List: " << "\n";
    for(BDE::List::const_iterator i = urlList.listBegin();
	i != urlList.listEnd(); ++i) {
      o << " " << (*i).s() << "\n";
    }
  }
  o << "Name: " << torrentAttrs[NAME].s() << "\n";
  Util::toStream(dctx->getFileEntries().begin(), dctx->getFileEntries().end(), o);
}

void computeFastSet
(std::vector<size_t>& fastSet, const std::string& ipaddr,
 size_t numPieces, const unsigned char* infoHash, size_t fastSetSize)
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

std::string generatePeerId
(const std::string& peerIdPrefix, const SharedHandle<Randomizer>& randomizer)
{
  std::string peerId = peerIdPrefix;
  peerId += Util::randomAlpha(20-peerIdPrefix.size(), randomizer);
  if(peerId.size() > 20) {
    peerId.erase(20);
  }
  return peerId;
}

static std::string peerId;

const std::string& generateStaticPeerId
(const std::string& peerIdPrefix, const SharedHandle<Randomizer>& randomizer)
{
  if(peerId.empty()) {
    peerId = generatePeerId(peerIdPrefix, randomizer);
  }
  return peerId;
}

void setStaticPeerId(const std::string& newPeerId)
{
  peerId = newPeerId;
}

// If PeerID is not generated, it is created with default peerIdPrefix
// (-aria2-) and SimpleRandomizer.
const unsigned char* getStaticPeerId()
{
  if(peerId.empty()) {
    return
      reinterpret_cast<const unsigned char*>(generateStaticPeerId(DEFAULT_PEER_ID_PREFIX, SimpleRandomizer::getInstance()).data());
  } else {
    return reinterpret_cast<const unsigned char*>(peerId.data());
  }
}

} // namespace bittorrent

} // namespace aria2
