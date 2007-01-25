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
#ifndef _D_PEER_STORAGE_H_
#define _D_PEER_STORAGE_H_

#include "common.h"
#include "Peer.h"

class TransferStat {
public:
  int32_t downloadSpeed;
  int32_t uploadSpeed;
  int64_t sessionDownloadLength;
  int64_t sessionUploadLength;
public:
  TransferStat():downloadSpeed(0), uploadSpeed(0),
		 sessionDownloadLength(0), sessionUploadLength(0) {}

  int32_t getDownloadSpeed() const {
    return downloadSpeed;
  }

  void setDownloadSpeed(int32_t s) { downloadSpeed = s; }

  int32_t getUploadSpeed() const {
    return uploadSpeed;
  }

  void setUploadSpeed(int32_t s) { uploadSpeed = s; }

  /**
   * Returns the number of bytes downloaded since the program started.
   * This is not the total number of bytes downloaded.
   */
  int64_t getSessionDownloadLength() const {
    return sessionDownloadLength;
  }

  void setSessionDownloadLength(int64_t s) { sessionDownloadLength = s; }

  /**
   * Returns the number of bytes uploaded since the program started.
   * This is not the total number of bytes uploaded.
   */
  int64_t getSessionUploadLength() const {
    return sessionUploadLength;
  }

  void setSessionUploadLength(int64_t s) { sessionUploadLength = s; }
};

class PeerStorage {
public:
  virtual ~PeerStorage() {}

  /**
   * Adds new peer to internal peer list.
   * If the peer is added successfully, returns true. Otherwise returns false.
   */
  virtual bool addPeer(const PeerHandle& peer) = 0;

  /**
   * Adds all peers in peers to internal peer list.
   */
  virtual void addPeer(const Peers& peers) = 0;

  /**
   * Returns internal peer list.
   */
  virtual const Peers& getPeers() = 0;

  /**
   * Returns one of the unused peers.
   */
  virtual PeerHandle getUnusedPeer() = 0;

  /**
   * Returns true if at least one unused peer exists.
   * Otherwise returns false.
   */
  virtual bool isPeerAvailable() = 0;
  
  /**
   * Returns the list of peers which are currently connected from localhost.
   */
  virtual Peers getActivePeers() = 0;

  /**
   * Calculates current download/upload statistics.
   */
  virtual TransferStat calculateStat() = 0;
};

typedef SharedHandle<PeerStorage> PeerStorageHandle;

#endif // _D_PEER_STORAGE_H_
