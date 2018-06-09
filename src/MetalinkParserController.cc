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
#include "MetalinkParserController.h"

#include <algorithm>

#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include "MetalinkMetaurl.h"
#include "FileEntry.h"
#include "a2functional.h"
#include "A2STR.h"
#include "uri.h"
#include "Signature.h"
#include "util.h"
#include "Checksum.h"
#include "ChunkChecksum.h"
#include "MessageDigest.h"
#ifdef ENABLE_BITTORRENT
#  include "magnet.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

MetalinkParserController::MetalinkParserController()
    : metalinker_{make_unique<Metalinker>()}
{
}

MetalinkParserController::~MetalinkParserController() = default;

void MetalinkParserController::reset()
{
  metalinker_ = make_unique<Metalinker>();
}

std::unique_ptr<Metalinker> MetalinkParserController::getResult()
{
  return std::move(metalinker_);
}

void MetalinkParserController::newEntryTransaction()
{
  tEntry_ = make_unique<MetalinkEntry>();
  tResource_.reset();
  tMetaurl_.reset();
  tChecksum_.reset();
  tChunkChecksumV4_.reset();
  tChunkChecksum_.reset();
}

void MetalinkParserController::setFileNameOfEntry(std::string filename)
{
  if (!tEntry_) {
    return;
  }
  if (!tEntry_->file) {
    tEntry_->file = make_unique<FileEntry>(util::escapePath(filename), 0, 0);
  }
  else {
    tEntry_->file->setPath(util::escapePath(filename));
  }
}

void MetalinkParserController::setFileLengthOfEntry(int64_t length)
{
  if (!tEntry_) {
    return;
  }
  if (!tEntry_->file) {
    return;
  }
  tEntry_->file->setLength(length);
  tEntry_->sizeKnown = true;
}

void MetalinkParserController::setVersionOfEntry(std::string version)
{
  if (!tEntry_) {
    return;
  }
  tEntry_->version = std::move(version);
}

void MetalinkParserController::setLanguageOfEntry(std::string language)
{
  if (!tEntry_) {
    return;
  }
  tEntry_->languages.push_back(std::move(language));
}

void MetalinkParserController::setOSOfEntry(std::string os)
{
  if (!tEntry_) {
    return;
  }
  tEntry_->oses.push_back(std::move(os));
}

void MetalinkParserController::setMaxConnectionsOfEntry(int maxConnections)
{
  if (!tEntry_) {
    return;
  }
  tEntry_->maxConnections = maxConnections;
}

void MetalinkParserController::commitEntryTransaction()
{
  if (!tEntry_) {
    return;
  }
  commitResourceTransaction();
  commitMetaurlTransaction();
  commitChecksumTransaction();
  commitChunkChecksumTransactionV4();
  commitChunkChecksumTransaction();
  commitSignatureTransaction();
  metalinker_->addEntry(std::move(tEntry_));
}

void MetalinkParserController::cancelEntryTransaction()
{
  cancelResourceTransaction();
  cancelMetaurlTransaction();
  cancelChecksumTransaction();
  cancelChunkChecksumTransactionV4();
  cancelChunkChecksumTransaction();
  cancelSignatureTransaction();
  tEntry_.reset();
}

void MetalinkParserController::newResourceTransaction()
{
  if (!tEntry_) {
    return;
  }
  tResource_ = make_unique<MetalinkResource>();
}

void MetalinkParserController::setURLOfResource(std::string url)
{
  if (!tResource_) {
    return;
  }
  std::string u = uri::joinUri(baseUri_, url);
  uri_split_result us;
  if (uri_split(&us, u.c_str()) == 0) {
    tResource_->url = std::move(u);
    if (tResource_->type == MetalinkResource::TYPE_UNKNOWN) {
      setTypeOfResource(
          uri::getFieldString(us, USR_SCHEME, tResource_->url.c_str()));
    }
  }
  else {
    tResource_->url = std::move(url);
  }
}

void MetalinkParserController::setTypeOfResource(std::string type)
{
  if (!tResource_) {
    return;
  }
  if (type == "ftp" || type == "sftp") {
    tResource_->type = MetalinkResource::TYPE_FTP;
  }
  else if (type == "http") {
    tResource_->type = MetalinkResource::TYPE_HTTP;
  }
  else if (type == "https") {
    tResource_->type = MetalinkResource::TYPE_HTTPS;
  }
  else if (type == "bittorrent" || type == "torrent") {
    // "torrent" is Metalink4Spec
    tResource_->type = MetalinkResource::TYPE_BITTORRENT;
  }
  else {
    tResource_->type = MetalinkResource::TYPE_NOT_SUPPORTED;
  }
}

void MetalinkParserController::setLocationOfResource(std::string location)
{
  if (!tResource_) {
    return;
  }
  tResource_->location = std::move(location);
}

void MetalinkParserController::setPriorityOfResource(int priority)
{
  if (!tResource_) {
    return;
  }
  tResource_->priority = priority;
}

void MetalinkParserController::setMaxConnectionsOfResource(int maxConnections)
{
  if (!tResource_) {
    return;
  }
  tResource_->maxConnections = maxConnections;
}

