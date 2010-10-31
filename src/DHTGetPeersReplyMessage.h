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
#ifndef D_DHT_GET_PEERS_REPLY_MESSAGE_H
#define D_DHT_GET_PEERS_REPLY_MESSAGE_H

#include "DHTResponseMessage.h"

#include <vector>

#include "DHTConstants.h"

namespace aria2 {

class Peer;

class DHTGetPeersReplyMessage:public DHTResponseMessage {
private:
  int family_;

  std::string token_;

  std::vector<SharedHandle<DHTNode> > closestKNodes_;

  std::vector<SharedHandle<Peer> > values_;
protected:
  virtual std::string toStringOptional() const;
public:
  DHTGetPeersReplyMessage(int family,
                          const SharedHandle<DHTNode>& localNode,
                          const SharedHandle<DHTNode>& remoteNode,
                          const std::string& token,
                          const std::string& transactionID);

  virtual ~DHTGetPeersReplyMessage();

  virtual void doReceivedAction();

  virtual SharedHandle<Dict> getResponse();

  virtual const std::string& getMessageType() const;

  virtual void accept(DHTMessageCallback* callback);

  const std::vector<SharedHandle<DHTNode> >& getClosestKNodes() const
  {
    return closestKNodes_;
  }

  const std::vector<SharedHandle<Peer> >& getValues() const
  {
    return values_;
  }

  void setClosestKNodes
  (const std::vector<SharedHandle<DHTNode> >& closestKNodes)
  {
    closestKNodes_ = closestKNodes;
  }

  void setValues(const std::vector<SharedHandle<Peer> >& peers)
  {
    values_ = peers;
  }
  
  const std::string& getToken() const
  {
    return token_;
  }

  static const std::string GET_PEERS;

  static const std::string TOKEN;

  static const std::string VALUES;

  static const std::string NODES;

  static const std::string NODES6;
};

} // namespace aria2

#endif // D_DHT_GET_PEERS_REPLY_MESSAGE_H
