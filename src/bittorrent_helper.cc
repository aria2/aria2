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
#include "fmt.h"
#include "BtConstants.h"
#include "MessageDigest.h"
#include "message_digest_helper.h"
#include "a2netcompat.h"
#include "BtConstants.h"
#include "bitfield.h"
#include "base32.h"
#include "magnet.h"
#include "bencode2.h"
#include "TorrentAttribute.h"
#include "SocketCore.h"
#include "Option.h"
#include "prefs.h"
#include "FileEntry.h"
#include "error_code.h"
#include "array_fun.h"
#include "DownloadFailureException.h"
#include "ValueBaseBencodeParser.h"

namespace aria2 {

namespace bittorrent {

namespace {
const char C_NAME[] = "name";
const char C_NAME_UTF8[] = "name.utf-8";
const char C_FILES[] = "files";
const char C_LENGTH[] = "length";
const char C_PATH[] = "path";
const char C_PATH_UTF8[] = "path.utf-8";
const char C_INFO[] = "info";
const char C_PIECES[] = "pieces";
const char C_PIECE_LENGTH[] = "piece length";
const char C_PRIVATE[] = "private";
const char C_URL_LIST[] = "url-list";
const char C_ANNOUNCE[] = "announce";
const char C_ANNOUNCE_LIST[] = "announce-list";
const char C_NODES[] = "nodes";
const char C_CREATION_DATE[] = "creation date";
const char C_COMMENT[] = "comment";
const char C_COMMENT_UTF8[] = "comment.utf-8";
const char C_CREATED_BY[] = "created by";

const char DEFAULT_PEER_ID_PREFIX[] = "aria2-";
const char DEFAULT_PEER_AGENT[] = "aria2/" PACKAGE_VERSION;
} // namespace

const std::string MULTI("multi");

const std::string SINGLE("single");

namespace {
void extractPieceHash(const std::shared_ptr<DownloadContext>& ctx,
                      const std::string& hashData, size_t hashLength,
                      size_t numPieces)
{
  std::vector<std::string> pieceHashes;
  pieceHashes.reserve(numPieces);
  for (size_t i = 0; i < numPieces; ++i) {
    const char* p = hashData.data() + i * hashLength;
    pieceHashes.push_back(std::string(p, p + hashLength));
  }
  ctx->setPieceHashes("sha-1", pieceHashes.begin(), pieceHashes.end());
}
} // namespace

namespace {
void extractUrlList(TorrentAttribute* torrent, std::vector<std::string>& uris,
                    const ValueBase* v)
{
  class UrlListVisitor : public ValueBaseVisitor {
  private:
    std::vector<std::string>& uris_;
    TorrentAttribute* torrent_;

  public:
    UrlListVisitor(std::vector<std::string>& uris, TorrentAttribute* torrent)
        : uris_(uris), torrent_(torrent)
    {
    }

    virtual void visit(const String& v) CXX11_OVERRIDE
    {
      std::string utf8Uri = util::encodeNonUtf8(v.s());
      uris_.push_back(utf8Uri);
      torrent_->urlList.push_back(utf8Uri);
    }

    virtual void visit(const Integer& v) CXX11_OVERRIDE {}
    virtual void visit(const Bool& v) CXX11_OVERRIDE {}
    virtual void visit(const Null& v) CXX11_OVERRIDE {}

    virtual void visit(const List& v) CXX11_OVERRIDE
    {
      for (auto& elem : v) {
        const String* uri = downcast<String>(elem);
        if (uri) {
          std::string utf8Uri = util::encodeNonUtf8(uri->s());
          uris_.push_back(utf8Uri);
          torrent_->urlList.push_back(utf8Uri);
        }
      }
    }
    virtual void visit(const Dict& v) CXX11_OVERRIDE {}
  };

  if (v) {
    UrlListVisitor visitor(uris, torrent);
    v->accept(visitor);
  }
}
} // namespace

namespace {
template <typename InputIterator, typename OutputIterator>
OutputIterator createUri(InputIterator first, InputIterator last,
                         OutputIterator out, const std::string& filePath)
{
  for (; first != last; ++first) {
    if (!(*first).empty() && (*first)[(*first).size() - 1] == '/') {
      *out++ = (*first) + filePath;
    }
    else {
      *out++ = (*first) + "/" + filePath;
    }
  }
  return out;
}
} // namespace

namespace {
void extractFileEntries(const std::shared_ptr<DownloadContext>& ctx,
                        TorrentAttribute* torrent, const Dict* infoDict,
                        const std::shared_ptr<Option>& option,
                        const std::string& defaultName,
                        const std::string& overrideName,
                        const std::vector<std::string>& urlList)
{
  std::string utf8Name;
  if (overrideName.empty()) {
    std::string nameKey;
    if (infoDict->containsKey(C_NAME_UTF8)) {
      nameKey = C_NAME_UTF8;
    }
    else {
      nameKey = C_NAME;
    }
    const String* nameData = downcast<String>(infoDict->get(nameKey));
    if (nameData) {
      utf8Name = util::encodeNonUtf8(nameData->s());
      if (util::detectDirTraversal(utf8Name)) {
        throw DL_ABORT_EX2(
            fmt(MSG_DIR_TRAVERSAL_DETECTED, nameData->s().c_str()),
            error_code::BITTORRENT_PARSE_ERROR);
      }
    }
    else {
      utf8Name = File(defaultName).getBasename();
      utf8Name += ".file";
    }
  }
  else {
    utf8Name = overrideName;
  }
  torrent->name = utf8Name;
  int maxConn = option->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
  std::vector<std::shared_ptr<FileEntry>> fileEntries;
  const List* filesList = downcast<List>(infoDict->get(C_FILES));
  if (filesList) {
    fileEntries.reserve(filesList->size());
    int64_t length = 0;
    int64_t offset = 0;
    // multi-file mode
    torrent->mode = BT_FILE_MODE_MULTI;
    for (auto& f : *filesList) {
      const Dict* fileDict = downcast<Dict>(f);
      if (!fileDict) {
        continue;
      }
      const Integer* fileLengthData =
          downcast<Integer>(fileDict->get(C_LENGTH));
      if (!fileLengthData) {
        throw DL_ABORT_EX2(fmt(MSG_MISSING_BT_INFO, C_LENGTH),
                           error_code::BITTORRENT_PARSE_ERROR);
      }

      if (fileLengthData->i() < 0) {
        throw DL_ABORT_EX2(
            fmt(MSG_NEGATIVE_LENGTH_BT_INFO, C_LENGTH, fileLengthData->i()),
            error_code::BITTORRENT_PARSE_ERROR);
      }

      if (length > std::numeric_limits<int64_t>::max() - fileLengthData->i()) {
        throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, length));
      }
      length += fileLengthData->i();
      if (fileLengthData->i() > std::numeric_limits<a2_off_t>::max()) {
        throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, length));
      }
      std::string pathKey;
      if (fileDict->containsKey(C_PATH_UTF8)) {
        pathKey = C_PATH_UTF8;
      }
      else {
        pathKey = C_PATH;
      }
      const List* pathList = downcast<List>(fileDict->get(pathKey));
      if (!pathList || pathList->empty()) {
        throw DL_ABORT_EX2("Path is empty.",
                           error_code::BITTORRENT_PARSE_ERROR);
      }

