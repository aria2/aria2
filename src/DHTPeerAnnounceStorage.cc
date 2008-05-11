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
#include "DHTPeerAnnounceStorage.h"
#include "DHTPeerAnnounceEntry.h"
#include "Peer.h"
#include "BtContext.h"
#include "DHTConstants.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTask.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Util.h"
#include <cstring>
#include <algorithm>

namespace aria2 {

DHTPeerAnnounceStorage::DHTPeerAnnounceStorage():
  _logger(LogFactory::getInstance()) {}

DHTPeerAnnounceStorage::~DHTPeerAnnounceStorage() {}

class FindPeerAnnounceEntry {
private:
  unsigned char _infoHash[DHT_ID_LENGTH];
public:
  FindPeerAnnounceEntry(const unsigned char* infoHash)
  {
    memcpy(_infoHash, infoHash, DHT_ID_LENGTH);
  }

  bool operator()(const SharedHandle<DHTPeerAnnounceEntry>& entry) const
  {
    return memcmp(_infoHash, entry->getInfoHash(), DHT_ID_LENGTH) == 0;
  }
};

SharedHandle<DHTPeerAnnounceEntry>
DHTPeerAnnounceStorage::getPeerAnnounceEntry(const unsigned char* infoHash)
{
  std::deque<SharedHandle<DHTPeerAnnounceEntry> >::iterator i = 
    std::find_if(_entries.begin(), _entries.end(), FindPeerAnnounceEntry(infoHash));
  SharedHandle<DHTPeerAnnounceEntry> entry;
  if(i == _entries.end()) {
    entry.reset(new DHTPeerAnnounceEntry(infoHash));
    _entries.push_back(entry);
  } else {
    entry = *i;
  }
  return entry;
}

void
DHTPeerAnnounceStorage::addPeerAnnounce(const unsigned char* infoHash,
					const std::string& ipaddr, uint16_t port)
{
  _logger->debug("Adding %s:%u to peer announce list: infoHash=%s",
		 ipaddr.c_str(), port, Util::toHex(infoHash, DHT_ID_LENGTH).c_str());
  getPeerAnnounceEntry(infoHash)->addPeerAddrEntry(PeerAddrEntry(ipaddr, port));
}

// add peer announce as localhost downloading the content
void DHTPeerAnnounceStorage::addPeerAnnounce(const BtContextHandle& ctx)
{
  _logger->debug("Adding localhost to peer announce list: infoHash=%s",
		 ctx->getInfoHashAsString().c_str());
  getPeerAnnounceEntry(ctx->getInfoHash())->setBtContext(ctx);
}

void DHTPeerAnnounceStorage::removePeerAnnounce(const BtContextHandle& ctx)
{
  std::deque<SharedHandle<DHTPeerAnnounceEntry> >::iterator i = 
    std::find_if(_entries.begin(), _entries.end(), FindPeerAnnounceEntry(ctx->getInfoHash()));
  if(i != _entries.end()) {
    (*i)->setBtContext(SharedHandle<BtContext>());
    if((*i)->empty()) {
      _entries.erase(i);
    }
  }
}

bool DHTPeerAnnounceStorage::contains(const unsigned char* infoHash) const
{
  return 
    std::find_if(_entries.begin(), _entries.end(), FindPeerAnnounceEntry(infoHash)) != _entries.end();
}

void DHTPeerAnnounceStorage::getPeers(std::deque<SharedHandle<Peer> >& peers,
				      const unsigned char* infoHash)
{
  std::deque<SharedHandle<DHTPeerAnnounceEntry> >::iterator i = 
    std::find_if(_entries.begin(), _entries.end(), FindPeerAnnounceEntry(infoHash));
  if(i != _entries.end() && !(*i)->empty()) {
    (*i)->getPeers(peers);
  }
}

void DHTPeerAnnounceStorage::handleTimeout()
{
  _logger->debug("Now purge peer announces which are timed out.");
  size_t numPeerAddr = 0;
  for(std::deque<SharedHandle<DHTPeerAnnounceEntry> >::iterator i = _entries.begin(); i != _entries.end();) {
    (*i)->removeStalePeerAddrEntry(DHT_PEER_ANNOUNCE_PURGE_INTERVAL);
    if((*i)->empty()) {
      _logger->debug("1 entry purged: infoHash=%s",
		     Util::toHex((*i)->getInfoHash(), DHT_ID_LENGTH).c_str());
      i = _entries.erase(i);
    } else {
      numPeerAddr += (*i)->countPeerAddrEntry();
      ++i;
    }
  }
  _logger->debug("Currently %zu peer announce entries, %zu PeerAddr entries",
		 _entries.size(), numPeerAddr);
}

void DHTPeerAnnounceStorage::announcePeer()
{
  _logger->debug("Now announcing peer.");
  for(std::deque<SharedHandle<DHTPeerAnnounceEntry> >::iterator i = _entries.begin(); i != _entries.end(); ++i) {
    if((*i)->getLastUpdated().elapsed(DHT_PEER_ANNOUNCE_INTERVAL)) {
      (*i)->notifyUpdate();
      SharedHandle<DHTTask> task = _taskFactory->createPeerAnnounceTask((*i)->getInfoHash());
      _taskQueue->addPeriodicTask2(task);
      _logger->debug("Added 1 peer announce: infoHash=%s",
		     Util::toHex((*i)->getInfoHash(), DHT_ID_LENGTH).c_str());
    }
  }
}

void DHTPeerAnnounceStorage::setTaskQueue(const SharedHandle<DHTTaskQueue>& taskQueue)
{
  _taskQueue = taskQueue;
}

void DHTPeerAnnounceStorage::setTaskFactory(const SharedHandle<DHTTaskFactory>& taskFactory)
{
  _taskFactory = taskFactory;
}

} // namespace aria2
