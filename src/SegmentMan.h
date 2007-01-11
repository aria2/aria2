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
#ifndef _D_SEGMENT_MAN_H_
#define _D_SEGMENT_MAN_H_

#include "common.h"
#include "Logger.h"
#include "Segment.h"
#include "Option.h"
#include "DiskWriter.h"
#include "Request.h"
#include "BitfieldMan.h"
#include "PeerStat.h"

using namespace std;

#define SEGMENT_FILE_EXTENSION ".aria2"

class SegmentEntry {
public:
  int cuid;
  Segment segment;
public:
  SegmentEntry(int cuid, const Segment& segment)
    :cuid(cuid), segment(segment) {}
  ~SegmentEntry() {}
};

typedef SharedHandle<SegmentEntry> SegmentEntryHandle;
typedef deque<SegmentEntryHandle> SegmentEntries;
typedef deque<PeerStatHandle> PeerStats;

/**
 * This class holds the download progress of the one download entry.
 */
class SegmentMan {
private:
  const Logger* logger;
  BitfieldMan* bitfield;
  SegmentEntries usedSegmentEntries;
  PeerStats peerStats;

  void read(FILE* file);
  FILE* openSegFile(const string& segFilename, const string& mode) const;
  bool onNullBitfield(Segment& segment, int cuid);
  Segment checkoutSegment(int cuid, int index);
  SegmentEntryHandle findSlowerSegmentEntry(const PeerStatHandle& peerStat) const;
public:
  /**
   * The total number of bytes to download.
   * If Transfer-Encoding is Chunked or Content-Length header is not provided,
   * then this value is set to be 0.
   */
  long long int totalSize;
  /**
   * Represents whether this download is splittable.
   * In Split download(or segmented download), http client establishes
   * more than one connections to the server, and downloads sevral parts of
   * a file at the same time. This boosts download speed.
   * This value is true by default. If total number of bytes is not known or
   * Chunked transfer encoding is used, then this value is set to be 0 by
   * DownloadCommand class.
   */
  bool isSplittable;
  /**
   * Represents whether the download is start or not.
   * The default value is false.
   */
  bool downloadStarted;
  /**
   * Respresents the file name of the downloaded file.
   * If the URL does not contain file name part(http://www.rednoah.com/, for 
   * example), this value may be 0 length string.
   * The default value is 0 length string.
   */
  string filename;
  /**
   * directory to store a file
   */
  string dir;
  /**
   * User defined file name for downloaded content
   */
  string ufilename;

  /**
   * Represents the number of failures(usually, DlAbortEx) in downloads.
   */
  int errors;

  const Option* option;
  DiskWriter* diskWriter;
  Requests reserved;

  SegmentMan();
  ~SegmentMan();
  
  // Initializes totalSize, isSplittable, downloadStarted, errors.
  // Clears command queue. Also, closes diskWriter.
  void init();

  /**
   * Returns dir+"/"+filename.
   * If filename is empty, then returns dir+"/"+"inex.html";
   */
  string getFilePath() const {
    return (dir == "" ? "." : dir)+"/"+
      (ufilename == "" ? 
       (filename == "" ? "index.html" : filename) : ufilename);
  }

  string getSegmentFilePath() const {
    return getFilePath()+SEGMENT_FILE_EXTENSION;
  }

  /**
   * Returns true only if the segment data file exists.
   * The file name of the segment data is filename appended by ".aria2".
   * If isSplittable is false, then returns simply false without any operation.
   */
  bool segmentFileExists() const;
  /**
   * Loads the segment data file.
   * If isSplittable is false, then returns without any operation.
   */
  void load();
  /**
   * Saves the segment data file.
   * If isSplittable is false, then returns without any operation.
   */
  void save() const;
  /**
   * Removes the segment data file.
   * If isSplittable is false, then returns without any operation.
   */
  void remove() const;
  /**
   * Returs true when the download has finished.
   * If downloadStarted is false or the number of the segments of this object
   * holds is 0, then returns false.
   */
  bool finished() const;
  /**
   * if finished() is true, then call remove()
   */
  void removeIfFinished() const;
  /**
   * Returns a vacant segment.
   * If there is no vacant segment, then returns a segment instance whose
   * isNull call is true.
   */
  bool getSegment(Segment& segment, int cuid);
  /**
   * Returns a segment whose index is index. 
   * If it has already assigned
   * to another cuid or has been downloaded, then returns a segment instance
   * whose isNull call is true.
   */
  bool getSegment(Segment& segment, int cuid, int index);
  /**
   * Updates download status.
   */
  bool updateSegment(int cuid, const Segment& segment);
  /**
   * Cancels all the segment which the command having given cuid
   * uses.
   */
  void cancelSegment(int cuid);
  /**
   * Tells SegmentMan that the segment has been downloaded successfully.
   */
  bool completeSegment(int cuid, const Segment& segment);
  /**
   * Initializes bitfield with the provided length parameters.
   */
  void initBitfield(int segmentLength, long long int totalLength);
  /**
   * Returns true if the segment whose index is index has been downloaded.
   */
  bool hasSegment(int index) const;
  /**
   * Returns the length of bytes downloaded.
   */
  long long int getDownloadLength() const;

  /**
   * Registers given peerStat if it has not been registerd.
   * Otherwise does nothing.
   */
  void registerPeerStat(const PeerStatHandle& peerStat);

  class FindPeerStat {
  private:
    int cuid;
  public:
    FindPeerStat(int cuid):cuid(cuid) {}

    bool operator()(const PeerStatHandle& peerStat) {
      if(peerStat->getCuid() == cuid) {
	return true;
      } else {
	return false;
      }
    }
  };

  /**
   * Returns peerStat whose cuid is given cuid. If it is not found, returns
   * PeerStatHandle(0).
   */
  PeerStatHandle getPeerStat(int cuid) const {
    PeerStats::const_iterator itr = find_if(peerStats.begin(), peerStats.end(),
					    FindPeerStat(cuid));
    if(itr == peerStats.end()) {
      // TODO
      return PeerStatHandle(0);
    } else {
      return *itr;
    }
  }

  /**
   * Returns current download speed in bytes per sec. 
   */
  int calculateDownloadSpeed() const;

  bool fileExists();

  bool shouldCancelDownloadForSafety();

};

#endif // _D_SEGMENT_MAN_H_
