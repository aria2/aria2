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

#include <cstring>

#include "Peer.h"
#include "DlAbortEx.h"
#include "HandshakeExtensionMessage.h"
#include "UTPexExtensionMessage.h"
#include "fmt.h"
#include "PeerStorage.h"
#include "ExtensionMessageRegistry.h"
#include "DownloadContext.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "UTMetadataRequestExtensionMessage.h"
#include "UTMetadataDataExtensionMessage.h"
#include "UTMetadataRejectExtensionMessage.h"
#include "message.h"
#include "PieceStorage.h"
#include "UTMetadataRequestTracker.h"
#include "RequestGroup.h"
#include "bencode2.h"

namespace aria2 {

// i686-w64-mingw32-g++ 4.6 does not support constructor delegate
DefaultExtensionMessageFactory::DefaultExtensionMessageFactory()
    : peerStorage_{nullptr},
      registry_{nullptr},
      dctx_{nullptr},
      messageFactory_{nullptr},
      dispatcher_{nullptr},
      tracker_{nullptr}
{
}

DefaultExtensionMessageFactory::DefaultExtensionMessageFactory(
    const std::shared_ptr<Peer>& peer, ExtensionMessageRegistry* registry)
    : peerStorage_{nullptr},
      peer_{peer},
      registry_{registry},
      dctx_{nullptr},
      messageFactory_{nullptr},
      dispatcher_{nullptr},
      tracker_{nullptr}
{
}

std::unique_ptr<ExtensionMessage>
DefaultExtensionMessageFactory::createMessage(const unsigned char* data,
                                              size_t length)
{
  uint8_t extensionMessageID = *data;
  if (extensionMessageID == 0) {
    // handshake
    auto m = HandshakeExtensionMessage::create(data, length);
    m->setPeer(peer_);
    m->setDownloadContext(dctx_);
    return std::move(m);
  }
  else {
    const char* extensionName = registry_->getExtensionName(extensionMessageID);
    if (!extensionName) {
      throw DL_ABORT_EX(
          fmt("No extension registered for extended message ID %u",
              extensionMessageID));
    }
    if (strcmp(extensionName, "ut_pex") == 0) {
      // uTorrent compatible Peer-Exchange
      auto m = UTPexExtensionMessage::create(data, length);
      m->setPeerStorage(peerStorage_);
      return std::move(m);
    }
    else if (strcmp(extensionName, "ut_metadata") == 0) {
      if (length == 0) {
        throw DL_ABORT_EX(fmt(MSG_TOO_SMALL_PAYLOAD_SIZE, "ut_metadata",
                              static_cast<unsigned long>(length)));
      }
      size_t end;
      auto decoded = bencode2::decode(data + 1, length - 1, end);
      const Dict* dict = downcast<Dict>(decoded);
      if (!dict) {
        throw DL_ABORT_EX("Bad ut_metadata: dictionary not found");
      }
      const Integer* msgType = downcast<Integer>(dict->get("msg_type"));
      if (!msgType) {
        throw DL_ABORT_EX("Bad ut_metadata: msg_type not found");
      }
      const Integer* index = downcast<Integer>(dict->get("piece"));
      if (!index || index->i() < 0) {
        throw DL_ABORT_EX("Bad ut_metadata: piece not found");
      }
      switch (msgType->i()) {
      case 0: {
        auto m =
            make_unique<UTMetadataRequestExtensionMessage>(extensionMessageID);
        m->setIndex(index->i());
        m->setDownloadContext(dctx_);
        m->setPeer(peer_);
        m->setBtMessageFactory(messageFactory_);
        m->setBtMessageDispatcher(dispatcher_);
        return std::move(m);
      }
      case 1: {
        if (end == length) {
          throw DL_ABORT_EX("Bad ut_metadata data: data not found");
        }
        const Integer* totalSize = downcast<Integer>(dict->get("total_size"));
        if (!totalSize || totalSize->i() < 0) {
          throw DL_ABORT_EX("Bad ut_metadata data: total_size not found");
        }
        auto m =
            make_unique<UTMetadataDataExtensionMessage>(extensionMessageID);
        m->setIndex(index->i());
        m->setTotalSize(totalSize->i());
        m->setData(&data[1 + end], &data[length]);
        m->setUTMetadataRequestTracker(tracker_);
        m->setPieceStorage(
            dctx_->getOwnerRequestGroup()->getPieceStorage().get());
        m->setDownloadContext(dctx_);
        return std::move(m);
      }
      case 2: {
        auto m =
            make_unique<UTMetadataRejectExtensionMessage>(extensionMessageID);
        m->setIndex(index->i());
        // No need to inject tracker because peer will be disconnected.
        return std::move(m);
      }
      default:
        throw DL_ABORT_EX(
            fmt("Bad ut_metadata: unknown msg_type=%" PRId64, msgType->i()));
      }
    }
    else {
      throw DL_ABORT_EX(fmt("Unsupported extension message received."
                            " extensionMessageID=%u, extensionName=%s",
                            extensionMessageID, extensionName));
    }
  }
}

void DefaultExtensionMessageFactory::setPeerStorage(PeerStorage* peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultExtensionMessageFactory::setPeer(const std::shared_ptr<Peer>& peer)
{
  peer_ = peer;
}

void DefaultExtensionMessageFactory::setExtensionMessageRegistry(
    ExtensionMessageRegistry* registry)
{
  registry_ = registry;
}

void DefaultExtensionMessageFactory::setDownloadContext(DownloadContext* dctx)
{
  dctx_ = dctx;
}

void DefaultExtensionMessageFactory::setBtMessageFactory(
    BtMessageFactory* factory)
{
  messageFactory_ = factory;
}

void DefaultExtensionMessageFactory::setBtMessageDispatcher(
    BtMessageDispatcher* disp)
{
  dispatcher_ = disp;
}

void DefaultExtensionMessageFactory::setUTMetadataRequestTracker(
    UTMetadataRequestTracker* tracker)
{
  tracker_ = tracker;
}

} // namespace aria2
