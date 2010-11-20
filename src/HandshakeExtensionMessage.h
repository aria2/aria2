/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#ifndef D_HANDSHAKE_EXTENSION_MESSAGE_H
#define D_HANDSHAKE_EXTENSION_MESSAGE_H

#include "ExtensionMessage.h"

#include <map>

#include "BtConstants.h"

namespace aria2 {

class Peer;
class DownloadContext;

class HandshakeExtensionMessage:public ExtensionMessage {
private:
  std::string clientVersion_;

  uint16_t tcpPort_;

  size_t metadataSize_;

  std::map<std::string, uint8_t> extensions_;

  SharedHandle<DownloadContext> dctx_;

  SharedHandle<Peer> peer_;
public:
  HandshakeExtensionMessage();

  virtual ~HandshakeExtensionMessage();

  virtual std::string getPayload();

  virtual uint8_t getExtensionMessageID()
  {
    return 0;
  }
  
  virtual const std::string& getExtensionName() const
  {
    return EXTENSION_NAME;
  }

  static const std::string EXTENSION_NAME;

  virtual std::string toString() const;

  virtual void doReceivedAction();

  void setClientVersion(const std::string& version)
  {
    clientVersion_ = version;
  }

  const std::string& getClientVersion() const
  {
    return clientVersion_;
  }

  void setTCPPort(uint16_t port)
  {
    tcpPort_ = port;
  }

  uint16_t getTCPPort() const
  {
    return tcpPort_;
  }

  size_t getMetadataSize()
  {
    return metadataSize_;
  }

  void setMetadataSize(size_t size)
  {
    metadataSize_ = size;
  }

  void setDownloadContext(const SharedHandle<DownloadContext>& dctx)
  {
    dctx_ = dctx;
  }

  void setExtension(const std::string& name, uint8_t id)
  {
    extensions_[name] = id;
  }

  void setExtensions(const Extensions& extensions)
  {
    extensions_ = extensions;
  }

  uint8_t getExtensionMessageID(const std::string& name) const;

  void setPeer(const SharedHandle<Peer>& peer);

  static SharedHandle<HandshakeExtensionMessage>
  create(const unsigned char* data, size_t dataLength);
};

typedef SharedHandle<HandshakeExtensionMessage> HandshakeExtensionMessageHandle;

} // namespace aria2

#endif // D_HANDSHAKE_EXTENSION_MESSAGE_H
