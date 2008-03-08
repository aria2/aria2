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
#include "DefaultBtProgressInfoFile.h"
#include "BtContext.h"
#include "PieceStorage.h"
#include "Piece.h"
#include "PeerStorage.h"
#include "BtRuntime.h"
#include "BitfieldMan.h"
#include "Option.h"
#include "TransferStat.h"
#include "BtRegistry.h"
#include "LogFactory.h"
#include "Logger.h"
#include "prefs.h"
#include "DlAbortEx.h"
#include "message.h"
#include "File.h"
#include "Util.h"
#include "a2io.h"
#include "DownloadFailureException.h"
#include <fstream>
#include <cerrno>

namespace aria2 {

DefaultBtProgressInfoFile::DefaultBtProgressInfoFile(const DownloadContextHandle& dctx,
						     const PieceStorageHandle& pieceStorage,
						     const Option* option):
  _dctx(dctx),
  _pieceStorage(pieceStorage),
  _option(option),
  _logger(LogFactory::getInstance())
{
  _filename = _dctx->getActualBasePath()+".aria2";
}

DefaultBtProgressInfoFile::~DefaultBtProgressInfoFile() {}

bool DefaultBtProgressInfoFile::isTorrentDownload()
{
  return !BtContextHandle(_dctx).isNull();
}

void DefaultBtProgressInfoFile::save() {
  _logger->info(MSG_SAVING_SEGMENT_FILE, _filename.c_str());
  std::string filenameTemp = _filename+"__temp";
  std::ofstream o(filenameTemp.c_str(), std::ios::out|std::ios::binary);
  o.exceptions(std::ios::failbit);
  try {
    bool torrentDownload = isTorrentDownload();
    // file version: 16 bits
    // value: '0'
    uint16_t version = 0;
    o.write(reinterpret_cast<const char*>(&version), sizeof(version));
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
      // infoHashLength:
      // length: 32 bits
      BtContextHandle btContext = _dctx;
      uint32_t infoHashLength = btContext->getInfoHashLength();
      o.write(reinterpret_cast<const char*>(&infoHashLength), sizeof(infoHashLength));
      // infoHash:
      o.write(reinterpret_cast<const char*>(btContext->getInfoHash()),
	      btContext->getInfoHashLength());
    } else {
      // infoHashLength:
      // length: 32 bits
      uint32_t infoHashLength = 0;
      o.write(reinterpret_cast<const char*>(&infoHashLength), sizeof(infoHashLength));
    }
    // pieceLength: 32 bits
    uint32_t pieceLength = _dctx->getPieceLength();
    o.write(reinterpret_cast<const char*>(&pieceLength), sizeof(pieceLength));
    // totalLength: 64 bits
    uint64_t totalLength = _dctx->getTotalLength();
    o.write(reinterpret_cast<const char*>(&totalLength), sizeof(totalLength));
    // uploadLength: 64 bits
    uint64_t uploadLength = 0;
    if(torrentDownload) {
      BtContextHandle btContext = _dctx;
      TransferStat stat = PEER_STORAGE(btContext)->calculateStat();
      uploadLength = stat.getAllTimeUploadLength();
    }
    o.write(reinterpret_cast<const char*>(&uploadLength), sizeof(uploadLength));
    // bitfieldLength: 32 bits
    uint32_t bitfieldLength = _pieceStorage->getBitfieldLength();
    o.write(reinterpret_cast<const char*>(&bitfieldLength), sizeof(bitfieldLength));
    // bitfield
    o.write(reinterpret_cast<const char*>(_pieceStorage->getBitfield()), _pieceStorage->getBitfieldLength());
    // the number of in-flight piece: 32 bits
    // TODO implement this
    uint32_t numInFlightPiece = _pieceStorage->countInFlightPiece();
    o.write(reinterpret_cast<const char*>(&numInFlightPiece), sizeof(numInFlightPiece));
    Pieces inFlightPieces = _pieceStorage->getInFlightPieces();
    for(Pieces::const_iterator itr = inFlightPieces.begin();
	itr != inFlightPieces.end(); ++itr) {
      uint32_t index = (*itr)->getIndex();
      o.write(reinterpret_cast<const char*>(&index), sizeof(index));
      uint32_t length = (*itr)->getLength();
      o.write(reinterpret_cast<const char*>(&length), sizeof(length));
      uint32_t bitfieldLength = (*itr)->getBitfieldLength();
      o.write(reinterpret_cast<const char*>(&bitfieldLength), sizeof(bitfieldLength));
      o.write(reinterpret_cast<const char*>((*itr)->getBitfield()), bitfieldLength);
    }

    o.close();
    _logger->info(MSG_SAVED_SEGMENT_FILE);
  } catch(std::ios::failure const& exception) {
    // TODO std::ios::failure doesn't give us the reasons of failure...
    throw new DlAbortEx(EX_SEGMENT_FILE_WRITE,
			_filename.c_str(), strerror(errno));
  }
  if(!File(filenameTemp).renameTo(_filename)) {
    throw new DlAbortEx(EX_SEGMENT_FILE_WRITE,
			_filename.c_str(), strerror(errno));
  }
}

