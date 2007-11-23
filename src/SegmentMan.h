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

class Segment;
typedef SharedHandle<Segment> SegmentHandle;
typedef deque<SegmentHandle> Segments;
class Logger;
class Option;
class PeerStat;
typedef SharedHandle<PeerStat> PeerStatHandle;
typedef deque<PeerStatHandle> PeerStats;
class DownloadContext;
typedef SharedHandle<DownloadContext> DownloadContextHandle;
class PieceStorage;
typedef SharedHandle<PieceStorage> PieceStorageHandle;
class Piece;
typedef SharedHandle<Piece> PieceHandle;

#define SEGMENT_FILE_EXTENSION ".aria2"

class SegmentEntry {
public:
  int32_t cuid;
  SegmentHandle segment;
public:
  SegmentEntry(int32_t cuid, const SegmentHandle& segment);

  ~SegmentEntry();
};

typedef SharedHandle<SegmentEntry> SegmentEntryHandle;
typedef deque<SegmentEntryHandle> SegmentEntries;

/**
 * This class holds the download progress of the one download entry.
 */
class SegmentMan {
private:
  const Option* _option;

  const Logger* logger;

  DownloadContextHandle _downloadContext;

  PieceStorageHandle _pieceStorage;

  SegmentEntries usedSegmentEntries;

  PeerStats peerStats;

  SegmentHandle checkoutSegment(int32_t cuid, const PieceHandle& piece);

  SegmentEntryHandle findSlowerSegmentEntry(const PeerStatHandle& peerStat) const;

  SegmentEntryHandle getSegmentEntryByIndex(int32_t index);
  
  SegmentEntryHandle getSegmentEntryByCuid(int32_t cuid);

  SegmentEntries::iterator getSegmentEntryIteratorByCuid(int32_t cuid);

public:
  SegmentMan(const Option* option,
	     const DownloadContextHandle& downloadContext = 0,
	     const PieceStorageHandle& pieceStorage = 0);

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
   * Returns a vacant segment.
   * If there is no vacant segment, then returns a segment instance whose
   * isNull call is true.
   */
  Segments getInFlightSegment(int32_t cuid);
  SegmentHandle getSegment(int32_t cuid);
  /**
   * Returns a segment whose index is index. 
   * If it has already assigned
   * to another cuid or has been downloaded, then returns a segment instance
   * whose isNull call is true.
   */
  SegmentHandle getSegment(int32_t cuid, int32_t index);
  /**
   * Updates download status.
   */
  //bool updateSegment(int cuid, const Segment& segment);
  /**
   * Cancels all the segment which the command having given cuid
   * uses.
   */
  void cancelSegment(int32_t cuid);
  /**
   * Tells SegmentMan that the segment has been downloaded successfully.
   */
  bool completeSegment(int32_t cuid, const SegmentHandle& segment);

  /**
   * Injects PieceStorage.
   */
  void setPieceStorage(const PieceStorageHandle& pieceStorage);

  /**
   * Injects DownloadContext.
   */
  void setDownloadContext(const DownloadContextHandle& downloadContext);

  /**
   * Returns true if the segment whose index is index has been downloaded.
   */
  bool hasSegment(int32_t index) const;
  /**
   * Returns the length of bytes downloaded.
   */
  int64_t getDownloadLength() const;

  /**
   * Registers given peerStat if it has not been registerd.
   * Otherwise does nothing.
   */
  void registerPeerStat(const PeerStatHandle& peerStat);

  /**
   * Returns peerStat whose cuid is given cuid. If it is not found, returns
   * 0.
   */
  PeerStatHandle getPeerStat(int32_t cuid) const;

  /**
   * Returns current download speed in bytes per sec. 
   */
  int32_t calculateDownloadSpeed() const;

  int32_t countFreePieceFrom(int32_t index) const;
};

typedef SharedHandle<SegmentMan> SegmentManHandle;
#endif // _D_SEGMENT_MAN_H_
