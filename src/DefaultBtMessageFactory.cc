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
#include "DefaultBtMessageFactory.h"
#include "DlAbortEx.h"
#include "bittorrent_helper.h"
#include "BtKeepAliveMessage.h"
#include "BtChokeMessage.h"
#include "BtUnchokeMessage.h"
#include "BtInterestedMessage.h"
#include "BtNotInterestedMessage.h"
#include "BtHaveMessage.h"
#include "BtBitfieldMessage.h"
#include "BtBitfieldMessageValidator.h"
#include "RangeBtMessageValidator.h"
#include "IndexBtMessageValidator.h"
#include "BtRequestMessage.h"
#include "BtCancelMessage.h"
#include "BtPieceMessage.h"
#include "BtPieceMessageValidator.h"
#include "BtPortMessage.h"
#include "BtHaveAllMessage.h"
#include "BtHaveNoneMessage.h"
#include "BtRejectMessage.h"
#include "BtSuggestPieceMessage.h"
#include "BtAllowedFastMessage.h"
#include "BtHandshakeMessage.h"
#include "BtHandshakeMessageValidator.h"
#include "BtExtendedMessage.h"
#include "ExtensionMessage.h"
#include "Peer.h"
#include "Piece.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "StringFormat.h"
#include "ExtensionMessageFactory.h"
#include "bittorrent_helper.h"

