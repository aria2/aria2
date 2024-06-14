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

#include <cstring>
#include <cstdio>
#include <array>

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
#include "fmt.h"
#include "array_fun.h"
#include "DownloadContext.h"
#include "BufferedFile.h"
#include "SHA1IOFile.h"
#include "BtConstants.h"
#ifdef ENABLE_BITTORRENT
#  include "PeerStorage.h"
#  include "BtRuntime.h"
#  include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace {
std::string createFilename(const std::shared_ptr<DownloadContext>& dctx,
                           const std::string& suffix)
{
  std::string t = dctx->getBasePath();
  t += suffix;
  return t;
}
} // namespace

DefaultBtProgressInfoFile::DefaultBtProgressInfoFile(
    const std::shared_ptr<DownloadContext>& dctx,
    const std::shared_ptr<PieceStorage>& pieceStorage, const Option* option)
    : dctx_(dctx),
      pieceStorage_(pieceStorage),
      option_(option),
      filename_(createFilename(dctx_, getSuffix()))
{
}

DefaultBtProgressInfoFile::~DefaultBtProgressInfoFile() = default;

void DefaultBtProgressInfoFile::updateFilename()
{
  filename_ = createFilename(dctx_, getSuffix());
}

bool DefaultBtProgressInfoFile::isTorrentDownload()
{
#ifdef ENABLE_BITTORRENT
  return btRuntime_.get();
#else  // !ENABLE_BITTORRENT
  return false;
#endif // !ENABLE_BITTORRENT
}

#define WRITE_CHECK(fp, ptr, count)                                            \
  if (fp.write((ptr), (count)) != (count)) {                                   \
    throw DL_ABORT_EX(fmt(EX_SEGMENT_FILE_WRITE, filename_.c_str()));          \
  }

// Since version 0001, Integers are saved in binary form, network byte order.
void DefaultBtProgressInfoFile::save(IOFile& fp)
{
#ifdef ENABLE_BITTORRENT
  bool torrentDownload = isTorrentDownload();
#else  // !ENABLE_BITTORRENT
  bool torrentDownload = false;
#endif // !ENABLE_BITTORRENT
  // file version: 16 bits
  // values: '1'
  char version[] = {0x00u, 0x01u};
  WRITE_CHECK(fp, version, sizeof(version));
  // extension: 32 bits
  // If this is BitTorrent download, then 0x00000001
  // Otherwise, 0x00000000
  char extension[4];
  memset(extension, 0, sizeof(extension));
  if (torrentDownload) {
    extension[3] = 1;
  }
  WRITE_CHECK(fp, extension, sizeof(extension));
  if (torrentDownload) {
#ifdef ENABLE_BITTORRENT
    // infoHashLength:
    // length: 32 bits
    const unsigned char* infoHash = bittorrent::getInfoHash(dctx_);
    uint32_t infoHashLengthNL = htonl(INFO_HASH_LENGTH);
    WRITE_CHECK(fp, &infoHashLengthNL, sizeof(infoHashLengthNL));
    // infoHash:
    WRITE_CHECK(fp, infoHash, INFO_HASH_LENGTH);
#endif // ENABLE_BITTORRENT
  }
  else {
    // infoHashLength:
    // length: 32 bits
    uint32_t infoHashLength = 0;
    WRITE_CHECK(fp, &infoHashLength, sizeof(infoHashLength));
  }
  // pieceLength: 32 bits
  uint32_t pieceLengthNL = htonl(dctx_->getPieceLength());
  WRITE_CHECK(fp, &pieceLengthNL, sizeof(pieceLengthNL));
  // totalLength: 64 bits
  uint64_t totalLengthNL = hton64(dctx_->getTotalLength());
  WRITE_CHECK(fp, &totalLengthNL, sizeof(totalLengthNL));
  // uploadLength: 64 bits
  uint64_t uploadLengthNL = 0;
#ifdef ENABLE_BITTORRENT
  if (torrentDownload) {
    uploadLengthNL = hton64(btRuntime_->getUploadLengthAtStartup() +
                            dctx_->getNetStat().getSessionUploadLength());
  }
#endif // ENABLE_BITTORRENT
  WRITE_CHECK(fp, &uploadLengthNL, sizeof(uploadLengthNL));
  // bitfieldLength: 32 bits
  uint32_t bitfieldLengthNL = htonl(pieceStorage_->getBitfieldLength());
  WRITE_CHECK(fp, &bitfieldLengthNL, sizeof(bitfieldLengthNL));
  // bitfield
  WRITE_CHECK(fp, pieceStorage_->getBitfield(),
              pieceStorage_->getBitfieldLength());
  // the number of in-flight piece: 32 bits
  // TODO implement this
  uint32_t numInFlightPieceNL = htonl(pieceStorage_->countInFlightPiece());
  WRITE_CHECK(fp, &numInFlightPieceNL, sizeof(numInFlightPieceNL));
  std::vector<std::shared_ptr<Piece>> inFlightPieces;
  inFlightPieces.reserve(pieceStorage_->countInFlightPiece());
  pieceStorage_->getInFlightPieces(inFlightPieces);
  for (std::vector<std::shared_ptr<Piece>>::const_iterator
           itr = inFlightPieces.begin(),
           eoi = inFlightPieces.end();
       itr != eoi; ++itr) {
    uint32_t indexNL = htonl((*itr)->getIndex());
    WRITE_CHECK(fp, &indexNL, sizeof(indexNL));
    uint32_t lengthNL = htonl((*itr)->getLength());
    WRITE_CHECK(fp, &lengthNL, sizeof(lengthNL));
    uint32_t bitfieldLengthNL = htonl((*itr)->getBitfieldLength());
    WRITE_CHECK(fp, &bitfieldLengthNL, sizeof(bitfieldLengthNL));
    WRITE_CHECK(fp, (*itr)->getBitfield(), (*itr)->getBitfieldLength());
  }
  if (fp.close() == EOF) {
    throw DL_ABORT_EX(fmt(EX_SEGMENT_FILE_WRITE, filename_.c_str()));
  }
}

