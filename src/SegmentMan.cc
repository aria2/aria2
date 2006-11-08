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
#include "SegmentMan.h"
#include "DlAbortEx.h"
#include "Util.h"
#include "File.h"
#include "message.h"
#include "prefs.h"
#include "LogFactory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

SegmentMan::SegmentMan():bitfield(0),
			 totalSize(0),
			 isSplittable(true), 
			 downloadStarted(false),
			 dir("."),
			 errors(0),
			 diskWriter(0) {
  logger = LogFactory::getInstance();
}

SegmentMan::~SegmentMan() {
  delete bitfield;
  delete diskWriter;
}

bool SegmentMan::segmentFileExists() const {
  if(!isSplittable) {
    return false;
  }
  string segFilename = getSegmentFilePath();
  File f(segFilename);
  if(f.isFile()) {
    logger->info(MSG_SEGMENT_FILE_EXISTS, segFilename.c_str());
    return true;
  } else {
    logger->info(MSG_SEGMENT_FILE_DOES_NOT_EXIST, segFilename.c_str());
    return false;
  }
}

void SegmentMan::load() {
  if(!isSplittable) {
    return;
  }
  string segFilename = getSegmentFilePath();
  logger->info(MSG_LOADING_SEGMENT_FILE, segFilename.c_str());
  FILE* segFile = openSegFile(segFilename, "r+");
  try {
    read(segFile);
    fclose(segFile);
  } catch(string ex) {
    fclose(segFile);
    throw new DlAbortEx(EX_SEGMENT_FILE_READ,
			segFilename.c_str(), strerror(errno));
  }
  logger->info(MSG_LOADED_SEGMENT_FILE);
}

void SegmentMan::save() const {
  if(!isSplittable || totalSize == 0) {
    return;
  }
  string segFilename = getSegmentFilePath();
  logger->info(MSG_SAVING_SEGMENT_FILE, segFilename.c_str());
  FILE* segFile = openSegFile(segFilename, "w");
  try {
    if(fwrite(&totalSize, sizeof(totalSize), 1, segFile) < 1) {
      throw string("writeError");
    }
    int segmentLength = bitfield->getBlockLength();
    if(fwrite(&segmentLength, sizeof(segmentLength), 1, segFile) < 1) {
      throw string("writeError");
    }
    if(bitfield) {
      int bitfieldLength = bitfield->getBitfieldLength();
      if(fwrite(&bitfieldLength, sizeof(bitfieldLength), 1, segFile) < 1) {
	throw string("writeError");
      }
      if(fwrite(bitfield->getBitfield(), bitfield->getBitfieldLength(),
		1, segFile) < 1) {
	throw string("writeError");
      }					 
    } else {
      int i = 0;
      if(fwrite(&i, sizeof(i), 1, segFile) < 1) {
	throw string("writeError");
      }
    }
    int usedSegmentCount = usedSegmentEntries.size();
    if(fwrite(&usedSegmentCount, sizeof(usedSegmentCount), 1, segFile) < 1) {
      throw string("writeError");
    }
    for(SegmentEntries::const_iterator itr = usedSegmentEntries.begin();
	itr != usedSegmentEntries.end(); itr++) {
      if(fwrite(&(*itr)->segment, sizeof(Segment), 1, segFile) < 1) {
	throw string("writeError");
      }
    }
    fclose(segFile);
    logger->info(MSG_SAVED_SEGMENT_FILE);
  } catch(string ex) {
    fclose(segFile);
    throw new DlAbortEx(EX_SEGMENT_FILE_WRITE,
			segFilename.c_str(), strerror(errno));
  }
}

FILE* SegmentMan::openSegFile(const string& segFilename, const string& mode) const {
  FILE* segFile = fopen(segFilename.c_str(), mode.c_str());
  if(segFile == NULL) {
    throw new DlAbortEx(EX_SEGMENT_FILE_OPEN,
			segFilename.c_str(), strerror(errno));
  }
  return segFile;
}

void SegmentMan::read(FILE* file) {
  assert(file != NULL);
  if(fread(&totalSize, sizeof(totalSize), 1, file) < 1) {
    throw string("readError");
  }
  int segmentSize;
  if(fread(&segmentSize, sizeof(segmentSize), 1, file) < 1) {
    throw string("readError");
  }
  int bitfieldLength;
  if(fread(&bitfieldLength, sizeof(bitfieldLength), 1, file) < 1) {
    throw string("readError");
  }
  if(bitfieldLength > 0) {
    initBitfield(segmentSize, totalSize);
    unsigned char* savedBitfield = new unsigned char[bitfield->getBitfieldLength()];
    if(fread(savedBitfield, bitfield->getBitfieldLength(), 1, file) < 1) {
      delete [] savedBitfield;
      throw string("readError");
    } else {
      bitfield->setBitfield(savedBitfield, bitfield->getBitfieldLength());
      delete [] savedBitfield;
    }
  }
  int segmentCount;
  if(fread(&segmentCount, sizeof(segmentCount), 1, file) < 1) {
    throw string("readError");
  }
  while(segmentCount--) {
    Segment seg;
    if(fread(&seg, sizeof(Segment), 1, file) < 1) {
      throw string("readError");
    }
    usedSegmentEntries.push_back(SegmentEntryHandle(new SegmentEntry(0, seg)));
  }
}