      std::vector<std::string> pathelem(pathList->size() + 1);
      pathelem[0] = utf8Name;
      auto pathelemOutItr = pathelem.begin();
      ++pathelemOutItr;
      for (auto& p : *pathList) {
        const String* elem = downcast<String>(p);
        if (elem) {
          (*pathelemOutItr++) = elem->s();
        }
        else {
          throw DL_ABORT_EX2("Path element is not string.",
                             error_code::BITTORRENT_PARSE_ERROR);
        }
      }
      std::string utf8Path = strjoin(
          pathelem.begin(), pathelem.end(), "/",
          std::function<std::string(const std::string&)>(util::encodeNonUtf8));
      if (util::detectDirTraversal(utf8Path)) {
        throw DL_ABORT_EX2(fmt(MSG_DIR_TRAVERSAL_DETECTED, utf8Path.c_str()),
                           error_code::BITTORRENT_PARSE_ERROR);
      }
      std::string pePath =
          strjoin(pathelem.begin(), pathelem.end(), "/",
                  std::function<std::string(const std::string&)>(
                      static_cast<std::string (*)(const std::string&)>(
                          util::percentEncode)));
      std::vector<std::string> uris;
      createUri(urlList.begin(), urlList.end(), std::back_inserter(uris),
                pePath);

      auto suffixPath = util::escapePath(utf8Path);

