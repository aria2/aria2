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
#include "DefaultBtProgressInfoFile.h"

#include <cerrno>
#include <cstring>
#include <fstream>

#include "PieceStorage.h"
#include "Piece.h"
#include "BitfieldMan.h"
#include "Option.h"
#include "TransferStat.h"
#include "LogFactory.h"
#include "Logger.h"
#include "prefs.h"
#include "DlAbortEx.h"
#include "message.h"
#include "File.h"
#include "util.h"
#include "a2io.h"
#include "DownloadFailureException.h"
#include "StringFormat.h"
#include "array_fun.h"
#include "DownloadContext.h"
#ifdef ENABLE_BITTORRENT
# include "PeerStorage.h"
# include "BtRuntime.h"
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

const std::string DefaultBtProgressInfoFile::V0000("0000");
const std::string DefaultBtProgressInfoFile::V0001("0001");

static std::string createFilename
(const SharedHandle<DownloadContext>& dctx, const std::string& suffix)
{
  std::string t = dctx->getBasePath();
  t += suffix;
  return t;
}

DefaultBtProgressInfoFile::DefaultBtProgressInfoFile
(const SharedHandle<DownloadContext>& dctx,
 const PieceStorageHandle& pieceStorage,
 const Option* option):
  dctx_(dctx),
  pieceStorage_(pieceStorage),
  option_(option),
  logger_(LogFactory::getInstance()),
  filename_(createFilename(dctx_, getSuffix()))
{}

DefaultBtProgressInfoFile::~DefaultBtProgressInfoFile() {}

void DefaultBtProgressInfoFile::updateFilename()
{
  filename_ = createFilename(dctx_, getSuffix());
}

bool DefaultBtProgressInfoFile::isTorrentDownload()
{
#ifdef ENABLE_BITTORRENT
  return !btRuntime_.isNull();
#else // !ENABLE_BITTORRENT
  return false;
#endif // !ENABLE_BITTORRENT
}

