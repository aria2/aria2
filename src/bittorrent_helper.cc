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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include "base32.h"
#include "magnet.h"
#include "ValueBase.h"
#include "bencode2.h"
#include "TorrentAttribute.h"

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
(const SharedHandle<TorrentAttribute>& torrent, std::vector<std::string>& uris,
 const ValueBase* v)
{
  class UrlListVisitor:public ValueBaseVisitor {
  private:
    std::vector<std::string>& uris_;
    const SharedHandle<TorrentAttribute>& torrent_;
  public:
    UrlListVisitor
    (std::vector<std::string>& uris,
     const SharedHandle<TorrentAttribute>& torrent):
      uris_(uris), torrent_(torrent) {}

    virtual void visit(const String& v)
    {
      uris_.push_back(v.s());
      torrent_->urlList.push_back(v.s());
    }

    virtual void visit(const Integer& v) {}

    virtual void visit(const List& v)
    {
      for(List::ValueType::const_iterator itr = v.begin(), eoi = v.end();
          itr != eoi; ++itr) {
        const String* uri = asString(*itr);
        if(uri) {
          uris_.push_back(uri->s());
          torrent_->urlList.push_back(uri->s());
        }
      }
    }
    virtual void visit(const Dict& v) {}
  };

  if(v) {
    UrlListVisitor visitor(uris, torrent);
    v->accept(visitor);
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
 const SharedHandle<TorrentAttribute>& torrent,
 const Dict* infoDict,
 const std::string& defaultName,
 const std::string& overrideName,
 const std::vector<std::string>& urlList)
{
  std::string name;
  if(overrideName.empty()) {
    std::string nameKey;
    if(infoDict->containsKey(C_NAME_UTF8)) {
      nameKey = C_NAME_UTF8;
    } else {
      nameKey = C_NAME;
    }
    const String* nameData = asString(infoDict->get(nameKey));
    if(nameData) {
      if(util::detectDirTraversal(nameData->s())) {
        throw DL_ABORT_EX
          (StringFormat
           (MSG_DIR_TRAVERSAL_DETECTED,nameData->s().c_str()).str());
      }
      name = nameData->s();
    } else {
      name = strconcat(File(defaultName).getBasename(), ".file");
    }
  } else {
    name = overrideName;
  }
  torrent->name = name;
  std::vector<SharedHandle<FileEntry> > fileEntries;
  const List* filesList = asList(infoDict->get(C_FILES));
  if(filesList) {
    fileEntries.reserve(filesList->size());
    uint64_t length = 0;
    off_t offset = 0;
    // multi-file mode
    torrent->mode = MULTI;
    for(List::ValueType::const_iterator itr = filesList->begin(),
          eoi = filesList->end(); itr != eoi; ++itr) {
      const Dict* fileDict = asDict(*itr);
      if(!fileDict) {
        continue;
      }
      const Integer* fileLengthData = asInteger(fileDict->get(C_LENGTH));
      if(!fileLengthData) {
        throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
                                       C_LENGTH.c_str()).str());
      }
      length += fileLengthData->i();

      std::string pathKey;
      if(fileDict->containsKey(C_PATH_UTF8)) {
        pathKey = C_PATH_UTF8;
      } else {
        pathKey = C_PATH;
      }
      const List* pathList = asList(fileDict->get(pathKey));
      if(!pathList || pathList->empty()) {
        throw DL_ABORT_EX("Path is empty.");
      }
      
      std::vector<std::string> pathelem(pathList->size()+1);
      pathelem[0] = name;
      std::vector<std::string>::iterator pathelemOutItr = pathelem.begin();
      ++pathelemOutItr;
      for(List::ValueType::const_iterator itr = pathList->begin(),
            eoi = pathList->end(); itr != eoi; ++itr) {
        const String* elem = asString(*itr);
        if(elem) {
          (*pathelemOutItr++) = elem->s();
        } else {
          throw DL_ABORT_EX("Path element is not string.");
        }
      }
      std::string path = strjoin(pathelem.begin(), pathelem.end(), '/');
      if(util::detectDirTraversal(path)) {
        throw DL_ABORT_EX
          (StringFormat(MSG_DIR_TRAVERSAL_DETECTED, path.c_str()).str());
      }
      std::string pePath =
        strjoin(pathelem.begin(), pathelem.end(), '/',
                std::ptr_fun(static_cast<std::string (*)(const std::string&)>
                             (util::percentEncode)));
      std::vector<std::string> uris;
      createUri(urlList.begin(), urlList.end(),std::back_inserter(uris),pePath);
      SharedHandle<FileEntry> fileEntry
        (new FileEntry(util::applyDir(ctx->getDir(), util::escapePath(path)),
                       fileLengthData->i(),
                       offset, uris));
      fileEntry->setOriginalName(path);
      fileEntries.push_back(fileEntry);
      offset += fileEntry->getLength();
    }
  } else {
    // single-file mode;
    torrent->mode = SINGLE;
    const Integer* lengthData = asInteger(infoDict->get(C_LENGTH));
    if(!lengthData) {
      throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
                                     C_LENGTH.c_str()).str());      
    }
    uint64_t totalLength = lengthData->i();

    // For each uri in urlList, if it ends with '/', then
    // concatenate name to it. Specification just says so.
    std::vector<std::string> uris;
    for(std::vector<std::string>::const_iterator i = urlList.begin(),
          eoi = urlList.end(); i != eoi; ++i) {
      if(util::endsWith(*i, A2STR::SLASH_C)) {
        uris.push_back((*i)+util::percentEncode(name));
      } else {
        uris.push_back(*i);
      }
    }

    SharedHandle<FileEntry> fileEntry
      (new FileEntry(util::applyDir(ctx->getDir(), util::escapePath(name)),
                     totalLength, 0,
                     uris));
    fileEntry->setOriginalName(name);
    fileEntries.push_back(fileEntry);
  }
  ctx->setFileEntries(fileEntries.begin(), fileEntries.end());
  if(torrent->mode == MULTI) {
    ctx->setBasePath(util::applyDir(ctx->getDir(), name));
  }
}

