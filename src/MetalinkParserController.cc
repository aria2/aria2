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
#include <vector>

#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include "MetalinkMetaurl.h"
#include "FileEntry.h"
#include "a2functional.h"
#include "A2STR.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
# include "messageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "Signature.h"
#include "util.h"

namespace aria2 {

const std::string MetalinkParserController::SHA1("sha1");// Metalink3Spec

MetalinkParserController::MetalinkParserController():
  _metalinker(new Metalinker())
{}

MetalinkParserController::~MetalinkParserController() {}

void MetalinkParserController::newEntryTransaction()
{
  _tEntry.reset(new MetalinkEntry());
  _tResource.reset();
  _tMetaurl.reset();
#ifdef ENABLE_MESSAGE_DIGEST
  _tChecksum.reset();
  _tChunkChecksumV4.reset();
  _tChunkChecksum.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setFileNameOfEntry(const std::string& filename)
{
  if(_tEntry.isNull()) {
    return;
  }
  std::vector<std::string> elements;
  util::split(filename, std::back_inserter(elements), "/");
  std::string path = util::joinPath(elements.begin(), elements.end());

  if(_tEntry->file.isNull()) {
    _tEntry->file.reset(new FileEntry(path, 0, 0));
  } else {
    _tEntry->file->setPath(path);
  }
}

void MetalinkParserController::setFileLengthOfEntry(uint64_t length)
{
  if(_tEntry.isNull()) {
    return;
  }
  if(_tEntry->file.isNull()) {
    _tEntry->file.reset(new FileEntry(A2STR::NIL, length, 0));
  } else {
    _tEntry->file->setLength(length);
  }
}

void MetalinkParserController::setVersionOfEntry(const std::string& version)
{
  if(_tEntry.isNull()) {
    return;
  }
  _tEntry->version = version;
}

void MetalinkParserController::setLanguageOfEntry(const std::string& language)
{
  if(_tEntry.isNull()) {
    return;
  }
  _tEntry->languages.push_back(language);
}

void MetalinkParserController::setOSOfEntry(const std::string& os)
{
  if(_tEntry.isNull()) {
    return;
  }
  _tEntry->oses.push_back(os);
}

void MetalinkParserController::setMaxConnectionsOfEntry(int maxConnections)
{
  if(_tEntry.isNull()) {
    return;
  }
  _tEntry->maxConnections = maxConnections;
}

void MetalinkParserController::commitEntryTransaction()
{
  if(_tEntry.isNull()) {
    return;
  }
  commitResourceTransaction();
  commitChecksumTransaction();
  commitChunkChecksumTransactionV4();
  commitChunkChecksumTransaction();
  commitSignatureTransaction();
  _metalinker->entries.push_back(_tEntry);
  _tEntry.reset();
}

void MetalinkParserController::cancelEntryTransaction()
{
  cancelResourceTransaction();
  cancelChecksumTransaction();
  cancelChunkChecksumTransactionV4();
  cancelChunkChecksumTransaction();
  cancelSignatureTransaction();
  _tEntry.reset();
}

void MetalinkParserController::newResourceTransaction()
{
  if(_tEntry.isNull()) {
    return;
  }
  _tResource.reset(new MetalinkResource());
}

void MetalinkParserController::setURLOfResource(const std::string& url)
{
  if(_tResource.isNull()) {
    return;
  }
  _tResource->url = url;
  // Metalink4Spec
  if(_tResource->type == MetalinkResource::TYPE_UNKNOWN) {
    // guess from URI sheme
    std::string::size_type pos = url.find("://");
    if(pos != std::string::npos) {
      setTypeOfResource(url.substr(0, pos));
    }
  }
}

void MetalinkParserController::setTypeOfResource(const std::string& type)
{
  if(_tResource.isNull()) {
    return;
  }
  if(type == MetalinkResource::FTP) {
    _tResource->type = MetalinkResource::TYPE_FTP;
  } else if(type == MetalinkResource::HTTP) {
    _tResource->type = MetalinkResource::TYPE_HTTP;
  } else if(type == MetalinkResource::HTTPS) {
    _tResource->type = MetalinkResource::TYPE_HTTPS;
  } else if(type == MetalinkResource::BITTORRENT) {
    _tResource->type = MetalinkResource::TYPE_BITTORRENT;
  } else if(type == MetalinkResource::TORRENT) { // Metalink4Spec
    _tResource->type = MetalinkResource::TYPE_BITTORRENT;
  } else {
    _tResource->type = MetalinkResource::TYPE_NOT_SUPPORTED;
  }
}

void MetalinkParserController::setLocationOfResource(const std::string& location)
{
  if(_tResource.isNull()) {
    return;
  }
  _tResource->location = location;
}

void MetalinkParserController::setPriorityOfResource(int priority)
{
  if(_tResource.isNull()) {
    return;
  }
  _tResource->priority = priority;
}

void MetalinkParserController::setMaxConnectionsOfResource(int maxConnections)
{
  if(_tResource.isNull()) {
    return;
  }
  _tResource->maxConnections = maxConnections;
}

void MetalinkParserController::commitResourceTransaction()
{
  if(_tResource.isNull()) {
    return;
  }
#ifdef ENABLE_BITTORRENT
  if(_tResource->type == MetalinkResource::TYPE_BITTORRENT) {
    SharedHandle<MetalinkMetaurl> metaurl(new MetalinkMetaurl());
    metaurl->url = _tResource->url;
    metaurl->priority = _tResource->priority;
    metaurl->mediatype = MetalinkMetaurl::MEDIATYPE_TORRENT;
    _tEntry->metaurls.push_back(metaurl);
  } else {
    _tEntry->resources.push_back(_tResource);
  }
#else // !ENABLE_BITTORRENT
  _tEntry->resources.push_back(_tResource);
#endif // !ENABLE_BITTORRENT
  _tResource.reset();
}

void MetalinkParserController::cancelResourceTransaction()
{
  _tResource.reset();
}

void MetalinkParserController::newChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tEntry.isNull()) {
    return;
  }
  _tChecksum.reset(new Checksum());
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setTypeOfChecksum(const std::string& type)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChecksum.isNull()) {
    return;
  }
  if(MessageDigestContext::supports(type)) {
    _tChecksum->setAlgo(type);
  } else {
    cancelChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setHashOfChecksum(const std::string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChecksum.isNull()) {
    return;
  }
  _tChecksum->setMessageDigest(md);
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::commitChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChecksum.isNull()) {
    return;
  }
  if(_tEntry->checksum.isNull() ||
     // Metalink3Spec
     (_tEntry->checksum->getAlgo() != MetalinkParserController::SHA1 &&
      // Metalink4Spec
      _tEntry->checksum->getAlgo() != MessageDigestContext::SHA1)) {
    _tEntry->checksum = _tChecksum;
  }
  _tChecksum.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::cancelChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  _tChecksum.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::newChunkChecksumTransactionV4()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tEntry.isNull()) {
    return;
  }
  _tChunkChecksumV4.reset(new ChunkChecksum());
  _tempChunkChecksumsV4.clear();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setTypeOfChunkChecksumV4(const std::string& type)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksumV4.isNull()) {
    return;
  }
  if(MessageDigestContext::supports(type)) {
    _tChunkChecksumV4->setAlgo(type);
  } else {
    cancelChunkChecksumTransactionV4();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setLengthOfChunkChecksumV4(size_t length)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksumV4.isNull()) {
    return;
  }
  if(length > 0) {
    _tChunkChecksumV4->setChecksumLength(length);
  } else {
    cancelChunkChecksumTransactionV4();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::addHashOfChunkChecksumV4(const std::string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksumV4.isNull()) {
    return;
  }
  _tempChunkChecksumsV4.push_back(md);
#endif // ENABLE_MESSAGE_DIGEST  
}

