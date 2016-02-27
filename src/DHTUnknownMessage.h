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
#ifndef D_DHT_UNKNOWN_MESSAGE_H
#define D_DHT_UNKNOWN_MESSAGE_H

#include "DHTMessage.h"

namespace aria2 {

class DHTUnknownMessage : public DHTMessage {
private:
  unsigned char* data_;
  size_t length_;
  std::string ipaddr_;
  uint16_t port_;

public:
  // _remoteNode is always null
  DHTUnknownMessage(const std::shared_ptr<DHTNode>& localNode,
                    const unsigned char* data, size_t length,
                    const std::string& ipaddr, uint16_t port);

  virtual ~DHTUnknownMessage();

  // do nothing
  virtual void doReceivedAction() CXX11_OVERRIDE;

  // do nothing; we don't use this message as outgoing message.
  virtual bool send() CXX11_OVERRIDE;

  // always return false
  virtual bool isReply() const CXX11_OVERRIDE;

  // returns "unknown"
  virtual const std::string& getMessageType() const CXX11_OVERRIDE;

  // show some sample bytes
  virtual std::string toString() const CXX11_OVERRIDE;

  static const std::string E;

  static const std::string UNKNOWN;
};

} // namespace aria2

#endif // D_DHT_UNKNOWN_MESSAGE_H