void MetalinkParserController::commitResourceTransaction()
{
  if (!tResource_) {
    return;
  }
#ifdef ENABLE_BITTORRENT
  if (tResource_->type == MetalinkResource::TYPE_BITTORRENT) {
    auto metaurl = make_unique<MetalinkMetaurl>();
    metaurl->url = std::move(tResource_->url);
    metaurl->priority = tResource_->priority;
    metaurl->mediatype = MetalinkMetaurl::MEDIATYPE_TORRENT;
    tEntry_->metaurls.push_back(std::move(metaurl));
  }
  else {
    tEntry_->resources.push_back(std::move(tResource_));
  }
#else  // !ENABLE_BITTORRENT
  tEntry_->resources.push_back(std::move(tResource_));
#endif // !ENABLE_BITTORRENT
  tResource_.reset();
}

void MetalinkParserController::cancelResourceTransaction()
{
  tResource_.reset();
}

void MetalinkParserController::newChecksumTransaction()
{
  if (!tEntry_) {
    return;
  }
  tChecksum_ = make_unique<Checksum>();
}

void MetalinkParserController::setTypeOfChecksum(std::string type)
{
  if (!tChecksum_) {
    return;
  }
  std::string calgo = MessageDigest::getCanonicalHashType(type);
  if (MessageDigest::supports(calgo)) {
    tChecksum_->setHashType(std::move(calgo));
  }
  else {
    cancelChecksumTransaction();
  }
}

void MetalinkParserController::setHashOfChecksum(std::string md)
{
  if (!tChecksum_) {
    return;
  }
  if (MessageDigest::isValidHash(tChecksum_->getHashType(), md)) {
    tChecksum_->setDigest(util::fromHex(md.begin(), md.end()));
  }
  else {
    cancelChecksumTransaction();
  }
}

void MetalinkParserController::commitChecksumTransaction()
{
  if (!tChecksum_) {
    return;
  }
  if (!tEntry_->checksum ||
      MessageDigest::isStronger(tChecksum_->getHashType(),
                                tEntry_->checksum->getHashType())) {
    tEntry_->checksum = std::move(tChecksum_);
  }
  tChecksum_.reset();
}

void MetalinkParserController::cancelChecksumTransaction()
{
  tChecksum_.reset();
}

void MetalinkParserController::newChunkChecksumTransactionV4()
{
  if (!tEntry_) {
    return;
  }
  tChunkChecksumV4_ = make_unique<ChunkChecksum>();
  tempChunkChecksumsV4_.clear();
}

void MetalinkParserController::setTypeOfChunkChecksumV4(std::string type)
{
  if (!tChunkChecksumV4_) {
    return;
  }
  std::string calgo = MessageDigest::getCanonicalHashType(type);
  if (MessageDigest::supports(calgo)) {
    tChunkChecksumV4_->setHashType(std::move(calgo));
  }
  else {
    cancelChunkChecksumTransactionV4();
  }
}

void MetalinkParserController::setLengthOfChunkChecksumV4(size_t length)
{
  if (!tChunkChecksumV4_) {
    return;
  }
  if (length > 0) {
    tChunkChecksumV4_->setPieceLength(length);
  }
  else {
    cancelChunkChecksumTransactionV4();
  }
}

void MetalinkParserController::addHashOfChunkChecksumV4(std::string md)
{
  if (!tChunkChecksumV4_) {
    return;
  }
  if (MessageDigest::isValidHash(tChunkChecksumV4_->getHashType(), md)) {
    tempChunkChecksumsV4_.push_back(util::fromHex(md.begin(), md.end()));
  }
  else {
    cancelChunkChecksumTransactionV4();
  }
}

void MetalinkParserController::commitChunkChecksumTransactionV4()
{
  if (!tChunkChecksumV4_) {
    return;
  }
  if (!tEntry_->chunkChecksum ||
      MessageDigest::isStronger(tChunkChecksumV4_->getHashType(),
                                tEntry_->chunkChecksum->getHashType())) {
    tChunkChecksumV4_->setPieceHashes(std::move(tempChunkChecksumsV4_));
    tEntry_->chunkChecksum = std::move(tChunkChecksumV4_);
  }
  tChunkChecksumV4_.reset();
}

void MetalinkParserController::cancelChunkChecksumTransactionV4()
{
  tChunkChecksumV4_.reset();
}

void MetalinkParserController::newChunkChecksumTransaction()
{
  if (!tEntry_) {
    return;
  }
  tChunkChecksum_ = make_unique<ChunkChecksum>();
  tempChunkChecksums_.clear();
}

void MetalinkParserController::setTypeOfChunkChecksum(std::string type)
{
  if (!tChunkChecksum_) {
    return;
  }
  std::string calgo = MessageDigest::getCanonicalHashType(type);
  if (MessageDigest::supports(calgo)) {
    tChunkChecksum_->setHashType(std::move(calgo));
  }
  else {
    cancelChunkChecksumTransaction();
  }
}