void DefaultBtProgressInfoFile::save()
{
  SHA1IOFile sha1io;

  save(sha1io);

  auto digest = sha1io.digest();
  if (digest == lastDigest_) {
    // We don't write control file if the content is not changed.
    return;
  }

  lastDigest_ = std::move(digest);

  A2_LOG_INFO(fmt(MSG_SAVING_SEGMENT_FILE, filename_.c_str()));
  std::string filenameTemp = filename_;
  filenameTemp += "__temp";
  {
    BufferedFile fp(filenameTemp.c_str(), BufferedFile::WRITE);
    if (!fp) {
      throw DL_ABORT_EX(fmt(EX_SEGMENT_FILE_WRITE, filename_.c_str()));
    }

    save(fp);
  }

  A2_LOG_INFO(MSG_SAVED_SEGMENT_FILE);

  if (!File(filenameTemp).renameTo(filename_)) {
    throw DL_ABORT_EX(fmt(EX_SEGMENT_FILE_WRITE, filename_.c_str()));
  }
}

#define READ_CHECK(fp, ptr, count)                                             \
  if (fp.read((ptr), (count)) != (count)) {                                    \
    throw DL_ABORT_EX(fmt(EX_SEGMENT_FILE_READ, filename_.c_str()));           \
  }

// It is assumed that integers are saved as:
// 1) host byte order if version == 0000
// 2) network byte order if version == 0001
void DefaultBtProgressInfoFile::load()
{
  A2_LOG_INFO(fmt(MSG_LOADING_SEGMENT_FILE, filename_.c_str()));
  BufferedFile fp(filename_.c_str(), BufferedFile::READ);
  if (!fp) {
    throw DL_ABORT_EX(fmt(EX_SEGMENT_FILE_READ, filename_.c_str()));
  }
  unsigned char versionBuf[2];
  READ_CHECK(fp, versionBuf, sizeof(versionBuf));
  std::string versionHex = util::toHex(versionBuf, sizeof(versionBuf));
  int version;
  if ("0000" == versionHex) {
    version = 0;
  }
  else if ("0001" == versionHex) {
    version = 1;
  }
  else {
    throw DL_ABORT_EX(
        fmt("Unsupported ctrl file version: %s", versionHex.c_str()));
  }
  unsigned char extension[4];
  READ_CHECK(fp, extension, sizeof(extension));
  bool infoHashCheckEnabled = false;
  if (extension[3] & 1 && isTorrentDownload()) {
    infoHashCheckEnabled = true;
    A2_LOG_DEBUG("InfoHash checking enabled.");
  }

  uint32_t infoHashLength;
  READ_CHECK(fp, &infoHashLength, sizeof(infoHashLength));
  if (version >= 1) {
    infoHashLength = ntohl(infoHashLength);
  }
  if (infoHashLength > INFO_HASH_LENGTH ||
      (infoHashLength != INFO_HASH_LENGTH && infoHashCheckEnabled)) {
    throw DL_ABORT_EX(fmt("Invalid info hash length: %d", infoHashLength));
  }
  if (infoHashLength > 0) {
    std::array<unsigned char, INFO_HASH_LENGTH> savedInfoHash;
    READ_CHECK(fp, savedInfoHash.data(), infoHashLength);
#ifdef ENABLE_BITTORRENT
    if (infoHashCheckEnabled) {
      const unsigned char* infoHash = bittorrent::getInfoHash(dctx_);
      if (memcmp(savedInfoHash.data(), infoHash, INFO_HASH_LENGTH) != 0) {
        throw DL_ABORT_EX(
            fmt("info hash mismatch. expected: %s, actual: %s",
                util::toHex(infoHash, INFO_HASH_LENGTH).c_str(),
                util::toHex(savedInfoHash.data(), infoHashLength).c_str()));
      }
    }
#endif // ENABLE_BITTORRENT
  }

  uint32_t pieceLength;
  READ_CHECK(fp, &pieceLength, sizeof(pieceLength));
  if (version >= 1) {
    pieceLength = ntohl(pieceLength);
  }

  if (pieceLength == 0) {
    throw DL_ABORT_EX("piece length must not be 0");
  }

  uint64_t totalLength;
  READ_CHECK(fp, &totalLength, sizeof(totalLength));
  if (version >= 1) {
    totalLength = ntoh64(totalLength);
  }
  if (totalLength != static_cast<uint64_t>(dctx_->getTotalLength())) {
    throw DL_ABORT_EX(
        fmt("total length mismatch. expected: %" PRId64 ", actual: %" PRId64 "",
            dctx_->getTotalLength(), static_cast<int64_t>(totalLength)));
  }
  uint64_t uploadLength;
  READ_CHECK(fp, &uploadLength, sizeof(uploadLength));
  if (version >= 1) {
    uploadLength = ntoh64(uploadLength);
  }
#ifdef ENABLE_BITTORRENT
  if (isTorrentDownload()) {
    btRuntime_->setUploadLengthAtStartup(uploadLength);
  }
#endif // ENABLE_BITTORRENT
  // TODO implement the conversion mechanism between different piece length.
  uint32_t bitfieldLength;
  READ_CHECK(fp, &bitfieldLength, sizeof(bitfieldLength));
  if (version >= 1) {
    bitfieldLength = ntohl(bitfieldLength);
  }
  uint32_t expectedBitfieldLength =
      ((totalLength + pieceLength - 1) / pieceLength + 7) / 8;
  if (expectedBitfieldLength != bitfieldLength) {
    throw DL_ABORT_EX(fmt("bitfield length mismatch. expected: %d, actual: %d",
                          expectedBitfieldLength, bitfieldLength));
  }

  auto savedBitfield = make_unique<unsigned char[]>((size_t)bitfieldLength);
  READ_CHECK(fp, savedBitfield.get(), bitfieldLength);
  if (pieceLength == static_cast<uint32_t>(dctx_->getPieceLength())) {
    pieceStorage_->setBitfield(savedBitfield.get(), bitfieldLength);

    uint32_t numInFlightPiece;
    READ_CHECK(fp, &numInFlightPiece, sizeof(numInFlightPiece));
    if (version >= 1) {
      numInFlightPiece = ntohl(numInFlightPiece);
    }
    std::vector<std::shared_ptr<Piece>> inFlightPieces;
    inFlightPieces.reserve(numInFlightPiece);
    while (numInFlightPiece--) {
      uint32_t index;
      READ_CHECK(fp, &index, sizeof(index));
      if (version >= 1) {
        index = ntohl(index);
      }
      if (!(index < dctx_->getNumPieces())) {
        throw DL_ABORT_EX(fmt("piece index out of range: %u", index));
      }
      uint32_t length;
      READ_CHECK(fp, &length, sizeof(length));
      if (version >= 1) {
        length = ntohl(length);
      }
      if (!(length <= static_cast<uint32_t>(dctx_->getPieceLength()))) {
        throw DL_ABORT_EX(fmt("piece length out of range: %u", length));
      }
      auto piece = std::make_shared<Piece>(index, length);
      uint32_t bitfieldLength;
      READ_CHECK(fp, &bitfieldLength, sizeof(bitfieldLength));
      if (version >= 1) {
        bitfieldLength = ntohl(bitfieldLength);
      }
      if (piece->getBitfieldLength() != bitfieldLength) {
        throw DL_ABORT_EX(
            fmt("piece bitfield length mismatch."
                " expected: %lu actual: %u",
                static_cast<unsigned long>(piece->getBitfieldLength()),
                bitfieldLength));
      }
      auto pieceBitfield = make_unique<unsigned char[]>((size_t)bitfieldLength);
      READ_CHECK(fp, pieceBitfield.get(), bitfieldLength);
      piece->setBitfield(pieceBitfield.get(), bitfieldLength);
      piece->setHashType(dctx_->getPieceHashType());

      inFlightPieces.push_back(piece);
    }
    pieceStorage_->addInFlightPiece(inFlightPieces);
  }
  else {
    uint32_t numInFlightPiece;
    READ_CHECK(fp, &numInFlightPiece, sizeof(numInFlightPiece));
    if (version >= 1) {
      numInFlightPiece = ntohl(numInFlightPiece);
    }
    BitfieldMan src(pieceLength, totalLength);
    src.setBitfield(savedBitfield.get(), bitfieldLength);
    if ((src.getCompletedLength() || numInFlightPiece) &&
        !option_->getAsBool(PREF_ALLOW_PIECE_LENGTH_CHANGE)) {
      throw DOWNLOAD_FAILURE_EXCEPTION2(
          "WARNING: Detected a change in piece length. You can proceed with"
          " --allow-piece-length-change=true, but you may lose some download"
          " progress.",
          error_code::PIECE_LENGTH_CHANGED);
    }
    BitfieldMan dest(dctx_->getPieceLength(), totalLength);
    util::convertBitfield(&dest, &src);
    pieceStorage_->setBitfield(dest.getBitfield(), dest.getBitfieldLength());
  }
  A2_LOG_INFO(MSG_LOADED_SEGMENT_FILE);
}

void DefaultBtProgressInfoFile::removeFile()
{
  if (exists()) {
    File f(filename_);
    f.remove();
  }
}

bool DefaultBtProgressInfoFile::exists()
{
  File f(filename_);
  if (f.isFile()) {
    A2_LOG_INFO(fmt(MSG_SEGMENT_FILE_EXISTS, filename_.c_str()));
    return true;
  }
  else {
    A2_LOG_INFO(fmt(MSG_SEGMENT_FILE_DOES_NOT_EXIST, filename_.c_str()));
    return false;
  }
}

#ifdef ENABLE_BITTORRENT
void DefaultBtProgressInfoFile::setPeerStorage(
    const std::shared_ptr<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtProgressInfoFile::setBtRuntime(
    const std::shared_ptr<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}
#endif // ENABLE_BITTORRENT

} // namespace aria2
