#ifndef _D_MOCK_BT_MESSAGE_FACTORY_H_
#define _D_MOCK_BT_MESSAGE_FACTORY_H_

#include "BtMessageFactory.h"

class MockBtMessageFactory : public BtMessageFactory {
public:
  MockBtMessageFactory() {}

  virtual ~MockBtMessageFactory() {}

  virtual BtMessageHandle
  createBtMessage(const unsigned char* msg, uint32_t msgLength) {
    return BtMessageHandle(0);
  };

  virtual BtMessageHandle
  createHandshakeMessage(const unsigned char* msg, uint32_t msgLength) {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle
  createHandshakeMessage(const unsigned char* infoHash,
			 const unsigned char* peerId) {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle
  createRequestMessage(const PieceHandle& piece, int32_t blockIndex) {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle
  createCancelMessage(int32_t index, int32_t begin, uint32_t length) {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle
  createPieceMessage(int32_t index, int32_t begin, uint32_t length) {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle createHaveMessage(int32_t index) {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle createChokeMessage() {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle createUnchokeMessage() {
    return BtMessageHandle(0);
  }
  
  virtual BtMessageHandle createInterestedMessage() {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle createNotInterestedMessage() {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle createBitfieldMessage() {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle createKeepAliveMessage() {
    return BtMessageHandle(0);
  }
  
  virtual BtMessageHandle createHaveAllMessage() {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle createHaveNoneMessage() {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle
  createRejectMessage(int32_t index, int32_t begin, uint32_t length) {
    return BtMessageHandle(0);
  }

  virtual BtMessageHandle createAllowedFastMessage(int32_t index) {
    return BtMessageHandle(0);
  }
};

typedef SharedHandle<MockBtMessageFactory> MockBtMessageFactoryHandle;

#endif // _D_MOCK_BT_MESSAGE_FACTORY_H_
