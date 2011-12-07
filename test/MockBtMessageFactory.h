#ifndef D_MOCK_BT_MESSAGE_FACTORY_H
#define D_MOCK_BT_MESSAGE_FACTORY_H

#include "BtMessageFactory.h"

namespace aria2 {

class ExtensionMessage;

class MockBtMessageFactory : public BtMessageFactory {
public:
  MockBtMessageFactory() {}

  virtual ~MockBtMessageFactory() {}

  virtual SharedHandle<BtMessage>
  createBtMessage(const unsigned char* msg, size_t msgLength) {
    return SharedHandle<BtMessage>();
  };

  virtual SharedHandle<BtHandshakeMessage>
  createHandshakeMessage(const unsigned char* msg, size_t msgLength) {
    return SharedHandle<BtHandshakeMessage>();
  }

  virtual SharedHandle<BtHandshakeMessage>
  createHandshakeMessage(const unsigned char* infoHash,
                         const unsigned char* peerId) {
    return SharedHandle<BtHandshakeMessage>();
  }

  virtual SharedHandle<BtMessage>
  createRequestMessage(const SharedHandle<Piece>& piece, size_t blockIndex) {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage>
  createCancelMessage(size_t index, int32_t begin, int32_t length) {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage>
  createPieceMessage(size_t index, int32_t begin, int32_t length) {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createHaveMessage(size_t index) {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createChokeMessage() {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createUnchokeMessage() {
    return SharedHandle<BtMessage>();
  }
  
  virtual SharedHandle<BtMessage> createInterestedMessage() {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createNotInterestedMessage() {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createBitfieldMessage() {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createKeepAliveMessage() {
    return SharedHandle<BtMessage>();
  }
  
  virtual SharedHandle<BtMessage> createHaveAllMessage() {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createHaveNoneMessage() {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage>
  createRejectMessage(size_t index, int32_t begin, int32_t length) {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createAllowedFastMessage(size_t index) {
    return SharedHandle<BtMessage>();
  }

  virtual SharedHandle<BtMessage> createPortMessage(uint16_t port)
  {
    return SharedHandle<BtMessage>();
  }
  
  virtual SharedHandle<BtMessage>
  createBtExtendedMessage(const SharedHandle<ExtensionMessage>& extmsg)
  {
    return SharedHandle<BtMessage>();
  }
};

} // namespace aria2

#endif // D_MOCK_BT_MESSAGE_FACTORY_H