void MetalinkParserController::commitChunkChecksumTransactionV4()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksumV4.isNull()) {
    return;
  }
  if(_tEntry->chunkChecksum.isNull() ||
     _tEntry->chunkChecksum->getAlgo() != MessageDigestContext::SHA1) {
    std::deque<std::string> checksums(_tempChunkChecksumsV4.begin(),
				      _tempChunkChecksumsV4.end());
    _tChunkChecksumV4->setChecksums(checksums);
    _tEntry->chunkChecksum = _tChunkChecksumV4;
  }
  _tChunkChecksumV4.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::cancelChunkChecksumTransactionV4()
{
#ifdef ENABLE_MESSAGE_DIGEST
  _tChunkChecksumV4.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::newChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tEntry.isNull()) {
    return;
  }
  _tChunkChecksum.reset(new ChunkChecksum());
  _tempChunkChecksums.clear();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setTypeOfChunkChecksum(const std::string& type)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  if(MessageDigestContext::supports(type)) {
    _tChunkChecksum->setAlgo(type);
  } else {
    cancelChunkChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setLengthOfChunkChecksum(size_t length)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  if(length > 0) {
    _tChunkChecksum->setChecksumLength(length);
  } else {
    cancelChunkChecksumTransaction();
  }
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::addHashOfChunkChecksum(size_t order, const std::string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  _tempChunkChecksums.push_back(std::pair<size_t, std::string>(order, md));
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::createNewHashOfChunkChecksum(size_t order)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  _tempHashPair.first = order;
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setMessageDigestOfChunkChecksum(const std::string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  _tempHashPair.second = md;
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::addHashOfChunkChecksum()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  _tempChunkChecksums.push_back(_tempHashPair);
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::commitChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  if(_tEntry->chunkChecksum.isNull() ||
     _tEntry->chunkChecksum->getAlgo() != MetalinkParserController::SHA1) {
    std::sort(_tempChunkChecksums.begin(), _tempChunkChecksums.end(), Ascend1st<std::pair<size_t, std::string> >());
    std::deque<std::string> checksums;
    std::transform(_tempChunkChecksums.begin(), _tempChunkChecksums.end(),
                   std::back_inserter(checksums), select2nd<std::pair<size_t, std::string> >());
    _tChunkChecksum->setChecksums(checksums);
    _tEntry->chunkChecksum = _tChunkChecksum;
  }
  _tChunkChecksum.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::cancelChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  _tChunkChecksum.reset();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::newSignatureTransaction()
{
  if(_tEntry.isNull()) {
    return;
  }
  _tSignature.reset(new Signature());
}

void MetalinkParserController::setTypeOfSignature(const std::string& type)
{
  if(_tSignature.isNull()) {
    return;
  }
  _tSignature->setType(type);
}

void MetalinkParserController::setFileOfSignature(const std::string& file)
{
  if(_tSignature.isNull()) {
    return;
  }
  _tSignature->setFile(file);
}

void MetalinkParserController::setBodyOfSignature(const std::string& body)
{
  if(_tSignature.isNull()) {
    return;
  }
  _tSignature->setBody(body);
}

void MetalinkParserController::commitSignatureTransaction()
{
  if(_tSignature.isNull()) {
    return;
  }
  _tEntry->setSignature(_tSignature);
  _tSignature.reset();
}

void MetalinkParserController::cancelSignatureTransaction()
{
  _tSignature.reset();
}

void MetalinkParserController::newMetaurlTransaction()
{
  if(_tEntry.isNull()) {
    return;
  }
  _tMetaurl.reset(new MetalinkMetaurl());
}

void MetalinkParserController::setURLOfMetaurl(const std::string& url)
{
  if(_tMetaurl.isNull()) {
    return;
  }
  _tMetaurl->url = url;
}

void MetalinkParserController::setMediatypeOfMetaurl
(const std::string& mediatype)
{
  if(_tMetaurl.isNull()) {
    return;
  }
  _tMetaurl->mediatype = mediatype;
}

void MetalinkParserController::setPriorityOfMetaurl(int priority)
{
  if(_tMetaurl.isNull()) {
    return;
  }
  _tMetaurl->priority = priority;
}

void MetalinkParserController::setNameOfMetaurl(const std::string& name)
{
  if(_tMetaurl.isNull()) {
    return;
  }
  _tMetaurl->name = name;
}

void MetalinkParserController::commitMetaurlTransaction()
{
  if(_tMetaurl.isNull()) {
    return;
  }
#ifdef ENABLE_BITTORRENT
  if(_tMetaurl->mediatype == MetalinkMetaurl::MEDIATYPE_TORRENT) {
    _tEntry->metaurls.push_back(_tMetaurl);
  }
#endif // ENABLE_BITTORRENT
  _tMetaurl.reset();
}

void MetalinkParserController::cancelMetaurlTransaction()
{
  _tMetaurl.reset();
}

} // namespace aria2
