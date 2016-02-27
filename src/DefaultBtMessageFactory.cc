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

DefaultBtMessageFactory::DefaultBtMessageFactory()
    : cuid_{0},
      downloadContext_{nullptr},
      pieceStorage_{nullptr},
      peerStorage_{nullptr},
      dhtEnabled_(false),
      dispatcher_{nullptr},
      requestFactory_{nullptr},
      peerConnection_{nullptr},
      extensionMessageFactory_{nullptr},
      localNode_{nullptr},
      routingTable_{nullptr},
      taskQueue_{nullptr},
      taskFactory_{nullptr},
      metadataGetMode_(false)
{
}

std::unique_ptr<BtMessage>
DefaultBtMessageFactory::createBtMessage(const unsigned char* data,
                                         size_t dataLength)
{
  auto msg = std::unique_ptr<AbstractBtMessage>{};
  if (dataLength == 0) {
    // keep-alive
    msg = make_unique<BtKeepAliveMessage>();
  }
  else {
    uint8_t id = bittorrent::getId(data);
    switch (id) {
    case BtChokeMessage::ID:
      msg = BtChokeMessage::create(data, dataLength);
      break;
    case BtUnchokeMessage::ID:
      msg = BtUnchokeMessage::create(data, dataLength);
      break;
    case BtInterestedMessage::ID: {
      auto m = BtInterestedMessage::create(data, dataLength);
      m->setPeerStorage(peerStorage_);
      msg = std::move(m);
      break;
    }
    case BtNotInterestedMessage::ID: {
      auto m = BtNotInterestedMessage::create(data, dataLength);
      m->setPeerStorage(peerStorage_);
      msg = std::move(m);
      break;
    }
    case BtHaveMessage::ID:
      msg = BtHaveMessage::create(data, dataLength);
      if (!metadataGetMode_) {
        msg->setBtMessageValidator(make_unique<IndexBtMessageValidator>(
            static_cast<BtHaveMessage*>(msg.get()),
            downloadContext_->getNumPieces()));
      }
      break;
    case BtBitfieldMessage::ID:
      msg = BtBitfieldMessage::create(data, dataLength);
      if (!metadataGetMode_) {
        msg->setBtMessageValidator(make_unique<BtBitfieldMessageValidator>(
            static_cast<BtBitfieldMessage*>(msg.get()),
            downloadContext_->getNumPieces()));
      }
      break;
    case BtRequestMessage::ID: {
      auto m = BtRequestMessage::create(data, dataLength);
      if (!metadataGetMode_) {
        m->setBtMessageValidator(make_unique<RangeBtMessageValidator>(
            static_cast<BtRequestMessage*>(m.get()),
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(m->getIndex())));
      }
      msg = std::move(m);
      break;
    }
    case BtPieceMessage::ID: {
      auto m = BtPieceMessage::create(data, dataLength);
      if (!metadataGetMode_) {
        m->setBtMessageValidator(make_unique<BtPieceMessageValidator>(
            static_cast<BtPieceMessage*>(m.get()),
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(m->getIndex())));
      }
      m->setDownloadContext(downloadContext_);
      m->setPeerStorage(peerStorage_);
      msg = std::move(m);
      break;
    }
    case BtCancelMessage::ID: {
      auto m = BtCancelMessage::create(data, dataLength);
      if (!metadataGetMode_) {
        m->setBtMessageValidator(make_unique<RangeBtMessageValidator>(
            static_cast<BtCancelMessage*>(m.get()),
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(m->getIndex())));
      }
      msg = std::move(m);
      break;
    }
    case BtPortMessage::ID: {
      auto m = BtPortMessage::create(data, dataLength);
      m->setLocalNode(localNode_);
      m->setRoutingTable(routingTable_);
      m->setTaskQueue(taskQueue_);
      m->setTaskFactory(taskFactory_);
      msg = std::move(m);
      break;
    }
    case BtSuggestPieceMessage::ID: {
      auto m = BtSuggestPieceMessage::create(data, dataLength);
      if (!metadataGetMode_) {
        m->setBtMessageValidator(make_unique<IndexBtMessageValidator>(
            static_cast<BtSuggestPieceMessage*>(m.get()),
            downloadContext_->getNumPieces()));
      }
      msg = std::move(m);
      break;
    }
    case BtHaveAllMessage::ID:
      msg = BtHaveAllMessage::create(data, dataLength);
      break;
    case BtHaveNoneMessage::ID:
      msg = BtHaveNoneMessage::create(data, dataLength);
      break;
    case BtRejectMessage::ID: {
      auto m = BtRejectMessage::create(data, dataLength);
      if (!metadataGetMode_) {
        m->setBtMessageValidator(make_unique<RangeBtMessageValidator>(
            static_cast<BtRejectMessage*>(m.get()),
            downloadContext_->getNumPieces(),
            pieceStorage_->getPieceLength(m->getIndex())));
      }
      msg = std::move(m);
      break;
    }
    case BtAllowedFastMessage::ID: {
      auto m = BtAllowedFastMessage::create(data, dataLength);
      if (!metadataGetMode_) {
        m->setBtMessageValidator(make_unique<IndexBtMessageValidator>(
            static_cast<BtAllowedFastMessage*>(m.get()),
            downloadContext_->getNumPieces()));
      }
      msg = std::move(m);
      break;
    }
    case BtExtendedMessage::ID: {
      if (peer_->isExtendedMessagingEnabled()) {
        msg = BtExtendedMessage::create(extensionMessageFactory_, peer_, data,
                                        dataLength);
      }
      else {
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
  return std::move(msg);
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
  if (metadataGetMode_) {
    msg->enableMetadataGetMode();
  }
}

std::unique_ptr<BtHandshakeMessage>
DefaultBtMessageFactory::createHandshakeMessage(const unsigned char* data,
                                                size_t dataLength)
{
  auto msg = BtHandshakeMessage::create(data, dataLength);
  msg->setBtMessageValidator(make_unique<BtHandshakeMessageValidator>(
      msg.get(), bittorrent::getInfoHash(downloadContext_)));
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtHandshakeMessage>
DefaultBtMessageFactory::createHandshakeMessage(const unsigned char* infoHash,
                                                const unsigned char* peerId)
{
  auto msg = make_unique<BtHandshakeMessage>(infoHash, peerId);
  msg->setDHTEnabled(dhtEnabled_);
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtRequestMessage> DefaultBtMessageFactory::createRequestMessage(
    const std::shared_ptr<Piece>& piece, size_t blockIndex)
{
  auto msg = make_unique<BtRequestMessage>(
      piece->getIndex(), blockIndex * piece->getBlockLength(),
      piece->getBlockLength(blockIndex), blockIndex);
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtCancelMessage>
DefaultBtMessageFactory::createCancelMessage(size_t index, int32_t begin,
                                             int32_t length)
{
  auto msg = make_unique<BtCancelMessage>(index, begin, length);
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtPieceMessage>
DefaultBtMessageFactory::createPieceMessage(size_t index, int32_t begin,
                                            int32_t length)
{
  auto msg = make_unique<BtPieceMessage>(index, begin, length);
  msg->setDownloadContext(downloadContext_);
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtHaveMessage>
DefaultBtMessageFactory::createHaveMessage(size_t index)
{
  auto msg = make_unique<BtHaveMessage>(index);
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtChokeMessage> DefaultBtMessageFactory::createChokeMessage()
{
  auto msg = make_unique<BtChokeMessage>();
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtUnchokeMessage>
DefaultBtMessageFactory::createUnchokeMessage()
{
  auto msg = make_unique<BtUnchokeMessage>();
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtInterestedMessage>
DefaultBtMessageFactory::createInterestedMessage()
{
  auto msg = make_unique<BtInterestedMessage>();
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtNotInterestedMessage>
DefaultBtMessageFactory::createNotInterestedMessage()
{
  auto msg = make_unique<BtNotInterestedMessage>();
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtBitfieldMessage>
DefaultBtMessageFactory::createBitfieldMessage()
{
  auto msg = make_unique<BtBitfieldMessage>(pieceStorage_->getBitfield(),
                                            pieceStorage_->getBitfieldLength());
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtKeepAliveMessage>
DefaultBtMessageFactory::createKeepAliveMessage()
{
  auto msg = make_unique<BtKeepAliveMessage>();
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtHaveAllMessage>
DefaultBtMessageFactory::createHaveAllMessage()
{
  auto msg = make_unique<BtHaveAllMessage>();
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtHaveNoneMessage>
DefaultBtMessageFactory::createHaveNoneMessage()
{
  auto msg = make_unique<BtHaveNoneMessage>();
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtRejectMessage>
DefaultBtMessageFactory::createRejectMessage(size_t index, int32_t begin,
                                             int32_t length)
{
  auto msg = make_unique<BtRejectMessage>(index, begin, length);
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtAllowedFastMessage>
DefaultBtMessageFactory::createAllowedFastMessage(size_t index)
{
  auto msg = make_unique<BtAllowedFastMessage>(index);
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtPortMessage>
DefaultBtMessageFactory::createPortMessage(uint16_t port)
{
  auto msg = make_unique<BtPortMessage>(port);
  setCommonProperty(msg.get());
  return msg;
}

std::unique_ptr<BtExtendedMessage>
DefaultBtMessageFactory::createBtExtendedMessage(
    std::unique_ptr<ExtensionMessage> exmsg)
{
  auto msg = make_unique<BtExtendedMessage>(std::move(exmsg));
  setCommonProperty(msg.get());
  return msg;
}

void DefaultBtMessageFactory::setTaskQueue(DHTTaskQueue* taskQueue)
{
  taskQueue_ = taskQueue;
}

void DefaultBtMessageFactory::setTaskFactory(DHTTaskFactory* taskFactory)
{
  taskFactory_ = taskFactory;
}

void DefaultBtMessageFactory::setPeer(const std::shared_ptr<Peer>& peer)
{
  peer_ = peer;
}

void DefaultBtMessageFactory::setDownloadContext(
    DownloadContext* downloadContext)
{
  downloadContext_ = downloadContext;
}

void DefaultBtMessageFactory::setPieceStorage(PieceStorage* pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtMessageFactory::setPeerStorage(PeerStorage* peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtMessageFactory::setBtMessageDispatcher(
    BtMessageDispatcher* dispatcher)
{
  dispatcher_ = dispatcher;
}

void DefaultBtMessageFactory::setExtensionMessageFactory(
    ExtensionMessageFactory* factory)
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