void MetalinkParserController::setLengthOfChunkChecksum(size_t length)
{
  if (!tChunkChecksum_) {
    return;
  }
  if (length > 0) {
    tChunkChecksum_->setPieceLength(length);
  }
  else {
    cancelChunkChecksumTransaction();
  }
}

void MetalinkParserController::addHashOfChunkChecksum(size_t order,
                                                      std::string md)
{
  if (!tChunkChecksum_) {
    return;
  }
  if (MessageDigest::isValidHash(tChunkChecksum_->getHashType(), md)) {
    tempChunkChecksums_.push_back(std::make_pair(order, std::move(md)));
  }
  else {
    cancelChunkChecksumTransaction();
  }
}

void MetalinkParserController::createNewHashOfChunkChecksum(size_t order)
{
  if (!tChunkChecksum_) {
    return;
  }
  tempHashPair_.first = order;
}

void MetalinkParserController::setMessageDigestOfChunkChecksum(std::string md)
{
  if (!tChunkChecksum_) {
    return;
  }
  if (MessageDigest::isValidHash(tChunkChecksum_->getHashType(), md)) {
    tempHashPair_.second = util::fromHex(md.begin(), md.end());
  }
  else {
    cancelChunkChecksumTransaction();
  }
}

void MetalinkParserController::addHashOfChunkChecksum()
{
  if (!tChunkChecksum_) {
    return;
  }
  tempChunkChecksums_.push_back(tempHashPair_);
}

void MetalinkParserController::commitChunkChecksumTransaction()
{
  if (!tChunkChecksum_) {
    return;
  }
  if (!tEntry_->chunkChecksum ||
      MessageDigest::isStronger(tChunkChecksum_->getHashType(),
                                tEntry_->chunkChecksum->getHashType())) {
    std::sort(std::begin(tempChunkChecksums_), std::end(tempChunkChecksums_));
    std::vector<std::string> pieceHashes;
    std::transform(
        std::begin(tempChunkChecksums_), std::end(tempChunkChecksums_),
        std::back_inserter(pieceHashes),
        [](const std::pair<size_t, std::string>& p) { return p.second; });
    tChunkChecksum_->setPieceHashes(std::move(pieceHashes));
    tEntry_->chunkChecksum = std::move(tChunkChecksum_);
  }
  tChunkChecksum_.reset();
}

void MetalinkParserController::cancelChunkChecksumTransaction()
{
  tChunkChecksum_.reset();
}

void MetalinkParserController::newSignatureTransaction()
{
  if (!tEntry_) {
    return;
  }
  tSignature_ = make_unique<Signature>();
}

void MetalinkParserController::setTypeOfSignature(std::string type)
{
  if (!tSignature_) {
    return;
  }
  tSignature_->setType(std::move(type));
}

void MetalinkParserController::setFileOfSignature(std::string file)
{
  if (!tSignature_) {
    return;
  }
  tSignature_->setFile(std::move(file));
}

void MetalinkParserController::setBodyOfSignature(std::string body)
{
  if (!tSignature_) {
    return;
  }
  tSignature_->setBody(std::move(body));
}

void MetalinkParserController::commitSignatureTransaction()
{
  if (!tSignature_) {
    return;
  }
  tEntry_->setSignature(std::move(tSignature_));
}

void MetalinkParserController::cancelSignatureTransaction()
{
  tSignature_.reset();
}

void MetalinkParserController::newMetaurlTransaction()
{
  if (!tEntry_) {
    return;
  }
  tMetaurl_ = make_unique<MetalinkMetaurl>();
}

void MetalinkParserController::setURLOfMetaurl(std::string url)
{
  if (!tMetaurl_) {
    return;
  }
#ifdef ENABLE_BITTORRENT
  if (magnet::parse(url)) {
    tMetaurl_->url = std::move(url);
  }
  else
#endif // ENABLE_BITTORRENT
  {
    std::string u = uri::joinUri(baseUri_, url);
    if (uri_split(nullptr, u.c_str()) == 0) {
      tMetaurl_->url = std::move(u);
    }
    else {
      tMetaurl_->url = std::move(url);
    }
  }
}

void MetalinkParserController::setMediatypeOfMetaurl(std::string mediatype)
{
  if (!tMetaurl_) {
    return;
  }
  tMetaurl_->mediatype = std::move(mediatype);
}

void MetalinkParserController::setPriorityOfMetaurl(int priority)
{
  if (!tMetaurl_) {
    return;
  }
  tMetaurl_->priority = priority;
}

void MetalinkParserController::setNameOfMetaurl(std::string name)
{
  if (!tMetaurl_) {
    return;
  }
  tMetaurl_->name = std::move(name);
}

void MetalinkParserController::commitMetaurlTransaction()
{
  if (!tMetaurl_) {
    return;
  }
#ifdef ENABLE_BITTORRENT
  if (tMetaurl_->mediatype == MetalinkMetaurl::MEDIATYPE_TORRENT) {
    tEntry_->metaurls.push_back(std::move(tMetaurl_));
  }
#endif // ENABLE_BITTORRENT
  tMetaurl_.reset();
}

void MetalinkParserController::cancelMetaurlTransaction() { tMetaurl_.reset(); }

} // namespace aria2
