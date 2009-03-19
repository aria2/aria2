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

#include <deque>
#include <map>

#include "SharedHandle.h"
#include "TimeA2.h"
#include "Command.h"

namespace aria2 {

class Segment;
class Logger;
class Option;
class PeerStat;
class DownloadContext;
class PieceStorage;
class Piece;

class SegmentEntry {
public:
  cuid_t cuid;
  SharedHandle<Segment> segment;
public:
  SegmentEntry(cuid_t cuid, const SharedHandle<Segment>& segment);

  ~SegmentEntry();
};

typedef SharedHandle<SegmentEntry> SegmentEntryHandle;
typedef std::deque<SegmentEntryHandle> SegmentEntries;

/**
 * This class holds the download progress of the one download entry.
 */
class SegmentMan {
private:
  const Option* _option;

  Logger* logger;

  SharedHandle<DownloadContext> _downloadContext;

  SharedHandle<PieceStorage> _pieceStorage;

  SegmentEntries usedSegmentEntries;

  std::deque<SharedHandle<PeerStat> > peerStats;

  // key: PeerStat's cuid, value: its download speed
  std::map<cuid_t, unsigned int> _peerStatDlspdMap;

  Time _lastPeerStatDlspdMapUpdated;

  SharedHandle<Segment> checkoutSegment(cuid_t cuid,
					const SharedHandle<Piece>& piece);

  SharedHandle<SegmentEntry> findSlowerSegmentEntry
  (const SharedHandle<PeerStat>& peerStat);
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
  uint64_t getTotalLength() const;

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
  void getInFlightSegment(std::deque<SharedHandle<Segment> >& segments,
			  cuid_t cuid);

  SharedHandle<Segment> getSegment(cuid_t cuid);

  /**
   * Returns a segment whose index is index. 
   * If it has already assigned
   * to another cuid or has been downloaded, then returns a segment instance
   * whose isNull call is true.
   */
  SharedHandle<Segment> getSegment(cuid_t cuid, size_t index);
  /**
   * Updates download status.
   */
  //bool updateSegment(int cuid, const Segment& segment);
  /**
   * Cancels all the segment which the command having given cuid
   * uses.
   */
  void cancelSegment(cuid_t cuid);
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
  uint64_t getDownloadLength() const;

  /**
   * Registers given peerStat if it has not been registerd and returns true.
   * Otherwise does nothing and returns false.
   */
  bool registerPeerStat(const SharedHandle<PeerStat>& peerStat);

  /**
   * Returns peerStat whose cuid is given cuid. If it is not found, returns
   * 0.
   */
  SharedHandle<PeerStat> getPeerStat(cuid_t cuid) const;


  const std::deque<SharedHandle<PeerStat> >& getPeerStats() const;

  /**
   * Returns current download speed in bytes per sec. 
   */
  unsigned int calculateDownloadSpeed();

  void updateDownloadSpeedFor(const SharedHandle<PeerStat>& pstat);

  /**
   * Returns the downloaded bytes in this session.
   */
  uint64_t calculateSessionDownloadLength() const;

  size_t countFreePieceFrom(size_t index) const;
};

typedef SharedHandle<SegmentMan> SegmentManHandle;

} // namespace aria2

#endif // _D_SEGMENT_MAN_H_