      auto fileEntry = std::make_shared<FileEntry>(
          util::applyDir(option->get(PREF_DIR), suffixPath),
          fileLengthData->i(), offset, uris);
      fileEntry->setOriginalName(utf8Path);
      fileEntry->setSuffixPath(suffixPath);
      fileEntry->setMaxConnectionPerServer(maxConn);
      fileEntries.push_back(fileEntry);
      offset += fileEntry->getLength();
    }
  }
  else {
    // single-file mode;
    torrent->mode = BT_FILE_MODE_SINGLE;
    const Integer* lengthData = downcast<Integer>(infoDict->get(C_LENGTH));
    if (!lengthData) {
      throw DL_ABORT_EX2(fmt(MSG_MISSING_BT_INFO, C_LENGTH),
                         error_code::BITTORRENT_PARSE_ERROR);
    }
    int64_t totalLength = lengthData->i();

    if (totalLength < 0) {
      throw DL_ABORT_EX2(
          fmt(MSG_NEGATIVE_LENGTH_BT_INFO, C_LENGTH, totalLength),
          error_code::BITTORRENT_PARSE_ERROR);
    }

    if (totalLength > std::numeric_limits<a2_off_t>::max()) {
      throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, totalLength));
    }
    // For each uri in urlList, if it ends with '/', then
    // concatenate name to it. Specification just says so.
    std::vector<std::string> uris;
    for (auto& elem : urlList) {
      if (!elem.empty() && elem[elem.size() - 1] == '/') {
        uris.push_back(elem + util::percentEncode(utf8Name));
      }
      else {
        uris.push_back(elem);
      }
    }

    auto suffixPath = util::escapePath(utf8Name);

    auto fileEntry = std::make_shared<FileEntry>(
        util::applyDir(option->get(PREF_DIR), suffixPath), totalLength, 0,
        uris);
    fileEntry->setOriginalName(utf8Name);
    fileEntry->setSuffixPath(suffixPath);
    fileEntry->setMaxConnectionPerServer(maxConn);
    fileEntries.push_back(fileEntry);
  }
  ctx->setFileEntries(fileEntries.begin(), fileEntries.end());
  if (torrent->mode == BT_FILE_MODE_MULTI) {
    ctx->setBasePath(
        util::applyDir(option->get(PREF_DIR), util::escapePath(utf8Name)));
  }
}
} // namespace

namespace {
void extractAnnounce(TorrentAttribute* torrent, const Dict* rootDict)
{
  const List* announceList = downcast<List>(rootDict->get(C_ANNOUNCE_LIST));
  if (announceList) {
    for (auto& elem : *announceList) {
      const List* tier = downcast<List>(elem);
      if (!tier) {
        continue;
      }
      std::vector<std::string> ntier;
      for (auto& t : *tier) {
        const String* uri = downcast<String>(t);
        if (uri) {
          ntier.push_back(util::encodeNonUtf8(util::strip(uri->s())));
        }
      }
      if (!ntier.empty()) {
        torrent->announceList.push_back(ntier);
      }
    }
  }
  else {
    const String* announce = downcast<String>(rootDict->get(C_ANNOUNCE));
    if (announce) {
      std::vector<std::string> tier;
      tier.push_back(util::encodeNonUtf8(util::strip(announce->s())));
      torrent->announceList.push_back(tier);
    }
  }
}
} // namespace

namespace {
void extractNodes(TorrentAttribute* torrent, const ValueBase* nodesListSrc)
{
  const List* nodesList = downcast<List>(nodesListSrc);
  if (nodesList) {
    for (auto& elem : *nodesList) {
      const List* addrPairList = downcast<List>(elem);
      if (!addrPairList || addrPairList->size() != 2) {
        continue;
      }
      const String* hostname = downcast<String>(addrPairList->get(0));
      if (!hostname) {
        continue;
      }
      std::string utf8Hostname =
          util::encodeNonUtf8(util::strip(hostname->s()));
      if (utf8Hostname.empty()) {
        continue;
      }
      const Integer* port = downcast<Integer>(addrPairList->get(1));
      if (!port || !(0 < port->i() && port->i() < 65536)) {
        continue;
      }
      torrent->nodes.push_back(std::make_pair(utf8Hostname, port->i()));
    }
  }
}
} // namespace

