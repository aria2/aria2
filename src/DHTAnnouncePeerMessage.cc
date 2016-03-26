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
#include "DHTAnnouncePeerReplyMessage.h"
#include "DlAbortEx.h"
#include "BtConstants.h"
#include "fmt.h"
#include "a2functional.h"

namespace aria2 {

const std::string DHTAnnouncePeerMessage::ANNOUNCE_PEER("announce_peer");

const std::string DHTAnnouncePeerMessage::INFO_HASH("info_hash");

const std::string DHTAnnouncePeerMessage::PORT("port");

const std::string DHTAnnouncePeerMessage::TOKEN("token");

DHTAnnouncePeerMessage::DHTAnnouncePeerMessage(
    const std::shared_ptr<DHTNode>& localNode,
    const std::shared_ptr<DHTNode>& remoteNode, const unsigned char* infoHash,
    uint16_t tcpPort, const std::string& token,
    const std::string& transactionID)
    : DHTQueryMessage{localNode, remoteNode, transactionID},
      token_{token},
      tcpPort_{tcpPort},
      peerAnnounceStorage_{nullptr},
      tokenTracker_{nullptr}
{
  memcpy(infoHash_, infoHash, DHT_ID_LENGTH);
}

void DHTAnnouncePeerMessage::doReceivedAction()
{
  peerAnnounceStorage_->addPeerAnnounce(
      infoHash_, getRemoteNode()->getIPAddress(), tcpPort_);

  getMessageDispatcher()->addMessageToQueue(
      getMessageFactory()->createAnnouncePeerReplyMessage(getRemoteNode(),
                                                          getTransactionID()));
}

std::unique_ptr<Dict> DHTAnnouncePeerMessage::getArgument()
{
  auto aDict = Dict::g();
  aDict->put(DHTMessage::ID, String::g(getLocalNode()->getID(), DHT_ID_LENGTH));
  aDict->put(INFO_HASH, String::g(infoHash_, DHT_ID_LENGTH));
  aDict->put(PORT, Integer::g(tcpPort_));
  aDict->put(TOKEN, token_);
  return aDict;
}

const std::string& DHTAnnouncePeerMessage::getMessageType() const
{
  return ANNOUNCE_PEER;
}

void DHTAnnouncePeerMessage::validate() const
{
  if (!tokenTracker_->validateToken(token_, infoHash_,
                                    getRemoteNode()->getIPAddress(),
                                    getRemoteNode()->getPort())) {
    throw DL_ABORT_EX(fmt(
        "Invalid token=%s from %s:%u", util::toHex(token_).c_str(),
        getRemoteNode()->getIPAddress().c_str(), getRemoteNode()->getPort()));
  }
}

void DHTAnnouncePeerMessage::setPeerAnnounceStorage(
    DHTPeerAnnounceStorage* storage)
{
  peerAnnounceStorage_ = storage;
}

void DHTAnnouncePeerMessage::setTokenTracker(DHTTokenTracker* tokenTracker)
{
  tokenTracker_ = tokenTracker;
}

std::string DHTAnnouncePeerMessage::toStringOptional() const
{
  return fmt("token=%s, info_hash=%s, tcpPort=%u", util::toHex(token_).c_str(),
             util::toHex(infoHash_, INFO_HASH_LENGTH).c_str(), tcpPort_);
}

} // namespace aria2
