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
#include "PeerMessageFactory.h"
#include "PeerInteraction.h"
#include "PeerMessageUtil.h"
#include "ChokeMessage.h"
#include "UnchokeMessage.h"
#include "InterestedMessage.h"
#include "NotInterestedMessage.h"
#include "HaveMessage.h"
#include "BitfieldMessage.h"
#include "RequestMessage.h"
#include "CancelMessage.h"
#include "PieceMessage.h"
#include "HandshakeMessage.h"
#include "KeepAliveMessage.h"
#include "PortMessage.h"
#include "HaveAllMessage.h"
#include "HaveNoneMessage.h"
#include "RejectMessage.h"
#include "AllowedFastMessage.h"
#include "SuggestPieceMessage.h"
#include "RequestSlot.h"
#include "DlAbortEx.h"

PeerMessageFactory::PeerMessageFactory(int cuid,
				       PeerInteraction* peerInteraction,
				       const PeerHandle& peer)
  :cuid(cuid),
   peerInteraction(peerInteraction),
   peer(peer) {}

PeerMessageFactory::~PeerMessageFactory() {}

PeerMessageHandle PeerMessageFactory::createPeerMessage(const char* msg, int msgLength) const {
  PeerMessage* peerMessage;
  if(msgLength == 0) {
    // keep-alive
    peerMessage = new KeepAliveMessage();
  } else {
    int id = PeerMessageUtil::getId(msg);
    switch(id) {
    case ChokeMessage::ID:
      peerMessage = ChokeMessage::create(msg, msgLength);
      break;
    case UnchokeMessage::ID:
      peerMessage = UnchokeMessage::create(msg, msgLength);
      break;
    case InterestedMessage::ID:
      peerMessage = InterestedMessage::create(msg, msgLength);
      break;
    case NotInterestedMessage::ID:
      peerMessage = NotInterestedMessage::create(msg, msgLength);
      break;
    case HaveMessage::ID:
      peerMessage = HaveMessage::create(msg, msgLength);
      ((HaveMessage*)peerMessage)->setPieces(peerInteraction->getTorrentMan()->
					     pieces);
      break;
    case BitfieldMessage::ID:
      peerMessage = BitfieldMessage::create(msg, msgLength);
      ((BitfieldMessage*)peerMessage)->setPieces(peerInteraction->
						 getTorrentMan()->pieces);
      break;
    case RequestMessage::ID:
      peerMessage = RequestMessage::create(msg, msgLength);
      ((RequestMessage*)peerMessage)->setPieces(peerInteraction->
						getTorrentMan()->pieces);
      ((RequestMessage*)peerMessage)->setPieceLength(peerInteraction->getTorrentMan()->getPieceLength(((RequestMessage*)peerMessage)->getIndex()));
      break;
    case CancelMessage::ID:
      peerMessage = CancelMessage::create(msg, msgLength);
      ((CancelMessage*)peerMessage)->setPieces(peerInteraction->getTorrentMan()->pieces);
      ((CancelMessage*)peerMessage)->setPieceLength(peerInteraction->getTorrentMan()->getPieceLength(((CancelMessage*)peerMessage)->getIndex()));
      break;
    case PieceMessage::ID:
      peerMessage = PieceMessage::create(msg, msgLength);
      ((PieceMessage*)peerMessage)->setPieces(peerInteraction->getTorrentMan()->pieces);
      ((PieceMessage*)peerMessage)->setPieceLength(peerInteraction->getTorrentMan()->getPieceLength(((PieceMessage*)peerMessage)->getIndex()));
      break;
    case PortMessage::ID:
      peerMessage = PortMessage::create(msg, msgLength);
      break;
    case HaveAllMessage::ID:
      peerMessage = HaveAllMessage::create(msg, msgLength);
      break;
    case HaveNoneMessage::ID:
      peerMessage = HaveNoneMessage::create(msg, msgLength);
      break;
    case RejectMessage::ID:
      peerMessage = RejectMessage::create(msg, msgLength);
      ((RejectMessage*)peerMessage)->setPieces(peerInteraction->getTorrentMan()->pieces);
      ((RejectMessage*)peerMessage)->setPieceLength(peerInteraction->getTorrentMan()->getPieceLength(((RejectMessage*)peerMessage)->getIndex()));
      break;
    case SuggestPieceMessage::ID:
      peerMessage = SuggestPieceMessage::create(msg, msgLength);
      ((SuggestPieceMessage*)peerMessage)->setPieces(peerInteraction->getTorrentMan()->pieces);
      break;
    case AllowedFastMessage::ID:
      peerMessage = AllowedFastMessage::create(msg, msgLength);
      ((AllowedFastMessage*)peerMessage)->setPieces(peerInteraction->getTorrentMan()->pieces);
      break;
    default:
      throw new DlAbortEx("Invalid message id. id = %d", id);
    }
  }
  PeerMessageHandle handle = PeerMessageHandle(peerMessage);
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle
PeerMessageFactory::createHandshakeMessage(const char* msg, int msgLength) const
{
  PeerMessageHandle handle =
    PeerMessageHandle(HandshakeMessage::create(msg, msgLength));
  
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle
PeerMessageFactory::createHandshakeMessage(const unsigned char* infoHash,
					  const char* peerId) const
{
  PeerMessageHandle handle =
    PeerMessageHandle(new HandshakeMessage(infoHash, peerId));
  setPeerMessageCommonProperty(handle);
  return handle;
}

void
PeerMessageFactory::setPeerMessageCommonProperty(PeerMessageHandle& peerMessage) const
{
  peerMessage->setPeer(peer);
  peerMessage->setCuid(cuid);
  peerMessage->setPeerInteraction(peerInteraction);
}

PeerMessageHandle PeerMessageFactory::createRequestMessage(const Piece& piece,
							   int blockIndex) const {
  PeerMessageHandle handle =
    PeerMessageHandle(new RequestMessage(piece.getIndex(),
					 blockIndex*piece.getBlockLength(),
					 piece.getBlockLength(blockIndex),
					 blockIndex));
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createCancelMessage(int index,
							  int begin,
							  int length) const {
  PeerMessageHandle handle =
    PeerMessageHandle(new CancelMessage(index, begin, length));
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createPieceMessage(int index,
							 int begin,
							 int length) const {
  PeerMessageHandle handle =
    PeerMessageHandle(new PieceMessage(index, begin, length));
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createHaveMessage(int index) const {
  PeerMessageHandle handle =
    PeerMessageHandle(new HaveMessage(index));
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createChokeMessage() const {
  PeerMessageHandle handle =
    PeerMessageHandle(new ChokeMessage());
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createUnchokeMessage() const {
  PeerMessageHandle handle =
    PeerMessageHandle(new UnchokeMessage());
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createInterestedMessage() const {
  PeerMessageHandle handle =
    PeerMessageHandle(new InterestedMessage());
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createNotInterestedMessage() const {
  PeerMessageHandle handle =
    PeerMessageHandle(new NotInterestedMessage());
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createBitfieldMessage() const {
  PeerMessageHandle handle =
    PeerMessageHandle(new BitfieldMessage(peerInteraction->getTorrentMan()->
					  getBitfield(),
					  peerInteraction->getTorrentMan()->
					  getBitfieldLength()));
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createKeepAliveMessage() const {
  PeerMessageHandle handle =
    PeerMessageHandle(new KeepAliveMessage());
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createHaveAllMessage() const {
  PeerMessageHandle handle =
    PeerMessageHandle(new HaveAllMessage());
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createHaveNoneMessage() const {
  PeerMessageHandle handle =
    PeerMessageHandle(new HaveNoneMessage());
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createRejectMessage(int index,
							  int begin,
							  int length) const {
  PeerMessageHandle handle =
    PeerMessageHandle(new RejectMessage(index, begin, length));
  setPeerMessageCommonProperty(handle);
  return handle;
}

PeerMessageHandle PeerMessageFactory::createAllowedFastMessage(int index) const {
  PeerMessageHandle handle =
    PeerMessageHandle(new AllowedFastMessage(index));
  setPeerMessageCommonProperty(handle);
  return handle;
}