namespace {
void processRootDictionary(const std::shared_ptr<DownloadContext>& ctx,
                           const ValueBase* root,
                           const std::shared_ptr<Option>& option,
                           const std::string& defaultName,
                           const std::string& overrideName,
                           const std::vector<std::string>& uris)
{
  const Dict* rootDict = downcast<Dict>(root);
  if (!rootDict) {
    throw DL_ABORT_EX2("torrent file does not contain a root dictionary.",
                       error_code::BITTORRENT_PARSE_ERROR);
  }
  const Dict* infoDict = downcast<Dict>(rootDict->get(C_INFO));
  if (!infoDict) {
    throw DL_ABORT_EX2(fmt(MSG_MISSING_BT_INFO, C_INFO),
                       error_code::BITTORRENT_PARSE_ERROR);
  }
  auto torrent = std::make_shared<TorrentAttribute>();

  // retrieve infoHash
  std::string encodedInfoDict = bencode2::encode(infoDict);
  unsigned char infoHash[INFO_HASH_LENGTH];
  message_digest::digest(infoHash, INFO_HASH_LENGTH,
                         MessageDigest::sha1().get(), encodedInfoDict.data(),
                         encodedInfoDict.size());
  torrent->infoHash.assign(&infoHash[0], &infoHash[INFO_HASH_LENGTH]);
  torrent->metadata = encodedInfoDict;
  torrent->metadataSize = encodedInfoDict.size();

  // calculate the number of pieces
  const String* piecesData = downcast<String>(infoDict->get(C_PIECES));
  if (!piecesData) {
    throw DL_ABORT_EX2(fmt(MSG_MISSING_BT_INFO, C_PIECES),
                       error_code::BITTORRENT_PARSE_ERROR);
  }
  // Commented out To download 0 length torrent.
  //   if(piecesData.s().empty()) {
  //     throw DL_ABORT_EX("The length of piece hash is 0.");
  //   }
  size_t numPieces = piecesData->s().size() / PIECE_HASH_LENGTH;
  // Commented out to download 0 length torrent.
  //   if(numPieces == 0) {
  //     throw DL_ABORT_EX("The number of pieces is 0.");
  //   }
  // retrieve piece length
  const Integer* pieceLengthData =
      downcast<Integer>(infoDict->get(C_PIECE_LENGTH));
  if (!pieceLengthData) {
    throw DL_ABORT_EX2(fmt(MSG_MISSING_BT_INFO, C_PIECE_LENGTH),
                       error_code::BITTORRENT_PARSE_ERROR);
  }

  if (pieceLengthData->i() < 0) {
    throw DL_ABORT_EX2(
        fmt(MSG_NEGATIVE_LENGTH_BT_INFO, C_PIECE_LENGTH, pieceLengthData->i()),
        error_code::BITTORRENT_PARSE_ERROR);
  }

  size_t pieceLength = pieceLengthData->i();
  ctx->setPieceLength(pieceLength);
  // retrieve piece hashes
  extractPieceHash(ctx, piecesData->s(), PIECE_HASH_LENGTH, numPieces);
  // private flag
  const Integer* privateData = downcast<Integer>(infoDict->get(C_PRIVATE));
  int privatefg = 0;
  if (privateData) {
    if (privateData->i() == 1) {
      privatefg = 1;
    }
  }
  if (privatefg) {
    torrent->privateTorrent = true;
  }
  // retrieve uri-list.
  // This implementation obeys HTTP-Seeding specification:
  // see http://www.getright.com/seedtorrent.html
  std::vector<std::string> urlList;
  extractUrlList(torrent.get(), urlList, rootDict->get(C_URL_LIST));
  urlList.insert(urlList.end(), uris.begin(), uris.end());
  std::sort(urlList.begin(), urlList.end());
  urlList.erase(std::unique(urlList.begin(), urlList.end()), urlList.end());

  // retrieve file entries
  extractFileEntries(ctx, torrent.get(), infoDict, option, defaultName,
                     overrideName, urlList);
  if ((ctx->getTotalLength() + pieceLength - 1) / pieceLength != numPieces) {
    throw DL_ABORT_EX2("Too few/many piece hash.",
                       error_code::BITTORRENT_PARSE_ERROR);
  }
  // retrieve announce
  extractAnnounce(torrent.get(), rootDict);
  // retrieve nodes
  extractNodes(torrent.get(), rootDict->get(C_NODES));

  const Integer* creationDate =
      downcast<Integer>(rootDict->get(C_CREATION_DATE));
  if (creationDate) {
    torrent->creationDate = creationDate->i();
  }
  const String* commentUtf8 = downcast<String>(rootDict->get(C_COMMENT_UTF8));
  if (commentUtf8) {
    torrent->comment = util::encodeNonUtf8(commentUtf8->s());
  }
  else {
    const String* comment = downcast<String>(rootDict->get(C_COMMENT));
    if (comment) {
      torrent->comment = util::encodeNonUtf8(comment->s());
    }
  }
  const String* createdBy = downcast<String>(rootDict->get(C_CREATED_BY));
  if (createdBy) {
    torrent->createdBy = util::encodeNonUtf8(createdBy->s());
  }

  ctx->setAttribute(CTX_ATTR_BT, std::move(torrent));
}
} // namespace

