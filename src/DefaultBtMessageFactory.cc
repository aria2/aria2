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
#include "PeerMessageUtil.h"
#include "BtKeepAliveMessage.h"
#include "BtChokeMessage.h"
#include "BtUnchokeMessage.h"
#include "BtInterestedMessage.h"
#include "BtNotInterestedMessage.h"
#include "BtHaveMessage.h"
#include "BtHaveMessageValidator.h"
#include "BtBitfieldMessage.h"
#include "BtBitfieldMessageValidator.h"
#include "BtRequestMessage.h"
#include "BtRequestMessageValidator.h"
#include "BtCancelMessage.h"
#include "BtCancelMessageValidator.h"
#include "BtPieceMessage.h"
#include "BtPieceMessageValidator.h"
#include "BtPortMessage.h"
#include "BtHaveAllMessage.h"
#include "BtHaveNoneMessage.h"
#include "BtRejectMessage.h"
#include "BtRejectMessageValidator.h"
#include "BtSuggestPieceMessage.h"
#include "BtSuggestPieceMessageValidator.h"
#include "BtAllowedFastMessage.h"
#include "BtAllowedFastMessageValidator.h"
#include "BtHandshakeMessage.h"
#include "BtHandshakeMessageValidator.h"
#include "BtExtendedMessage.h"
#include "ExtensionMessage.h"

DefaultBtMessageFactory::DefaultBtMessageFactory():cuid(0),
						   btContext(0),
						   pieceStorage(0),
						   peer(0),
						   _dhtEnabled(false)
{}

DefaultBtMessageFactory::~DefaultBtMessageFactory() {}