static void extractAnnounce
(const SharedHandle<TorrentAttribute>& torrent, const Dict* rootDict)
{
  const List* announceList = asList(rootDict->get(C_ANNOUNCE_LIST));
  if(announceList) {
    for(List::ValueType::const_iterator tierIter = announceList->begin(),
          eoi = announceList->end(); tierIter != eoi; ++tierIter) {
      const List* tier = asList(*tierIter);
      if(!tier) {
        continue;
      }
      std::vector<std::string> ntier;
      for(List::ValueType::const_iterator uriIter = tier->begin(),
            eoi2 = tier->end(); uriIter != eoi2; ++uriIter) {
        const String* uri = asString(*uriIter);
        if(uri) {
          ntier.push_back(util::trim(uri->s()));
        }
      }
      if(!ntier.empty()) {
        torrent->announceList.push_back(ntier);
      }
    }
  } else {
    const String* announce = asString(rootDict->get(C_ANNOUNCE));
    if(announce) {
      std::vector<std::string> tier;
      tier.push_back(util::trim(announce->s()));
      torrent->announceList.push_back(tier);
    }
  }
}

static void extractNodes
(const SharedHandle<TorrentAttribute>& torrent, const ValueBase* nodesListSrc)
{
  const List* nodesList = asList(nodesListSrc);
  if(nodesList) {
    for(List::ValueType::const_iterator i = nodesList->begin(),
          eoi = nodesList->end(); i != eoi; ++i) {
      const List* addrPairList = asList(*i);
      if(!addrPairList || addrPairList->size() != 2) {
        continue;
      }
      const String* hostname = asString(addrPairList->get(0));
      if(!hostname) {
        continue;
      }
      if(util::trim(hostname->s()).empty()) {
        continue;
      }
      const Integer* port = asInteger(addrPairList->get(1));
      if(!port || !(0 < port->i() && port->i() < 65536)) {
        continue;
      }
      torrent->nodes.push_back(std::make_pair(hostname->s(), port->i()));
    }
  }
}

