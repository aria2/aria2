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
#ifndef D_UT_PEX_EXTENSION_MESSAGE_H
#define D_UT_PEX_EXTENSION_MESSAGE_H

#include "ExtensionMessage.h"

#include <utility>
#include <vector>
#include <memory>

#include "a2time.h"
#include "a2functional.h"

namespace aria2 {

class PeerStorage;
class Peer;

class UTPexExtensionMessage : public ExtensionMessage {
private:
  uint8_t extensionMessageID_;

  std::vector<std::shared_ptr<Peer>> freshPeers_;

  std::vector<std::shared_ptr<Peer>> droppedPeers_;

  PeerStorage* peerStorage_;

  std::chrono::seconds interval_;

  size_t maxFreshPeer_;

  size_t maxDroppedPeer_;

  std::pair<std::pair<std::string, std::string>,
            std::pair<std::string, std::string>>
  createCompactPeerListAndFlag(const std::vector<std::shared_ptr<Peer>>& peers);

public:
  UTPexExtensionMessage(uint8_t extensionMessageID);

  virtual std::string getPayload() CXX11_OVERRIDE;

  virtual uint8_t getExtensionMessageID() const CXX11_OVERRIDE
  {
    return extensionMessageID_;
  }

  virtual const char* getExtensionName() const CXX11_OVERRIDE
  {
    return EXTENSION_NAME;
  }

  static const char EXTENSION_NAME[];

  virtual std::string toString() const CXX11_OVERRIDE;

  virtual void doReceivedAction() CXX11_OVERRIDE;

  bool addFreshPeer(const std::shared_ptr<Peer>& peer);

  const std::vector<std::shared_ptr<Peer>>& getFreshPeers() const;

  bool freshPeersAreFull() const;

  bool addDroppedPeer(const std::shared_ptr<Peer>& peer);

  const std::vector<std::shared_ptr<Peer>>& getDroppedPeers() const;

  bool droppedPeersAreFull() const;

  void setPeerStorage(PeerStorage* peerStorage);

  static std::unique_ptr<UTPexExtensionMessage>
  create(const unsigned char* data, size_t len);

  void setMaxFreshPeer(size_t maxFreshPeer);

  size_t getMaxFreshPeer() const { return maxFreshPeer_; }

  void setMaxDroppedPeer(size_t maxDroppedPeer);

  size_t getMaxDroppedPeer() const { return maxDroppedPeer_; }

  constexpr static auto DEFAULT_INTERVAL = 1_min;
};

} // namespace aria2

#endif // D_UT_PEX_EXTENSION_MESSAGE_H
