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
#include "DefaultBtInteractive.h"
#include "prefs.h"
#include "message.h"
#include "BtHandshakeMessage.h"
#include "Util.h"
#include "BtKeepAliveMessage.h"
#include "BtChokeMessage.h"
#include "BtUnchokeMessage.h"
#include "DlAbortEx.h"

void DefaultBtInteractive::initiateHandshake() {
  BtMessageHandle message = messageFactory->createHandshakeMessage(btContext->getInfoHash(),
								   btContext->getPeerId());
  dispatcher->addMessageToQueue(message);
  dispatcher->sendMessages();
}

BtMessageHandle DefaultBtInteractive::receiveHandshake(bool quickReply) {
    BtHandshakeMessageHandle message =
      btMessageReceiver->receiveHandshake(quickReply);
    if(message.isNull()) {
      return 0;
    }
    peer->setPeerId(message->getPeerId());
    logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		 peer->ipaddr.c_str(), peer->port,
		 message->toString().c_str());
    return message;
}

BtMessageHandle DefaultBtInteractive::receiveAndSendHandshake() {
  return receiveHandshake(true);
}

void DefaultBtInteractive::doPostHandshakeProcessing() {
  // TODO where is the valid place to rest haveCheckTime?
  haveCheckPoint.reset();
  keepAliveCheckPoint.reset();
  floodingCheckPoint.reset();
  addBitfieldMessageToQueue();
  addAllowedFastMessageToQueue();  
  sendPendingMessage();
}

void DefaultBtInteractive::addBitfieldMessageToQueue() {
  if(peer->isFastExtensionEnabled()) {
    if(pieceStorage->downloadFinished()) {
      dispatcher->addMessageToQueue(messageFactory->createHaveAllMessage());
    } else if(pieceStorage->getCompletedLength() > 0) {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    } else {
      dispatcher->addMessageToQueue(messageFactory->createHaveNoneMessage());
    }
  } else {
    if(pieceStorage->getCompletedLength() > 0) {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    }
  }
}

void DefaultBtInteractive::addAllowedFastMessageToQueue() {
  if(peer->isFastExtensionEnabled()) {
    Integers fastSet = Util::computeFastSet(peer->ipaddr,
					    btContext->getInfoHash(),
					    btContext->getNumPieces(),
					    allowedFastSetSize);
    for(Integers::const_iterator itr = fastSet.begin();
	itr != fastSet.end(); itr++) {
      dispatcher->addMessageToQueue(messageFactory->createAllowedFastMessage(*itr));
    }
  }
}

void DefaultBtInteractive::decideChoking() {
  if(peer->shouldBeChoking()) {
    if(!peer->amChoking) {
      dispatcher->addMessageToQueue(messageFactory->createChokeMessage());
    }
  } else {
    if(peer->amChoking) {
      dispatcher->addMessageToQueue(messageFactory->createUnchokeMessage());
    }
  }
}

void DefaultBtInteractive::checkHave() {
  Integers indexes =
    pieceStorage->getAdvertisedPieceIndexes(cuid, haveCheckPoint);
  haveCheckPoint.reset();
  if(indexes.size() >= 20) {
    if(peer->isFastExtensionEnabled() && pieceStorage->downloadFinished()) {
      dispatcher->addMessageToQueue(messageFactory->createHaveAllMessage());
    } else {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    }
  } else {
    for(Integers::iterator itr = indexes.begin(); itr != indexes.end(); itr++) {
      dispatcher->addMessageToQueue(messageFactory->createHaveMessage(*itr));
    }
  }
}

void DefaultBtInteractive::sendKeepAlive() {
  if(keepAliveCheckPoint.elapsed(keepAliveInterval)) {
    dispatcher->addMessageToQueue(messageFactory->createKeepAliveMessage());
    dispatcher->sendMessages();
    keepAliveCheckPoint.reset();
  }
}

void DefaultBtInteractive::receiveMessages() {
  for(int i = 0; i < 50; i++) {
    if(maxDownloadSpeedLimit > 0) {
      TransferStat stat = peerStorage->calculateStat();
      if(maxDownloadSpeedLimit < stat.downloadSpeed) {
	break;
      }
    }
    BtMessageHandle message = btMessageReceiver->receiveMessage();
    if(message.isNull()) {
      break;
    }
    logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		 peer->ipaddr.c_str(), peer->port,
		 message->toString().c_str());
    message->doReceivedAction();

    switch(message->getId()) {
    case BtKeepAliveMessage::ID:
      floodingStat.incKeepAliveCount();
      break;
    case BtChokeMessage::ID:
      if(!peer->peerChoking) {
	floodingStat.incChokeUnchokeCount();
      }
      break;
    case BtUnchokeMessage::ID:
      if(peer->peerChoking) {
	floodingStat.incChokeUnchokeCount();
      }
      break;
    }
  }
}

