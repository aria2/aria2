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
#ifndef D_DEFAULT_BT_REQUEST_FACTORY_H
#define D_DEFAULT_BT_REQUEST_FACTORY_H

#include "BtRequestFactory.h"

#include <deque>

#include "Command.h"

namespace aria2 {

class PieceStorage;
class Peer;
class BtMessageDispatcher;
class BtMessageFactory;
class Piece;

class DefaultBtRequestFactory : public BtRequestFactory {
private:
  std::vector<std::unique_ptr<BtRequestMessage>>
  createRequestMessagesOnEndGame(size_t max);

  PieceStorage* pieceStorage_;
  std::shared_ptr<Peer> peer_;
  BtMessageDispatcher* dispatcher_;
  BtMessageFactory* messageFactory_;
  std::deque<std::shared_ptr<Piece>> pieces_;
  cuid_t cuid_;

public:
  DefaultBtRequestFactory();

  virtual ~DefaultBtRequestFactory();

  virtual void
  addTargetPiece(const std::shared_ptr<Piece>& piece) CXX11_OVERRIDE;

  virtual void
  removeTargetPiece(const std::shared_ptr<Piece>& piece) CXX11_OVERRIDE;

  virtual void removeAllTargetPiece() CXX11_OVERRIDE;

  virtual size_t countTargetPiece() CXX11_OVERRIDE { return pieces_.size(); }

  virtual size_t countMissingBlock() CXX11_OVERRIDE;

  virtual void removeCompletedPiece() CXX11_OVERRIDE;

  virtual void doChokedAction() CXX11_OVERRIDE;

  virtual std::vector<std::unique_ptr<BtRequestMessage>>
  createRequestMessages(size_t max, bool endGame) CXX11_OVERRIDE;

  virtual std::vector<size_t> getTargetPieceIndexes() const CXX11_OVERRIDE;

  std::deque<std::shared_ptr<Piece>>& getTargetPieces() { return pieces_; }

  void setPieceStorage(PieceStorage* pieceStorage);

  void setPeer(const std::shared_ptr<Peer>& peer);

  void setBtMessageDispatcher(BtMessageDispatcher* dispatcher);

  void setBtMessageFactory(BtMessageFactory* factory);

  void setCuid(cuid_t cuid) { cuid_ = cuid; }
};

} // namespace aria2

#endif // D_DEFAULT_BT_REQUEST_FACTORY_H
