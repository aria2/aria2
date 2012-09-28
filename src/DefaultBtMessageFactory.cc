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

SharedHandle<BtMessage>
DefaultBtMessageFactory::createBtMessage
(const unsigned char* data, size_t dataLength)
{
  SharedHandle<AbstractBtMessage> msg;
  if(dataLength == 0) {
    // keep-alive
    msg.reset(new BtKeepAliveMessage());
  } else {
    uint8_t id = bittorrent::getId(data);
    switch(id) {
    case BtChokeMessage::ID:
      msg.reset(BtChokeMessage::create(data, dataLength));
      break;
    case BtUnchokeMessage::ID:
      msg.reset(BtUnchokeMessage::create(data, dataLength));
      break;
    case BtInterestedMessage::ID:
      {
        BtInterestedMessage* m = BtInterestedMessage::create(data, dataLength);
        m->setPeerStorage(peerStorage_);
        msg.reset(m);
      }
      break;
    case BtNotInterestedMessage::ID:
      {
        BtNotInterestedMessage* m =
          BtNotInterestedMessage::create(data, dataLength);
        m->setPeerStorage(peerStorage_);
        msg.reset(m);
      }
      break;
    case BtHaveMessage::ID:
      msg.reset(BtHaveMessage::create(data, dataLength));
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> v
          (new IndexBtMessageValidator(static_cast<BtHaveMessage*>(msg.get()),
                                       downloadContext_->getNumPieces()));
        msg->setBtMessageValidator(v);
      }
      break;
    case BtBitfieldMessage::ID:
      msg.reset(BtBitfieldMessage::create(data, dataLength));
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> v
          (new BtBitfieldMessageValidator
           (static_cast<BtBitfieldMessage*>(msg.get()),
            downloadContext_->getNumPieces()));
        msg->setBtMessageValidator(v);
      }
      break;
    case BtRequestMessage::ID: {
      BtRequestMessage* m = BtRequestMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new RangeBtMessageValidator
           (m,
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(m->getIndex())));
        m->setBtMessageValidator(validator);
      }
      msg.reset(m);
      break;
    }
    case BtCancelMessage::ID: {
      BtCancelMessage* m = BtCancelMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new RangeBtMessageValidator
           (m,
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(m->getIndex())));
        m->setBtMessageValidator(validator);
      }
      msg.reset(m);
      break;
    }
    case BtPieceMessage::ID: {
      BtPieceMessage* m = BtPieceMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new BtPieceMessageValidator
           (m,
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(m->getIndex())));
        m->setBtMessageValidator(validator);
      }
      m->setDownloadContext(downloadContext_);
      m->setPeerStorage(peerStorage_);
      msg.reset(m);
      break;
    }
    case BtHaveAllMessage::ID:
      msg.reset(BtHaveAllMessage::create(data, dataLength));
      break;
    case BtHaveNoneMessage::ID:
      msg.reset(BtHaveNoneMessage::create(data, dataLength));
      break;
    case BtRejectMessage::ID: {
      BtRejectMessage* m = BtRejectMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new RangeBtMessageValidator
           (m,
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(m->getIndex())));
        m->setBtMessageValidator(validator);
      }
      msg.reset(m);
      break;
    }
    case BtSuggestPieceMessage::ID: {
      BtSuggestPieceMessage* m =
        BtSuggestPieceMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new IndexBtMessageValidator(m, downloadContext_->getNumPieces()));
        m->setBtMessageValidator(validator);
      }
      msg.reset(m);
      break;
    }
    case BtAllowedFastMessage::ID: {
      BtAllowedFastMessage* m = BtAllowedFastMessage::create(data, dataLength);
      if(!metadataGetMode_) {
        SharedHandle<BtMessageValidator> validator
          (new IndexBtMessageValidator(m, downloadContext_->getNumPieces()));
        m->setBtMessageValidator(validator);
      }
      msg.reset(m);
      break;
    }
    case BtPortMessage::ID: {
      BtPortMessage* m = BtPortMessage::create(data, dataLength);
      m->setLocalNode(localNode_);
      m->setRoutingTable(routingTable_);
      m->setTaskQueue(taskQueue_);
      m->setTaskFactory(taskFactory_);
      msg.reset(m);
      break;
    }
    case BtExtendedMessage::ID: {
      if(peer_->isExtendedMessagingEnabled()) {
        msg.reset(BtExtendedMessage::create(extensionMessageFactory_,
                                            peer_, data, dataLength));
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
  setCommonProperty(msg.get());
  return msg;
}

void DefaultBtMessageFactory::setCommonProperty(AbstractBtMessage* msg)
{
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
  SharedHandle<BtMessageValidator> validator
    (new BtHandshakeMessageValidator
     (msg.get(), bittorrent::getInfoHash(downloadContext_)));
  msg->setBtMessageValidator(validator);
  setCommonProperty(msg.get());
  return msg;
}

SharedHandle<BtHandshakeMessage>
DefaultBtMessageFactory::createHandshakeMessage(const unsigned char* infoHash,
                                                const unsigned char* peerId)
{
  SharedHandle<BtHandshakeMessage> msg
    (new BtHandshakeMessage(infoHash, peerId));
  msg->setDHTEnabled(dhtEnabled_);
  setCommonProperty(msg.get());
  return msg;
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createRequestMessage
(const SharedHandle<Piece>& piece, size_t blockIndex)
{
  BtRequestMessage* msg
    (new BtRequestMessage(piece->getIndex(),
                          blockIndex*piece->getBlockLength(),
                          piece->getBlockLength(blockIndex),
                          blockIndex));
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createCancelMessage
(size_t index, int32_t begin, int32_t length)
{
  BtCancelMessage* msg(new BtCancelMessage(index, begin, length));
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createPieceMessage
(size_t index, int32_t begin, int32_t length)
{
  BtPieceMessage* msg(new BtPieceMessage(index, begin, length));
  msg->setDownloadContext(downloadContext_);
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createHaveMessage(size_t index)
{
  BtHaveMessage* msg(new BtHaveMessage(index));
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createChokeMessage()
{
  BtChokeMessage* msg(new BtChokeMessage());
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createUnchokeMessage()
{
  BtUnchokeMessage* msg(new BtUnchokeMessage());
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createInterestedMessage()
{
  BtInterestedMessage* msg(new BtInterestedMessage());
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createNotInterestedMessage()
{
  BtNotInterestedMessage* msg(new BtNotInterestedMessage());
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createBitfieldMessage()
{
  BtBitfieldMessage* msg
    (new BtBitfieldMessage(pieceStorage_->getBitfield(),
                           pieceStorage_->getBitfieldLength()));
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createKeepAliveMessage()
{
  BtKeepAliveMessage* msg(new BtKeepAliveMessage());
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createHaveAllMessage()
{
  BtHaveAllMessage* msg(new BtHaveAllMessage());
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createHaveNoneMessage()
{
  BtHaveNoneMessage* msg(new BtHaveNoneMessage());
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createRejectMessage
(size_t index, int32_t begin, int32_t length)
{
  BtRejectMessage* msg(new BtRejectMessage(index, begin, length));
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createAllowedFastMessage(size_t index)
{
  BtAllowedFastMessage* msg(new BtAllowedFastMessage(index));
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createPortMessage(uint16_t port)
{
  BtPortMessage* msg(new BtPortMessage(port));
  setCommonProperty(msg);
  return SharedHandle<BtMessage>(msg);
}

SharedHandle<BtMessage>
DefaultBtMessageFactory::createBtExtendedMessage
(const SharedHandle<ExtensionMessage>& msg)
{
  BtExtendedMessage* m(new BtExtendedMessage(msg));
  setCommonProperty(m);
  return SharedHandle<BtMessage>(m);
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
