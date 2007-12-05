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
#include "MetalinkParserController.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
#endif // ENABLE_MESSAGE_DIGEST

MetalinkParserController::MetalinkParserController():
  _metalinker(new Metalinker()),
  _tEntry(0),
  _tResource(0)
#ifdef ENABLE_MESSAGE_DIGEST
  ,
  _tChecksum(0),
  _tChunkChecksum(0)
#endif // ENABLE_MESSAGE_DIGEST
{}

MetalinkParserController::~MetalinkParserController() {}

MetalinkerHandle MetalinkParserController::getResult() const
{
  return _metalinker;
}

void MetalinkParserController::newEntryTransaction()
{
  _tEntry = new MetalinkEntry();
  _tResource = 0;
#ifdef ENABLE_MESSAGE_DIGEST
  _tChecksum = 0;
  _tChunkChecksum = 0;
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setFileNameOfEntry(const string& filename)
{
  if(_tEntry.isNull()) {
    return;
  }
  if(_tEntry->file.isNull()) {
    _tEntry->file = new FileEntry(filename, 0, 0);
  } else {
    _tEntry->file->setPath(filename);
  }
}

void MetalinkParserController::setFileLengthOfEntry(int64_t length)
{
  if(_tEntry.isNull()) {
    return;
  }
  if(_tEntry->file.isNull()) {
    _tEntry->file = new FileEntry("", length, 0);
  } else {
    _tEntry->file->setLength(length);
  }
}

void MetalinkParserController::setVersionOfEntry(const string& version)
{
  if(_tEntry.isNull()) {
    return;
  }
  _tEntry->version = version;
}

void MetalinkParserController::setLanguageOfEntry(const string& language)
{
  if(_tEntry.isNull()) {
    return;
  }
  _tEntry->language = language;
}

void MetalinkParserController::setOSOfEntry(const string& os)
{
  if(_tEntry.isNull()) {
    return;
  }
  _tEntry->os = os;
}

void MetalinkParserController::setMaxConnectionsOfEntry(int32_t maxConnections)
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
  commitChunkChecksumTransaction();
  _metalinker->entries.push_back(_tEntry);
  _tEntry = 0;
}

void MetalinkParserController::cancelEntryTransaction()
{
  cancelResourceTransaction();
  cancelChecksumTransaction();
  cancelChunkChecksumTransaction();
  _tEntry = 0;
}

void MetalinkParserController::newResourceTransaction()
{
  if(_tEntry.isNull()) {
    return;
  }
  _tResource = new MetalinkResource();
}

void MetalinkParserController::setURLOfResource(const string& url)
{
  if(_tResource.isNull()) {
    return;
  }
  _tResource->url = url;
}

void MetalinkParserController::setTypeOfResource(const string& type)
{
  if(_tResource.isNull()) {
    return;
  }
  if(type == "ftp") {
    _tResource->type = MetalinkResource::TYPE_FTP;
  } else if(type == "http") {
    _tResource->type = MetalinkResource::TYPE_HTTP;
  } else if(type == "https") {
    _tResource->type = MetalinkResource::TYPE_HTTPS;
  } else if(type == "bittorrent") {
    _tResource->type = MetalinkResource::TYPE_BITTORRENT;
  } else {
    _tResource->type = MetalinkResource::TYPE_NOT_SUPPORTED;
  }
}

void MetalinkParserController::setLocationOfResource(const string& location)
{
  if(_tResource.isNull()) {
    return;
  }
  _tResource->location = location;
}

void MetalinkParserController::setPreferenceOfResource(int32_t preference)
{
  if(_tResource.isNull()) {
    return;
  }
  _tResource->preference = preference;
}

void MetalinkParserController::setMaxConnectionsOfResource(int32_t maxConnections)
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
  _tEntry->resources.push_back(_tResource);
  _tResource = 0;
}

void MetalinkParserController::cancelResourceTransaction()
{
  _tResource = 0;
}

void MetalinkParserController::newChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tEntry.isNull()) {
    return;
  }
  _tChecksum = new Checksum();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setTypeOfChecksum(const string& type)
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

void MetalinkParserController::setHashOfChecksum(const string& md)
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
  if(_tEntry->checksum.isNull() || _tEntry->checksum->getAlgo() != "sha1") {
    _tEntry->checksum = _tChecksum;
  }
  _tChecksum = 0;
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::cancelChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  _tChecksum = 0;
#endif // ENABLE_MESSAGE_DIGEST
}
  
void MetalinkParserController::newChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tEntry.isNull()) {
    return;
  }
  _tChunkChecksum = new ChunkChecksum();
  _tempChunkChecksums.clear();
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setTypeOfChunkChecksum(const string& type)
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

void MetalinkParserController::setLengthOfChunkChecksum(int32_t length)
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

void MetalinkParserController::addHashOfChunkChecksum(int32_t order, const string& md)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  _tempChunkChecksums.push_back(pair<int32_t, string>(order, md));
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::createNewHashOfChunkChecksum(int32_t order)
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  _tempHashPair.first = order;
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::setMessageDigestOfChunkChecksum(const string& md)
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

bool firstAsc(const pair<int32_t, string>& p1, const pair<int32_t, string>& p2)
{
  return p1.first < p2.first;
}

class GetSecond
{
private:
  Strings& ss;
public:
  GetSecond(Strings& s):ss(s) {}

  void operator()(const pair<int32_t, string>& p)
  {
    ss.push_back(p.second);
  }
};

void MetalinkParserController::commitChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  if(_tChunkChecksum.isNull()) {
    return;
  }
  if(_tEntry->chunkChecksum.isNull() || _tEntry->chunkChecksum->getAlgo() != "sha1") {
    sort(_tempChunkChecksums.begin(), _tempChunkChecksums.end(), firstAsc);
    Strings checksums;
    for_each(_tempChunkChecksums.begin(), _tempChunkChecksums.end(), GetSecond(checksums));
    
    _tChunkChecksum->setChecksums(checksums);
    _tEntry->chunkChecksum = _tChunkChecksum;
  }
  _tChunkChecksum = 0;
#endif // ENABLE_MESSAGE_DIGEST
}

void MetalinkParserController::cancelChunkChecksumTransaction()
{
#ifdef ENABLE_MESSAGE_DIGEST
  _tChunkChecksum = 0;
#endif // ENABLE_MESSAGE_DIGEST
}


