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
#ifndef D_BT_MESSAGE_FACTORY_H
#define D_BT_MESSAGE_FACTORY_H

#include "common.h"
#include "SharedHandle.h"

namespace aria2 {

class BtMessage;
class BtHandshakeMessage;
class Piece;
class ExtensionMessage;

class BtMessageFactory {
public:
  virtual ~BtMessageFactory() {}

  virtual SharedHandle<BtMessage>
  createBtMessage(const unsigned char* msg, size_t msgLength) = 0;

  virtual SharedHandle<BtHandshakeMessage>
  createHandshakeMessage(const unsigned char* msg, size_t msgLength) = 0;

  virtual SharedHandle<BtHandshakeMessage>
  createHandshakeMessage(const unsigned char* infoHash,
                         const unsigned char* peerId) = 0;

  virtual SharedHandle<BtMessage>
  createRequestMessage(const SharedHandle<Piece>& piece, size_t blockIndex) = 0;

  virtual SharedHandle<BtMessage>
  createCancelMessage(size_t index, int32_t begin, int32_t length) = 0;

  virtual SharedHandle<BtMessage>
  createPieceMessage(size_t index, int32_t begin, int32_t length) = 0;

  virtual SharedHandle<BtMessage> createHaveMessage(size_t index) = 0;

  virtual SharedHandle<BtMessage> createChokeMessage() = 0;

  virtual SharedHandle<BtMessage> createUnchokeMessage() = 0;
  
  virtual SharedHandle<BtMessage> createInterestedMessage() = 0;

  virtual SharedHandle<BtMessage> createNotInterestedMessage() = 0;

  virtual SharedHandle<BtMessage> createBitfieldMessage() = 0;

  virtual SharedHandle<BtMessage> createKeepAliveMessage() = 0;
  
  virtual SharedHandle<BtMessage> createHaveAllMessage() = 0;

  virtual SharedHandle<BtMessage> createHaveNoneMessage() = 0;

  virtual SharedHandle<BtMessage>
  createRejectMessage(size_t index, int32_t begin, int32_t length) = 0;

  virtual SharedHandle<BtMessage> createAllowedFastMessage(size_t index) = 0;

  virtual SharedHandle<BtMessage> createPortMessage(uint16_t port) = 0;

  virtual SharedHandle<BtMessage>
  createBtExtendedMessage(const SharedHandle<ExtensionMessage>& msg) = 0;
};

typedef SharedHandle<BtMessageFactory> BtMessageFactoryHandle;

} // namespace aria2

#endif // D_BT_MESSAGE_FACTORY_H
