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
#ifndef D_BT_ANNOUNCE_H
#define D_BT_ANNOUNCE_H

#include "common.h"

#include <string>

#include "SharedHandle.h"
#include "a2time.h"

namespace aria2 {

class BtAnnounce {
public:
  virtual ~BtAnnounce() {}

  /**
   * Returns true if announce is required.
   * Otherwise returns false.
   *
   * There are 4 announce timings:
   * 1) started: when a download just started.
   * 2) stopped: when the client quits.
   * 3) completed: when a download just completed.
   * 4) When a certain amount of time, aka announce interval, specified by
   *    a tracker, is elapsed.
   */
  virtual bool isAnnounceReady() = 0;

  /**
   * Returns announe URL with all necessary parameters included.
   */
  virtual std::string getAnnounceUrl() = 0;

  /**
   * Tells that the announce process has just started.
   */
  virtual void announceStart() = 0;

  /**
   * Tells that the announce succeeded.
   */
  virtual void announceSuccess() = 0;

  /**
   * Tells that the announce failed.
   */
  virtual void announceFailure() = 0;

  /**
   * Returns true if all announce attempt failed.
   */
  virtual bool isAllAnnounceFailed() = 0;

  /**
   * Resets announce status.
   */
  virtual void resetAnnounce() = 0;

  /**
   * Processes the repsponse from the tracker.
   */
  virtual void processAnnounceResponse(const unsigned char* trackerResponse,
                                       size_t trackerResponseLength) = 0;

  /**
   * Returns true if no more announce is needed.
   */
  virtual bool noMoreAnnounce() = 0;

  /**
   * Shuffles the URLs in each announce tier.
   */
  virtual void shuffleAnnounce() = 0;

  virtual void overrideMinInterval(time_t interval) = 0;

  virtual void setTcpPort(uint16_t port) = 0;

  static const std::string FAILURE_REASON;

  static const std::string WARNING_MESSAGE;

  static const std::string TRACKER_ID;

  static const std::string INTERVAL;

  static const std::string MIN_INTERVAL;

  static const std::string COMPLETE;

  static const std::string INCOMPLETE;

  static const std::string PEERS;

  static const std::string PEERS6;

  static const time_t DEFAULT_ANNOUNCE_INTERVAL = 120;
};

typedef SharedHandle<BtAnnounce> BtAnnounceHandle;

} // namespace aria2

#endif // D_BT_ANNOUNCE_H
