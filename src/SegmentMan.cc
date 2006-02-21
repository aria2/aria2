/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "SegmentMan.h"
#include "DlAbortEx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "Util.h"
#include "File.h"
#include "message.h"

SegmentMan::SegmentMan():totalSize(0),isSplittable(true),downloadStarted(false),dir(".") {}

SegmentMan::~SegmentMan() {}

void SegmentMan::unregisterId(int cuid) {
  for(vector<Segment>::iterator itr = segments.begin(); itr != segments.end(); itr++) {
    if((*itr).cuid == cuid) {
      cuid = 0;
    }
  }
}

bool SegmentMan::getSegment(Segment& seg, int cuid) {
  //Segment s = { 0, 0, 0, false };

  if(segments.empty()) {
    logger->debug("assign new segment { sp = 0, ep = "+(totalSize == 0 ? "0" : Util::llitos(totalSize-1))+" } to cuid "+Util::llitos(cuid));
    //seg = { cuid, 0, totalSize == 0 ? 0 : totalSize-1, 0, false };
    seg.cuid = cuid;
    seg.sp = 0;
    seg.ep = totalSize == 0 ? 0 : totalSize-1;
    seg.ds = 0;
    seg.finish = false;
    segments.push_back(seg);
    return true;
  }
  for(vector<Segment>::iterator itr = segments.begin(); itr != segments.end(); itr++) {
    if((*itr).cuid == cuid && !(*itr).finish) {
//       logger->debug("return an existing segment { "
// 		    "sp = "+Util::ulitos((*itr).sp)+", "+
// 		    "ep = "+Util::ulitos((*itr).ep)+", "
// 		    "ds = "+Util::ulitos((*itr).ds)+" } to "+
// 		    "cuid "+Util::ulitos((*itr).cuid));
      seg = *itr;
      return true;
    }
  }
  if(!isSplittable) {
    return false;
  }
  for(vector<Segment>::iterator itr = segments.begin(); itr != segments.end(); itr++) {
    Segment& s = *itr;
    if(s.finish) {
      continue;
    }
    if(s.cuid == 0) {
      s.cuid = cuid;
      seg = s;
      return true;
    }
  }
  for(vector<Segment>::iterator itr = segments.begin(); itr != segments.end(); itr++) {
    Segment& s = *itr;
    if(s.finish) {
      continue;
    }
    if(s.ep-(s.sp+s.ds) > 524288) {
      long long int nep = (s.ep-(s.sp+s.ds))/2+(s.sp+s.ds);
      //nseg = { cuid, nep+1, s.ep, 0, false };
      seg.cuid = cuid;
      seg.sp = nep+1;
      seg.ep = s.ep;
      seg.ds = 0;
      seg.finish = false;
      s.ep = nep;
      logger->debug("return new segment { "
		    "sp = "+Util::llitos(seg.sp)+", "+
		    "ep = "+Util::llitos(seg.ep)+", "
		    "ds = "+Util::llitos(seg.ds)+" } to "+
		    "cuid "+Util::llitos(cuid));
      logger->debug("update segment { "
		    "sp = "+Util::llitos(s.sp)+", "+
		    "ep = "+Util::llitos(s.ep)+", "
		    "ds = "+Util::llitos(s.ds)+" } of "+
		    "cuid "+Util::llitos(s.cuid));
      segments.push_back(seg);
      return true;
    }
  }
  return false;
}

void SegmentMan::updateSegment(const Segment& segment) {
  for(vector<Segment>::iterator itr = segments.begin(); itr != segments.end(); itr++) {
    if((*itr).cuid == segment.cuid && 
       (*itr).sp == segment.sp &&
       (*itr).ep == segment.ep) {
      *itr = segment;
      break;
    }
  } 
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
  read(segFile);
  fclose(segFile);
  logger->info(MSG_LOADED_SEGMENT_FILE);
  for(vector<Segment>::iterator itr = segments.begin(); itr != segments.end();
      itr++) {
    (*itr).cuid = 0;
  }
}

void SegmentMan::save() const {
  if(!isSplittable || totalSize == 0) {
    return;
  }
  string segFilename = getSegmentFilePath();
  logger->info(MSG_SAVING_SEGMENT_FILE, segFilename.c_str());
  FILE* segFile = openSegFile(segFilename, "w");
  if(fwrite(&totalSize, sizeof(totalSize), 1, segFile) < 1) {
    throw new DlAbortEx(strerror(errno));
  }
  for(vector<Segment>::const_iterator itr = segments.begin(); itr != segments.end(); itr++) {
    if(fwrite(&*itr, sizeof(Segment), 1, segFile) < 1) {
      throw new DlAbortEx(strerror(errno));
    }
  }
  fclose(segFile);
  logger->info(MSG_SAVED_SEGMENT_FILE);
}

FILE* SegmentMan::openSegFile(string segFilename, string mode) const {
  FILE* segFile = fopen(segFilename.c_str(), mode.c_str());
  if(segFile == NULL) {
    throw new DlAbortEx(strerror(errno));
  }
  return segFile;
}

void SegmentMan::read(FILE* file) {
  assert(file != NULL);
  if(fread(&totalSize, sizeof(totalSize), 1, file) < 1) {
    throw new DlAbortEx(strerror(errno));
  }
  while(1) {
    Segment seg;
    if(fread(&seg, sizeof(Segment), 1, file) < 1) {
      if(ferror(file)) {
	throw new DlAbortEx(strerror(errno));
      } else if(feof(file)) {
	break;
      }
    }
    segments.push_back(seg);
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
  if(!downloadStarted || segments.size() == 0) {
    return false;
  }
  for(vector<Segment>::const_iterator itr = segments.begin(); itr != segments.end(); itr++) {
    if(!(*itr).finish) {
      return false;
    }
  }
  return true;
}

void SegmentMan::removeIfFinished() const {
  if(finished()) {
    remove();
  }
}

long long int SegmentMan::getDownloadedSize() const {
  long long int size = 0;
  for(vector<Segment>::const_iterator itr = segments.begin(); itr != segments.end(); itr++) {
    size += (*itr).ds;
  }
  return size;
}