// Since version 0001, Integers are saved in binary form, network byte order.
void DefaultBtProgressInfoFile::save()
{
  logger_->info(MSG_SAVING_SEGMENT_FILE, filename_.c_str());
  std::string filenameTemp = filename_+"__temp";
  {
    std::ofstream o(filenameTemp.c_str(), std::ios::out|std::ios::binary);
    if(!o) {
      throw DL_ABORT_EX(StringFormat(EX_SEGMENT_FILE_WRITE,
                                     filename_.c_str(), strerror(errno)).str());
    }
#ifdef ENABLE_BITTORRENT
    bool torrentDownload = isTorrentDownload();
#else // !ENABLE_BITTORRENT
    bool torrentDownload = false;
#endif // !ENABLE_BITTORRENT

    // file version: 16 bits
    // values: '1'
    char version[] = { 0x00, 0x01 };
    o.write(version, sizeof(version));
    // extension: 32 bits
    // If this is BitTorrent download, then 0x00000001
    // Otherwise, 0x00000000
    char extension[4];
    memset(extension, 0, sizeof(extension));
    if(torrentDownload) {
      extension[3] = 1;
    }
    o.write(reinterpret_cast<const char*>(&extension), sizeof(extension));
    if(torrentDownload) {
#ifdef ENABLE_BITTORRENT
      // infoHashLength:
      // length: 32 bits
      const unsigned char* infoHash = bittorrent::getInfoHash(dctx_);
      uint32_t infoHashLengthNL = htonl(INFO_HASH_LENGTH);
      o.write(reinterpret_cast<const char*>(&infoHashLengthNL),
              sizeof(infoHashLengthNL));
      // infoHash:
      o.write(reinterpret_cast<const char*>(infoHash), INFO_HASH_LENGTH);
#endif // ENABLE_BITTORRENT
    } else {
      // infoHashLength:
      // length: 32 bits
      uint32_t infoHashLength = 0;
      o.write(reinterpret_cast<const char*>(&infoHashLength),
              sizeof(infoHashLength));
    }
    // pieceLength: 32 bits
    uint32_t pieceLengthNL = htonl(dctx_->getPieceLength());
    o.write(reinterpret_cast<const char*>(&pieceLengthNL),
            sizeof(pieceLengthNL));
    // totalLength: 64 bits
    uint64_t totalLengthNL = hton64(dctx_->getTotalLength());
    o.write(reinterpret_cast<const char*>(&totalLengthNL),
            sizeof(totalLengthNL));
    // uploadLength: 64 bits
    uint64_t uploadLengthNL = 0;
#ifdef ENABLE_BITTORRENT
    if(torrentDownload) {
      TransferStat stat = peerStorage_->calculateStat();
      uploadLengthNL = hton64(stat.getAllTimeUploadLength());
    }
#endif // ENABLE_BITTORRENT
    o.write(reinterpret_cast<const char*>(&uploadLengthNL),
            sizeof(uploadLengthNL));
    // bitfieldLength: 32 bits
    uint32_t bitfieldLengthNL = htonl(pieceStorage_->getBitfieldLength());
    o.write(reinterpret_cast<const char*>(&bitfieldLengthNL),
            sizeof(bitfieldLengthNL));
    // bitfield
    o.write(reinterpret_cast<const char*>(pieceStorage_->getBitfield()),
            pieceStorage_->getBitfieldLength());
    // the number of in-flight piece: 32 bits
    // TODO implement this
    uint32_t numInFlightPieceNL = htonl(pieceStorage_->countInFlightPiece());
    o.write(reinterpret_cast<const char*>(&numInFlightPieceNL),
            sizeof(numInFlightPieceNL));
    std::vector<SharedHandle<Piece> > inFlightPieces;
    inFlightPieces.reserve(pieceStorage_->countInFlightPiece());
    pieceStorage_->getInFlightPieces(inFlightPieces);
    for(std::vector<SharedHandle<Piece> >::const_iterator itr =
          inFlightPieces.begin(), eoi = inFlightPieces.end();
        itr != eoi; ++itr) {
      uint32_t indexNL = htonl((*itr)->getIndex());
      o.write(reinterpret_cast<const char*>(&indexNL), sizeof(indexNL));
      uint32_t lengthNL = htonl((*itr)->getLength());
      o.write(reinterpret_cast<const char*>(&lengthNL), sizeof(lengthNL));
      uint32_t bitfieldLengthNL = htonl((*itr)->getBitfieldLength());
      o.write(reinterpret_cast<const char*>(&bitfieldLengthNL),
              sizeof(bitfieldLengthNL));
      o.write(reinterpret_cast<const char*>((*itr)->getBitfield()),
              (*itr)->getBitfieldLength());
    }
    o.flush();
    if(!o) {
      throw DL_ABORT_EX(StringFormat(EX_SEGMENT_FILE_WRITE,
                                     filename_.c_str(), strerror(errno)).str());
    }
    logger_->info(MSG_SAVED_SEGMENT_FILE);
  }
  if(!File(filenameTemp).renameTo(filename_)) {
    throw DL_ABORT_EX(StringFormat(EX_SEGMENT_FILE_WRITE,
                                   filename_.c_str(), strerror(errno)).str());
  }
}

#define CHECK_STREAM(in, length)                                        \
  if(in.gcount() != length) {                                           \
    throw DL_ABORT_EX(StringFormat(EX_SEGMENT_FILE_READ,                \
                                   filename_.c_str(),"Unexpected EOF").str()); \
  }                                                                     \
  if(!in) {                                                             \
    throw DL_ABORT_EX(StringFormat(EX_SEGMENT_FILE_READ,                \
                                   filename_.c_str(), strerror(errno)).str()); \
  }

