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
#include "fmt.h"
#include "ExtensionMessageFactory.h"
#include "bittorrent_helper.h"

namespace aria2 {

DefaultBtMessageFactory::DefaultBtMessageFactory():
  cuid_(0),
  dhtEnabled_(false),
  dispatcher_(0),
  requestFactory_(0),
  peerConnection_(0),
  localNode_(0),
  routingTable_(0),
  taskQueue_(0),
  taskFactory_(0),
  metadataGetMode_(false)
{}

DefaultBtMessageFactory::~DefaultBtMessageFactory() {}

BtMessageHandle
DefaultBtMessageFactory::createBtMessage
(const unsigned char* data, size_t dataLength)
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
        m->setPeerStorage(peerStorage_);
        msg = m;
      }
      break;
    case BtNotInterestedMessage::ID:
      {
        SharedHandle<BtNotInterestedMessage> m =
          BtNotInterestedMessage::create(data, dataLength);
        m->setPeerStorage(peerStorage_);
        msg = m;
      }
      break;
    case BtHaveMessage::ID:
      msg = BtHaveMessage::create(data, dataLength);
      {
        if(!metadataGetMode_) {
          SharedHandle<BtMessageValidator> v
            (new IndexBtMessageValidator(static_cast<BtHaveMessage*>(msg.get()),
                                         downloadContext_->getNumPieces()));
          msg->setBtMessageValidator(v);
        }
      }
      break;
    case BtBitfieldMessage::ID:
      msg = BtBitfieldMessage::create(data, dataLength);
      {
        if(!metadataGetMode_) {
          SharedHandle<BtMessageValidator> v
            (new BtBitfieldMessageValidator
             (static_cast<BtBitfieldMessage*>(msg.get()),
              downloadContext_->getNumPieces()));
          msg->setBtMessageValidator(v);
        }
      }
      break;
    case BtRequestMessage::ID: {
      BtRequestMessageHandle temp = BtRequestMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new RangeBtMessageValidator
           (temp.get(),
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(temp->getIndex())));
        temp->setBtMessageValidator(validator);
      }
      msg = temp;
      break;
    }
    case BtCancelMessage::ID: {
      BtCancelMessageHandle temp = BtCancelMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new RangeBtMessageValidator
           (temp.get(),
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(temp->getIndex())));
        temp->setBtMessageValidator(validator);
      }
      msg = temp;
      break;
    }
    case BtPieceMessage::ID: {
      BtPieceMessageHandle temp = BtPieceMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        BtMessageValidatorHandle validator
          (new BtPieceMessageValidator
           (temp.get(),
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(temp->getIndex())));
        temp->setBtMessageValidator(validator);
      }
      temp->setDownloadContext(downloadContext_);
      temp->setPeerStorage(peerStorage_);
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
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new RangeBtMessageValidator
           (temp.get(),
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(temp->getIndex())));
        temp->setBtMessageValidator(validator);
      }
      msg = temp;
      break;
    }
    case BtSuggestPieceMessage::ID: {
      BtSuggestPieceMessageHandle temp =
        BtSuggestPieceMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new IndexBtMessageValidator(temp.get(),
                                       downloadContext_->getNumPieces()));
        temp->setBtMessageValidator(validator);
      }
      msg = temp;
      break;
    }
    case BtAllowedFastMessage::ID: {
      BtAllowedFastMessageHandle temp =
        BtAllowedFastMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new IndexBtMessageValidator(temp.get(),
                                       downloadContext_->getNumPieces()));
        temp->setBtMessageValidator(validator);
      }
      msg = temp;
      break;
    }
    case BtPortMessage::ID: {
      SharedHandle<BtPortMessage> temp =
        BtPortMessage::create(data, dataLength);
      temp->setLocalNode(localNode_);
      temp->setRoutingTable(routingTable_);
      temp->setTaskQueue(taskQueue_);
      temp->setTaskFactory(taskFactory_);
      msg = temp;
      break;
    }
    case BtExtendedMessage::ID: {
      if(peer_->isExtendedMessagingEnabled()) {
        msg = BtExtendedMessage::create(extensionMessageFactory_,
                                        peer_, data, dataLength);
      } else {
        throw DL_ABORT_EX("Received extended message from peer during"
                          " a session with extended messaging disabled.");
      }
      break;
    }
    default:
      throw DL_ABORT_EX(fmt("Invalid message ID. id=%u", id));
    }
  }
  setCommonProperty(msg);
  return msg;
}

void DefaultBtMessageFactory::setCommonProperty
(const AbstractBtMessageHandle& msg) {
  msg->setCuid(cuid_);
  msg->setPeer(peer_);
  msg->setPieceStorage(pieceStorage_);
  msg->setBtMessageDispatcher(dispatcher_);
  msg->setBtRequestFactory(requestFactory_);
  msg->setBtMessageFactory(this);
  msg->setPeerConnection(peerConnection_);
  if(metadataGetMode_) {
    msg->enableMetadataGetMode();
  }
}

