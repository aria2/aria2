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
#ifndef D_SEGMENT_MAN_H
#define D_SEGMENT_MAN_H

#include "common.h"

#include <deque>
#include <vector>
#include <map>

#include "SharedHandle.h"
#include "TimerA2.h"
#include "Command.h"
#include "BitfieldMan.h"

namespace aria2 {

class Segment;
class Option;
class PeerStat;
class DownloadContext;
class PieceStorage;
class Piece;
class FileEntry;

struct SegmentEntry {
  cuid_t cuid;
  SharedHandle<Segment> segment;

  SegmentEntry(cuid_t cuid, const SharedHandle<Segment>& segment);
  ~SegmentEntry();

  // Don't allow copying
  SegmentEntry(const SegmentEntry&);
  SegmentEntry& operator=(const SegmentEntry&);
};

typedef SharedHandle<SegmentEntry> SegmentEntryHandle;
typedef std::deque<SegmentEntryHandle> SegmentEntries;

/**
 * This class holds the download progress of the one download entry.
 */
class SegmentMan {
private:
  const Option* option_;

  SharedHandle<DownloadContext> downloadContext_;

  SharedHandle<PieceStorage> pieceStorage_;

  SegmentEntries usedSegmentEntries_;

  // Remember writtenLength for each segment. The key is an index of a
  // segment. The value is writtenLength for that segment.
  std::map<size_t, int32_t> segmentWrittenLengthMemo_;

  // Used for calculating download speed.
  std::vector<SharedHandle<PeerStat> > peerStats_;

  // Keep track of fastest PeerStat for each server
  std::vector<SharedHandle<PeerStat> > fastestPeerStats_;

  // key: PeerStat's cuid, value: its download speed
  std::map<cuid_t, int> peerStatDlspdMap_;

  Timer lastPeerStatDlspdMapUpdated_;

  int cachedDlspd_;

  BitfieldMan ignoreBitfield_;

  SharedHandle<Segment> checkoutSegment(cuid_t cuid,
                                        const SharedHandle<Piece>& piece);

  void cancelSegmentInternal(cuid_t cuid, const SharedHandle<Segment>& segment);
public:
  SegmentMan(const Option* option,
             const SharedHandle<DownloadContext>& downloadContext,
             const SharedHandle<PieceStorage>& pieceStorage);

  ~SegmentMan();


  // Initializes totalSize, isSplittable, downloadStarted, errors.
  // Clears command queue. Also, closes diskWriter.
  void init();

  /**
   * The total number of bytes to download.
   * If Transfer-Encoding is Chunked or Content-Length header is not provided,
   * then this value is set to be 0.
   */
  int64_t getTotalLength() const;

  /**
   * Returs true when the download has finished.
   * If downloadStarted is false or the number of the segments of this object
   * holds is 0, then returns false.
   */
  bool downloadFinished() const;

  /**
   * Fill segments which are assigned to the command whose CUID is cuid.
   * This function doesn't clear passed segments.
   */
  void getInFlightSegment(std::vector<SharedHandle<Segment> >& segments,
                          cuid_t cuid);

  SharedHandle<Segment> getSegment(cuid_t cuid, size_t minSplitSize);

  // Checkouts segments in the range of fileEntry and push back to
  // segments until segments.size() < maxSegments holds false
  void getSegment(std::vector<SharedHandle<Segment> >& segments,
                  cuid_t cuid,
                  size_t minSplitSize,
                  const SharedHandle<FileEntry>& fileEntry,
                  size_t maxSegments);

  /**
   * Returns a segment whose index is index. 
   * If it has already assigned
   * to another cuid or has been downloaded, then returns a segment instance
   * whose isNull call is true.
   */
  SharedHandle<Segment> getSegmentWithIndex(cuid_t cuid, size_t index);

  // Returns a currently used segment whose index is index and written
  // length is 0.  The current owner(in idle state) of segment cancels
  // the segment and cuid command acquires the ownership of the
  // segment.  If no such segment exists, returns null.
  SharedHandle<Segment> getCleanSegmentIfOwnerIsIdle(cuid_t cuid, size_t index);

  /**
   * Updates download status.
   */
  //bool updateSegment(int cuid, const Segment& segment);
  /**
   * Cancels all the segment which the command having given cuid
   * uses.
   */
  void cancelSegment(cuid_t cuid);

  void cancelSegment(cuid_t cuid, const SharedHandle<Segment>& segment);

  void cancelAllSegments();

  void eraseSegmentWrittenLengthMemo();

  /**
   * Tells SegmentMan that the segment has been downloaded successfully.
   */
  bool completeSegment(cuid_t cuid, const SharedHandle<Segment>& segment);

  /**
   * Injects PieceStorage.
   */
  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  /**
   * Injects DownloadContext.
   */
  void setDownloadContext(const SharedHandle<DownloadContext>& downloadContext);

  /**
   * Returns true if the segment whose index is index has been downloaded.
   */
  bool hasSegment(size_t index) const;
  /**
   * Returns the length of bytes downloaded.
   */
  int64_t getDownloadLength() const;


  // If there is inactive PeerStat in peerStats_, it is replaced with
  // given peerStat. If no such PeerStat exist, the given peerStat is
  // inserted.
  void registerPeerStat(const SharedHandle<PeerStat>& peerStat);

  const std::vector<SharedHandle<PeerStat> >& getPeerStats() const
  {
    return peerStats_;
  }

  SharedHandle<PeerStat> getPeerStat(cuid_t cuid) const;

  // If there is slower PeerStat than given peerStat for the same
  // hostname and protocol in fastestPeerStats_, the former is
  // replaced with latter. If there are no PeerStat with same hostname
  // and protocol with given peerStat, given peerStat is inserted.
  void updateFastestPeerStat(const SharedHandle<PeerStat>& peerStat);

  const std::vector<SharedHandle<PeerStat> >& getFastestPeerStats() const
  {
    return fastestPeerStats_;
  }

  /**
   * Returns current download speed in bytes per sec. 
   */
  int calculateDownloadSpeed();

  void updateDownloadSpeedFor(const SharedHandle<PeerStat>& pstat);

  /**
   * Returns the downloaded bytes in this session.
   */
  int64_t calculateSessionDownloadLength() const;

  size_t countFreePieceFrom(size_t index) const;

  // Excludes segments that fileEntry covers from segment selection.
  void ignoreSegmentFor(const SharedHandle<FileEntry>& fileEntry);

  // Includes segments that fileEntry covers in segment selection.
  void recognizeSegmentFor(const SharedHandle<FileEntry>& fileEntry);

  bool allSegmentsIgnored() const;
};

} // namespace aria2

#endif // D_SEGMENT_MAN_H