void SegmentMan::remove() const {
  if(!isSplittable) {
    return;
  }
  if(segmentFileExists()) {
    File f(getSegmentFilePath());
    f.remove();
  }
}

bool SegmentMan::finished() const {
  if(!downloadStarted) {
    return false;
  }
  if(!bitfield) {
    return false;
  }
  assert(bitfield);
  return bitfield->isAllBitSet();
}

void SegmentMan::removeIfFinished() const {
  if(finished()) {
    remove();
  }
}

void SegmentMan::init() {
  totalSize = 0;
  isSplittable = false;
  downloadStarted = false;
  errors = 0;
  //segments.clear();
  usedSegmentEntries.clear();
  delete bitfield;
  bitfield = 0;
  peerStats.clear();
  diskWriter->closeFile();
  
}

void SegmentMan::initBitfield(int segmentLength, long long int totalLength) {
  this->bitfield = new BitfieldMan(segmentLength, totalLength);
}

class FindSegmentEntryByIndex {
private:
  int index;
public:
  FindSegmentEntryByIndex(int index):index(index) {}
  
  bool operator()(const SegmentEntryHandle& entry) {
    return entry->segment.index == index;
  }
};

class FindSegmentEntryByCuid {
private:
  int cuid;
public:
  FindSegmentEntryByCuid(int cuid):cuid(cuid) {}

  bool operator()(const SegmentEntryHandle& entry) {
    return entry->cuid == cuid;
  }
};

Segment SegmentMan::checkoutSegment(int cuid, int index) {
  logger->debug("Attach segment#%d to CUID#%d.", index, cuid);
  bitfield->setUseBit(index);

  SegmentEntries::iterator itr = find_if(usedSegmentEntries.begin(),
					 usedSegmentEntries.end(),
					 FindSegmentEntryByIndex(index));
  Segment segment;
  if(itr == usedSegmentEntries.end()) {
    segment = Segment(index, bitfield->getBlockLength(index),
		       bitfield->getBlockLength());
    SegmentEntryHandle entry =
      SegmentEntryHandle(new SegmentEntry(cuid, segment));
    usedSegmentEntries.push_back(entry);
  } else {
    (*itr)->cuid = cuid;
    segment = (*itr)->segment;
  }

  logger->debug("index=%d, length=%d, segmentLength=%d, writtenLength=%d",
		segment.index, segment.length, segment.segmentLength,
		segment.writtenLength);
  return segment;
}

bool SegmentMan::onNullBitfield(Segment& segment, int cuid) {
  if(usedSegmentEntries.size() == 0) {
    segment = Segment(0, 0, 0);
    usedSegmentEntries.push_back(SegmentEntryHandle(new SegmentEntry(cuid, segment)));
    return true;
  } else {
    SegmentEntries::iterator itr = find_if(usedSegmentEntries.begin(),
					   usedSegmentEntries.end(),
					   FindSegmentEntryByCuid(cuid));
    if(itr == usedSegmentEntries.end()) {
      return false;
    } else {
      segment = (*itr)->segment;
      return true;
    }
  }
}

SegmentEntryHandle SegmentMan::findSlowerSegmentEntry(const PeerStatHandle& peerStat) const {
  int speed = (int)(peerStat->getAvgDownloadSpeed()*0.8);
  SegmentEntryHandle slowSegmentEntry(0);
  for(SegmentEntries::const_iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); itr++) {
    const SegmentEntryHandle& segmentEntry = *itr;
    if(segmentEntry->cuid == 0) {
      continue;
    }
    PeerStatHandle p = getPeerStat(segmentEntry->cuid);
    if(!p.get() || p->getCuid() == peerStat->getCuid() ||
       p->getStatus() != PeerStat::ACTIVE ||
       !p->getDownloadStartTime().elapsed(option->getAsInt(PREF_STARTUP_IDLE_TIME))) {
      continue;
    }
    int pSpeed = p->calculateDownloadSpeed(); 
    if(pSpeed < speed) {
      speed = pSpeed;
      slowSegmentEntry = segmentEntry;
    }
  }
  return slowSegmentEntry;
}

