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
#include "DHTAnnouncePeerMessage.h"
#include "DHTNode.h"
#include "Data.h"
#include "Dictionary.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "Util.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTTokenTracker.h"
#include "DlAbortEx.h"
#include "BtConstants.h"
#include "StringFormat.h"
#include <cstring>

namespace aria2 {

const std::string DHTAnnouncePeerMessage::ANNOUNCE_PEER("announce_peer");

const std::string DHTAnnouncePeerMessage::INFO_HASH("info_hash");

const std::string DHTAnnouncePeerMessage::PORT("port");

const std::string DHTAnnouncePeerMessage::TOKEN("token");

DHTAnnouncePeerMessage::DHTAnnouncePeerMessage(const SharedHandle<DHTNode>& localNode,
					       const SharedHandle<DHTNode>& remoteNode,
					       const unsigned char* infoHash,
					       uint16_t tcpPort,
					       const std::string& token,
					       const std::string& transactionID):
  DHTQueryMessage(localNode, remoteNode, transactionID),
  _token(token),
  _tcpPort(tcpPort)
{
  memcpy(_infoHash, infoHash, DHT_ID_LENGTH);
}

DHTAnnouncePeerMessage::~DHTAnnouncePeerMessage() {}

void DHTAnnouncePeerMessage::doReceivedAction()
{
  _peerAnnounceStorage->addPeerAnnounce(_infoHash, _remoteNode->getIPAddress(),
					_tcpPort);

  SharedHandle<DHTMessage> reply =
    _factory->createAnnouncePeerReplyMessage(_remoteNode, _transactionID);
  _dispatcher->addMessageToQueue(reply);
}

Dictionary* DHTAnnouncePeerMessage::getArgument()
{
  Dictionary* a = new Dictionary();
  a->put(DHTMessage::ID, new Data(reinterpret_cast<const char*>(_localNode->getID()),
			DHT_ID_LENGTH));
  a->put(INFO_HASH, new Data(reinterpret_cast<const char*>(_infoHash),
			       DHT_ID_LENGTH));
  a->put(PORT, new Data(Util::uitos(_tcpPort), true));
  a->put(TOKEN, new Data(_token));
  
  return a;
}

std::string DHTAnnouncePeerMessage::getMessageType() const
{
  return ANNOUNCE_PEER;
}

void DHTAnnouncePeerMessage::validate() const
{
  if(!_tokenTracker->validateToken(_token, _infoHash,
				   _remoteNode->getIPAddress(),
				   _remoteNode->getPort())) {
    throw DlAbortEx
      (StringFormat("Invalid token=%s from %s:%u",
		    Util::toHex(_token).c_str(),
		    _remoteNode->getIPAddress().c_str(),
		    _remoteNode->getPort()).str());
  }
}

void DHTAnnouncePeerMessage::setPeerAnnounceStorage(const WeakHandle<DHTPeerAnnounceStorage>& storage)
{
  _peerAnnounceStorage = storage;
}

void DHTAnnouncePeerMessage::setTokenTracker(const WeakHandle<DHTTokenTracker>& tokenTracker)
{
  _tokenTracker = tokenTracker;
}

std::string DHTAnnouncePeerMessage::toStringOptional() const
{
  return "token="+Util::toHex(_token)+
    ", info_hash="+Util::toHex(_infoHash, INFO_HASH_LENGTH)+
    ", tcpPort="+Util::uitos(_tcpPort);
}

} // namespace aria2
