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
#include "DefaultExtensionMessageFactory.h"
#include "Peer.h"
#include "DlAbortEx.h"
#include "HandshakeExtensionMessage.h"
#include "UTPexExtensionMessage.h"
#include "LogFactory.h"
#include "Logger.h"
#include "StringFormat.h"
#include "PeerStorage.h"
#include "ExtensionMessageRegistry.h"
#include "DownloadContext.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "UTMetadataRequestExtensionMessage.h"
#include "UTMetadataDataExtensionMessage.h"
#include "UTMetadataRejectExtensionMessage.h"
#include "message.h"
#include "bencode.h"
#include "PieceStorage.h"
#include "UTMetadataRequestTracker.h"
#include "RequestGroup.h"

namespace aria2 {

DefaultExtensionMessageFactory::DefaultExtensionMessageFactory():
  _logger(LogFactory::getInstance()) {}

DefaultExtensionMessageFactory::DefaultExtensionMessageFactory
(const SharedHandle<Peer>& peer,
 const SharedHandle<ExtensionMessageRegistry>& registry):
  _peer(peer),
  _registry(registry),
  _logger(LogFactory::getInstance()) {}

DefaultExtensionMessageFactory::~DefaultExtensionMessageFactory() {}

ExtensionMessageHandle
DefaultExtensionMessageFactory::createMessage(const unsigned char* data, size_t length)
{
  uint8_t extensionMessageID = *data;
  if(extensionMessageID == 0) {
    // handshake
    HandshakeExtensionMessageHandle m = HandshakeExtensionMessage::create(data, length);
    m->setPeer(_peer);
    m->setDownloadContext(_dctx);
    return m;
  } else {
    std::string extensionName = _registry->getExtensionName(extensionMessageID);
    if(extensionName.empty()) {
      throw DL_ABORT_EX
        (StringFormat("No extension registered for extended message ID %u",
                      extensionMessageID).str());
    }
    if(extensionName == "ut_pex") {
      // uTorrent compatible Peer-Exchange
      UTPexExtensionMessageHandle m =
        UTPexExtensionMessage::create(data, length);
      m->setPeerStorage(_peerStorage);
      return m;
    } else if(extensionName == "ut_metadata") {
      if(length == 0) {
        throw DL_ABORT_EX(StringFormat(MSG_TOO_SMALL_PAYLOAD_SIZE,
                                       "ut_metadata", length).str());
      }
      size_t end;
      BDE dict = bencode::decode(data+1, length-1, end);
      if(!dict.isDict()) {
        throw DL_ABORT_EX("Bad ut_metadata: dictionary not found");
      }
      const BDE& msgType = dict["msg_type"];
      if(!msgType.isInteger()) {
        throw DL_ABORT_EX("Bad ut_metadata: msg_type not found");
      }
      const BDE& index = dict["piece"];
      if(!index.isInteger()) {
        throw DL_ABORT_EX("Bad ut_metadata: piece not found");
      }
      switch(msgType.i()) {
      case 0: {
        SharedHandle<UTMetadataRequestExtensionMessage> m
          (new UTMetadataRequestExtensionMessage(extensionMessageID));
        m->setIndex(index.i());
        m->setDownloadContext(_dctx);
        m->setPeer(_peer);
        m->setBtMessageFactory(_messageFactory);
        m->setBtMessageDispatcher(_dispatcher);
        return m;
      }
      case 1: {
        if(end == length) {
          throw DL_ABORT_EX("Bad ut_metadata data: data not found");
        }
        const BDE& totalSize = dict["total_size"];
        if(!totalSize.isInteger()) {
          throw DL_ABORT_EX("Bad ut_metadata data: total_size not found");
        }
        SharedHandle<UTMetadataDataExtensionMessage> m
          (new UTMetadataDataExtensionMessage(extensionMessageID));
        m->setIndex(index.i());
        m->setTotalSize(totalSize.i());
        m->setData(std::string(&data[1+end], &data[length]));
        m->setUTMetadataRequestTracker(_tracker);
        m->setPieceStorage(_dctx->getOwnerRequestGroup()->getPieceStorage());
        m->setDownloadContext(_dctx);
        return m;
      }
      case 2: {
        SharedHandle<UTMetadataRejectExtensionMessage> m
          (new UTMetadataRejectExtensionMessage(extensionMessageID));
        m->setIndex(index.i());
        // No need to inject tracker because peer will be disconnected.
        return m;
      }
      default:
        throw DL_ABORT_EX(StringFormat("Bad ut_metadata: unknown msg_type=%u",
                                       msgType.i()).str());
      }
    } else {
      throw DL_ABORT_EX
        (StringFormat("Unsupported extension message received. extensionMessageID=%u, extensionName=%s",
                      extensionMessageID, extensionName.c_str()).str());
    }
  }
}

void DefaultExtensionMessageFactory::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  _peerStorage = peerStorage;
}

void DefaultExtensionMessageFactory::setPeer(const SharedHandle<Peer>& peer)
{
  _peer = peer;
}

} // namespace aria2
