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
#ifndef D_INITIATOR_MSE_HANDSHAKE_COMMAND_H
#define D_INITIATOR_MSE_HANDSHAKE_COMMAND_H

#include "PeerAbstractCommand.h"

namespace aria2 {

class RequestGroup;
class PeerStorage;
class PieceStorage;
class BtRuntime;
class MSEHandshake;
class Option;

class InitiatorMSEHandshakeCommand : public PeerAbstractCommand {
public:
  enum Seq {
    INITIATOR_SEND_KEY,
    INITIATOR_SEND_KEY_PENDING,
    INITIATOR_WAIT_KEY,
    INITIATOR_SEND_STEP2_PENDING,
    INITIATOR_FIND_VC_MARKER,
    INITIATOR_RECEIVE_PAD_D_LENGTH,
    INITIATOR_RECEIVE_PAD_D,
  };
private:
  RequestGroup* requestGroup_;

  SharedHandle<PeerStorage> peerStorage_;

  SharedHandle<PieceStorage> pieceStorage_;

  SharedHandle<BtRuntime> btRuntime_;

  Seq sequence_;
  MSEHandshake* mseHandshake_;

  const SharedHandle<Option>& getOption() const;

  void tryNewPeer();
protected:
  virtual bool executeInternal();
  virtual bool prepareForNextPeer(time_t wait);
  virtual void onAbort();
  virtual bool exitBeforeExecute();
public:
  InitiatorMSEHandshakeCommand
  (cuid_t cuid,
   RequestGroup* requestGroup,
   const SharedHandle<Peer>& peer,
   DownloadEngine* e,
   const SharedHandle<BtRuntime>& btRuntime,
   const SharedHandle<SocketCore>& s);

  virtual ~InitiatorMSEHandshakeCommand();

  void setPeerStorage(const SharedHandle<PeerStorage>& peerStorage);

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);
};

} // namespace aria2

#endif // D_INITIATOR_MSE_HANDSHAKE_COMMAND_H