bool SegmentMan::getSegment(Segment& segment, int cuid) {
  if(!bitfield) {
    return onNullBitfield(segment, cuid);
  }
  SegmentEntries::iterator itr = find_if(usedSegmentEntries.begin(),
					 usedSegmentEntries.end(),
					 FindSegmentEntryByCuid(cuid));
  if(itr != usedSegmentEntries.end()) {
    segment = (*itr)->segment;
    return true;
  }
  int index = bitfield->getSparseMissingUnusedIndex();
  if(index == -1) {
    PeerStatHandle myPeerStat = getPeerStat(cuid);
    if(!myPeerStat.get()) {
      return false;
    }
    SegmentEntryHandle slowSegmentEntry = findSlowerSegmentEntry(myPeerStat);
    if(slowSegmentEntry.get()) {
      logger->info("CUID#%d cancels segment index=%d. CUID#%d handles it instead.",
		   slowSegmentEntry->cuid,
		   slowSegmentEntry->segment.index,
		   cuid);
      PeerStatHandle slowPeerStat = getPeerStat(slowSegmentEntry->cuid);
      slowPeerStat->requestIdle();
      cancelSegment(slowSegmentEntry->cuid);
      segment = checkoutSegment(cuid, slowSegmentEntry->segment.index);
      return true;
    } else {
      return false;
    }
  } else {
    segment = checkoutSegment(cuid, index);
    return true;
  }
}

bool SegmentMan::getSegment(Segment& segment, int cuid, int index) {
  if(!bitfield) {
    return onNullBitfield(segment, cuid);
  }
  if(index < 0 || bitfield->countBlock() <= index) {
    return false;
  }
  if(bitfield->isBitSet(index) || bitfield->isUseBitSet(index)) {
    return false;
  } else {
    segment = checkoutSegment(cuid, index);
    return true;
  }
}

bool SegmentMan::updateSegment(int cuid, const Segment& segment) {
  if(segment.isNull()) {
    return false;
  }
  SegmentEntries::iterator itr = find_if(usedSegmentEntries.begin(),
					 usedSegmentEntries.end(),
					 FindSegmentEntryByCuid(cuid));
  if(itr == usedSegmentEntries.end()) {
    return false;
  } else {
    (*itr)->segment = segment;
    return true;
  }
}

class CancelSegment {
private:
  int cuid;
  BitfieldMan* bitfield;
public:
  CancelSegment(int cuid, BitfieldMan* bitfield):cuid(cuid),
						 bitfield(bitfield) {}
  
  void operator()(SegmentEntryHandle& entry) {
    if(entry->cuid == cuid) {
      bitfield->unsetUseBit(entry->segment.index);
      entry->cuid = 0;
    }
  }
};

void SegmentMan::cancelSegment(int cuid) {
  if(bitfield) {
    for_each(usedSegmentEntries.begin(), usedSegmentEntries.end(),
	     CancelSegment(cuid, bitfield));
  } else {
    usedSegmentEntries.clear();
  }
}

bool SegmentMan::completeSegment(int cuid, const Segment& segment) {
  if(segment.isNull()) {
    return false;
  }
  if(bitfield) {
    bitfield->unsetUseBit(segment.index);
    bitfield->setBit(segment.index);
  } else {
    initBitfield(option->getAsInt(PREF_SEGMENT_SIZE), segment.writtenLength);
    bitfield->setAllBit();
  }
  SegmentEntries::iterator itr = find_if(usedSegmentEntries.begin(),
					 usedSegmentEntries.end(),
					 FindSegmentEntryByCuid(cuid));
  if(itr == usedSegmentEntries.end()) {
    return false;
  } else {
    usedSegmentEntries.erase(itr);
    return true;
  }
}

bool SegmentMan::hasSegment(int index) const {
  if(bitfield) {
    return bitfield->isBitSet(index);
  } else {
    return false;
  }
}

long long int SegmentMan::getDownloadLength() const {
  long long int dlLength = 0;
  if(bitfield) {
    dlLength += bitfield->getCompletedLength();
  }
  for(SegmentEntries::const_iterator itr = usedSegmentEntries.begin();
      itr != usedSegmentEntries.end(); itr++) {
    dlLength += (*itr)->segment.writtenLength;
  }
  return dlLength;
}

void SegmentMan::registerPeerStat(const PeerStatHandle& peerStat) {
  PeerStatHandle temp = getPeerStat(peerStat->getCuid());
  if(!temp.get()) {
    peerStats.push_back(peerStat);
  }
}

int SegmentMan::calculateDownloadSpeed() const {
  int speed = 0;
  for(PeerStats::const_iterator itr = peerStats.begin();
      itr != peerStats.end(); itr++) {
    const PeerStatHandle& peerStat = *itr;
    if(peerStat->getStatus() == PeerStat::ACTIVE) {
      speed += peerStat->calculateDownloadSpeed();
    }
  }
  return speed;
}
