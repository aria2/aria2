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
#include "BtUnchokeMessage.h"
#include "Peer.h"
#include "SocketBuffer.h"

namespace aria2 {

const char BtUnchokeMessage::NAME[] = "unchoke";

BtUnchokeMessage::BtUnchokeMessage() : ZeroBtMessage(ID, NAME) {}

std::unique_ptr<BtUnchokeMessage>
BtUnchokeMessage::create(const unsigned char* data, size_t dataLength)
{
  return ZeroBtMessage::create<BtUnchokeMessage>(data, dataLength);
}

void BtUnchokeMessage::doReceivedAction()
{
  if (isMetadataGetMode()) {
    return;
  }
  getPeer()->peerChoking(false);
}

bool BtUnchokeMessage::sendPredicate() const { return getPeer()->amChoking(); }

namespace {
struct ThisProgressUpdate : public ProgressUpdate {
  ThisProgressUpdate(std::shared_ptr<Peer> peer) : peer(std::move(peer)) {}
  virtual void update(size_t length, bool complete) CXX11_OVERRIDE
  {
    if (complete) {
      peer->amChoking(false);
    }
  }
  std::shared_ptr<Peer> peer;
};
} // namespace

std::unique_ptr<ProgressUpdate> BtUnchokeMessage::getProgressUpdate()
{
  return make_unique<ThisProgressUpdate>(getPeer());
}

} // namespace aria2
