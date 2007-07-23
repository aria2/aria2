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
#include "BtRegistry.h"
#include "LogFactory.h"
#include "prefs.h"
#include "DlAbortEx.h"
#include "message.h"
#include "File.h"
#include "Util.h"
#include "a2io.h"
#include <errno.h>

DefaultBtProgressInfoFile::DefaultBtProgressInfoFile(const BtContextHandle& btContext,
						     const Option* option):
  btContext(btContext),
  option(option),
  pieceStorage(PIECE_STORAGE(btContext)),
  btRuntime(BT_RUNTIME(btContext)),
  peerStorage(PEER_STORAGE(btContext))
{
  logger = LogFactory::getInstance();
  string storeDir = option->get(PREF_DIR);
  filename = storeDir+"/"+btContext->getName()+".aria2";
}

DefaultBtProgressInfoFile::~DefaultBtProgressInfoFile() {}

void DefaultBtProgressInfoFile::save() {
  logger->info(MSG_SAVING_SEGMENT_FILE, filename.c_str());
  FILE* file = openFile(filename, "wb");
  try {
    if(fwrite(btContext->getInfoHash(),
	      btContext->getInfoHashLength(), 1, file) < 1) {
      throw string("writeError:info hash");
    }
    if(fwrite(pieceStorage->getBitfield(),
	      pieceStorage->getBitfieldLength(), 1, file) < 1) {
      throw string("writeError:bitfield");
    }
    TransferStat stat = peerStorage->calculateStat();
    int64_t allTimeDownloadLength = pieceStorage->getCompletedLength();
    if(fwrite(&allTimeDownloadLength,
	      sizeof(allTimeDownloadLength), 1, file) < 1) {
      throw string("writeError:download length");
    }
    int64_t allTimeUploadLength =
      btRuntime->getUploadLengthAtStartup()+
      stat.getSessionUploadLength();
    if(fwrite(&allTimeUploadLength,
	      sizeof(allTimeUploadLength), 1, file) < 1) {
      throw string("writeError:upload length");
    }
    fclose(file);
    logger->info(MSG_SAVED_SEGMENT_FILE);
  } catch(string ex) {
    fclose(file);
    throw new DlAbortEx(EX_SEGMENT_FILE_WRITE,
			filename.c_str(), strerror(errno));
  }
}

void DefaultBtProgressInfoFile::load() {
  logger->info(MSG_LOADING_SEGMENT_FILE, filename.c_str());
  FILE* file = openFile(filename, "r+b");
  unsigned char* savedInfoHash = 0;
  unsigned char* savedBitfield = 0;
  try {
    savedInfoHash = new unsigned char[btContext->getInfoHashLength()];
    savedBitfield = new unsigned char[pieceStorage->getBitfieldLength()];
    if(fread(savedInfoHash, btContext->getInfoHashLength(), 1, file) < 1) {
      throw string("readError");
    }
    if(Util::toHex(savedInfoHash, btContext->getInfoHashLength()) != 
       btContext->getInfoHashAsString()) {
      throw string("infoHashMismatch");
    }
    if(fread(savedBitfield, pieceStorage->getBitfieldLength(), 1, file) < 1) {
      throw string("readError");
    }
    pieceStorage->setBitfield(savedBitfield,
			      pieceStorage->getBitfieldLength());
    // allTimeDownloadLength exists for only a compatibility reason.
    int64_t allTimeDownloadLength;
    if(fread(&allTimeDownloadLength,
	     sizeof(allTimeDownloadLength), 1, file) < 1) {
      throw string("readError");
    }
    int64_t allTimeUploadLength;
    if(fread(&allTimeUploadLength,
	     sizeof(allTimeUploadLength), 1, file) < 1) {
      throw string("readError");
    }
    btRuntime->setUploadLengthAtStartup(allTimeUploadLength);
    delete [] savedBitfield;
    savedBitfield = 0;
    delete [] savedInfoHash;
    savedInfoHash = 0;
    fclose(file);
  } catch(string ex) {
    if(savedBitfield) {
      delete [] savedBitfield;
    }
    if(savedInfoHash) {
      delete [] savedInfoHash;
    }
    fclose(file);
    if(ex == "infoHashMismatch") {
      throw new DlAbortEx(EX_INFOHASH_MISMATCH_IN_SEGFILE);
    } else {
      throw new DlAbortEx(EX_SEGMENT_FILE_READ,
			  filename.c_str(), strerror(errno));
    }
  }
  logger->info(MSG_LOADED_SEGMENT_FILE);
}

void DefaultBtProgressInfoFile::removeFile() {
  if(exists()) {
    File f(filename);
    f.remove();
  }
}

FILE* DefaultBtProgressInfoFile::openFile(const string& filename,
					  const string& mode) const
{
  FILE* file = fopen(filename.c_str(), mode.c_str());
  if(!file) {
    throw new DlAbortEx(EX_SEGMENT_FILE_OPEN,
			filename.c_str(), strerror(errno));
  }
#ifdef HAVE_SETMODE
  setmode(fileno(file), O_BINARY);
#endif
  return file;
}

bool DefaultBtProgressInfoFile::exists() {
  File f(filename);
  if(f.isFile()) {
    logger->info(MSG_SEGMENT_FILE_EXISTS, filename.c_str());
    return true;
  } else {
    logger->info(MSG_SEGMENT_FILE_DOES_NOT_EXIST, filename.c_str());
    return false;
  }
}
