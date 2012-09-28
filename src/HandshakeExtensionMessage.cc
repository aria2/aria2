/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "HandshakeExtensionMessage.h"
#include "Peer.h"
#include "util.h"
#include "DlAbortEx.h"
#include "LogFactory.h"
#include "Logger.h"
#include "message.h"
#include "fmt.h"
#include "bencode2.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "RequestGroup.h"
#include "PieceStorage.h"
#include "FileEntry.h"

namespace aria2 {

const char HandshakeExtensionMessage::EXTENSION_NAME[] = "handshake";

HandshakeExtensionMessage::HandshakeExtensionMessage()
  : tcpPort_(0),
    metadataSize_(0)
{}

HandshakeExtensionMessage::~HandshakeExtensionMessage() {}

std::string HandshakeExtensionMessage::getPayload()
{
  Dict dict;
  if(!clientVersion_.empty()) {
    dict.put("v", clientVersion_);
  }
  if(tcpPort_ > 0) {
    dict.put("p", Integer::g(tcpPort_));
  }
  SharedHandle<Dict> extDict = Dict::g();
  for(int i = 0; i < ExtensionMessageRegistry::MAX_EXTENSION; ++i) {
    int id = extreg_.getExtensionMessageID(i);
    if(id) {
      extDict->put(strBtExtension(i), Integer::g(id));
    }
  }
  dict.put("m", extDict);
  if(metadataSize_) {
    dict.put("metadata_size", Integer::g(metadataSize_));
  }
  return bencode2::encode(&dict);
}

std::string HandshakeExtensionMessage::toString() const
{
  std::string s(fmt("%s client=%s, tcpPort=%u, metadataSize=%lu",
                    getExtensionName(),
                    util::percentEncode(clientVersion_).c_str(),
                    tcpPort_,
                    static_cast<unsigned long>(metadataSize_)));
  for(int i = 0; i < ExtensionMessageRegistry::MAX_EXTENSION; ++i) {
    int id = extreg_.getExtensionMessageID(i);
    if(id) {
      s += fmt(", %s=%u", strBtExtension(i), id);
    }
  }
  return s;
}

void HandshakeExtensionMessage::doReceivedAction()
{
  if(tcpPort_ > 0) {
    peer_->setPort(tcpPort_);
    peer_->setIncomingPeer(false);
  }
  for(int i = 0; i < ExtensionMessageRegistry::MAX_EXTENSION; ++i) {
    int id = extreg_.getExtensionMessageID(i);
    if(id) {
      peer_->setExtension(i, id);
    }
  }
  SharedHandle<TorrentAttribute> attrs = bittorrent::getTorrentAttrs(dctx_);
  if(attrs->metadata.empty()) {
    if(!peer_->getExtensionMessageID(ExtensionMessageRegistry::UT_METADATA)) {
      // TODO In metadataGetMode, if peer don't support metadata
      // transfer, should we drop connection? There is a possibility
      // that peer can still tell us peers using PEX.
      throw DL_ABORT_EX("Peer doesn't support ut_metadata extension. Goodbye.");
    }
    if(metadataSize_ > 0) {
      if(attrs->metadataSize) {
        if(metadataSize_ != attrs->metadataSize) {
          throw DL_ABORT_EX("Wrong metadata_size. Which one is correct!?");
        }
      } else {
        attrs->metadataSize = metadataSize_;
        dctx_->getFirstFileEntry()->setLength(metadataSize_);
        dctx_->markTotalLengthIsKnown();
        dctx_->getOwnerRequestGroup()->initPieceStorage();

        SharedHandle<PieceStorage> pieceStorage =
          dctx_->getOwnerRequestGroup()->getPieceStorage();
        // We enter 'end game' mode from the start to get metadata
        // quickly.
        pieceStorage->enterEndGame();
      }
      peer_->reconfigureSessionResource(dctx_->getPieceLength(),
                                        dctx_->getTotalLength());
      peer_->setAllBitfield();
    } else {
      throw DL_ABORT_EX("Peer didn't provide metadata_size."
                        " It seems that it doesn't have whole metadata.");
    }
  }
}

void HandshakeExtensionMessage::setPeer(const SharedHandle<Peer>& peer)
{
  peer_ = peer;
}

void HandshakeExtensionMessage::setExtension(int key, uint8_t id)
{
  extreg_.setExtensionMessageID(key, id);
}

void HandshakeExtensionMessage::setExtensions(const Extensions& extensions)
{
  extreg_.setExtensions(extensions);
}

uint8_t HandshakeExtensionMessage::getExtensionMessageID(int key) const
{
  return extreg_.getExtensionMessageID(key);
}

HandshakeExtensionMessage*
HandshakeExtensionMessage::create(const unsigned char* data, size_t length)
{
  if(length < 1) {
    throw DL_ABORT_EX
      (fmt(MSG_TOO_SMALL_PAYLOAD_SIZE,
           EXTENSION_NAME, static_cast<unsigned long>(length)));
  }
  A2_LOG_DEBUG(fmt("Creating HandshakeExtensionMessage from %s",
                   util::percentEncode(data, length).c_str()));
  SharedHandle<ValueBase> decoded = bencode2::decode(data+1, length - 1);
  const Dict* dict = downcast<Dict>(decoded);
  if(!dict) {
    throw DL_ABORT_EX
      ("Unexpected payload format for extended message handshake");
  }
  HandshakeExtensionMessage* msg(new HandshakeExtensionMessage());
  const Integer* port = downcast<Integer>(dict->get("p"));
  if(port && 0 < port->i() && port->i() < 65536) {
    msg->tcpPort_ = port->i();
  }
  const String* version = downcast<String>(dict->get("v"));
  if(version) {
    msg->clientVersion_ = version->s();
  }
  const Dict* extDict = downcast<Dict>(dict->get("m"));
  if(extDict) {
    for(Dict::ValueType::const_iterator i = extDict->begin(),
          eoi = extDict->end(); i != eoi; ++i) {
      const Integer* extId = downcast<Integer>((*i).second);
      if(extId) {
        int key = keyBtExtension((*i).first.c_str());
        if(key == ExtensionMessageRegistry::MAX_EXTENSION) {
          A2_LOG_DEBUG(fmt("Unsupported BitTorrent extension %s=%" PRId64,
                           (*i).first.c_str(), extId->i()));
        } else {
          msg->setExtension(key, extId->i());
        }
      }
    }
  }
  const Integer* metadataSize = downcast<Integer>(dict->get("metadata_size"));
  // Only accept metadata smaller than 1MiB
  if(metadataSize && metadataSize->i() <= 1024*1024) {
    msg->metadataSize_ = metadataSize->i();
  }
  return msg;
}

} // namespace aria2
