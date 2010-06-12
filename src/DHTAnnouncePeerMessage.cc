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
#include "DHTAnnouncePeerMessage.h"

#include <cstring>

#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "util.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTTokenTracker.h"
#include "DlAbortEx.h"
#include "BtConstants.h"
#include "StringFormat.h"
#include "bencode.h"
#include "a2functional.h"

namespace aria2 {

const std::string DHTAnnouncePeerMessage::ANNOUNCE_PEER("announce_peer");

const std::string DHTAnnouncePeerMessage::INFO_HASH("info_hash");

const std::string DHTAnnouncePeerMessage::PORT("port");

const std::string DHTAnnouncePeerMessage::TOKEN("token");

DHTAnnouncePeerMessage::DHTAnnouncePeerMessage
(const SharedHandle<DHTNode>& localNode,
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
  _peerAnnounceStorage->addPeerAnnounce
    (_infoHash, getRemoteNode()->getIPAddress(), _tcpPort);

  SharedHandle<DHTMessage> reply =
    getMessageFactory()->createAnnouncePeerReplyMessage
    (getRemoteNode(), getTransactionID());
  getMessageDispatcher()->addMessageToQueue(reply);
}

BDE DHTAnnouncePeerMessage::getArgument()
{
  BDE aDict = BDE::dict();
  aDict[DHTMessage::ID] = BDE(getLocalNode()->getID(), DHT_ID_LENGTH);
  aDict[INFO_HASH] = BDE(_infoHash, DHT_ID_LENGTH);
  aDict[PORT] = _tcpPort;
  aDict[TOKEN] = _token;
  return aDict;
}

const std::string& DHTAnnouncePeerMessage::getMessageType() const
{
  return ANNOUNCE_PEER;
}

void DHTAnnouncePeerMessage::validate() const
{
  if(!_tokenTracker->validateToken(_token, _infoHash,
                                   getRemoteNode()->getIPAddress(),
                                   getRemoteNode()->getPort())) {
    throw DL_ABORT_EX
      (StringFormat("Invalid token=%s from %s:%u",
                    util::toHex(_token).c_str(),
                    getRemoteNode()->getIPAddress().c_str(),
                    getRemoteNode()->getPort()).str());
  }
}

void DHTAnnouncePeerMessage::setPeerAnnounceStorage
(const WeakHandle<DHTPeerAnnounceStorage>& storage)
{
  _peerAnnounceStorage = storage;
}

void DHTAnnouncePeerMessage::setTokenTracker
(const WeakHandle<DHTTokenTracker>& tokenTracker)
{
  _tokenTracker = tokenTracker;
}

std::string DHTAnnouncePeerMessage::toStringOptional() const
{
  return strconcat("token=", util::toHex(_token),
                   ", info_hash=", util::toHex(_infoHash, INFO_HASH_LENGTH),
                   ", tcpPort=", util::uitos(_tcpPort));
}

} // namespace aria2