BtMessageHandle
DefaultBtMessageFactory::createBtMessage(const unsigned char* data, int32_t dataLength)
{
  AbstractBtMessageHandle msg(0);
  if(dataLength == 0) {
    // keep-alive
    msg = new BtKeepAliveMessage();
  } else {
    int8_t id = PeerMessageUtil::getId(data);
    switch(id) {
    case BtChokeMessage::ID:
      msg = BtChokeMessage::create(data, dataLength);
      break;
    case BtUnchokeMessage::ID:
      msg = BtUnchokeMessage::create(data, dataLength);
      break;
    case BtInterestedMessage::ID:
      msg = BtInterestedMessage::create(data, dataLength);
      break;
    case BtNotInterestedMessage::ID:
      msg = BtNotInterestedMessage::create(data, dataLength);
      break;
    case BtHaveMessage::ID:
      msg = BtHaveMessage::create(data, dataLength);
      msg->setBtMessageValidator(new BtHaveMessageValidator((BtHaveMessage*)msg.get(),
							    btContext->getNumPieces()));
      break;
    case BtBitfieldMessage::ID:
      msg = BtBitfieldMessage::create(data, dataLength);
      msg->setBtMessageValidator(new BtBitfieldMessageValidator((BtBitfieldMessage*)msg.get(),
								btContext->getNumPieces()));
      break;
    case BtRequestMessage::ID: {
      BtRequestMessageHandle temp = BtRequestMessage::create(data, dataLength);
      BtMessageValidatorHandle validator =
	new BtRequestMessageValidator(temp.get(),
				      btContext->getNumPieces(),
				      pieceStorage->getPieceLength(temp->getIndex()));
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtCancelMessage::ID: {
      BtCancelMessageHandle temp = BtCancelMessage::create(data, dataLength);
      BtMessageValidatorHandle validator =
	new BtCancelMessageValidator(temp.get(),
				     btContext->getNumPieces(),
				     pieceStorage->getPieceLength(temp->getIndex()));
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtPieceMessage::ID: {
      BtPieceMessageHandle temp = BtPieceMessage::create(data, dataLength);
      BtMessageValidatorHandle validator =
	new BtPieceMessageValidator(temp.get(),
				    btContext->getNumPieces(),
				    pieceStorage->getPieceLength(temp->getIndex()));
      temp->setBtMessageValidator(validator);
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
      BtMessageValidatorHandle validator =
	new BtRejectMessageValidator(temp.get(),
				     btContext->getNumPieces(),
				     pieceStorage->getPieceLength(temp->getIndex()));
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtSuggestPieceMessage::ID: {
      BtSuggestPieceMessageHandle temp = BtSuggestPieceMessage::create(data, dataLength);
      BtMessageValidatorHandle validator =
	new BtSuggestPieceMessageValidator(temp.get(),
					   btContext->getNumPieces());
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtAllowedFastMessage::ID: {
      BtAllowedFastMessageHandle temp = BtAllowedFastMessage::create(data, dataLength);
      BtMessageValidatorHandle validator =
	new BtAllowedFastMessageValidator(temp.get(),
					  btContext->getNumPieces());
      temp->setBtMessageValidator(validator);
      msg = temp;
      break;
    }
    case BtPortMessage::ID: {
      SharedHandle<BtPortMessage> temp = BtPortMessage::create(data, dataLength);
      temp->setTaskQueue(_taskQueue);
      temp->setTaskFactory(_taskFactory);
      msg = temp;
      break;
    }
    case BtExtendedMessage::ID: {
      if(peer->isExtendedMessagingEnabled()) {
	msg = BtExtendedMessage::create(btContext, peer, (const char*)data, dataLength);
      } else {
	throw new DlAbortEx("Received extended message from peer during a session with extended messaging disabled.");
      }
      break;
    }
    default:
      throw new DlAbortEx("Invalid message ID. id=%d", id);
    }
  }
  setCommonProperty(msg);
  return msg;
}

void DefaultBtMessageFactory::setCommonProperty(const AbstractBtMessageHandle& msg) {
  msg->setCuid(cuid);
  msg->setPeer(peer);
  msg->setBtContext(btContext);
  msg->setBtMessageDispatcher(dispatcher);
  msg->setBtRequestFactory(requestFactory);
  msg->setBtMessageFactory(this);
  msg->setPeerConnection(peerConnection);
}

BtMessageHandle
DefaultBtMessageFactory::createHandshakeMessage(const unsigned char* data, int32_t dataLength)
{
  BtHandshakeMessageHandle msg = BtHandshakeMessage::create(data, dataLength);
  BtMessageValidatorHandle validator =
    new BtHandshakeMessageValidator(msg.get(),
				    btContext->getInfoHash());
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createHandshakeMessage(const unsigned char* infoHash,
						const unsigned char* peerId)
{
  BtHandshakeMessageHandle msg = new BtHandshakeMessage(infoHash, peerId);
  BtMessageValidatorHandle validator =
    new BtHandshakeMessageValidator(msg.get(),
				    btContext->getInfoHash());
  msg->setBtMessageValidator(validator);
  msg->setDHTEnabled(_dhtEnabled);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createRequestMessage(const PieceHandle& piece, int32_t blockIndex)
{
  BtRequestMessageHandle msg =
    new BtRequestMessage(piece->getIndex(),
			 blockIndex*piece->getBlockLength(),
			 piece->getBlockLength(blockIndex),
			 blockIndex);
  BtMessageValidatorHandle validator =
    new BtRequestMessageValidator(msg.get(),
				  btContext->getNumPieces(),
				  pieceStorage->getPieceLength(msg->getIndex()));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createCancelMessage(int32_t index, int32_t begin, int32_t length)
{
  BtCancelMessageHandle msg = new BtCancelMessage(index, begin, length);
  BtMessageValidatorHandle validator =
    new BtCancelMessageValidator(msg.get(),
				 btContext->getNumPieces(),
				 pieceStorage->getPieceLength(index));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createPieceMessage(int32_t index, int32_t begin, int32_t length)
{
  BtPieceMessageHandle msg = new BtPieceMessage(index, begin, length);
  BtMessageValidatorHandle validator =
    new BtPieceMessageValidator(msg.get(),
				btContext->getNumPieces(),
				pieceStorage->getPieceLength(index));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createHaveMessage(int32_t index)
{
  BtHaveMessageHandle msg = new BtHaveMessage(index);
  msg->setBtMessageValidator(new BtHaveMessageValidator(msg.get(),
							btContext->getNumPieces()));
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createChokeMessage()
{
  BtChokeMessageHandle msg = new BtChokeMessage();
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createUnchokeMessage()
{
  BtUnchokeMessageHandle msg = new BtUnchokeMessage();
  setCommonProperty(msg);
  return msg;
}
 
BtMessageHandle
DefaultBtMessageFactory::createInterestedMessage()
{
  BtInterestedMessageHandle msg = new BtInterestedMessage();
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createNotInterestedMessage()
{
  BtNotInterestedMessageHandle msg = new BtNotInterestedMessage();
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createBitfieldMessage()
{
  BtBitfieldMessageHandle msg =
    new BtBitfieldMessage(pieceStorage->getBitfield(),
			  pieceStorage->getBitfieldLength());
  msg->setBtMessageValidator(new BtBitfieldMessageValidator(msg.get(),
							    btContext->getNumPieces()));
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createKeepAliveMessage()
{
  BtKeepAliveMessageHandle msg = new BtKeepAliveMessage();
  setCommonProperty(msg);
  return msg;
} 

BtMessageHandle
DefaultBtMessageFactory::createHaveAllMessage()
{
  BtHaveAllMessageHandle msg = new BtHaveAllMessage();
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createHaveNoneMessage()
{
  BtHaveNoneMessageHandle msg = new BtHaveNoneMessage();
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createRejectMessage(int32_t index, int32_t begin, int32_t length)
{
  BtRejectMessageHandle msg = new BtRejectMessage(index, begin, length);
  BtMessageValidatorHandle validator =
    new BtRejectMessageValidator(msg.get(),
				 btContext->getNumPieces(),
				 pieceStorage->getPieceLength(index));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createAllowedFastMessage(int32_t index)
{
  BtAllowedFastMessageHandle msg = new BtAllowedFastMessage(index);
  BtMessageValidatorHandle validator =
    new BtAllowedFastMessageValidator(msg.get(),
				      btContext->getNumPieces());
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createPortMessage(uint16_t port)
{
  SharedHandle<BtPortMessage> msg = new BtPortMessage(port);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createBtExtendedMessage(const ExensionMessageHandle& msg)
{
  BtExtendedMessageHandle m = new BtExtendedMessage(msg);
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
