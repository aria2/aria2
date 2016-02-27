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
#ifndef D_DHT_PEER_LOOKUP_TASK_H
#define D_DHT_PEER_LOOKUP_TASK_H

#include "DHTAbstractNodeLookupTask.h"
#include <map>

namespace aria2 {

class DownloadContext;
class Peer;
class PeerStorage;
class DHTGetPeersReplyMessage;

class DHTPeerLookupTask
    : public DHTAbstractNodeLookupTask<DHTGetPeersReplyMessage> {
private:
  std::map<std::string, std::string> tokenStorage_;

  std::shared_ptr<PeerStorage> peerStorage_;
  uint16_t tcpPort_;

public:
  DHTPeerLookupTask(const std::shared_ptr<DownloadContext>& downloadContext,
                    uint16_t tcpPort);

  virtual void
  getNodesFromMessage(std::vector<std::shared_ptr<DHTNode>>& nodes,
                      const DHTGetPeersReplyMessage* message) CXX11_OVERRIDE;

  virtual void
  onReceivedInternal(const DHTGetPeersReplyMessage* message) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTMessage>
  createMessage(const std::shared_ptr<DHTNode>& remoteNode) CXX11_OVERRIDE;

  virtual std::unique_ptr<DHTMessageCallback> createCallback() CXX11_OVERRIDE;

  virtual void onFinish() CXX11_OVERRIDE;

  void setPeerStorage(const std::shared_ptr<PeerStorage>& peerStorage);
};

} // namespace aria2

#endif // D_DHT_PEER_LOOKUP_TASK_H
