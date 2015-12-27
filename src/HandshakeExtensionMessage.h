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

#include <memory>

#include "BtConstants.h"
#include "ExtensionMessageRegistry.h"

namespace aria2 {

class Peer;
class DownloadContext;

class HandshakeExtensionMessage : public ExtensionMessage {
private:
  std::string clientVersion_;

  uint16_t tcpPort_;

  size_t metadataSize_;

  ExtensionMessageRegistry extreg_;

  DownloadContext* dctx_;

  std::shared_ptr<Peer> peer_;

public:
  HandshakeExtensionMessage();

  virtual std::string getPayload() CXX11_OVERRIDE;

  virtual uint8_t getExtensionMessageID() const CXX11_OVERRIDE { return 0; }

  virtual const char* getExtensionName() const CXX11_OVERRIDE
  {
    return EXTENSION_NAME;
  }

  static const char EXTENSION_NAME[];

  virtual std::string toString() const CXX11_OVERRIDE;

  virtual void doReceivedAction() CXX11_OVERRIDE;

  void setClientVersion(const std::string& version)
  {
    clientVersion_ = version;
  }

  const std::string& getClientVersion() const { return clientVersion_; }

  void setTCPPort(uint16_t port) { tcpPort_ = port; }

  uint16_t getTCPPort() const { return tcpPort_; }

  size_t getMetadataSize() { return metadataSize_; }

  void setMetadataSize(size_t size) { metadataSize_ = size; }

  void setDownloadContext(DownloadContext* dctx) { dctx_ = dctx; }

  void setExtension(int key, uint8_t id);

  void setExtensions(const Extensions& extensions);

  uint8_t getExtensionMessageID(int key) const;

  void setPeer(const std::shared_ptr<Peer>& peer);

  static std::unique_ptr<HandshakeExtensionMessage>
  create(const unsigned char* data, size_t dataLength);
};

} // namespace aria2

#endif // D_HANDSHAKE_EXTENSION_MESSAGE_H
