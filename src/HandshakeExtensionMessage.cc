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
#include "HandshakeExtensionMessage.h"
#include "Peer.h"
#include "util.h"
#include "DlAbortEx.h"
#include "LogFactory.h"
#include "Logger.h"
#include "message.h"
#include "StringFormat.h"
#include "bencode.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "RequestGroup.h"
#include "PieceStorage.h"

namespace aria2 {

const std::string HandshakeExtensionMessage::EXTENSION_NAME = "handshake";

HandshakeExtensionMessage::HandshakeExtensionMessage():
  _tcpPort(0),
  _metadataSize(0),
  _logger(LogFactory::getInstance()) {}

HandshakeExtensionMessage::~HandshakeExtensionMessage() {}

std::string HandshakeExtensionMessage::getBencodedData()
{
  BDE dict = BDE::dict();
  if(!_clientVersion.empty()) {
    dict["v"] = _clientVersion;
  }
  if(_tcpPort > 0) {
    dict["p"] = _tcpPort;
  }
  BDE extDict = BDE::dict();
  for(std::map<std::string, uint8_t>::const_iterator itr = _extensions.begin();
      itr != _extensions.end(); ++itr) {
    const std::map<std::string, uint8_t>::value_type& vt = *itr;
    extDict[vt.first] = vt.second;
  }
  dict["m"] = extDict;
  if(_metadataSize) {
    dict["metadata_size"] = _metadataSize;
  }
  return bencode::encode(dict);
}

std::string HandshakeExtensionMessage::toString() const
{
  std::string s = getExtensionName();
  if(!_clientVersion.empty()) {
    strappend(s, " client=", util::urlencode(_clientVersion));
  }
  if(_tcpPort > 0) {
    strappend(s, ", tcpPort=", util::uitos(_tcpPort));
  }
  if(_metadataSize) {
    strappend(s, ", metadataSize=", util::uitos(_metadataSize));
  }
  for(std::map<std::string, uint8_t>::const_iterator itr = _extensions.begin();
      itr != _extensions.end(); ++itr) {
    const std::map<std::string, uint8_t>::value_type& vt = *itr;
    strappend(s, ", ", vt.first, "=", util::uitos(vt.second));
  }
  return s;
}

void HandshakeExtensionMessage::doReceivedAction()
{
  if(_tcpPort > 0) {
    _peer->port = _tcpPort;
    _peer->setIncomingPeer(false);
  }
  for(std::map<std::string, uint8_t>::const_iterator itr = _extensions.begin();
      itr != _extensions.end(); ++itr) {
    const std::map<std::string, uint8_t>::value_type& vt = *itr;
    _peer->setExtension(vt.first, vt.second);
  }
  if(_metadataSize > 0) {
    BDE& attrs = _dctx->getAttribute(bittorrent::BITTORRENT);
    if(attrs.containsKey(bittorrent::METADATA_SIZE)) {
      if(_metadataSize != (size_t)attrs[bittorrent::METADATA_SIZE].i()) {
	throw DL_ABORT_EX("Wrong metadata_size. Which one is correct!?");
      }
    } else {
      attrs[bittorrent::METADATA_SIZE] = _metadataSize;
      _dctx->getFirstFileEntry()->setLength(_metadataSize);
      _dctx->markTotalLengthIsKnown();
      _dctx->getOwnerRequestGroup()->initPieceStorage();
      
      SharedHandle<PieceStorage> pieceStorage =
	_dctx->getOwnerRequestGroup()->getPieceStorage();
      pieceStorage->setEndGamePieceNum(0);
    }
  }
}

void HandshakeExtensionMessage::setPeer(const PeerHandle& peer)
{
  _peer = peer;
}

uint8_t HandshakeExtensionMessage::getExtensionMessageID(const std::string& name) const
{
  std::map<std::string, uint8_t>::const_iterator i = _extensions.find(name);
  if(i == _extensions.end()) {
    return 0;
  } else {
    return (*i).second;
  }
}

HandshakeExtensionMessageHandle
HandshakeExtensionMessage::create(const unsigned char* data, size_t length)
{
  if(length < 1) {
    throw DL_ABORT_EX
      (StringFormat(MSG_TOO_SMALL_PAYLOAD_SIZE,
		    EXTENSION_NAME.c_str(), length).str());
  }
  HandshakeExtensionMessageHandle msg(new HandshakeExtensionMessage());
  msg->_logger->debug("Creating HandshakeExtensionMessage from %s",
		      util::urlencode(data, length).c_str());
  const BDE dict = bencode::decode(data+1, length-1);
  if(!dict.isDict()) {
    throw DL_ABORT_EX("Unexpected payload format for extended message handshake");
  }
  const BDE& port = dict["p"];
  if(port.isInteger() && 0 < port.i() && port.i() < 65536) {
    msg->_tcpPort = port.i();
  }
  const BDE& version = dict["v"];
  if(version.isString()) {
    msg->_clientVersion = version.s();
  }
  const BDE& extDict = dict["m"];
  if(extDict.isDict()) {
    for(BDE::Dict::const_iterator i = extDict.dictBegin();
	i != extDict.dictEnd(); ++i) {
      if((*i).second.isInteger()) {
	msg->_extensions[(*i).first] = (*i).second.i();
      }
    }
  }
  const BDE& metadataSize = dict["metadata_size"];
  // Only accept metadata smaller than 1MiB
  if(metadataSize.isInteger() && metadataSize.i() <= 1024*1024) {
    msg->_metadataSize = metadataSize.i();
  }
  return msg;
}

} // namespace aria2
