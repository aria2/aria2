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
#include "AbstractBtMessage.h"
#include "Peer.h"
#include "PieceStorage.h"
#include "BtMessageValidator.h"

namespace aria2 {

AbstractBtMessage::AbstractBtMessage(uint8_t id, const char* name)
    : BtMessage(id),
      invalidate_(false),
      uploading_(false),
      cuid_(0),
      name_(name),
      pieceStorage_(nullptr),
      dispatcher_(nullptr),
      messageFactory_(nullptr),
      requestFactory_(nullptr),
      peerConnection_(nullptr),
      metadataGetMode_(false)
{
}

AbstractBtMessage::~AbstractBtMessage() = default;

void AbstractBtMessage::setPeer(const std::shared_ptr<Peer>& peer)
{
  peer_ = peer;
}

void AbstractBtMessage::validate()
{
  if (validator_) {
    validator_->validate();
  }
}

void AbstractBtMessage::setBtMessageValidator(
    std::unique_ptr<BtMessageValidator> validator)
{
  validator_ = std::move(validator);
}

void AbstractBtMessage::setPieceStorage(PieceStorage* pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void AbstractBtMessage::setBtMessageDispatcher(BtMessageDispatcher* dispatcher)
{
  dispatcher_ = dispatcher;
}

void AbstractBtMessage::setPeerConnection(PeerConnection* peerConnection)
{
  peerConnection_ = peerConnection;
}

void AbstractBtMessage::setBtMessageFactory(BtMessageFactory* factory)
{
  messageFactory_ = factory;
}

void AbstractBtMessage::setBtRequestFactory(BtRequestFactory* factory)
{
  requestFactory_ = factory;
}

} // namespace aria2
