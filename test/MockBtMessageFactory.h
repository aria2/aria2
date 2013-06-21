#ifndef D_MOCK_BT_MESSAGE_FACTORY_H
#define D_MOCK_BT_MESSAGE_FACTORY_H

#include "BtMessageFactory.h"

namespace aria2 {

class ExtensionMessage;

class MockBtMessageFactory : public BtMessageFactory {
public:
  MockBtMessageFactory() {}

  virtual ~MockBtMessageFactory() {}

  virtual std::shared_ptr<BtMessage>
  createBtMessage(const unsigned char* msg, size_t msgLength) {
    return std::shared_ptr<BtMessage>();
  };

  virtual std::shared_ptr<BtHandshakeMessage>
  createHandshakeMessage(const unsigned char* msg, size_t msgLength) {
    return std::shared_ptr<BtHandshakeMessage>();
  }

  virtual std::shared_ptr<BtHandshakeMessage>
  createHandshakeMessage(const unsigned char* infoHash,
                         const unsigned char* peerId) {
    return std::shared_ptr<BtHandshakeMessage>();
  }

  virtual std::shared_ptr<BtMessage>
  createRequestMessage(const std::shared_ptr<Piece>& piece, size_t blockIndex) {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage>
  createCancelMessage(size_t index, int32_t begin, int32_t length) {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage>
  createPieceMessage(size_t index, int32_t begin, int32_t length) {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createHaveMessage(size_t index) {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createChokeMessage() {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createUnchokeMessage() {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createInterestedMessage() {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createNotInterestedMessage() {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createBitfieldMessage() {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createKeepAliveMessage() {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createHaveAllMessage() {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createHaveNoneMessage() {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage>
  createRejectMessage(size_t index, int32_t begin, int32_t length) {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createAllowedFastMessage(size_t index) {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage> createPortMessage(uint16_t port)
  {
    return std::shared_ptr<BtMessage>();
  }

  virtual std::shared_ptr<BtMessage>
  createBtExtendedMessage(const std::shared_ptr<ExtensionMessage>& extmsg)
  {
    return std::shared_ptr<BtMessage>();
  }
};

} // namespace aria2

#endif // D_MOCK_BT_MESSAGE_FACTORY_H