static void processRootDictionary
(const SharedHandle<DownloadContext>& ctx,
 const SharedHandle<ValueBase>& root,
 const std::string& defaultName,
 const std::string& overrideName,
 const std::vector<std::string>& uris)
{
  const Dict* rootDict = asDict(root);
  if(!rootDict) {
    throw DL_ABORT_EX("torrent file does not contain a root dictionary.");
  }
  const Dict* infoDict = asDict(rootDict->get(C_INFO));
  if(!infoDict) {
    throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
                                   C_INFO.c_str()).str());
  }
  SharedHandle<TorrentAttribute> torrent(new TorrentAttribute());

  // retrieve infoHash
  std::string encodedInfoDict = bencode2::encode(infoDict);
  unsigned char infoHash[INFO_HASH_LENGTH];
  MessageDigestHelper::digest(infoHash, INFO_HASH_LENGTH,
                              MessageDigestContext::SHA1,
                              encodedInfoDict.data(),
                              encodedInfoDict.size());
  torrent->infoHash = std::string(&infoHash[0], &infoHash[INFO_HASH_LENGTH]);
  torrent->metadata = encodedInfoDict;
  torrent->metadataSize = encodedInfoDict.size();

  // calculate the number of pieces
  const String* piecesData = asString(infoDict->get(C_PIECES));
  if(!piecesData) {
    throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
                                   C_PIECES.c_str()).str());
  }
  // Commented out To download 0 length torrent.
  //   if(piecesData.s().empty()) {
  //     throw DL_ABORT_EX("The length of piece hash is 0.");
  //   }
  size_t numPieces = piecesData->s().size()/PIECE_HASH_LENGTH;
  // Commented out to download 0 length torrent.
  //   if(numPieces == 0) {
  //     throw DL_ABORT_EX("The number of pieces is 0.");
  //   }
  // retrieve piece length
  const Integer* pieceLengthData = asInteger(infoDict->get(C_PIECE_LENGTH));
  if(!pieceLengthData) {
    throw DL_ABORT_EX(StringFormat(MSG_MISSING_BT_INFO,
                                   C_PIECE_LENGTH.c_str()).str());
  }
  size_t pieceLength = pieceLengthData->i();
  ctx->setPieceLength(pieceLength);
  // retrieve piece hashes
  extractPieceHash(ctx, piecesData->s(), PIECE_HASH_LENGTH, numPieces);
  // private flag
  const Integer* privateData = asInteger(infoDict->get(C_PRIVATE));
  int privatefg = 0;
  if(privateData) {
    if(privateData->i() == 1) {
      privatefg = 1;
    }
  }
  if(privatefg) {
    torrent->privateTorrent = true;
  }
  // retrieve uri-list.
  // This implemantation obeys HTTP-Seeding specification:
  // see http://www.getright.com/seedtorrent.html
  std::vector<std::string> urlList;
  extractUrlList(torrent, urlList, rootDict->get(C_URL_LIST).get());
  urlList.insert(urlList.end(), uris.begin(), uris.end());
  std::sort(urlList.begin(), urlList.end());
  urlList.erase(std::unique(urlList.begin(), urlList.end()), urlList.end());

  // retrieve file entries
  extractFileEntries
    (ctx, torrent, infoDict, defaultName, overrideName, urlList);
  if((ctx->getTotalLength()+pieceLength-1)/pieceLength != numPieces) {
    throw DL_ABORT_EX("Too few/many piece hash.");
  }
  // retrieve announce
  extractAnnounce(torrent, rootDict);
  // retrieve nodes
  extractNodes(torrent, rootDict->get(C_NODES).get());

  const Integer* creationDate = asInteger(rootDict->get(C_CREATION_DATE));
  if(creationDate) {
    torrent->creationDate = creationDate->i();
  }
  const String* commentUtf8 = asString(rootDict->get(C_COMMENT_UTF8));
  if(commentUtf8) {
    torrent->comment = commentUtf8->s();
  } else {
    const String* comment = asString(rootDict->get(C_COMMENT));
    if(comment) {
      torrent->comment = comment->s();
    }
  }
  const String* createdBy = asString(rootDict->get(C_CREATED_BY));
  if(createdBy) {
    torrent->createdBy = createdBy->s();
  }

  ctx->setAttribute(BITTORRENT, torrent);
}

void load(const std::string& torrentFile,
          const SharedHandle<DownloadContext>& ctx,
          const std::string& overrideName)
{
  processRootDictionary(ctx,
                        bencode2::decodeFromFile(torrentFile),
                        torrentFile,
                        overrideName,
                        std::vector<std::string>());
}

