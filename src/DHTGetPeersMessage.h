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
#ifndef D_DHT_GET_PEERS_MESSAGE_H
#define D_DHT_GET_PEERS_MESSAGE_H

#include "DHTQueryMessage.h"
#include "DHTConstants.h"
#include "A2STR.h"

namespace aria2 {

class DHTPeerAnnounceStorage;
class DHTTokenTracker;

class DHTGetPeersMessage:public DHTQueryMessage {
private:
  unsigned char infoHash_[DHT_ID_LENGTH];

  DHTPeerAnnounceStorage* peerAnnounceStorage_;

  DHTTokenTracker* tokenTracker_;
protected:
  virtual std::string toStringOptional() const;
public:
  DHTGetPeersMessage(const SharedHandle<DHTNode>& localNode,
                     const SharedHandle<DHTNode>& remoteNode,
                     const unsigned char* infoHash,
                     const std::string& transactionID = A2STR::NIL);

  virtual ~DHTGetPeersMessage();

  virtual void doReceivedAction();

  virtual SharedHandle<Dict> getArgument();
  
  virtual const std::string& getMessageType() const;

  const unsigned char* getInfoHash() const
  {
    return infoHash_;
  }

  void setPeerAnnounceStorage(DHTPeerAnnounceStorage* storage);

  void setTokenTracker(DHTTokenTracker* tokenTracker);

  static const std::string GET_PEERS;

  static const std::string INFO_HASH;

};

} // namespace aria2

#endif // D_DHT_GET_PEERS_MESSAGE_H