void load(const std::string& torrentFile,
          const std::shared_ptr<DownloadContext>& ctx,
          const std::shared_ptr<Option>& option,
          const std::string& overrideName)
{
  ValueBaseBencodeParser parser;
  processRootDictionary(ctx, parseFile(parser, torrentFile).get(), option,
                        torrentFile, overrideName, std::vector<std::string>());
}

void load(const std::string& torrentFile,
          const std::shared_ptr<DownloadContext>& ctx,
          const std::shared_ptr<Option>& option,
          const std::vector<std::string>& uris, const std::string& overrideName)
{
  ValueBaseBencodeParser parser;
  processRootDictionary(ctx, parseFile(parser, torrentFile).get(), option,
                        torrentFile, overrideName, uris);
}

void loadFromMemory(const unsigned char* content, size_t length,
                    const std::shared_ptr<DownloadContext>& ctx,
                    const std::shared_ptr<Option>& option,
                    const std::string& defaultName,
                    const std::string& overrideName)
{
  processRootDictionary(ctx, bencode2::decode(content, length).get(), option,
                        defaultName, overrideName, std::vector<std::string>());
}

void loadFromMemory(const unsigned char* content, size_t length,
                    const std::shared_ptr<DownloadContext>& ctx,
                    const std::shared_ptr<Option>& option,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName)
{
  processRootDictionary(ctx, bencode2::decode(content, length).get(), option,
                        defaultName, overrideName, uris);
}

void loadFromMemory(const std::string& context,
                    const std::shared_ptr<DownloadContext>& ctx,
                    const std::shared_ptr<Option>& option,
                    const std::string& defaultName,
                    const std::string& overrideName)
{
  processRootDictionary(ctx, bencode2::decode(context).get(), option,
                        defaultName, overrideName, std::vector<std::string>());
}

void loadFromMemory(const std::string& context,
                    const std::shared_ptr<DownloadContext>& ctx,
                    const std::shared_ptr<Option>& option,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName)
{
  processRootDictionary(ctx, bencode2::decode(context).get(), option,
                        defaultName, overrideName, uris);
}

void loadFromMemory(const ValueBase* torrent,
                    const std::shared_ptr<DownloadContext>& ctx,
                    const std::shared_ptr<Option>& option,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName)
{
  processRootDictionary(ctx, torrent, option, defaultName, overrideName, uris);
}

TorrentAttribute* getTorrentAttrs(const std::shared_ptr<DownloadContext>& dctx)
{
  return getTorrentAttrs(dctx.get());
}

TorrentAttribute* getTorrentAttrs(DownloadContext* dctx)
{
  return static_cast<TorrentAttribute*>(dctx->getAttribute(CTX_ATTR_BT).get());
}

const unsigned char* getInfoHash(const std::shared_ptr<DownloadContext>& dctx)
{
  return getInfoHash(dctx.get());
}

const unsigned char* getInfoHash(DownloadContext* dctx)
{
  return reinterpret_cast<const unsigned char*>(
      getTorrentAttrs(dctx)->infoHash.data());
}

std::string getInfoHashString(const std::shared_ptr<DownloadContext>& dctx)
{
  return getInfoHashString(dctx.get());
}

std::string getInfoHashString(DownloadContext* dctx)
{
  return util::toHex(getTorrentAttrs(dctx)->infoHash);
}

