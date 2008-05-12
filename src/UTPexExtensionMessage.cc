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
#include "UTPexExtensionMessage.h"
#include "Peer.h"
#include "BtContext.h"
#include "Dictionary.h"
#include "Data.h"
#include "BencodeVisitor.h"
#include "Util.h"
#include "PeerMessageUtil.h"
#include "BtRegistry.h"
#include "PeerStorage.h"
#include "CompactPeerListProcessor.h"
#include "MetaFileUtil.h"
#include "DlAbortEx.h"
#include "message.h"
#include "StringFormat.h"

namespace aria2 {

const std::string UTPexExtensionMessage::EXTENSION_NAME = "ut_pex";

UTPexExtensionMessage::UTPexExtensionMessage(uint8_t extensionMessageID):
  _extensionMessageID(extensionMessageID) {}

UTPexExtensionMessage::~UTPexExtensionMessage() {}

std::string UTPexExtensionMessage::getBencodedData()
{
  SharedHandle<Dictionary> d(new Dictionary());
  std::pair<std::string, std::string> freshPeerPair = createCompactPeerListAndFlag(_freshPeers);
  std::pair<std::string, std::string> droppedPeerPair = createCompactPeerListAndFlag(_droppedPeers);
  d->put("added", new Data(freshPeerPair.first));
  d->put("added.f", new Data(freshPeerPair.second));
  d->put("dropped", new Data(droppedPeerPair.first));

  BencodeVisitor v;
  d->accept(&v);
  return v.getBencodedData();
}

std::pair<std::string, std::string> UTPexExtensionMessage::createCompactPeerListAndFlag(const Peers& peers)
{
  std::string addrstring;
  std::string flagstring;
  for(Peers::const_iterator itr = peers.begin(); itr != peers.end(); ++itr) {
    unsigned char compact[6];
    if(PeerMessageUtil::createcompact(compact, (*itr)->ipaddr, (*itr)->port)) {
      addrstring.append(&compact[0], &compact[6]);
      flagstring += (*itr)->isSeeder() ? "2" : "0";
    }
  }
  return std::pair<std::string, std::string>(addrstring, flagstring);
}

std::string UTPexExtensionMessage::toString() const
{
  return "ut_pex added="+Util::uitos(_freshPeers.size())+", dropped="+
    Util::uitos(_droppedPeers.size());
}

void UTPexExtensionMessage::doReceivedAction()
{
  PEER_STORAGE(_btContext)->addPeer(_freshPeers);
}

void UTPexExtensionMessage::addFreshPeer(const PeerHandle& peer)
{
  _freshPeers.push_back(peer);
}

const Peers& UTPexExtensionMessage::getFreshPeers() const
{
  return _freshPeers;
}

void UTPexExtensionMessage::addDroppedPeer(const PeerHandle& peer)
{
  _droppedPeers.push_back(peer);
}

const Peers& UTPexExtensionMessage::getDroppedPeers() const
{
  return _droppedPeers;
}

void UTPexExtensionMessage::setBtContext(const BtContextHandle& btContext)
{
  _btContext = btContext;
}

UTPexExtensionMessageHandle
UTPexExtensionMessage::create(const BtContextHandle& btContext,
			      const unsigned char* data, size_t len)
{
  if(len < 1) {
    throw DlAbortEx(StringFormat(MSG_TOO_SMALL_PAYLOAD_SIZE,
				 EXTENSION_NAME.c_str(), len).str());
  }
  UTPexExtensionMessageHandle msg(new UTPexExtensionMessage(*data));
  SharedHandle<MetaEntry> root(MetaFileUtil::bdecoding(data+1, len-1));
  if(root.isNull()) {
    return msg;
  }
  const Dictionary* d = dynamic_cast<const Dictionary*>(root.get());
  if(d) {
    CompactPeerListProcessor proc;
    const Data* added = dynamic_cast<const Data*>(d->get("added"));
    if(added) {
      proc.extractPeer(msg->_freshPeers, added);
    }
    const Data* dropped = dynamic_cast<const Data*>(d->get("dropped"));
    if(dropped) {
      proc.extractPeer(msg->_droppedPeers, dropped);
    }
  }
  return msg;
}

} // namespace aria2
