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
#include "ReceiverMSEHandshakeCommand.h"
#include "PeerReceiveHandshakeCommand.h"
#include "PeerConnection.h"
#include "DownloadEngine.h"
#include "BtHandshakeMessage.h"
#include "DlAbortEx.h"
#include "Peer.h"
#include "message.h"
#include "Socket.h"
#include "Logger.h"
#include "prefs.h"
#include "Option.h"
#include "MSEHandshake.h"
#include "ARC4Encryptor.h"
#include "ARC4Decryptor.h"
#include "RequestGroupMan.h"
#include "BtRegistry.h"
#include "BtContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtAnnounce.h"
#include "BtRuntime.h"
#include "BtProgressInfoFile.h"

namespace aria2 {

ReceiverMSEHandshakeCommand::ReceiverMSEHandshakeCommand
(int32_t cuid,
 const SharedHandle<Peer>& peer,
 DownloadEngine* e,
 const SharedHandle<SocketCore>& s):

  PeerAbstractCommand(cuid, peer, e, s),
  _sequence(RECEIVER_IDENTIFY_HANDSHAKE),
  _mseHandshake(new MSEHandshake(cuid, s, e->option))
{
  setTimeout(e->option->getAsInt(PREF_PEER_CONNECTION_TIMEOUT));
}

ReceiverMSEHandshakeCommand::~ReceiverMSEHandshakeCommand()
{
  delete _mseHandshake;
}

bool ReceiverMSEHandshakeCommand::exitBeforeExecute()
{
  return e->isHaltRequested() || e->_requestGroupMan->downloadFinished();
}

bool ReceiverMSEHandshakeCommand::executeInternal()
{
  switch(_sequence) {
  case RECEIVER_IDENTIFY_HANDSHAKE: {
    MSEHandshake::HANDSHAKE_TYPE type = _mseHandshake->identifyHandshakeType();
    switch(type) {
    case MSEHandshake::HANDSHAKE_NOT_YET:
      break;
    case MSEHandshake::HANDSHAKE_ENCRYPTED:
      _mseHandshake->initEncryptionFacility(false);
      _sequence = RECEIVER_WAIT_KEY;
      break;
    case MSEHandshake::HANDSHAKE_LEGACY: {
      if(e->option->getAsBool(PREF_BT_REQUIRE_CRYPTO)) {
	throw DlAbortEx("The legacy BitTorrent handshake is not acceptable by the preference.");
      }
      SharedHandle<PeerConnection> peerConnection
	(new PeerConnection(cuid, socket, e->option));
      peerConnection->presetBuffer(_mseHandshake->getBuffer(),
				   _mseHandshake->getBufferLength());
      Command* c = new PeerReceiveHandshakeCommand(cuid, peer, e, socket,
						   peerConnection);
      e->commands.push_back(c);
      return true;
    }
    default:
      throw DlAbortEx("Not supported handshake type.");
    }
    break;
  }
  case RECEIVER_WAIT_KEY: {
    if(_mseHandshake->receivePublicKey()) {
      if(_mseHandshake->sendPublicKey()) {
	_sequence = RECEIVER_FIND_HASH_MARKER;
      } else {
	setWriteCheckSocket(socket);
	_sequence = RECEIVER_SEND_KEY_PENDING;
      }
    }
    break;
  }
  case RECEIVER_SEND_KEY_PENDING:
    if(_mseHandshake->sendPublicKey()) {
      disableWriteCheckSocket();
      _sequence = RECEIVER_FIND_HASH_MARKER;
    }
    break;
  case RECEIVER_FIND_HASH_MARKER: {
    if(_mseHandshake->findReceiverHashMarker()) {
      _sequence = RECEIVER_RECEIVE_PAD_C_LENGTH;
    }
    break;
  }
  case RECEIVER_RECEIVE_PAD_C_LENGTH: {
    if(_mseHandshake->receiveReceiverHashAndPadCLength
       (e->getBtRegistry()->getAllBtContext())) {
      _sequence = RECEIVER_RECEIVE_PAD_C;
    }
    break;
  }
  case RECEIVER_RECEIVE_PAD_C: {
    if(_mseHandshake->receivePad()) {
      _sequence = RECEIVER_RECEIVE_IA_LENGTH;
    }
    break;
  }
  case RECEIVER_RECEIVE_IA_LENGTH: {
    if(_mseHandshake->receiveReceiverIALength()) {
      _sequence = RECEIVER_RECEIVE_IA;
    }
    break;
  }
  case RECEIVER_RECEIVE_IA: {
    if(_mseHandshake->receiveReceiverIA()) {
      if(_mseHandshake->sendReceiverStep2()) {
	createCommand();
	return true;
      } else {
	setWriteCheckSocket(socket);
	_sequence = RECEIVER_SEND_STEP2_PENDING;
      }
    }
    break;
  }
  case RECEIVER_SEND_STEP2_PENDING:
    if(_mseHandshake->sendReceiverStep2()) {
      disableWriteCheckSocket();
      createCommand();
      return true;
    }
    break;
  }
  e->commands.push_back(this);
  return false;
}

void ReceiverMSEHandshakeCommand::createCommand()
{
  SharedHandle<PeerConnection> peerConnection
    (new PeerConnection(cuid, socket, e->option));
  if(_mseHandshake->getNegotiatedCryptoType() == MSEHandshake::CRYPTO_ARC4) {
    peerConnection->enableEncryption(_mseHandshake->getEncryptor(),
				     _mseHandshake->getDecryptor());
  }
  if(_mseHandshake->getIALength() > 0) {
    peerConnection->presetBuffer(_mseHandshake->getIA(),
				 _mseHandshake->getIALength());
  }
  // TODO add _mseHandshake->getInfoHash() to PeerReceiveHandshakeCommand
  // as a hint. If this info hash and one in BitTorrent Handshake does not
  // match, then drop connection.
  Command* c =
    new PeerReceiveHandshakeCommand(cuid, peer, e, socket, peerConnection);
  e->commands.push_back(c);
}

} // namespace aria2