std::vector<size_t> computeFastSet(const std::string& ipaddr, size_t numPieces,
                                   const unsigned char* infoHash,
                                   size_t fastSetSize)
{
  std::vector<size_t> fastSet;
  unsigned char compact[COMPACT_LEN_IPV6];
  int compactlen = packcompact(compact, ipaddr, 0);
  if (compactlen != COMPACT_LEN_IPV4) {
    return fastSet;
  }
  if (numPieces < fastSetSize) {
    fastSetSize = numPieces;
  }
  unsigned char tx[24];
  memcpy(tx, compact, 4);
  if ((tx[0] & 0x80u) == 0 || (tx[0] & 0x40u) == 0) {
    tx[2] = 0x00u;
    tx[3] = 0x00u;
  }
  else {
    tx[3] = 0x00u;
  }
  memcpy(tx + 4, infoHash, 20);
  unsigned char x[20];
  auto sha1 = MessageDigest::sha1();
  message_digest::digest(x, sizeof(x), sha1.get(), tx, 24);
  while (fastSet.size() < fastSetSize) {
    for (size_t i = 0; i < 5 && fastSet.size() < fastSetSize; ++i) {
      size_t j = i * 4;
      uint32_t ny;
      memcpy(&ny, x + j, 4);
      uint32_t y = ntohl(ny);
      size_t index = y % numPieces;
      if (std::find(std::begin(fastSet), std::end(fastSet), index) ==
          std::end(fastSet)) {

        fastSet.push_back(index);
      }
    }
    unsigned char temp[20];
    sha1->reset();
    message_digest::digest(temp, sizeof(temp), sha1.get(), x, sizeof(x));
    memcpy(x, temp, sizeof(x));
  }

  return fastSet;
}

std::string generatePeerId(const std::string& peerIdPrefix)
{
  std::string peerId = peerIdPrefix;
  unsigned char buf[20];
  int len = 20 - peerIdPrefix.size();
  if (len > 0) {
    util::generateRandomData(buf, len);
    peerId.append(&buf[0], &buf[len]);
  }
  if (peerId.size() > 20) {
    peerId.erase(20);
  }
  return peerId;
}

namespace {
std::string peerId;
std::string peerAgent;
} // namespace

const std::string& generateStaticPeerId(const std::string& peerIdPrefix)
{
  if (peerId.empty()) {
    peerId = generatePeerId(peerIdPrefix);
  }
  return peerId;
}

const std::string& generateStaticPeerAgent(const std::string& peerAgentNew)
{
  if (peerAgent.empty()) {
    peerAgent = peerAgentNew;
  }
  return peerAgent;
}

void setStaticPeerId(const std::string& newPeerId) { peerId = newPeerId; }
void setStaticPeerAgent(const std::string& newPeerAgent)
{
  peerAgent = newPeerAgent;
}

// If PeerID is not generated, it is created with default peerIdPrefix
// (aria2-).
const unsigned char* getStaticPeerId()
{
  if (peerId.empty()) {
    return reinterpret_cast<const unsigned char*>(
        generateStaticPeerId(DEFAULT_PEER_ID_PREFIX).data());
  }
  else {
    return reinterpret_cast<const unsigned char*>(peerId.data());
  }
}

// If PeerAgent is not generated, it is created with default agent
// aria2/PACKAGE_VERSION
const std::string& getStaticPeerAgent()
{
  if (peerAgent.empty()) {
    generateStaticPeerAgent(DEFAULT_PEER_AGENT);
  }
  return peerAgent;
}

uint8_t getId(const unsigned char* msg) { return msg[0]; }

uint64_t getLLIntParam(const unsigned char* msg, size_t pos)
{
  uint64_t nParam;
  memcpy(&nParam, msg + pos, sizeof(nParam));
  return ntoh64(nParam);
}

uint32_t getIntParam(const unsigned char* msg, size_t pos)
{
  uint32_t nParam;
  memcpy(&nParam, msg + pos, sizeof(nParam));
  return ntohl(nParam);
}

uint16_t getShortIntParam(const unsigned char* msg, size_t pos)
{
  uint16_t nParam;
  memcpy(&nParam, msg + pos, sizeof(nParam));
  return ntohs(nParam);
}

void checkIndex(size_t index, size_t pieces)
{
  if (!(index < pieces)) {
    throw DL_ABORT_EX(
        fmt("Invalid index: %lu", static_cast<unsigned long>(index)));
  }
}

void checkBegin(int32_t begin, int32_t pieceLength)
{
  if (!(begin < pieceLength)) {
    throw DL_ABORT_EX(fmt("Invalid begin: %d", begin));
  }
}

void checkLength(int32_t length)
{
  if (length > static_cast<int32_t>(MAX_BLOCK_LENGTH)) {
    throw DL_ABORT_EX(fmt("Length too long: %d > %dKB", length,
                          static_cast<int32_t>(MAX_BLOCK_LENGTH / 1024)));
  }
  if (length == 0) {
    throw DL_ABORT_EX(fmt("Invalid length: %d", length));
  }
}

void checkRange(int32_t begin, int32_t length, int32_t pieceLength)
{
  if (!(0 < length)) {
    throw DL_ABORT_EX(fmt("Invalid range: begin=%d, length=%d", begin, length));
  }
  int32_t end = begin + length;
  if (!(end <= pieceLength)) {
    throw DL_ABORT_EX(fmt("Invalid range: begin=%d, length=%d", begin, length));
  }
}

