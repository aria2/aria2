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
#ifndef _D_DEFAULT_BT_REQUEST_FACTORY_H_
#define _D_DEFAULT_BT_REQUEST_FACTORY_H_

#include "BtRequestFactory.h"
#include "BtContext.h"
#include "PieceStorage.h"
#include "Piece.h"
#include "Peer.h"
#include "BtMessageDispatcher.h"
#include "BtRegistry.h"
#include "LogFactory.h"

class DefaultBtRequestFactory : public BtRequestFactory {
private:
  int cuid;
  BtContextHandle btContext;
  PieceStorageHandle pieceStorage;
  PeerHandle peer;
  BtMessageDispatcherHandle dispatcher;
  Pieces pieces;
public:
  DefaultBtRequestFactory():
    cuid(0),
    btContext(0),
    pieceStorage(0),
    peer(0),
    dispatcher(0)
  {
    LogFactory::getInstance()->debug("DefaultBtRequestFactory::instantiated");
  }

  virtual ~DefaultBtRequestFactory()
  {
    LogFactory::getInstance()->debug("DefaultBtRequestFactory::deleted");
  }

  virtual void addTargetPiece(const PieceHandle& piece) {
    pieces.push_back(piece);
  }

  virtual void removeTargetPiece(const PieceHandle& piece);

  virtual void removeAllTargetPiece();

  virtual int countTargetPiece() {
    return pieces.size();
  }

  virtual void removeCompletedPiece();

  virtual BtMessages createRequestMessages(int max);

  virtual BtMessages createRequestMessagesOnEndGame(int max);

  Pieces& getTargetPieces() {
    return pieces;
  }

  void setCuid(int32_t cuid) {
    this->cuid = cuid;
  }

  int32_t getCuid() const {
    return cuid;
  }

  void setBtContext(const BtContextHandle& btContext) {
    this->btContext = btContext;
    this->pieceStorage = PIECE_STORAGE(btContext);
  }

  BtContextHandle getBtContext() const {
    return btContext;
  }

  PeerHandle getPeer() const {
    return peer;
  }

  void setPeer(const PeerHandle& peer) {
    this->peer = peer;
  }

  void setBtMessageDispatcher(const BtMessageDispatcherHandle& dispatcher) {
    this->dispatcher = dispatcher;
  }
};

typedef SharedHandle<DefaultBtRequestFactory> DefaultBtRequestFactoryHandle;

#endif // _D_DEFAULT_BT_REQUEST_FACTORY_H_
