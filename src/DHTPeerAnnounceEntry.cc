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
#include "DHTPeerAnnounceEntry.h"

#include <cstring>
#include <algorithm>

#include "Peer.h"
#include "wallclock.h"

namespace aria2 {

DHTPeerAnnounceEntry::DHTPeerAnnounceEntry(const unsigned char* infoHash)
{
  memcpy(infoHash_, infoHash, DHT_ID_LENGTH);
}

DHTPeerAnnounceEntry::~DHTPeerAnnounceEntry() {}

void DHTPeerAnnounceEntry::addPeerAddrEntry(const PeerAddrEntry& entry)
{
  std::vector<PeerAddrEntry>::iterator i =
    std::find(peerAddrEntries_.begin(), peerAddrEntries_.end(), entry);
  if(i == peerAddrEntries_.end()) {
    peerAddrEntries_.push_back(entry);
  } else {
    (*i).notifyUpdate();
  }
  notifyUpdate();
}

size_t DHTPeerAnnounceEntry::countPeerAddrEntry() const
{
  return peerAddrEntries_.size();
}

namespace {
class FindStaleEntry {
private:
  time_t timeout_;
public:
  FindStaleEntry(time_t timeout):timeout_(timeout) {}

  bool operator()(const PeerAddrEntry& entry) const
  {
    if(entry.getLastUpdated().difference(global::wallclock()) >= timeout_) {
      return true;
    } else {
      return false;
    }
  }
};
} // namespace

void DHTPeerAnnounceEntry::removeStalePeerAddrEntry(time_t timeout)
{
  peerAddrEntries_.erase(std::remove_if(peerAddrEntries_.begin(), peerAddrEntries_.end(),
                                        FindStaleEntry(timeout)), peerAddrEntries_.end());
}

bool DHTPeerAnnounceEntry::empty() const
{
  return peerAddrEntries_.empty();
}

void DHTPeerAnnounceEntry::getPeers
(std::vector<SharedHandle<Peer> >& peers) const
{
  for(std::vector<PeerAddrEntry>::const_iterator i = peerAddrEntries_.begin(),
        eoi = peerAddrEntries_.end(); i != eoi; ++i) {
    SharedHandle<Peer> peer(new Peer((*i).getIPAddress(), (*i).getPort()));
    peers.push_back(peer);
  }
}

void DHTPeerAnnounceEntry::notifyUpdate()
{
  lastUpdated_ = global::wallclock();
}

} // namespace aria2