void load(const std::string& torrentFile,
          const SharedHandle<DownloadContext>& ctx,
          const std::vector<std::string>& uris,
          const std::string& overrideName)
{
  processRootDictionary(ctx,
                        bencode2::decodeFromFile(torrentFile),
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
                        bencode2::decode(content, length),
                        defaultName,
                        overrideName,
                        std::vector<std::string>());
}

void loadFromMemory(const unsigned char* content,
                    size_t length,
                    const SharedHandle<DownloadContext>& ctx,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName)
{
  processRootDictionary(ctx,
                        bencode2::decode(content, length),
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
     bencode2::decode(context),
     defaultName, overrideName,
     std::vector<std::string>());
}

void loadFromMemory(const std::string& context,
                    const SharedHandle<DownloadContext>& ctx,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName)
{
  processRootDictionary
    (ctx,
     bencode2::decode(context),
     defaultName, overrideName,
     uris);
}

SharedHandle<TorrentAttribute> getTorrentAttrs
(const SharedHandle<DownloadContext>& dctx)
{
  return static_pointer_cast<TorrentAttribute>(dctx->getAttribute(BITTORRENT));
}

const unsigned char*
getInfoHash(const SharedHandle<DownloadContext>& dctx)
{
  return reinterpret_cast<const unsigned char*>
    (getTorrentAttrs(dctx)->infoHash.data());
}

std::string
getInfoHashString(const SharedHandle<DownloadContext>& dctx)
{
  return util::toHex(getTorrentAttrs(dctx)->infoHash);
}

void print(std::ostream& o, const SharedHandle<DownloadContext>& dctx)
{
  SharedHandle<TorrentAttribute> torrentAttrs = getTorrentAttrs(dctx);
  o << "*** BitTorrent File Information ***" << "\n";
  if(!torrentAttrs->comment.empty()) {
    o << "Comment: " << torrentAttrs->comment << "\n";
  }
  if(torrentAttrs->creationDate) {
    struct tm* staticNowtmPtr;
    char buf[26];
    time_t t = torrentAttrs->creationDate;
    if((staticNowtmPtr = localtime(&t)) != 0 &&
       asctime_r(staticNowtmPtr, buf) != 0) {
      // buf includes "\n"
      o << "Creation Date: " << buf;
    }
  }
  if(!torrentAttrs->createdBy.empty()) {
    o << "Created By: " << torrentAttrs->createdBy << "\n";
  }
  o << "Mode: " << torrentAttrs->mode << "\n";
  o << "Announce:" << "\n";
  for(std::vector<std::vector<std::string> >::const_iterator tierIter =
        torrentAttrs->announceList.begin(),
        eoi = torrentAttrs->announceList.end(); tierIter != eoi; ++tierIter) {
    for(std::vector<std::string>::const_iterator i = (*tierIter).begin(),
          eoi2 = (*tierIter).end(); i != eoi2; ++i) {
      o << " " << *i;
    }
    o << "\n";
  }
  o << "Info Hash: "
    << util::toHex(torrentAttrs->infoHash) << "\n"
    << "Piece Length: "
    << util::abbrevSize(dctx->getPieceLength()) << "B\n"
    << "The Number of Pieces: "
    << dctx->getNumPieces() << "\n"
    << "Total Length: "
    << util::abbrevSize(dctx->getTotalLength()) << "B ("
    << util::uitos(dctx->getTotalLength(), true) << ")\n";
  if(!torrentAttrs->urlList.empty()) {
    o << "URL List: " << "\n";
    for(std::vector<std::string>::const_iterator i =
          torrentAttrs->urlList.begin(),
          eoi = torrentAttrs->urlList.end(); i != eoi; ++i) {
      o << " " << *i << "\n";
    }
  }
  o << "Name: " << torrentAttrs->name << "\n"
    << "Magnet URI: " << torrent2Magnet(torrentAttrs) << "\n";
  util::toStream
    (dctx->getFileEntries().begin(), dctx->getFileEntries().end(), o);
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
                  host, NI_MAXHOST, 0, 0,
                  NI_NUMERICHOST);
  if(s) {
    return std::pair<std::string, uint16_t>();
  }
  uint16_t port_nworder;
  memcpy(&port_nworder, compact+sizeof(uint32_t), sizeof(uint16_t));
  uint16_t port = ntohs(port_nworder);
  return std::make_pair(host, port);
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

