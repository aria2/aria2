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
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
# include "MessageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_BITTORRENT
# include "magnet.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

MetalinkParserController::MetalinkParserController():
  metalinker_(new Metalinker())
{}

MetalinkParserController::~MetalinkParserController() {}

void MetalinkParserController::reset()
{
  metalinker_.reset(new Metalinker());
}

void MetalinkParserController::newEntryTransaction()
{
  tEntry_.reset(new MetalinkEntry());
  tResource_.reset();
  tMetaurl_.reset();
#ifdef ENABLE_MESSAGE_DIGEST
  tChecksum_.reset();
  tChunkChecksumV4_.reset();
  tChunkChecksum_.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setFileNameOfEntry(const std::string& filename)
{
  if(!tEntry_) {
    return;
  }
  if(!tEntry_->file) {
    tEntry_->file.reset(new FileEntry(util::escapePath(filename), 0, 0));
  } else {
    tEntry_->file->setPath(util::escapePath(filename));
  }
}

void MetalinkParserController::setFileLengthOfEntry(int64_t length)
{
  if(!tEntry_) {
    return;
  }
  if(!tEntry_->file) {
    return;
  }
  tEntry_->file->setLength(length);
  tEntry_->sizeKnown = true;
}

void MetalinkParserController::setVersionOfEntry(const std::string& version)
{
  if(!tEntry_) {
    return;
  }
  tEntry_->version = version;
}

void MetalinkParserController::setLanguageOfEntry(const std::string& language)
{
  if(!tEntry_) {
    return;
  }
  tEntry_->languages.push_back(language);
}

void MetalinkParserController::setOSOfEntry(const std::string& os)
{
  if(!tEntry_) {
    return;
  }
  tEntry_->oses.push_back(os);
}

void MetalinkParserController::setMaxConnectionsOfEntry(int maxConnections)
{
  if(!tEntry_) {
    return;
  }
  tEntry_->maxConnections = maxConnections;
}

void MetalinkParserController::commitEntryTransaction()
{
  if(!tEntry_) {
    return;
  }
  commitResourceTransaction();
  commitMetaurlTransaction();
  commitChecksumTransaction();
  commitChunkChecksumTransactionV4();
  commitChunkChecksumTransaction();
  commitSignatureTransaction();
  metalinker_->addEntry(tEntry_);
  tEntry_.reset();
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
  if(!tEntry_) {
    return;
  }
  tResource_.reset(new MetalinkResource());
}

void MetalinkParserController::setURLOfResource(const std::string& url)
{
  if(!tResource_) {
    return;
  }
  std::string u = uri::joinUri(baseUri_, url);
  uri::UriStruct us;
  if(uri::parse(us, u)) {
    tResource_->url = u;
    if(tResource_->type == MetalinkResource::TYPE_UNKNOWN) {
      setTypeOfResource(us.protocol);
    }
  } else {
    tResource_->url = url;
  }
}

void MetalinkParserController::setTypeOfResource(const std::string& type)
{
  if(!tResource_) {
    return;
  }
  if(type == MetalinkResource::FTP) {
    tResource_->type = MetalinkResource::TYPE_FTP;
  } else if(type == MetalinkResource::HTTP) {
    tResource_->type = MetalinkResource::TYPE_HTTP;
  } else if(type == MetalinkResource::HTTPS) {
    tResource_->type = MetalinkResource::TYPE_HTTPS;
  } else if(type == MetalinkResource::BITTORRENT) {
    tResource_->type = MetalinkResource::TYPE_BITTORRENT;
  } else if(type == MetalinkResource::TORRENT) { // Metalink4Spec
    tResource_->type = MetalinkResource::TYPE_BITTORRENT;
  } else {
    tResource_->type = MetalinkResource::TYPE_NOT_SUPPORTED;
  }
}

void MetalinkParserController::setLocationOfResource(const std::string& location)
{
  if(!tResource_) {
    return;
  }
  tResource_->location = location;
}

void MetalinkParserController::setPriorityOfResource(int priority)
{
  if(!tResource_) {
    return;
  }
  tResource_->priority = priority;
}

void MetalinkParserController::setMaxConnectionsOfResource(int maxConnections)
{
  if(!tResource_) {
    return;
  }
  tResource_->maxConnections = maxConnections;
}

void MetalinkParserController::commitResourceTransaction()
{
  if(!tResource_) {
    return;
  }
#ifdef ENABLE_BITTORRENT
  if(tResource_->type == MetalinkResource::TYPE_BITTORRENT) {
    SharedHandle<MetalinkMetaurl> metaurl(new MetalinkMetaurl());
    metaurl->url = tResource_->url;
    metaurl->priority = tResource_->priority;
    metaurl->mediatype = MetalinkMetaurl::MEDIATYPE_TORRENT;
    tEntry_->metaurls.push_back(metaurl);
  } else {
    tEntry_->resources.push_back(tResource_);
  }
#else // !ENABLE_BITTORRENT
  tEntry_->resources.push_back(tResource_);
#endif // !ENABLE_BITTORRENT
  tResource_.reset();
}

void MetalinkParserController::cancelResourceTransaction()
{
  tResource_.reset();
}

void MetalinkParserController::newChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tEntry_) {
    return;
  }
  tChecksum_.reset(new Checksum());
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setTypeOfChecksum(const std::string& type)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChecksum_) {
    return;
  }
  std::string calgo = MessageDigest::getCanonicalHashType(type);
  if(MessageDigest::supports(calgo)) {
    tChecksum_->setHashType(calgo);
  } else {
    cancelChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setHashOfChecksum(const std::string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChecksum_) {
    return;
  }
  if(MessageDigest::isValidHash(tChecksum_->getHashType(), md)) {
    tChecksum_->setDigest(util::fromHex(md.begin(), md.end()));
  } else {
    cancelChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::commitChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChecksum_) {
    return;
  }
  if(!tEntry_->checksum ||
     MessageDigest::isStronger(tChecksum_->getHashType(),
                               tEntry_->checksum->getHashType())) {
    tEntry_->checksum = tChecksum_;
  }
  tChecksum_.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::cancelChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  tChecksum_.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::newChunkChecksumTransactionV4()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tEntry_) {
    return;
  }
  tChunkChecksumV4_.reset(new ChunkChecksum());
  tempChunkChecksumsV4_.clear();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setTypeOfChunkChecksumV4(const std::string& type)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksumV4_) {
    return;
  }
  std::string calgo = MessageDigest::getCanonicalHashType(type);
  if(MessageDigest::supports(calgo)) {
    tChunkChecksumV4_->setHashType(calgo);
  } else {
    cancelChunkChecksumTransactionV4();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setLengthOfChunkChecksumV4(size_t length)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksumV4_) {
    return;
  }
  if(length > 0) {
    tChunkChecksumV4_->setPieceLength(length);
  } else {
    cancelChunkChecksumTransactionV4();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::addHashOfChunkChecksumV4(const std::string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksumV4_) {
    return;
  }
  if(MessageDigest::isValidHash(tChunkChecksumV4_->getHashType(), md)) {
    tempChunkChecksumsV4_.push_back(util::fromHex(md.begin(), md.end()));
  } else {
    cancelChunkChecksumTransactionV4();
  }
#endif // ENABLE_MESSAGE_DIGEST  
}

