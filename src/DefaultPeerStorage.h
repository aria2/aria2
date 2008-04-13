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
#ifndef _D_DEFAULT_PEER_STORAGE_H_
#define _D_DEFAULT_PEER_STORAGE_H_

#include "PeerStorage.h"

namespace aria2 {

#define MAX_PEER_LIST_SIZE 60
#define MAX_PEER_ERROR 5

class BtContext;
class Option;
class Logger;
class BtRuntime;
class BtSeederStateChoke;
class BtLeecherStateChoke;

class DefaultPeerStorage : public PeerStorage {
private:
  SharedHandle<BtContext> btContext;
  const Option* option;
  std::deque<SharedHandle<Peer> > peers;
  size_t maxPeerListSize;
  Logger* logger;
  SharedHandle<BtRuntime> btRuntime;
  uint64_t removedPeerSessionDownloadLength;
  uint64_t removedPeerSessionUploadLength;

  BtSeederStateChoke* _seederStateChoke;
  BtLeecherStateChoke* _leecherStateChoke;

  bool isPeerAlreadyAdded(const SharedHandle<Peer>& peer);
public:
  DefaultPeerStorage(const SharedHandle<BtContext>& btContext,
		     const Option* option);

  virtual ~DefaultPeerStorage();

  void setBtRuntime(const SharedHandle<BtRuntime>& btRuntime) {
    this->btRuntime = btRuntime;
  }

  SharedHandle<BtRuntime> getBtRuntime() const { return btRuntime; }

  virtual bool addPeer(const SharedHandle<Peer>& peer);

  size_t countPeer() const;

  virtual SharedHandle<Peer> getUnusedPeer();

  SharedHandle<Peer> getPeer(const std::string& ipaddr, uint16_t port) const;

  virtual void addPeer(const std::deque<SharedHandle<Peer> >& peers);

  virtual const std::deque<SharedHandle<Peer> >& getPeers();

  virtual bool isPeerAvailable();

  virtual std::deque<SharedHandle<Peer> > getActivePeers();

  virtual TransferStat calculateStat();

  virtual void returnPeer(const SharedHandle<Peer>& peer);

  virtual bool chokeRoundIntervalElapsed();

  virtual void executeChoke();

  void setMaxPeerListSize(size_t size) { this->maxPeerListSize = size; }
 
  size_t getMaxPeerListSize() const { return maxPeerListSize; }

  void deleteUnusedPeer(size_t delSize);
  
  void onErasingPeer(const SharedHandle<Peer>& peer);

  void onReturningPeer(const SharedHandle<Peer>& peer);

};

} // namespace aria2

#endif // _D_DEFAULT_PEER_STORAGE_H_
