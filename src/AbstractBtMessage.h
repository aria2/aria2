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
#ifndef D_ABSTRACT_BT_MESSAGE_H
#define D_ABSTRACT_BT_MESSAGE_H

#include "BtMessage.h"
#include "Command.h"

namespace aria2 {

class PieceStorage;
class Peer;
class BtMessageDispatcher;
class BtMessageFactory;
class BtRequestFactory;
class PeerConnection;
class BtMessageValidator;

class AbstractBtMessage : public BtMessage {
private:
  bool invalidate_;
  bool uploading_;
  cuid_t cuid_;

  const char* name_;

  PieceStorage* pieceStorage_;

  std::shared_ptr<Peer> peer_;

  BtMessageDispatcher* dispatcher_;

  BtMessageFactory* messageFactory_;

  BtRequestFactory* requestFactory_;

  PeerConnection* peerConnection_;

  std::unique_ptr<BtMessageValidator> validator_;

  bool metadataGetMode_;

protected:
  PieceStorage* getPieceStorage() const { return pieceStorage_; }

  PeerConnection* getPeerConnection() const { return peerConnection_; }

  BtMessageDispatcher* getBtMessageDispatcher() const { return dispatcher_; }

  BtRequestFactory* getBtRequestFactory() const { return requestFactory_; }

  BtMessageFactory* getBtMessageFactory() const { return messageFactory_; }

  bool isMetadataGetMode() const { return metadataGetMode_; }

public:
  AbstractBtMessage(uint8_t id, const char* name);

  virtual ~AbstractBtMessage();

  virtual bool isInvalidate() CXX11_OVERRIDE { return invalidate_; }

  void setInvalidate(bool invalidate) { invalidate_ = invalidate; }

  virtual bool isUploading() CXX11_OVERRIDE { return uploading_; }

  void setUploading(bool uploading) { uploading_ = uploading; }

  cuid_t getCuid() const { return cuid_; }

  void setCuid(cuid_t cuid) { cuid_ = cuid; }

  const std::shared_ptr<Peer>& getPeer() const { return peer_; }

  void setPeer(const std::shared_ptr<Peer>& peer);

  virtual void doReceivedAction() CXX11_OVERRIDE {}

  virtual void validate() CXX11_OVERRIDE;

  virtual void onQueued() CXX11_OVERRIDE {}

  virtual void onAbortOutstandingRequestEvent(
      const BtAbortOutstandingRequestEvent& event) CXX11_OVERRIDE
  {
  }

  virtual void onCancelSendingPieceEvent(const BtCancelSendingPieceEvent& event)
      CXX11_OVERRIDE
  {
  }

  virtual void onChokingEvent(const BtChokingEvent& event) CXX11_OVERRIDE {}

  void setBtMessageValidator(std::unique_ptr<BtMessageValidator> validator);

  void setPieceStorage(PieceStorage* pieceStorage);

  void setBtMessageDispatcher(BtMessageDispatcher* dispatcher);

  void setPeerConnection(PeerConnection* peerConnection);

  void setBtMessageFactory(BtMessageFactory* factory);

  void setBtRequestFactory(BtRequestFactory* factory);

  const char* getName() const { return name_; }

  void enableMetadataGetMode() { metadataGetMode_ = true; }
};

} // namespace aria2

#endif // D_ABSTRACT_BT_MESSAGE_H
