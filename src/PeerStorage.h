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
#ifndef D_PEER_STORAGE_H
#define D_PEER_STORAGE_H

#include "common.h"

#include <deque>
#include <vector>

#include "SharedHandle.h"
#include "TransferStat.h"

namespace aria2 {

class Peer;

class PeerStorage {
public:
  virtual ~PeerStorage() {}

  /**
   * Adds new peer to the internal peer list.
   * If the peer is added successfully, returns true. Otherwise returns false.
   */
  virtual bool addPeer(const SharedHandle<Peer>& peer) = 0;

  /**
   * Adds all peers in peers to internal peer list.
   */
  virtual void addPeer(const std::vector<SharedHandle<Peer> >& peers) = 0;

  /**
   * Returns internal peer list.
   */
  virtual const std::deque<SharedHandle<Peer> >& getPeers() = 0;


  /**
   * Returns the number of peers.
   */
  virtual size_t countPeer() const = 0;

  /**
   * Returns internal dropped peer list.
   */
  virtual const std::deque<SharedHandle<Peer> >& getDroppedPeers() = 0;

  /**
   * Returns one of the unused peers.
   */
  virtual SharedHandle<Peer> getUnusedPeer() = 0;

  /**
   * Returns true if at least one unused peer exists.
   * Otherwise returns false.
   */
  virtual bool isPeerAvailable() = 0;
  
  /**
   * Returns the list of peers which are currently connected from localhost.
   */
  virtual void getActivePeers(std::vector<SharedHandle<Peer> >& peers) = 0;

  /**
   * Calculates current download/upload statistics.
   */
  virtual TransferStat calculateStat() = 0;

  virtual void updateTransferStatFor(const SharedHandle<Peer>& peer) = 0;

  virtual TransferStat getTransferStatFor(const SharedHandle<Peer>& peer) = 0;

  /**
   * Returns true if peer with ipaddr should be ignored because, for
   * example, it sends bad data.
   */
  virtual bool isBadPeer(const std::string& ipaddr) = 0;

  /**
   * Adds peer with ipaddr in bad peer set.
   */
  virtual void addBadPeer(const std::string& ipaddr) = 0;

  /**
   * Tells PeerStorage object that peer is no longer used in the session.
   */
  virtual void returnPeer(const SharedHandle<Peer>& peer) = 0;

  virtual bool chokeRoundIntervalElapsed() = 0;

  virtual void executeChoke() = 0;
};

typedef SharedHandle<PeerStorage> PeerStorageHandle;

} // namespace aria2

#endif // D_PEER_STORAGE_H