void DefaultBtProgressInfoFile::load() 
{
  _logger->info(MSG_LOADING_SEGMENT_FILE, _filename.c_str());
  std::ifstream in(_filename.c_str(), std::ios::in|std::ios::binary);
  in.exceptions(std::ios::failbit);
  unsigned char* savedInfoHash = 0;
  unsigned char* savedBitfield = 0;
  try {
    unsigned char version[2];
    in.read((char*)version, sizeof(version));
    if(std::string("0000") != Util::toHex(version, sizeof(version))) {
      throw new DlAbortEx("Unsupported ctrl file version: %s",
			  Util::toHex(version, sizeof(version)).c_str());
    }
    unsigned char extension[4];
    in.read((char*)extension, sizeof(extension));

    bool infoHashCheckEnabled = false;
    if(extension[3]&1 && isTorrentDownload()) {
      infoHashCheckEnabled = true;
      _logger->debug("InfoHash checking enabled.");
    }

    uint32_t infoHashLength;
    in.read(reinterpret_cast<char*>(&infoHashLength), sizeof(infoHashLength));
    if(infoHashLength < 0 || infoHashLength == 0 && infoHashCheckEnabled) {
      throw new DlAbortEx("Invalid info hash length: %d", infoHashLength);
    }
    if(infoHashLength > 0) {
      savedInfoHash = new unsigned char[infoHashLength];
      in.read(reinterpret_cast<char*>(savedInfoHash), infoHashLength);
      BtContextHandle btContext = _dctx;
      if(infoHashCheckEnabled &&
	 Util::toHex(savedInfoHash, infoHashLength) != btContext->getInfoHashAsString()) {
	throw new DlAbortEx("info hash mismatch. expected: %s, actual: %s",
			    btContext->getInfoHashAsString().c_str(),
			    Util::toHex(savedInfoHash, infoHashLength).c_str());
      }
      delete [] savedInfoHash;
      savedInfoHash = 0;
    }

    uint32_t pieceLength;
    in.read(reinterpret_cast<char*>(&pieceLength), sizeof(pieceLength));

    uint64_t totalLength;
    in.read(reinterpret_cast<char*>(&totalLength), sizeof(totalLength));
    if(totalLength != _dctx->getTotalLength()) {
      throw new DlAbortEx("total length mismatch. expected: %s, actual: %s",
			  Util::itos(_dctx->getTotalLength()).c_str(),
			  Util::itos(totalLength).c_str());
    }
    uint64_t uploadLength;
    in.read(reinterpret_cast<char*>(&uploadLength), sizeof(uploadLength));
    if(isTorrentDownload()) {
      BT_RUNTIME(BtContextHandle(_dctx))->setUploadLengthAtStartup(uploadLength);
    }

    // TODO implement the conversion mechanism between different piece length.
    uint32_t bitfieldLength;
    in.read(reinterpret_cast<char*>(&bitfieldLength), sizeof(bitfieldLength));
    uint32_t expectedBitfieldLength = ((totalLength+pieceLength-1)/pieceLength+7)/8;
    if(expectedBitfieldLength != bitfieldLength) {
      throw new DlAbortEx("bitfield length mismatch. expected: %d, actual: %d",
			  expectedBitfieldLength,
			  bitfieldLength);
    }

    savedBitfield = new unsigned char[bitfieldLength];
    in.read(reinterpret_cast<char*>(savedBitfield), bitfieldLength);

    if(pieceLength == _dctx->getPieceLength()) {
      _pieceStorage->setBitfield(savedBitfield, bitfieldLength);
      delete [] savedBitfield;
      savedBitfield = 0;

      uint32_t numInFlightPiece;
      in.read(reinterpret_cast<char*>(&numInFlightPiece), sizeof(numInFlightPiece));
      
      Pieces inFlightPieces;
      while(numInFlightPiece--) {
	uint32_t index;
	in.read(reinterpret_cast<char*>(&index), sizeof(index));
	if(!(index < _dctx->getNumPieces())) {
	  throw new DlAbortEx("piece index out of range: %u", index);
	}
	uint32_t length;
	in.read(reinterpret_cast<char*>(&length), sizeof(length));
	if(!(length <=_dctx->getPieceLength())) {
	  throw new DlAbortEx("piece length out of range: %u", length);
	}
	PieceHandle piece = new Piece(index, length);
	uint32_t bitfieldLength;
	in.read(reinterpret_cast<char*>(&bitfieldLength), sizeof(bitfieldLength));
	if(piece->getBitfieldLength() != bitfieldLength) {
	  throw new DlAbortEx("piece bitfield length mismatch. expected: %u actual: %u",
			      piece->getBitfieldLength(), bitfieldLength);
	}
	savedBitfield = new unsigned char[bitfieldLength];
	in.read(reinterpret_cast<char*>(savedBitfield), bitfieldLength);
	piece->setBitfield(savedBitfield, bitfieldLength);
	delete [] savedBitfield;
	savedBitfield = 0;
	
	inFlightPieces.push_back(piece);
      }
      _pieceStorage->addInFlightPiece(inFlightPieces);
    } else {
      uint32_t numInFlightPiece;
      in.read(reinterpret_cast<char*>(&numInFlightPiece), sizeof(numInFlightPiece));
      BitfieldMan src(pieceLength, totalLength);
      src.setBitfield(savedBitfield, bitfieldLength);
      if((src.getCompletedLength() || numInFlightPiece) &&
	 !_option->getAsBool(PREF_ALLOW_PIECE_LENGTH_CHANGE)) {
	throw new DownloadFailureException("WARNING: Detected a change in piece length. You can proceed with --allow-piece-length-change=true, but you may lose some download progress.");
      }
      BitfieldMan dest(_dctx->getPieceLength(), totalLength);
      Util::convertBitfield(&dest, &src);
      _pieceStorage->setBitfield(dest.getBitfield(), dest.getBitfieldLength());
      delete [] savedBitfield;
      savedBitfield = 0;
    }
    _logger->info(MSG_LOADED_SEGMENT_FILE);
  } catch(std::ios::failure const& exception) {
    delete [] savedBitfield;
    delete [] savedInfoHash;
    // TODO std::ios::failure doesn't give us the reasons of failure...
    throw new DlAbortEx(EX_SEGMENT_FILE_READ,
			_filename.c_str(), strerror(errno));
  } 
}

void DefaultBtProgressInfoFile::removeFile() {
  if(exists()) {
    File f(_filename);
    f.remove();
  }
}

bool DefaultBtProgressInfoFile::exists() {
  File f(_filename);
  if(f.isFile()) {
    _logger->info(MSG_SEGMENT_FILE_EXISTS, _filename.c_str());
    return true;
  } else {
    _logger->info(MSG_SEGMENT_FILE_DOES_NOT_EXIST, _filename.c_str());
    return false;
  }
}

} // namespace aria2
