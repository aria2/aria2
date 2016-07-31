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
#ifndef D_DHT_MESSAGE_FACTORY_H
#define D_DHT_MESSAGE_FACTORY_H

#include "common.h"

#include <string>
#include <vector>
#include <memory>

#include "A2STR.h"
#include "ValueBase.h"

namespace aria2 {

class DHTMessage;
class DHTQueryMessage;
class DHTResponseMessage;
class DHTPingMessage;
class DHTPingReplyMessage;
class DHTFindNodeMessage;
class DHTFindNodeReplyMessage;
class DHTGetPeersMessage;
class DHTGetPeersReplyMessage;
class DHTAnnouncePeerMessage;
class DHTAnnouncePeerReplyMessage;
class DHTUnknownMessage;
class DHTNode;
class Peer;

class DHTMessageFactory {
public:
  virtual ~DHTMessageFactory() = default;

  virtual std::unique_ptr<DHTQueryMessage>
  createQueryMessage(const Dict* dict, const std::string& ipaddr,
                     uint16_t port) = 0;

  virtual std::unique_ptr<DHTResponseMessage>
  createResponseMessage(const std::string& messageType, const Dict* dict,
                        const std::string& ipaddr, uint16_t port) = 0;

  virtual std::unique_ptr<DHTPingMessage>
  createPingMessage(const std::shared_ptr<DHTNode>& remoteNode,
                    const std::string& transactionID = A2STR::NIL) = 0;

  virtual std::unique_ptr<DHTPingReplyMessage>
  createPingReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                         const unsigned char* id,
                         const std::string& transactionID) = 0;

  virtual std::unique_ptr<DHTFindNodeMessage>
  createFindNodeMessage(const std::shared_ptr<DHTNode>& remoteNode,
                        const unsigned char* targetNodeID,
                        const std::string& transactionID = A2STR::NIL) = 0;

  virtual std::unique_ptr<DHTFindNodeReplyMessage> createFindNodeReplyMessage(
      const std::shared_ptr<DHTNode>& remoteNode,
      std::vector<std::shared_ptr<DHTNode>> closestKNodes,
      const std::string& transactionID) = 0;

  virtual std::unique_ptr<DHTGetPeersMessage>
  createGetPeersMessage(const std::shared_ptr<DHTNode>& remoteNode,
                        const unsigned char* infoHash,
                        const std::string& transactionID = A2STR::NIL) = 0;

  virtual std::unique_ptr<DHTGetPeersReplyMessage> createGetPeersReplyMessage(
      const std::shared_ptr<DHTNode>& remoteNode,
      std::vector<std::shared_ptr<DHTNode>> closestKNodes,
      std::vector<std::shared_ptr<Peer>> peers, const std::string& token,
      const std::string& transactionID) = 0;

  virtual std::unique_ptr<DHTAnnouncePeerMessage>
  createAnnouncePeerMessage(const std::shared_ptr<DHTNode>& remoteNode,
                            const unsigned char* infoHash, uint16_t tcpPort,
                            const std::string& token,
                            const std::string& transactionID = A2STR::NIL) = 0;

  virtual std::unique_ptr<DHTAnnouncePeerReplyMessage>
  createAnnouncePeerReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                                 const std::string& transactionID) = 0;

  virtual std::unique_ptr<DHTUnknownMessage>
  createUnknownMessage(const unsigned char* data, size_t length,
                       const std::string& ipaddr, uint16_t port) = 0;
};

} // namespace aria2

#endif // D_DHT_MESSAGE_FACTORY_H