SharedHandle<BtHandshakeMessage>
DefaultBtMessageFactory::createHandshakeMessage
(const unsigned char* data, size_t dataLength)
{
  SharedHandle<BtHandshakeMessage> msg =
    BtHandshakeMessage::create(data, dataLength);
  BtMessageValidatorHandle validator
    (new BtHandshakeMessageValidator
     (msg.get(), bittorrent::getInfoHash(downloadContext_)));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

SharedHandle<BtHandshakeMessage>
DefaultBtMessageFactory::createHandshakeMessage(const unsigned char* infoHash,
                                                const unsigned char* peerId)
{
  SharedHandle<BtHandshakeMessage> msg
    (new BtHandshakeMessage(infoHash, peerId));
  BtMessageValidatorHandle validator
    (new BtHandshakeMessageValidator
     (msg.get(), bittorrent::getInfoHash(downloadContext_)));
  msg->setBtMessageValidator(validator);
  msg->setDHTEnabled(dhtEnabled_);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createRequestMessage
(const SharedHandle<Piece>& piece, size_t blockIndex)
{
  BtRequestMessageHandle msg
    (new BtRequestMessage(piece->getIndex(),
                          blockIndex*piece->getBlockLength(),
                          piece->getBlockLength(blockIndex),
                          blockIndex));
  SharedHandle<BtMessageValidator> validator
    (new RangeBtMessageValidator
     (msg.get(),
      downloadContext_->getNumPieces(),
      pieceStorage_->getPieceLength(msg->getIndex())));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createCancelMessage
(size_t index, int32_t begin, int32_t length)
{
  BtCancelMessageHandle msg(new BtCancelMessage(index, begin, length));
  SharedHandle<BtMessageValidator> validator
    (new RangeBtMessageValidator
     (msg.get(),
      downloadContext_->getNumPieces(),
      pieceStorage_->getPieceLength(index)));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createPieceMessage
(size_t index, int32_t begin, int32_t length)
{
  BtPieceMessageHandle msg(new BtPieceMessage(index, begin, length));
  BtMessageValidatorHandle validator
    (new BtPieceMessageValidator(msg.get(),
                                 downloadContext_->getNumPieces(),
                                 pieceStorage_->getPieceLength(index)));
  msg->setBtMessageValidator(validator);
  msg->setDownloadContext(downloadContext_);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createHaveMessage(size_t index)
{
  BtHaveMessageHandle msg(new BtHaveMessage(index));
  SharedHandle<BtMessageValidator> v
    (new IndexBtMessageValidator(msg.get(), downloadContext_->getNumPieces()));
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
    (new BtBitfieldMessage(pieceStorage_->getBitfield(),
                           pieceStorage_->getBitfieldLength()));
  SharedHandle<BtMessageValidator> v
    (new BtBitfieldMessageValidator(msg.get(),
                                    downloadContext_->getNumPieces()));
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
DefaultBtMessageFactory::createRejectMessage
(size_t index, int32_t begin, int32_t length)
{
  BtRejectMessageHandle msg(new BtRejectMessage(index, begin, length));
  SharedHandle<BtMessageValidator> validator
    (new RangeBtMessageValidator
     (msg.get(),
      downloadContext_->getNumPieces(),
      pieceStorage_->getPieceLength(index)));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg);
  return msg;
}

BtMessageHandle
DefaultBtMessageFactory::createAllowedFastMessage(size_t index)
{
  BtAllowedFastMessageHandle msg(new BtAllowedFastMessage(index));
  SharedHandle<BtMessageValidator> validator
    (new IndexBtMessageValidator(msg.get(), downloadContext_->getNumPieces()));
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
DefaultBtMessageFactory::createBtExtendedMessage
(const ExtensionMessageHandle& msg)
{
  BtExtendedMessageHandle m(new BtExtendedMessage(msg));
  setCommonProperty(m);
  return m;
}

void DefaultBtMessageFactory::setTaskQueue(DHTTaskQueue* taskQueue)
{
  taskQueue_ = taskQueue;
}

void DefaultBtMessageFactory::setTaskFactory(DHTTaskFactory* taskFactory)
{
  taskFactory_ = taskFactory;
}

void DefaultBtMessageFactory::setPeer(const SharedHandle<Peer>& peer)
{
  peer_ = peer;
}

void DefaultBtMessageFactory::setDownloadContext
(const SharedHandle<DownloadContext>& downloadContext)
{
  downloadContext_ = downloadContext;
}

void DefaultBtMessageFactory::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtMessageFactory::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtMessageFactory::setBtMessageDispatcher
(BtMessageDispatcher* dispatcher)
{
  dispatcher_ = dispatcher;
}

void DefaultBtMessageFactory::setExtensionMessageFactory
(const SharedHandle<ExtensionMessageFactory>& factory)
{
  extensionMessageFactory_ = factory;
}

void DefaultBtMessageFactory::setLocalNode(DHTNode* localNode)
{
  localNode_ = localNode;
}

void DefaultBtMessageFactory::setRoutingTable(DHTRoutingTable* routingTable)
{
  routingTable_ = routingTable;
}

void DefaultBtMessageFactory::setBtRequestFactory(BtRequestFactory* factory)
{
  requestFactory_ = factory;
}

void DefaultBtMessageFactory::setPeerConnection(PeerConnection* connection)
{
  peerConnection_ = connection;
}

} // namespace aria2