void MetalinkParserController::commitChunkChecksumTransactionV4()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksumV4_) {
    return;
  }
  if(!tEntry_->chunkChecksum ||
     MessageDigest::isStronger(tChunkChecksumV4_->getHashType(),
                               tEntry_->chunkChecksum->getHashType())) {
    std::vector<std::string> pieceHashes(tempChunkChecksumsV4_.begin(),
                                         tempChunkChecksumsV4_.end());
    tChunkChecksumV4_->setPieceHashes(pieceHashes);
    tEntry_->chunkChecksum = tChunkChecksumV4_;
  }
  tChunkChecksumV4_.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::cancelChunkChecksumTransactionV4()
{
#ifdef ENABLE_MESSAGE_DIGEST
  tChunkChecksumV4_.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::newChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tEntry_) {
    return;
  }
  tChunkChecksum_.reset(new ChunkChecksum());
  tempChunkChecksums_.clear();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setTypeOfChunkChecksum(const std::string& type)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksum_) {
    return;
  }
  std::string calgo = MessageDigest::getCanonicalHashType(type);
  if(MessageDigest::supports(calgo)) {
    tChunkChecksum_->setHashType(calgo);
  } else {
    cancelChunkChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setLengthOfChunkChecksum(size_t length)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksum_) {
    return;
  }
  if(length > 0) {
    tChunkChecksum_->setPieceLength(length);
  } else {
    cancelChunkChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::addHashOfChunkChecksum(size_t order, const std::string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksum_) {
    return;
  }
  if(MessageDigest::isValidHash(tChunkChecksum_->getHashType(), md)) {
    tempChunkChecksums_.push_back(std::make_pair(order, md));
  } else {
    cancelChunkChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::createNewHashOfChunkChecksum(size_t order)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksum_) {
    return;
  }
  tempHashPair_.first = order;
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setMessageDigestOfChunkChecksum(const std::string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksum_) {
    return;
  }
  if(MessageDigest::isValidHash(tChunkChecksum_->getHashType(), md)) {
    tempHashPair_.second = util::fromHex(md.begin(), md.end());
  } else {
    cancelChunkChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::addHashOfChunkChecksum()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksum_) {
    return;
  }
  tempChunkChecksums_.push_back(tempHashPair_);
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::commitChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(!tChunkChecksum_) {
    return;
  }
  if(!tEntry_->chunkChecksum ||
     MessageDigest::isStronger(tChunkChecksum_->getHashType(),
                               tEntry_->chunkChecksum->getHashType())) {
    std::sort(tempChunkChecksums_.begin(), tempChunkChecksums_.end(),
              Ascend1st<std::pair<size_t, std::string> >());
    std::vector<std::string> pieceHashes;
    std::transform(tempChunkChecksums_.begin(), tempChunkChecksums_.end(),
                   std::back_inserter(pieceHashes),
                   select2nd<std::pair<size_t, std::string> >());
    tChunkChecksum_->setPieceHashes(pieceHashes);
    tEntry_->chunkChecksum = tChunkChecksum_;
  }
  tChunkChecksum_.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::cancelChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  tChunkChecksum_.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::newSignatureTransaction()
{
  if(!tEntry_) {
    return;
  }
  tSignature_.reset(new Signature());
}

void MetalinkParserController::setTypeOfSignature(const std::string& type)
{
  if(!tSignature_) {
    return;
  }
  tSignature_->setType(type);
}

void MetalinkParserController::setFileOfSignature(const std::string& file)
{
  if(!tSignature_) {
    return;
  }
  tSignature_->setFile(file);
}

void MetalinkParserController::setBodyOfSignature(const std::string& body)
{
  if(!tSignature_) {
    return;
  }
  tSignature_->setBody(body);
}

void MetalinkParserController::commitSignatureTransaction()
{
  if(!tSignature_) {
    return;
  }
  tEntry_->setSignature(tSignature_);
  tSignature_.reset();
}

void MetalinkParserController::cancelSignatureTransaction()
{
  tSignature_.reset();
}

void MetalinkParserController::newMetaurlTransaction()
{
  if(!tEntry_) {
    return;
  }
  tMetaurl_.reset(new MetalinkMetaurl());
}

void MetalinkParserController::setURLOfMetaurl(const std::string& url)
{
  if(!tMetaurl_) {
    return;
  }
#ifdef ENABLE_BITTORRENT
  if(magnet::parse(url)) {
    tMetaurl_->url = url;
  } else
#endif // ENABLE_BITTORRENT
    {
      std::string u = uri::joinUri(baseUri_, url);
      uri::UriStruct us;
      if(uri::parse(us, u)) {
        tMetaurl_->url = u;
      } else {
        tMetaurl_->url = url;
      }
    }
}

void MetalinkParserController::setMediatypeOfMetaurl
(const std::string& mediatype)
{
  if(!tMetaurl_) {
    return;
  }
  tMetaurl_->mediatype = mediatype;
}

void MetalinkParserController::setPriorityOfMetaurl(int priority)
{
  if(!tMetaurl_) {
    return;
  }
  tMetaurl_->priority = priority;
}

void MetalinkParserController::setNameOfMetaurl(const std::string& name)
{
  if(!tMetaurl_) {
    return;
  }
  tMetaurl_->name = name;
}

void MetalinkParserController::commitMetaurlTransaction()
{
  if(!tMetaurl_) {
    return;
  }
#ifdef ENABLE_BITTORRENT
  if(tMetaurl_->mediatype == MetalinkMetaurl::MEDIATYPE_TORRENT) {
    tEntry_->metaurls.push_back(tMetaurl_);
  }
#endif // ENABLE_BITTORRENT
  tMetaurl_.reset();
}

void MetalinkParserController::cancelMetaurlTransaction()
{
  tMetaurl_.reset();
}

} // namespace aria2