void checkBitfield(const unsigned char* bitfield, size_t bitfieldLength,
                   size_t pieces)
{
  if (!(bitfieldLength == (pieces + 7) / 8)) {
    throw DL_ABORT_EX(fmt("Invalid bitfield length: %lu",
                          static_cast<unsigned long>(bitfieldLength)));
  }
  // Check if last byte contains garbage set bit.
  if (bitfield[bitfieldLength - 1] & ~bitfield::lastByteMask(pieces)) {
    throw DL_ABORT_EX("Invalid bitfield");
  }
}

void setLLIntParam(unsigned char* dest, uint64_t param)
{
  uint64_t nParam = hton64(param);
  memcpy(dest, &nParam, sizeof(nParam));
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

void createPeerMessageString(unsigned char* msg, size_t msgLength,
                             size_t payloadLength, uint8_t messageId)
{
  assert(msgLength >= 5);
  memset(msg, 0, msgLength);
  setIntParam(msg, payloadLength);
  msg[4] = messageId;
}

size_t packcompact(unsigned char* compact, const std::string& addr,
                   uint16_t port)
{
  size_t len = net::getBinAddr(compact, addr);
  if (len == 0) {
    return 0;
  }
  uint16_t portN = htons(port);
  memcpy(compact + len, &portN, sizeof(portN));
  return len + 2;
}

std::pair<std::string, uint16_t> unpackcompact(const unsigned char* compact,
                                               int family)
{
  std::pair<std::string, uint16_t> r;
  int portOffset = family == AF_INET ? 4 : 16;
  char buf[NI_MAXHOST];
  if (inetNtop(family, compact, buf, sizeof(buf)) == 0) {
    r.first = buf;
    uint16_t portN;
    memcpy(&portN, compact + portOffset, sizeof(portN));
    r.second = ntohs(portN);
  }
  return r;
}

void assertPayloadLengthGreater(size_t threshold, size_t actual,
                                const char* msgName)
{
  if (actual <= threshold) {
    throw DL_ABORT_EX(fmt(MSG_TOO_SMALL_PAYLOAD_SIZE, msgName,
                          static_cast<unsigned long>(actual)));
  }
}

void assertPayloadLengthEqual(size_t expected, size_t actual,
                              const char* msgName)
{
  if (expected != actual) {
    throw DL_ABORT_EX(fmt(EX_INVALID_PAYLOAD_SIZE, msgName,
                          static_cast<unsigned long>(actual),
                          static_cast<unsigned long>(expected)));
  }
}

void assertID(uint8_t expected, const unsigned char* data, const char* msgName)
{
  uint8_t id = getId(data);
  if (expected != id) {
    throw DL_ABORT_EX(fmt(EX_INVALID_BT_MESSAGE_ID, id, msgName, expected));
  }
}

std::unique_ptr<TorrentAttribute> parseMagnet(const std::string& magnet)
{
  auto r = magnet::parse(magnet);
  if (!r) {
    throw DL_ABORT_EX2("Bad BitTorrent Magnet URI.",
                       error_code::MAGNET_PARSE_ERROR);
  }
  const List* xts = downcast<List>(r->get("xt"));
  if (!xts) {
    throw DL_ABORT_EX2("Missing xt parameter in Magnet URI.",
                       error_code::MAGNET_PARSE_ERROR);
  }
  auto attrs = make_unique<TorrentAttribute>();
  std::string infoHash;
  for (auto xtiter = xts->begin(), eoi = xts->end();
       xtiter != eoi && infoHash.empty(); ++xtiter) {
    const String* xt = downcast<String>(*xtiter);
    if (util::startsWith(xt->s(), "urn:btih:")) {
      size_t size = xt->s().end() - xt->s().begin() - 9;
      if (size == 32) {
        std::string rawhash =
            base32::decode(xt->s().begin() + 9, xt->s().end());
        if (rawhash.size() == 20) {
          infoHash.swap(rawhash);
        }
      }
      else if (size == 40) {
        std::string rawhash = util::fromHex(xt->s().begin() + 9, xt->s().end());
        if (!rawhash.empty()) {
          infoHash.swap(rawhash);
        }
      }
    }
  }
  if (infoHash.empty()) {
    throw DL_ABORT_EX2("Bad BitTorrent Magnet URI. "
                       "No valid BitTorrent Info Hash found.",
                       error_code::MAGNET_PARSE_ERROR);
  }
  const List* trs = downcast<List>(r->get("tr"));
  if (trs) {
    for (auto& tr : *trs) {
      std::vector<std::string> tier;
      tier.push_back(util::encodeNonUtf8(downcast<String>(tr)->s()));
      attrs->announceList.push_back(tier);
    }
  }
  std::string name = "[METADATA]";
  const List* dns = downcast<List>(r->get("dn"));
  if (dns && !dns->empty()) {
    const String* dn = downcast<String>(dns->get(0));
    name += util::encodeNonUtf8(dn->s());
  }
  else {
    name += util::toHex(infoHash);
  }
  attrs->infoHash = infoHash;
  attrs->name = name;
  return attrs;
}

void loadMagnet(const std::string& magnet,
                const std::shared_ptr<DownloadContext>& dctx)
{
  dctx->setAttribute(CTX_ATTR_BT, parseMagnet(magnet));
}

std::string metadata2Torrent(const std::string& metadata,
                             const TorrentAttribute* attrs)
{
  std::string torrent = "d";

  List announceList;
  for (auto& elem : attrs->announceList) {
    auto tier = List::g();
    for (auto& uri : elem) {
      tier->append(uri);
    }
    if (!tier->empty()) {
      announceList.append(std::move(tier));
    }
  }
  if (!announceList.empty()) {
    torrent += "13:announce-list";
    torrent += bencode2::encode(&announceList);
  }
  torrent += "4:info";
  torrent += metadata;
  torrent += "e";
  return torrent;
}

std::string torrent2Magnet(const TorrentAttribute* attrs)
{
  std::string uri = "magnet:?";
  if (!attrs->infoHash.empty()) {
    uri += "xt=urn:btih:";
    uri += util::toUpper(util::toHex(attrs->infoHash));
  }
  else {
    return A2STR::NIL;
  }
  if (!attrs->name.empty()) {
    uri += "&dn=";
    uri += util::percentEncode(attrs->name);
  }
  for (auto& elem : attrs->announceList) {
    for (auto& e : elem) {
      uri += "&tr=";
      uri += util::percentEncode(e);
    }
  }
  return uri;
}

int getCompactLength(int family)
{
  if (family == AF_INET) {
    return COMPACT_LEN_IPV4;
  }
  else if (family == AF_INET6) {
    return COMPACT_LEN_IPV6;
  }
  else {
    return 0;
  }
}

void removeAnnounceUri(TorrentAttribute* attrs,
                       const std::vector<std::string>& uris)
{
  if (uris.empty()) {
    return;
  }
  if (std::find(uris.begin(), uris.end(), "*") == uris.end()) {
    for (auto i = attrs->announceList.begin();
         i != attrs->announceList.end();) {
      for (auto j = (*i).begin(); j != (*i).end();) {
        if (std::find(uris.begin(), uris.end(), *j) == uris.end()) {
          ++j;
        }
        else {
          j = (*i).erase(j);
        }
      }
      if ((*i).empty()) {
        i = attrs->announceList.erase(i);
      }
      else {
        ++i;
      }
    }
  }
  else {
    attrs->announceList.clear();
  }
}

void addAnnounceUri(TorrentAttribute* attrs,
                    const std::vector<std::string>& uris)
{
  for (auto& uri : uris) {
    std::vector<std::string> tier;
    tier.push_back(uri);
    attrs->announceList.push_back(tier);
  }
}

void adjustAnnounceUri(TorrentAttribute* attrs,
                       const std::shared_ptr<Option>& option)
{
  std::vector<std::string> excludeUris;
  std::vector<std::string> addUris;
  const std::string& exTracker = option->get(PREF_BT_EXCLUDE_TRACKER);
  util::split(exTracker.begin(), exTracker.end(),
              std::back_inserter(excludeUris), ',', true);
  const std::string& btTracker = option->get(PREF_BT_TRACKER);
  util::split(btTracker.begin(), btTracker.end(), std::back_inserter(addUris),
              ',', true);
  removeAnnounceUri(attrs, excludeUris);
  addAnnounceUri(attrs, addUris);
}

const char* getModeString(BtFileMode mode)
{
  switch (mode) {
  case BT_FILE_MODE_SINGLE:
    return "single";
  case BT_FILE_MODE_MULTI:
    return "multi";
  default:
    return "";
  }
}

} // namespace bittorrent

} // namespace aria2