void DefaultBtInteractive::decideInterest() {
  if(pieceStorage->hasMissingPiece(peer)) {
    if(!peer->amInterested) {
      logger->debug("CUID#%d - Interested in the peer", cuid);
      dispatcher->
	addMessageToQueue(messageFactory->createInterestedMessage());
    }
  } else {
    if(peer->amInterested) {
      logger->debug("CUID#%d - Not interested in the peer", cuid);
      dispatcher->
	addMessageToQueue(messageFactory->createNotInterestedMessage());
    }
  }
}

void DefaultBtInteractive::fillPiece(int maxPieceNum) {
  if(pieceStorage->hasMissingPiece(peer)) {
    if(peer->peerChoking) {
      if(peer->isFastExtensionEnabled()) {
	while(btRequestFactory->countTargetPiece() < maxPieceNum) {
	  PieceHandle piece = pieceStorage->getMissingFastPiece(peer);
	  if(piece.isNull()) {
	    break;
	  } else {
	    btRequestFactory->addTargetPiece(piece);
	  }
	}
      }
    } else {
      while(btRequestFactory->countTargetPiece() < maxPieceNum) {
	PieceHandle piece = pieceStorage->getMissingPiece(peer);
	if(piece.isNull()) {
	  break;
	} else {
	  btRequestFactory->addTargetPiece(piece);
	}
      }
    }
  }
}

void DefaultBtInteractive::addRequests() {
  uint32_t MAX_PENDING_REQUEST;
  if(peer->getLatency() < 500) {
    MAX_PENDING_REQUEST = 24;
  } else if(peer->getLatency() < 1500) {
    MAX_PENDING_REQUEST = 12;
  } else {
    MAX_PENDING_REQUEST = 6;
  }
  uint32_t pieceNum;
  if(pieceStorage->isEndGame()) {
    pieceNum = 1;
  } else {
    uint32_t blocks = DIV_FLOOR(btContext->getPieceLength(), BLOCK_LENGTH);
    pieceNum = DIV_FLOOR(MAX_PENDING_REQUEST, blocks);
  }
  fillPiece(pieceNum);

  uint32_t reqNumToCreate =
    MAX_PENDING_REQUEST <= dispatcher->countOutstandingRequest() ?
    0 : MAX_PENDING_REQUEST-dispatcher->countOutstandingRequest();
  if(reqNumToCreate > 0) {
    //logger->debug("CUID#%d - %u requets to go.", cuid, reqNumToCreate);
    BtMessages requests;
    if(pieceStorage->isEndGame()) {
      requests = btRequestFactory->createRequestMessagesOnEndGame(reqNumToCreate);
    } else {
      requests = btRequestFactory->createRequestMessages(reqNumToCreate);
    }
    dispatcher->addMessageToQueue(requests);
  }
}

void DefaultBtInteractive::cancelAllPiece() {
  btRequestFactory->removeAllTargetPiece();
}

void DefaultBtInteractive::sendPendingMessage() {
  dispatcher->sendMessages();
}

void DefaultBtInteractive::detectMessageFlooding() {
  if(floodingCheckPoint.elapsed(FLOODING_CHECK_INTERVAL)) {
    if(floodingStat.getChokeUnchokeCount() >= 2 ||
       floodingStat.getKeepAliveCount() >= 2) {
      throw new DlAbortEx("Flooding detected.");
    } else {
      floodingStat.reset();
    }
    floodingCheckPoint.reset();
  }
}

void DefaultBtInteractive::doInteractionProcessing() {
  decideChoking();

  detectMessageFlooding();

  dispatcher->checkRequestSlotAndDoNecessaryThing();

  checkHave();

  sendKeepAlive();

  receiveMessages();
  
  btRequestFactory->removeCompletedPiece();

  decideInterest();
  if(!pieceStorage->downloadFinished()) {
    addRequests();
  }
  sendPendingMessage();
}