// It is assumed that integers are saved as:
// 1) host byte order if version == 0000
// 2) network byte order if version == 0001
void DefaultBtProgressInfoFile::load() 
{
  logger_->info(MSG_LOADING_SEGMENT_FILE, filename_.c_str());
  std::ifstream in(filename_.c_str(), std::ios::in|std::ios::binary);
  if(!in) {                                                             \
    throw DL_ABORT_EX(StringFormat(EX_SEGMENT_FILE_READ,                \
                                   filename_.c_str(), strerror(errno)).str());
  }
  unsigned char versionBuf[2];
  in.read(reinterpret_cast<char*>(versionBuf), sizeof(versionBuf));
  CHECK_STREAM(in, sizeof(versionBuf));
  std::string versionHex = util::toHex(versionBuf, sizeof(versionBuf));
  int version;
  if(DefaultBtProgressInfoFile::V0000 == versionHex) {
    version = 0;
  } else if(DefaultBtProgressInfoFile::V0001 == versionHex) {
    version = 1;
  } else {
    throw DL_ABORT_EX
      (StringFormat("Unsupported ctrl file version: %s",
                    versionHex.c_str()).str());
  }
  unsigned char extension[4];
  in.read(reinterpret_cast<char*>(extension), sizeof(extension));
  CHECK_STREAM(in, sizeof(extension));
  bool infoHashCheckEnabled = false;
  if(extension[3]&1 && isTorrentDownload()) {
    infoHashCheckEnabled = true;
    if(logger_->debug()) {
      logger_->debug("InfoHash checking enabled.");
    }
  }

  uint32_t infoHashLength;
  in.read(reinterpret_cast<char*>(&infoHashLength), sizeof(infoHashLength));
  CHECK_STREAM(in, sizeof(infoHashLength));
  if(version >= 1) {
    infoHashLength = ntohl(infoHashLength);
  }
  if((infoHashLength < 0) ||
     ((infoHashLength == 0) && infoHashCheckEnabled)) {
    throw DL_ABORT_EX
      (StringFormat("Invalid info hash length: %d", infoHashLength).str());
  }
  if(infoHashLength > 0) {
    array_ptr<unsigned char> savedInfoHash(new unsigned char[infoHashLength]);
    in.read(reinterpret_cast<char*>
            (static_cast<unsigned char*>(savedInfoHash)), infoHashLength);
    CHECK_STREAM(in, static_cast<int>(infoHashLength));
#ifdef ENABLE_BITTORRENT
    if(infoHashCheckEnabled) {
      const unsigned char* infoHash = bittorrent::getInfoHash(dctx_);
      if(infoHashLength != INFO_HASH_LENGTH ||
         memcmp(savedInfoHash, infoHash, INFO_HASH_LENGTH) != 0) {
        throw DL_ABORT_EX
          (StringFormat("info hash mismatch. expected: %s, actual: %s",
                        util::toHex(infoHash, INFO_HASH_LENGTH).c_str(),
                        util::toHex(savedInfoHash, infoHashLength).c_str()
                        ).str());
      }
    }
#endif // ENABLE_BITTORRENT
  }

  uint32_t pieceLength;
  in.read(reinterpret_cast<char*>(&pieceLength), sizeof(pieceLength));
  CHECK_STREAM(in, sizeof(pieceLength));
  if(version >= 1) {
    pieceLength = ntohl(pieceLength);
  }

  uint64_t totalLength;
  in.read(reinterpret_cast<char*>(&totalLength), sizeof(totalLength));
  CHECK_STREAM(in, sizeof(totalLength));
  if(version >= 1) {
    totalLength = ntoh64(totalLength);
  }
  if(totalLength != dctx_->getTotalLength()) {
    throw DL_ABORT_EX
      (StringFormat("total length mismatch. expected: %s, actual: %s",
                    util::itos(dctx_->getTotalLength()).c_str(),
                    util::itos(totalLength).c_str()).str());
  }
  uint64_t uploadLength;
  in.read(reinterpret_cast<char*>(&uploadLength), sizeof(uploadLength));
  CHECK_STREAM(in, sizeof(uploadLength));
  if(version >= 1) {
    uploadLength = ntoh64(uploadLength);
  }
#ifdef ENABLE_BITTORRENT
  if(isTorrentDownload()) {
    btRuntime_->setUploadLengthAtStartup(uploadLength);
  }
#endif // ENABLE_BITTORRENT
  // TODO implement the conversion mechanism between different piece length.
  uint32_t bitfieldLength;
  in.read(reinterpret_cast<char*>(&bitfieldLength), sizeof(bitfieldLength));
  CHECK_STREAM(in, sizeof(bitfieldLength));
  if(version >= 1) {
    bitfieldLength = ntohl(bitfieldLength);
  }
  uint32_t expectedBitfieldLength =
    ((totalLength+pieceLength-1)/pieceLength+7)/8;
  if(expectedBitfieldLength != bitfieldLength) {
    throw DL_ABORT_EX
      (StringFormat("bitfield length mismatch. expected: %d, actual: %d",
                    expectedBitfieldLength,
                    bitfieldLength).str());
  }

  array_ptr<unsigned char> savedBitfield(new unsigned char[bitfieldLength]);
  in.read(reinterpret_cast<char*>
          (static_cast<unsigned char*>(savedBitfield)), bitfieldLength);
  CHECK_STREAM(in, static_cast<int>(bitfieldLength));
  if(pieceLength == dctx_->getPieceLength()) {
    pieceStorage_->setBitfield(savedBitfield, bitfieldLength);

    uint32_t numInFlightPiece;
    in.read(reinterpret_cast<char*>(&numInFlightPiece),
            sizeof(numInFlightPiece));
    CHECK_STREAM(in, sizeof(numInFlightPiece));
    if(version >= 1) {
      numInFlightPiece = ntohl(numInFlightPiece);
    }
    std::vector<SharedHandle<Piece> > inFlightPieces;
    inFlightPieces.reserve(numInFlightPiece);
    while(numInFlightPiece--) {
      uint32_t index;
      in.read(reinterpret_cast<char*>(&index), sizeof(index));
      CHECK_STREAM(in, sizeof(index));
      if(version >= 1) {
        index = ntohl(index);
      }
      if(!(index < dctx_->getNumPieces())) {
        throw DL_ABORT_EX
          (StringFormat("piece index out of range: %u", index).str());
      }
      uint32_t length;
      in.read(reinterpret_cast<char*>(&length), sizeof(length));
      CHECK_STREAM(in, sizeof(length));
      if(version >= 1) {
        length = ntohl(length);
      }
      if(!(length <=dctx_->getPieceLength())) {
        throw DL_ABORT_EX
          (StringFormat("piece length out of range: %u", length).str());
      }
      SharedHandle<Piece> piece(new Piece(index, length));
      uint32_t bitfieldLength;
      in.read(reinterpret_cast<char*>(&bitfieldLength),
              sizeof(bitfieldLength));
      CHECK_STREAM(in, sizeof(bitfieldLength));
      if(version >= 1) {
        bitfieldLength = ntohl(bitfieldLength);
      }
      if(piece->getBitfieldLength() != bitfieldLength) {
        throw DL_ABORT_EX
          (StringFormat("piece bitfield length mismatch."
                        " expected: %u actual: %u",
                        piece->getBitfieldLength(), bitfieldLength).str());
      }
      array_ptr<unsigned char> pieceBitfield
        (new unsigned char[bitfieldLength]);
      in.read(reinterpret_cast<char*>
              (static_cast<unsigned char*>(pieceBitfield)), bitfieldLength);
      CHECK_STREAM(in, static_cast<int>(bitfieldLength));
      piece->setBitfield(pieceBitfield, bitfieldLength);

#ifdef ENABLE_MESSAGE_DIGEST

      piece->setHashAlgo(dctx_->getPieceHashAlgo());

#endif // ENABLE_MESSAGE_DIGEST
        
      inFlightPieces.push_back(piece);
    }
    pieceStorage_->addInFlightPiece(inFlightPieces);
  } else {
    uint32_t numInFlightPiece;
    in.read(reinterpret_cast<char*>(&numInFlightPiece),
            sizeof(numInFlightPiece));
    CHECK_STREAM(in, sizeof(numInFlightPiece));
    if(version >= 1) {
      numInFlightPiece = ntohl(numInFlightPiece);
    }
    BitfieldMan src(pieceLength, totalLength);
    src.setBitfield(savedBitfield, bitfieldLength);
    if((src.getCompletedLength() || numInFlightPiece) &&
       !option_->getAsBool(PREF_ALLOW_PIECE_LENGTH_CHANGE)) {
      throw DOWNLOAD_FAILURE_EXCEPTION
        ("WARNING: Detected a change in piece length. You can proceed with"
         " --allow-piece-length-change=true, but you may lose some download"
         " progress.");
    }
    BitfieldMan dest(dctx_->getPieceLength(), totalLength);
    util::convertBitfield(&dest, &src);
    pieceStorage_->setBitfield(dest.getBitfield(), dest.getBitfieldLength());
  }
  logger_->info(MSG_LOADED_SEGMENT_FILE);
}

void DefaultBtProgressInfoFile::removeFile()
{
  if(exists()) {
    File f(filename_);
    f.remove();
  }
}

bool DefaultBtProgressInfoFile::exists()
{
  File f(filename_);
  if(f.isFile()) {
    logger_->info(MSG_SEGMENT_FILE_EXISTS, filename_.c_str());
    return true;
  } else {
    logger_->info(MSG_SEGMENT_FILE_DOES_NOT_EXIST, filename_.c_str());
    return false;
  }
}

#ifdef ENABLE_BITTORRENT
void DefaultBtProgressInfoFile::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtProgressInfoFile::setBtRuntime
(const SharedHandle<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}
#endif // ENABLE_BITTORRENT

} // namespace aria2
