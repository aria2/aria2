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
#include "DefaultExtensionMessageFactory.h"
#include "BtContext.h"
#include "Peer.h"
#include "DlAbortEx.h"
#include "HandshakeExtensionMessage.h"
#include "UTPexExtensionMessage.h"
#include "LogFactory.h"
#include "BtRegistry.h"

DefaultExtensionMessageFactory::DefaultExtensionMessageFactory():
  _btContext(0),
  _peer(0),
  _logger(LogFactory::getInstance()) {}

DefaultExtensionMessageFactory::DefaultExtensionMessageFactory(const BtContextHandle& btContext,
							       const PeerHandle& peer):
  _btContext(btContext),
  _peer(peer),
  _logger(LogFactory::getInstance()) {}

DefaultExtensionMessageFactory::~DefaultExtensionMessageFactory() {}

ExtensionMessageHandle
DefaultExtensionMessageFactory::createMessage(const char* data, size_t length)
{
  uint8_t extensionMessageID = *data;
  if(extensionMessageID == 0) {
    // handshake
    HandshakeExtensionMessageHandle m = HandshakeExtensionMessage::create(data, length);
    m->setBtContext(_btContext);
    m->setPeer(_peer);
    return m;
  } else {
    string extensionName = getExtensionName(extensionMessageID);
    if(extensionName.empty()) {
      throw new DlAbortEx("No extension registered for extended message ID %u",
			  extensionMessageID);
    }
    if(extensionName == "ut_pex") {
      // uTorrent compatible Peer-Exchange
      UTPexExtensionMessageHandle m =
	UTPexExtensionMessage::create(_btContext, data, length);
      m->setBtContext(_btContext);
      return m;
    } else {
      throw new DlAbortEx("Unsupported extension message received. extensionMessageID=%u, extensionName=%s", extensionMessageID, extensionName.c_str());
    }
  }
}

void DefaultExtensionMessageFactory::setBtContext(const BtContextHandle& btContext)
{
  _btContext = btContext;
}

void DefaultExtensionMessageFactory::setPeer(const PeerHandle& peer)
{
  _peer = peer;
}
