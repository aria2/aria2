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
#ifndef D_BT_HANDSHAKE_MESSAGE_H
#define D_BT_HANDSHAKE_MESSAGE_H

#include "SimpleBtMessage.h"

#include <array>

#include "BtConstants.h"

namespace aria2 {

class BtHandshakeMessage : public SimpleBtMessage {
public:
  constexpr static size_t PSTR_LENGTH = 19;
  constexpr static size_t RESERVED_LENGTH = 8;
  constexpr static size_t MESSAGE_LENGTH = 68;
  const static unsigned char BT_PSTR[];

private:
  uint8_t pstrlen_;
  std::array<unsigned char, PSTR_LENGTH> pstr_;
  std::array<unsigned char, RESERVED_LENGTH> reserved_;
  std::array<unsigned char, INFO_HASH_LENGTH> infoHash_;
  std::array<unsigned char, PEER_ID_LENGTH> peerId_;

  void init();

public:
  BtHandshakeMessage();
  /**
   * infoHash must be 20 byte length.
   * peerId must be 20 byte length.
   */
  BtHandshakeMessage(const unsigned char* infoHash,
                     const unsigned char* peerId);

  static std::unique_ptr<BtHandshakeMessage> create(const unsigned char* data,
                                                    size_t dataLength);

  virtual ~BtHandshakeMessage();

  static const uint8_t ID = INT8_MAX;

  const static char NAME[];

  virtual void doReceivedAction() CXX11_OVERRIDE{};

  virtual std::vector<unsigned char> createMessage() CXX11_OVERRIDE;

  virtual std::string toString() const CXX11_OVERRIDE;

  bool isFastExtensionSupported() const;

  bool isExtendedMessagingEnabled() const;

  bool isDHTEnabled() const;

  void setDHTEnabled(bool enabled)
  {
    if (enabled) {
      reserved_[7] |= 0x01u;
    }
    else {
      reserved_[7] &= ~0x01u;
    }
  }

  uint8_t getPstrlen() const { return pstrlen_; }

  const unsigned char* getPstr() const { return pstr_.data(); }

  const unsigned char* getReserved() const { return reserved_.data(); }

  const unsigned char* getInfoHash() const { return infoHash_.data(); }

  void setInfoHash(const unsigned char* infoHash);

  const unsigned char* getPeerId() const { return peerId_.data(); }

  void setPeerId(const unsigned char* peerId);
};

} // namespace aria2

#endif // D_HANDSHAKE_MESSAGE_H
