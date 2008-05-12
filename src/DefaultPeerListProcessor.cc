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
#include "DefaultPeerListProcessor.h"
#include "Peer.h"
#include "List.h"
#include "Dictionary.h"
#include "Data.h"

namespace aria2 {

DefaultPeerListProcessor::DefaultPeerListProcessor() {}

DefaultPeerListProcessor::~DefaultPeerListProcessor() {}

bool DefaultPeerListProcessor::canHandle(const MetaEntry* peersEntry) const {
  const List* peersList = dynamic_cast<const List*>(peersEntry);
  return peersList != 0;
}

void DefaultPeerListProcessor::extractPeer
(std::deque<SharedHandle<Peer> >& peers, const MetaEntry* peersEntry)
{
  const List* peersList = dynamic_cast<const List*>(peersEntry);
  if(!peersList) {
    return;
  }
  const std::deque<MetaEntry*>& metaList = peersList->getList();
  for(std::deque<MetaEntry*>::const_iterator itr = metaList.begin();
      itr != metaList.end(); itr++) {
    const Dictionary* peerDic = dynamic_cast<const Dictionary*>(*itr);
    if(!peerDic) {
      break;
    }
    const Data* ip = dynamic_cast<const Data*>(peerDic->get("ip"));
    const Data* port = dynamic_cast<const Data*>(peerDic->get("port"));
    if(!ip || !port || !port->isNumber()) {
      continue;
    }
    PeerHandle peer(new Peer(ip->toString(), port->toInt()));
    peers.push_back(peer);
  }
}

} // namespace aria2
