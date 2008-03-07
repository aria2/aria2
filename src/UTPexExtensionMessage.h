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
#ifndef _D_UT_PEX_EXTENSION_MESSAGE_H_
#define _D_UT_PEX_EXTENSION_MESSAGE_H_

#include "ExtensionMessage.h"
#include <utility>
#include <deque>

namespace aria2 {

class BtContext;
class Peer;
class UTPexExtensionMessage;
typedef SharedHandle<UTPexExtensionMessage> UTPexExtensionMessageHandle;

class UTPexExtensionMessage:public ExtensionMessage {
private:
  uint8_t _extensionMessageID;

  std::deque<SharedHandle<Peer> > _freshPeers;

  std::deque<SharedHandle<Peer> > _droppedPeers;

  SharedHandle<BtContext> _btContext;

  std::pair<std::string, std::string>
  createCompactPeerListAndFlag(const std::deque<SharedHandle<Peer> >& peers);

public:
  UTPexExtensionMessage(uint8_t extensionMessageID);

  virtual ~UTPexExtensionMessage();

  virtual std::string getBencodedData();

  virtual uint8_t getExtensionMessageID()
  {
    return _extensionMessageID;
  }
  
  virtual const std::string& getExtensionName() const
  {
    return EXTENSION_NAME;
  }

  static const std::string EXTENSION_NAME;

  virtual std::string toString() const;

  virtual void doReceivedAction();

  void addFreshPeer(const SharedHandle<Peer>& peer);

  const std::deque<SharedHandle<Peer> >& getFreshPeers() const;

  void addDroppedPeer(const SharedHandle<Peer>& peer);

  const std::deque<SharedHandle<Peer> >& getDroppedPeers() const;

  void setBtContext(const SharedHandle<BtContext>& btContext);

  static UTPexExtensionMessageHandle create(const SharedHandle<BtContext>& btContext,
					    const unsigned char* data, size_t len);
};

typedef SharedHandle<UTPexExtensionMessage> UTPexExtensionMessageHandle;

} // namespace aria2

#endif // _D_UT_PEX_EXTENSION_MESSAGE_H_
