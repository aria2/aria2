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
#ifndef D_DEFAULT_PEER_STORAGE_H
#define D_DEFAULT_PEER_STORAGE_H

#include "PeerStorage.h"

#include <string>
#include <map>

#include "TimerA2.h"

namespace aria2 {

class BtRuntime;
class BtSeederStateChoke;
class BtLeecherStateChoke;
class PieceStorage;

class DefaultPeerStorage : public PeerStorage {
private:
  std::shared_ptr<BtRuntime> btRuntime_;
  std::shared_ptr<PieceStorage> pieceStorage_;
  size_t maxPeerListSize_;

  // This contains ip address and port pair and is used to ensure that
  // no duplicate peers are stored.
  std::set<std::pair<std::string, uint16_t>> uniqPeers_;
  // Unused (not connected) peers, sorted by last added.
  std::deque<std::shared_ptr<Peer>> unusedPeers_;
  // The set of used peers. Some of them are not connected yet. To
  // know it is connected or not, call Peer::isActive().
  PeerSet usedPeers_;

  std::deque<std::shared_ptr<Peer>> droppedPeers_;

  std::unique_ptr<BtSeederStateChoke> seederStateChoke_;
  std::unique_ptr<BtLeecherStateChoke> leecherStateChoke_;

  Timer lastTransferStatMapUpdated_;

  std::map<std::string, Timer> badPeers_;
  Timer lastBadPeerCleaned_;

  bool isPeerAlreadyAdded(const std::shared_ptr<Peer>& peer);
  void addUniqPeer(const std::shared_ptr<Peer>& peer);

  void addDroppedPeer(const std::shared_ptr<Peer>& peer);

public:
  DefaultPeerStorage();

  virtual ~DefaultPeerStorage();

  // TODO We need addAndCheckoutPeer for incoming peers
  virtual bool addPeer(const std::shared_ptr<Peer>& peer) CXX11_OVERRIDE;

  virtual size_t countAllPeer() const CXX11_OVERRIDE;

  std::shared_ptr<Peer> getPeer(const std::string& ipaddr, uint16_t port) const;

  virtual void
  addPeer(const std::vector<std::shared_ptr<Peer>>& peers) CXX11_OVERRIDE;

  std::shared_ptr<Peer> addAndCheckoutPeer(const std::shared_ptr<Peer>& peer,
                                           cuid_t cuid) CXX11_OVERRIDE;

  const std::deque<std::shared_ptr<Peer>>& getUnusedPeers();

  virtual const PeerSet& getUsedPeers() CXX11_OVERRIDE;

  virtual const std::deque<std::shared_ptr<Peer>>&
  getDroppedPeers() CXX11_OVERRIDE;

  virtual bool isPeerAvailable() CXX11_OVERRIDE;

  virtual bool isBadPeer(const std::string& ipaddr) CXX11_OVERRIDE;

  virtual void addBadPeer(const std::string& ipaddr) CXX11_OVERRIDE;

  virtual std::shared_ptr<Peer> checkoutPeer(cuid_t cuid) CXX11_OVERRIDE;

  virtual void returnPeer(const std::shared_ptr<Peer>& peer) CXX11_OVERRIDE;

  virtual bool chokeRoundIntervalElapsed() CXX11_OVERRIDE;

  virtual void executeChoke() CXX11_OVERRIDE;

  void deleteUnusedPeer(size_t delSize);

  void onErasingPeer(const std::shared_ptr<Peer>& peer);

  void onReturningPeer(const std::shared_ptr<Peer>& peer);

  void setPieceStorage(const std::shared_ptr<PieceStorage>& pieceStorage);

  void setBtRuntime(const std::shared_ptr<BtRuntime>& btRuntime);

  void setMaxPeerListSize(size_t maxPeerListSize)
  {
    maxPeerListSize_ = maxPeerListSize;
  }
};

} // namespace aria2

#endif // D_DEFAULT_PEER_STORAGE_H