namespace aria2 {

DefaultBtMessageFactory::DefaultBtMessageFactory():cuid(0),
						   _dhtEnabled(false)
{}

DefaultBtMessageFactory::~DefaultBtMessageFactory() {}

BtMessageHandle
DefaultBtMessageFactory::createBtMessage(const unsigned char* data, size_t dataLength)
{
  AbstractBtMessageHandle msg;
  if(dataLength == 0) {
    // keep-alive
    msg.reset(new BtKeepAliveMessage());
  } else {
    uint8_t id = bittorrent::getId(data);
    switch(id) {
    case BtChokeMessage::ID:
      msg = BtChokeMessage::create(data, dataLength);
      break;
    case BtUnchokeMessage::ID:
      msg = BtUnchokeMessage::create(data, dataLength);
      break;
    case BtInterestedMessage::ID:
      {
	SharedHandle<BtInterestedMessage> m =
	  BtInterestedMessage::create(data, dataLength);
	m->setPeerStorage(_peerStorage);
	msg = m;
      }
      break;
    case BtNotInterestedMessage::ID:
      {
	SharedHandle<BtNotInterestedMessage> m =
	  BtNotInterestedMessage::create(data, dataLength);
	m->setPeerStorage(_peerStorage);
	msg = m;
      }
      break;
    case BtHaveMessage::ID:
      msg = BtHaveMessage::create(data, dataLength);
      {
	SharedHandle<BtMessageValidator> v
	  (new IndexBtMessageValidator(static_cast<BtHaveMessage*>(msg.get()),
				       _downloadContext->getNumPieces()));
	msg->setBtMessageValidator(v);
      }
      break;
    case BtBitfieldMessage::ID:
      msg = BtBitfieldMessage::create(data, dataLength);
      {
	SharedHandle<BtMessageValidator> v
	  (new BtBitfieldMessageValidator
	   (static_cast<BtBitfieldMessage*>(msg.get()),
	    _downloadContext->getNumPieces()));
	msg->setBtMessageValidator(v);
      }
      break;
    case BtRequestMessage::ID: {
      BtRequestMessageHandle temp = BtRequestMessage::create(data, dataLength);
      SharedHandle<BtMessageValidator> validator
	(new RangeBtMessageValidator
	 (temp.get(),
	  _downloadContext->getNumPieces(),
	  _pieceStorage->getPieceLength(temp->getIndex())));
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtCancelMessage::ID: {
      BtCancelMessageHandle temp = BtCancelMessage::create(data, dataLength);
      SharedHandle<BtMessageValidator> validator
	(new RangeBtMessageValidator
	 (temp.get(),
	  _downloadContext->getNumPieces(),
	  _pieceStorage->getPieceLength(temp->getIndex())));
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtPieceMessage::ID: {
      BtPieceMessageHandle temp = BtPieceMessage::create(data, dataLength);
      BtMessageValidatorHandle validator
	(new BtPieceMessageValidator(temp.get(),
				     _downloadContext->getNumPieces(),
				     _pieceStorage->getPieceLength(temp->getIndex())));
      temp->setBtMessageValidator(validator);
      temp->setDownloadContext(_downloadContext);
      msg = temp;
      break;
    }
    case BtHaveAllMessage::ID:
      msg = BtHaveAllMessage::create(data, dataLength);
      break;
    case BtHaveNoneMessage::ID:
      msg = BtHaveNoneMessage::create(data, dataLength);
      break;
    case BtRejectMessage::ID: {
      BtRejectMessageHandle temp = BtRejectMessage::create(data, dataLength);
      SharedHandle<BtMessageValidator> validator
	(new RangeBtMessageValidator
	 (temp.get(),
	  _downloadContext->getNumPieces(),
	  _pieceStorage->getPieceLength(temp->getIndex())));
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtSuggestPieceMessage::ID: {
      BtSuggestPieceMessageHandle temp = BtSuggestPieceMessage::create(data, dataLength);
      SharedHandle<BtMessageValidator> validator
	(new IndexBtMessageValidator(temp.get(),
				     _downloadContext->getNumPieces()));
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtAllowedFastMessage::ID: {
      BtAllowedFastMessageHandle temp = BtAllowedFastMessage::create(data, dataLength);
      SharedHandle<BtMessageValidator> validator
	(new IndexBtMessageValidator(temp.get(),
				     _downloadContext->getNumPieces()));
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtPortMessage::ID: {
      SharedHandle<BtPortMessage> temp = BtPortMessage::create(data, dataLength);
      temp->setLocalNode(_localNode);
      temp->setRoutingTable(_routingTable);
      temp->setTaskQueue(_taskQueue);
      temp->setTaskFactory(_taskFactory);
      msg = temp;
      break;
    }
    case BtExtendedMessage::ID: {
      if(peer->isExtendedMessagingEnabled()) {
	msg = BtExtendedMessage::create(_extensionMessageFactory,
					peer, data, dataLength);
      } else {
	throw DL_ABORT_EX("Received extended message from peer during a session with extended messaging disabled.");
      }
      break;
    }
    default:
      throw DL_ABORT_EX(StringFormat("Invalid message ID. id=%u", id).str());
    }
  }
  setCommonProperty(msg);
  return msg;
}

void DefaultBtMessageFactory::setCommonProperty(const AbstractBtMessageHandle& msg) {
  msg->setCuid(cuid);
  msg->setPeer(peer);
  msg->setPieceStorage(_pieceStorage);
  msg->setBtMessageDispatcher(dispatcher);
  msg->setBtRequestFactory(requestFactory);
  msg->setBtMessageFactory(WeakHandle<BtMessageFactory>(this));
  msg->setPeerConnection(peerConnection);
}

SharedHandle<BtHandshakeMessage>
DefaultBtMessageFactory::createHandshakeMessage(const unsigned char* data, size_t dataLength)
{
  SharedHandle<BtHandshakeMessage> msg = BtHandshakeMessage::create(data, dataLength);
  BtMessageValidatorHandle validator
    (new BtHandshakeMessageValidator
     (msg.get(), bittorrent::getInfoHash(_downloadContext)));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

SharedHandle<BtHandshakeMessage>
DefaultBtMessageFactory::createHandshakeMessage(const unsigned char* infoHash,
						const unsigned char* peerId)
{
  SharedHandle<BtHandshakeMessage> msg(new BtHandshakeMessage(infoHash, peerId));
  BtMessageValidatorHandle validator
    (new BtHandshakeMessageValidator
     (msg.get(), bittorrent::getInfoHash(_downloadContext)));
  msg->setBtMessageValidator(validator);
  msg->setDHTEnabled(_dhtEnabled);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createRequestMessage(const PieceHandle& piece, size_t blockIndex)
{
  BtRequestMessageHandle msg
    (new BtRequestMessage(piece->getIndex(),
			  blockIndex*piece->getBlockLength(),
			  piece->getBlockLength(blockIndex),
			  blockIndex));
  SharedHandle<BtMessageValidator> validator
    (new RangeBtMessageValidator
     (msg.get(),
      _downloadContext->getNumPieces(),
      _pieceStorage->getPieceLength(msg->getIndex())));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createCancelMessage(size_t index, uint32_t begin, size_t length)
{
  BtCancelMessageHandle msg(new BtCancelMessage(index, begin, length));
  SharedHandle<BtMessageValidator> validator
    (new RangeBtMessageValidator
     (msg.get(),
      _downloadContext->getNumPieces(),
      _pieceStorage->getPieceLength(index)));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createPieceMessage(size_t index, uint32_t begin, size_t length)
{
  BtPieceMessageHandle msg(new BtPieceMessage(index, begin, length));
  BtMessageValidatorHandle validator
    (new BtPieceMessageValidator(msg.get(),
				_downloadContext->getNumPieces(),
				_pieceStorage->getPieceLength(index)));
  msg->setBtMessageValidator(validator);
  msg->setDownloadContext(_downloadContext);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createHaveMessage(size_t index)
{
  BtHaveMessageHandle msg(new BtHaveMessage(index));
  SharedHandle<BtMessageValidator> v
    (new IndexBtMessageValidator(msg.get(), _downloadContext->getNumPieces()));
  msg->setBtMessageValidator(v);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createChokeMessage()
{
  BtChokeMessageHandle msg(new BtChokeMessage());
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createUnchokeMessage()
{
  BtUnchokeMessageHandle msg(new BtUnchokeMessage());
  setCommonProperty(msg);
  return msg;
}
 
BtMessageHandle
DefaultBtMessageFactory::createInterestedMessage()
{
  BtInterestedMessageHandle msg(new BtInterestedMessage());
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createNotInterestedMessage()
{
  BtNotInterestedMessageHandle msg(new BtNotInterestedMessage());
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createBitfieldMessage()
{
  BtBitfieldMessageHandle msg
    (new BtBitfieldMessage(_pieceStorage->getBitfield(),
			   _pieceStorage->getBitfieldLength()));
  SharedHandle<BtMessageValidator> v
    (new BtBitfieldMessageValidator(msg.get(),
				    _downloadContext->getNumPieces()));
  msg->setBtMessageValidator(v);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createKeepAliveMessage()
{
  BtKeepAliveMessageHandle msg(new BtKeepAliveMessage());
  setCommonProperty(msg);
  return msg;
} 

BtMessageHandle
DefaultBtMessageFactory::createHaveAllMessage()
{
  BtHaveAllMessageHandle msg(new BtHaveAllMessage());
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createHaveNoneMessage()
{
  BtHaveNoneMessageHandle msg(new BtHaveNoneMessage());
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createRejectMessage(size_t index, uint32_t begin, size_t length)
{
  BtRejectMessageHandle msg(new BtRejectMessage(index, begin, length));
  SharedHandle<BtMessageValidator> validator
    (new RangeBtMessageValidator
     (msg.get(),
      _downloadContext->getNumPieces(),
      _pieceStorage->getPieceLength(index)));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createAllowedFastMessage(size_t index)
{
  BtAllowedFastMessageHandle msg(new BtAllowedFastMessage(index));
  SharedHandle<BtMessageValidator> validator
    (new IndexBtMessageValidator(msg.get(), _downloadContext->getNumPieces()));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createPortMessage(uint16_t port)
{
  SharedHandle<BtPortMessage> msg(new BtPortMessage(port));
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createBtExtendedMessage(const ExtensionMessageHandle& msg)
{
  BtExtendedMessageHandle m(new BtExtendedMessage(msg));
  setCommonProperty(m);
  return m;
}

void DefaultBtMessageFactory::setTaskQueue(const WeakHandle<DHTTaskQueue>& taskQueue)
{
  _taskQueue = taskQueue;
}

void DefaultBtMessageFactory::setTaskFactory(const WeakHandle<DHTTaskFactory>& taskFactory)
{
  _taskFactory = taskFactory;
}

void DefaultBtMessageFactory::setPeer(const SharedHandle<Peer>& peer)
{
  this->peer = peer;
}

void DefaultBtMessageFactory::setDownloadContext
(const SharedHandle<DownloadContext>& downloadContext)
{
  _downloadContext = downloadContext;
}

void DefaultBtMessageFactory::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  _pieceStorage = pieceStorage;
}

void DefaultBtMessageFactory::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  _peerStorage = peerStorage;
}

void DefaultBtMessageFactory::setBtMessageDispatcher(const WeakHandle<BtMessageDispatcher>& dispatcher)
{
  this->dispatcher = dispatcher;
}

void DefaultBtMessageFactory::setExtensionMessageFactory
(const SharedHandle<ExtensionMessageFactory>& factory)
{
  _extensionMessageFactory = factory;
}

void DefaultBtMessageFactory::setLocalNode(const WeakHandle<DHTNode>& localNode)
{
  _localNode = localNode;
}

void DefaultBtMessageFactory::setRoutingTable(const WeakHandle<DHTRoutingTable>& routingTable)
{
  _routingTable = routingTable;
}

void DefaultBtMessageFactory::setBtRequestFactory(const WeakHandle<BtRequestFactory>& factory)
{
  this->requestFactory = factory;
}

void DefaultBtMessageFactory::setPeerConnection(const WeakHandle<PeerConnection>& connection)
{
  this->peerConnection = connection;
}

} // namespace aria2