SharedHandle<TorrentAttribute> parseMagnet(const std::string& magnet)
{
  SharedHandle<Dict> r = magnet::parse(magnet);
  if(r.isNull()) {
    throw DL_ABORT_EX("Bad BitTorrent Magnet URI.");
  }
  const List* xts = asList(r->get("xt"));
  if(!xts) {
    throw DL_ABORT_EX("Missing xt parameter in Magnet URI.");
  }
  SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
  std::string infoHash;
  for(List::ValueType::const_iterator xtiter = xts->begin(),
        eoi = xts->end(); xtiter != eoi && infoHash.empty(); ++xtiter) {
    const String* xt = asString(*xtiter);
    if(util::startsWith(xt->s(), "urn:btih:")) {
      std::string xtarg = xt->s().substr(9);
      size_t size = xtarg.size();
      if(size == 32) {
        std::string rawhash = base32::decode(xtarg);
        if(rawhash.size() == 20) {
          infoHash.swap(rawhash);
        }
      } else if(size == 40) {
        std::string rawhash = util::fromHex(xtarg);
        if(!rawhash.empty()) {
          infoHash.swap(rawhash);
        }
      }
    }
  }
  if(infoHash.empty()) {
    throw DL_ABORT_EX("Bad BitTorrent Magnet URI. "
                      "No valid BitTorrent Info Hash found.");
  }
  const List* trs = asList(r->get("tr"));
  if(trs) {
    for(List::ValueType::const_iterator i = trs->begin(), eoi = trs->end();
        i != eoi; ++i) {
      std::vector<std::string> tier;
      tier.push_back(asString(*i)->s());
      attrs->announceList.push_back(tier);
    }
  }
  std::string name = "[METADATA]";
  const List* dns = asList(r->get("dn"));
  if(dns && !dns->empty()) {
    const String* dn = asString(dns->get(0));
    name += dn->s();
  } else {
    name += util::toHex(infoHash);
  }
  attrs->infoHash = infoHash;
  attrs->name = name;
  return attrs;
}

void loadMagnet
(const std::string& magnet, const SharedHandle<DownloadContext>& dctx)
{
  SharedHandle<TorrentAttribute> attrs = parseMagnet(magnet);
  dctx->setAttribute(BITTORRENT, attrs);
}

std::string metadata2Torrent
(const std::string& metadata, const SharedHandle<TorrentAttribute>& attrs)
{
  std::string torrent = "d";

  List announceList;
  for(std::vector<std::vector<std::string> >::const_iterator tierIter =
        attrs->announceList.begin(),
        eoi = attrs->announceList.end(); tierIter != eoi; ++tierIter) {
    SharedHandle<List> tier = List::g();
    for(std::vector<std::string>::const_iterator uriIter = (*tierIter).begin(),
          eoi2 = (*tierIter).end(); uriIter != eoi2; ++uriIter) {
      tier->append(String::g(*uriIter));
    }
    if(!tier->empty()) {
      announceList.append(tier);
    }
  }
  if(!announceList.empty()) {
    torrent += "13:announce-list";
    torrent += bencode2::encode(&announceList);
  }
  torrent +=
    strconcat("4:info", metadata, "e");
  return torrent;
}

std::string torrent2Magnet(const SharedHandle<TorrentAttribute>& attrs)
{
  std::string uri = "magnet:?";
  if(!attrs->infoHash.empty()) {
    uri += "xt=urn:btih:";
    uri += util::toUpper(util::toHex(attrs->infoHash));
  } else {
    return A2STR::NIL;
  }
  if(!attrs->name.empty()) {
    uri += "&dn=";
    uri += util::percentEncode(attrs->name);
  }
  for(std::vector<std::vector<std::string> >::const_iterator tierIter =
        attrs->announceList.begin(),
        eoi = attrs->announceList.end(); tierIter != eoi; ++tierIter) {
    for(std::vector<std::string>::const_iterator uriIter = (*tierIter).begin(),
          eoi2 = (*tierIter).end(); uriIter != eoi2; ++uriIter) {
      uri += "&tr=";
      uri += util::percentEncode(*uriIter);
    }
  }
  return uri;
}

} // namespace bittorrent

} // namespace aria2
